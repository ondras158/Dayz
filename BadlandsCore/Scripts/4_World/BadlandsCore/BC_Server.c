// Hlavní serverová třída - drží všechny moduly a přeposílá jim události.

class BC_Server
{
	protected static ref BC_Server s_Instance;

	ref BC_Wallets m_Wallets;
	ref BC_ServerLoadouts m_Loadouts;
	ref BC_ServerSpawns m_Spawns;
	ref BC_ServerSafeZones m_SafeZones;
	ref BC_ServerMessages m_Messages;
	ref BC_ServerAdmin m_Admin;
	ref BC_ServerTrader m_Trader;

	protected float m_TickTimer;
	protected static const float TICK_SECONDS = 1.0;

	static BC_Server Get()
	{
		return s_Instance;
	}

	static void Start()
	{
		if (!s_Instance && g_Game.IsServer())
			s_Instance = new BC_Server();
	}

	void BC_Server()
	{
		Print(BC_TAG + "Spoustim server modul...");

		BC_TraderConfig traderCfg = BC_ServerTrader.LoadConfig();
		m_Wallets = new BC_Wallets(traderCfg.StartingBalance);

		m_Loadouts = new BC_ServerLoadouts();
		m_Spawns = new BC_ServerSpawns();
		m_SafeZones = new BC_ServerSafeZones();
		m_Messages = new BC_ServerMessages();
		m_Admin = new BC_ServerAdmin(m_Wallets, traderCfg.CurrencyName);
		m_Trader = new BC_ServerTrader(m_Wallets, traderCfg);

		DayZGame.Event_OnRPC.Insert(OnRPC);
		Print(BC_TAG + "Server modul spusten.");
	}

	void ~BC_Server()
	{
		DayZGame.Event_OnRPC.Remove(OnRPC);
	}

	// ------------------------------------------------------------ události z MissionServer

	void OnNewCharacter(PlayerBase player)
	{
		m_Loadouts.Equip(player);
		m_Spawns.OnNewCharacter(player);
	}

	void OnConnect(PlayerBase player)
	{
		// chvíli počkat, než klient dokončí načítání, jinak se zpráva ztratí
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(m_Messages.OnConnect, 8000, false, player);
	}

	void OnDisconnect(string steamId)
	{
		if (steamId != "")
			m_Wallets.Forget(steamId);
	}

	void OnUpdate(float timeslice)
	{
		m_TickTimer += timeslice;
		if (m_TickTimer < TICK_SECONDS)
			return;

		float delta = m_TickTimer;
		m_TickTimer = 0;

		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);

		m_Spawns.OnTick();
		m_SafeZones.OnTick(players, delta);
		m_Messages.OnTick(delta);
	}

	// ------------------------------------------------------------ události z entit

	void OnPlayerKilled(PlayerBase victim, Object killer)
	{
		PlayerBase killerPlayer = BC_Util.ResolveKillerPlayer(killer);
		m_Messages.OnPlayerKilled(victim, killerPlayer, killer);

		if (killerPlayer && killerPlayer != victim)
			m_Trader.RewardPlayerKill(killerPlayer);
	}

	void OnZombieKilled(ZombieBase zombie, Object killer)
	{
		PlayerBase killerPlayer = BC_Util.ResolveKillerPlayer(killer);
		if (killerPlayer)
			m_Trader.RewardZombieKill(killerPlayer);
	}

	// ------------------------------------------------------------ RPC od klientů

	void OnRPC(PlayerIdentity sender, Object target, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType < BC_RPC_SPAWN_MENU || rpcType > BC_RPC_TRADER_UPDATE)
			return;
		if (!sender)
			return;

		if (m_Spawns.OnRPC(sender, rpcType, ctx))
			return;
		if (m_Admin.OnRPC(sender, rpcType, ctx))
			return;
		m_Trader.OnRPC(sender, rpcType, ctx);
	}
}
