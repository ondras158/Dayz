// Hlavní serverová třída - drží všechny moduly a přeposílá jim události.

class OS_Server
{
	protected static ref OS_Server s_Instance;

	ref OS_Wallets m_Wallets;
	ref OS_ServerLoadouts m_Loadouts;
	ref OS_ServerSpawns m_Spawns;
	ref OS_ServerSafeZones m_SafeZones;
	ref OS_ServerPvP m_PvP;
	ref OS_ServerBases m_Bases;
	ref OS_ServerCooking m_Cooking;
	ref OS_ServerSkills m_Skills;
	ref OS_ServerHud m_Hud;
	ref OS_ServerSleep m_Sleep;
	ref OS_ServerAirdrops m_Airdrops;
	protected int m_PlayerCount;
	ref OS_ServerMessages m_Messages;
	ref OS_ServerAdmin m_Admin;
	ref OS_ServerTrader m_Trader;
	ref OS_ServerVehicles m_Vehicles;
	ref OS_ServerBank m_Bank;
	ref OS_ServerGroups m_Groups;
	ref OS_ServerFarms m_Farms;
	ref OS_Payments m_Payments;

	protected string m_Currency;
	protected float m_TickTimer;
	protected static const float TICK_SECONDS = 1.0;

	static OS_Server Get()
	{
		return s_Instance;
	}

	static void Start()
	{
		if (!s_Instance && g_Game.IsServer())
			s_Instance = new OS_Server();
	}

	void OS_Server()
	{
		Print(OS_TAG + "Spoustim server modul...");

		// jazyk jako první - výchozí konfigy i zprávy už v něm
		OS_Lang.LoadServer();

		OS_TraderConfig traderCfg = OS_ServerTrader.LoadConfig();
		m_Wallets = new OS_Wallets(traderCfg.StartingBalance);

		OS_BankConfig bankCfg = OS_ServerBank.LoadConfig();
		string currency = bankCfg.CurrencyName;
		OS_Money.SetDenominations(bankCfg.Denominations);

		m_Loadouts = new OS_ServerLoadouts();
		m_Spawns = new OS_ServerSpawns();
		m_SafeZones = new OS_ServerSafeZones();
		m_PvP = new OS_ServerPvP(m_SafeZones);
		m_Messages = new OS_ServerMessages();
		m_Currency = currency;
		m_Payments = new OS_Payments(m_Wallets, bankCfg, currency);
		m_Groups = new OS_ServerGroups(m_Payments, m_Wallets, currency);
		m_Bases = new OS_ServerBases(m_Payments, m_Groups, currency);
		m_Cooking = new OS_ServerCooking();
		m_Skills = new OS_ServerSkills(m_Payments, currency);
		m_Hud = new OS_ServerHud();
		m_Sleep = new OS_ServerSleep();
		m_Airdrops = new OS_ServerAirdrops();
		m_Farms = new OS_ServerFarms(m_Payments, m_Groups, currency);
		m_Bank = new OS_ServerBank(m_Wallets, bankCfg, currency, m_Groups);

		m_Admin = new OS_ServerAdmin(m_Wallets, currency);
		m_Trader = new OS_ServerTrader(m_Payments, traderCfg, m_Admin, currency);
		m_Vehicles = new OS_ServerVehicles(m_Payments, currency, m_Groups, m_Admin);

		// výkupna úrody: výrobky farem z Farms.json
		array<string> farmProduce = new array<string>;
		foreach (OS_FarmAnimalType farmType : m_Farms.m_Cfg.Animals)
		{
			if (!farmType || !farmType.Products)
				continue;
			foreach (OS_FarmProduct product : farmType.Products)
			{
				if (product)
					farmProduce.Insert(product.ClassName);
			}
		}
		m_Trader.SetFarmProduce(farmProduce);

		DayZGame.Event_OnRPC.Insert(OnRPC);
		Print(OS_TAG + "Server modul spusten.");
	}

	void ~OS_Server()
	{
		DayZGame.Event_OnRPC.Remove(OnRPC);
	}

	// ------------------------------------------------------------ události z MissionServer

	void OnNewCharacter(PlayerBase player)
	{
		// jazyk dřív, než klient dostane menu spawnu
		if (player && player.GetIdentity())
			OS_Lang.SendToClient(player.GetIdentity());
		m_Loadouts.Equip(player);
		m_Spawns.OnNewCharacter(player);
		m_Sleep.OnNewCharacter(player);
	}

	void OnConnect(PlayerBase player)
	{
		// nastavení pro klienta (měna pro názvy bankovek)
		if (player && player.GetIdentity())
		{
			OS_Lang.SendToClient(player.GetIdentity());

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(m_Currency);
			rpc.Send(null, OS_RPC_SETTINGS, true, player.GetIdentity());
			m_Groups.SendGroupId(player);
			if (m_Groups.m_Cfg.AutoTuneOnJoin)
				m_Groups.TuneRadios(player, false);
			m_Vehicles.SendPersonalGarage(player);
			m_Farms.SendData(player);   // poloha farmy pro mapu
			m_PvP.SendZones(player.GetIdentity());   // PvP a bezpečné zóny na mapě
			m_Admin.SendAdminFlag(player);           // admin: teleport z mapy, ESP, freecam
			m_Admin.CheckBan(player);
		}

		// chvíli počkat, než klient dokončí načítání, jinak se zpráva ztratí
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(m_Messages.OnConnect, 8000, false, player);
	}

	void OnDisconnect(string steamId)
	{
		if (steamId != "")
		{
			m_Wallets.Forget(steamId);
			m_Skills.Forget(steamId);
			m_Sleep.OnDisconnect(steamId);
		}
	}

	void OnUpdate(float timeslice)
	{
		// airdropy padají plynule - každý snímek
		m_Airdrops.OnUpdate(timeslice, m_PlayerCount);

		m_TickTimer += timeslice;
		if (m_TickTimer < TICK_SECONDS)
			return;

		float delta = m_TickTimer;
		m_TickTimer = 0;

		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		m_PlayerCount = players.Count();

		m_Spawns.OnTick();
		m_SafeZones.OnTick(players, delta);
		m_PvP.OnTick(players);
		m_Bases.OnTick(delta);
		m_Cooking.OnTick(players, delta);
		m_Skills.OnTick(players, delta);
		m_Sleep.OnTick(players, delta);
		m_Hud.OnTick(players, delta);
		m_Messages.OnTick(delta);
		m_Groups.OnTick(delta);
		m_Farms.OnTick(delta);
	}

	// ------------------------------------------------------------ události z entit

	void OnPlayerKilled(PlayerBase victim, Object killer)
	{
		PlayerBase killerPlayer = OS_Util.ResolveKillerPlayer(killer);
		m_Messages.OnPlayerKilled(victim, killerPlayer, killer);

		if (killerPlayer && killerPlayer != victim)
		{
			m_Trader.RewardPlayerKill(killerPlayer);
			m_Skills.OnPlayerKilled(killerPlayer);
		}
	}

	void OnZombieKilled(ZombieBase zombie, Object killer)
	{
		PlayerBase killerPlayer = OS_Util.ResolveKillerPlayer(killer);
		if (killerPlayer)
		{
			m_Trader.RewardZombieKill(killerPlayer);
			m_Skills.OnZombieKilled(killerPlayer);
		}
	}

	// ------------------------------------------------------------ RPC od klientů

	void OnRPC(PlayerIdentity sender, Object target, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType < OS_RPC_FIRST || rpcType > OS_RPC_LAST)
			return;
		if (!sender)
			return;

		// klávesa obchodu: u obchodníka s věcmi -> obchod, u autocentra -> menu aut
		if (rpcType == OS_RPC_TRADER_OPEN)
		{
			// pořadí: bankomat (malý okruh), obchodník, autocentrum
			PlayerBase player = OS_Util.FindPlayer(sender);
			if (player && player.IsAlive())
			{
				if (m_Bank.TryOpen(player))
					return;
				if (!m_Trader.NearTrader(player) && m_Vehicles.TryOpen(player))
					return;
				if (!m_Trader.NearTrader(player) && m_Farms.TryOpenTrader(player))
					return;
			}
		}

		if (m_Spawns.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Admin.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Vehicles.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Bank.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Groups.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Farms.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Bases.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Cooking.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Skills.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Sleep.OnRPC(sender, rpcType, ctx))
			return;
		m_Trader.OnRPC(sender, rpcType, ctx);
	}
}
