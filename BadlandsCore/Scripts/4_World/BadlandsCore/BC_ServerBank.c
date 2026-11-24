// Bankomaty: vklad hotovosti na účet a výběr z účtu.

class BC_ServerBank
{
	ref BC_BankConfig m_Cfg;
	protected BC_Wallets m_Wallets;
	protected string m_Currency;
	protected ref array<string> m_AtmClassesLower = new array<string>;

	void BC_ServerBank(BC_Wallets wallets, BC_BankConfig cfg, string currency)
	{
		m_Wallets = wallets;
		m_Cfg = cfg;
		m_Currency = currency;

		if (m_Cfg.AtmObjectClasses)
		{
			foreach (string cls : m_Cfg.AtmObjectClasses)
			{
				m_AtmClassesLower.Insert(BC_TraderScanner.Lower(cls));
			}
		}
	}

	static BC_BankConfig LoadConfig()
	{
		BC_BankConfig defaults = new BC_BankConfig();
		defaults.FillDefaults();
		return BC_ConfigFile<BC_BankConfig>.Load("Bank.json", defaults, new BC_BankConfig());
	}

	bool IsNearAtm(PlayerBase player)
	{
		if (!player)
			return false;

		if (m_Cfg.Atms)
		{
			foreach (BC_AtmLocation atm : m_Cfg.Atms)
			{
				if (atm && BC_Util.Distance2D(player.GetPosition(), atm.X, atm.Z) <= atm.Radius)
					return true;
			}
		}

		if (m_AtmClassesLower.Count() > 0)
		{
			array<Object> objects = new array<Object>;
			g_Game.GetObjectsAtPosition(player.GetPosition(), m_Cfg.AtmObjectRadius, objects, null);
			foreach (Object obj : objects)
			{
				if (obj && m_AtmClassesLower.Find(BC_TraderScanner.Lower(obj.GetType())) > -1)
					return true;
			}
		}
		return false;
	}

	bool TryOpen(PlayerBase player)
	{
		if (!IsNearAtm(player))
			return false;
		SendData(player);
		return true;
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != BC_RPC_BANK_ACTION && rpcType != BC_RPC_BANK_TRANSFER)
			return false;

		PlayerBase player = BC_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		if (!IsNearAtm(player))
		{
			BC_Util.Notify(player, "Bankomat", "Musis stat u bankomatu.");
			return true;
		}

		if (rpcType == BC_RPC_BANK_TRANSFER)
		{
			string targetId;
			int transferAmount;
			if (ctx.Read(targetId) && ctx.Read(transferAmount))
				Transfer(player, targetId, transferAmount);
			SendData(player);
			return true;
		}

		int action;
		int amount;
		if (!ctx.Read(action) || !ctx.Read(amount))
			return true;

		if (action == BC_BankAction.DEPOSIT)
			Deposit(player, amount);
		else if (action == BC_BankAction.WITHDRAW)
			Withdraw(player, amount);

		SendData(player);
		return true;
	}

	//! amount -1 = vše
	protected void Deposit(PlayerBase player, int amount)
	{
		int cash = BC_Money.CountCash(player);
		if (amount < 0)
			amount = cash;
		if (amount <= 0)
		{
			BC_Util.Notify(player, "Bankomat", "Nemas u sebe zadnou hotovost.");
			return;
		}
		if (amount > cash)
		{
			BC_Util.Notify(player, "Bankomat", "Tolik hotovosti u sebe nemas.");
			return;
		}

		int changeLeftover;
		if (!BC_Money.TakeCash(player, amount, changeLeftover))
			return;
		// drobné pod 10 Kr, které nešly vrátit v bankovkách, jdou taky na účet
		if (changeLeftover > 0)
			m_Wallets.Add(BC_Util.SteamId(player), changeLeftover);

		int fee = Math.Floor(amount * m_Cfg.DepositFeePercent / 100);
		int deposited = amount - fee;
		m_Wallets.Add(BC_Util.SteamId(player), deposited);

		string msg = "Vlozeno " + deposited.ToString() + " " + m_Currency;
		if (fee > 0)
			msg = msg + " (poplatek " + fee.ToString() + ")";
		BC_Util.Notify(player, "Bankomat", msg);
		Print(BC_TAG + "BANKA " + BC_Util.PlayerName(player) + " vklad " + amount.ToString());
	}

	protected void Withdraw(PlayerBase player, int amount)
	{
		string steamId = BC_Util.SteamId(player);
		int bank = m_Wallets.GetBalance(steamId);
		if (amount < 0)
			amount = bank;
		// bankomat vydává jen bankovky (násobky 10)
		amount = amount - amount % 10;
		if (amount <= 0)
		{
			BC_Util.Notify(player, "Bankomat", "Vybrat jde nejmene 10 Kr (bankomat vydava jen bankovky).");
			return;
		}

		if (!m_Wallets.TrySpend(steamId, amount))
		{
			BC_Util.Notify(player, "Bankomat", "Na uctu nemas dost penez.");
			return;
		}

		int undelivered = BC_Money.GiveCash(player, amount);
		if (undelivered > 0)
			m_Wallets.Add(steamId, undelivered);
		BC_Util.Notify(player, "Bankomat", "Vybrano " + amount.ToString() + " " + m_Currency);
		Print(BC_TAG + "BANKA " + BC_Util.PlayerName(player) + " vyber " + amount.ToString());
	}

	protected void Transfer(PlayerBase player, string targetId, int amount)
	{
		if (!m_Cfg.AllowTransfers)
		{
			BC_Util.Notify(player, "Bankomat", "Prevody jsou vypnute.");
			return;
		}

		targetId.TrimInPlace();
		string senderId = BC_Util.SteamId(player);

		if (!BC_Wallets.IsValidSteamId(targetId))
		{
			BC_Util.Notify(player, "Bankomat", "Neplatne Steam ID (17 cislic).");
			return;
		}
		if (targetId == senderId)
		{
			BC_Util.Notify(player, "Bankomat", "Nemuzes posilat penize sam sobe.");
			return;
		}
		if (amount < m_Cfg.MinTransfer)
		{
			BC_Util.Notify(player, "Bankomat", "Minimalni castka je " + m_Cfg.MinTransfer.ToString() + " " + m_Currency + ".");
			return;
		}

		PlayerBase target = BC_Util.FindPlayerBySteamId(targetId);
		if (!target && !m_Wallets.HasAccount(targetId))
		{
			BC_Util.Notify(player, "Bankomat", "Hrac s timto Steam ID na serveru nikdy nehral.");
			return;
		}

		int fee = Math.Floor(amount * m_Cfg.TransferFeePercent / 100);
		int total = amount + fee;
		if (!m_Wallets.TrySpend(senderId, total))
		{
			BC_Util.Notify(player, "Bankomat", "Na uctu nemas dost penez (" + total.ToString() + " " + m_Currency + ").");
			return;
		}

		m_Wallets.Add(targetId, amount);

		string targetName = targetId;
		if (target)
			targetName = BC_Util.PlayerName(target);

		string msg = "Odeslano " + amount.ToString() + " " + m_Currency + " hraci " + targetName;
		if (fee > 0)
			msg = msg + " (poplatek " + fee.ToString() + ")";
		BC_Util.Notify(player, "Bankomat", msg);

		if (target)
			BC_Util.Notify(target, "Banka", BC_Util.PlayerName(player) + " ti poslal " + amount.ToString() + " " + m_Currency + " na ucet.");

		Print(BC_TAG + "BANKA prevod " + amount.ToString() + " od " + BC_Util.PlayerName(player) + " (" + senderId + ") pro " + targetName + " (" + targetId + ")");
	}

	protected void SendData(PlayerBase player)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Currency);
		rpc.Write(BC_Money.CountCash(player));
		rpc.Write(m_Wallets.GetBalance(BC_Util.SteamId(player)));
		rpc.Write(m_Cfg.DepositFeePercent);

		// hráči online (pro převod), bez sebe sama
		array<string> names = new array<string>;
		array<string> ids = new array<string>;
		if (m_Cfg.AllowTransfers)
		{
			array<Man> players = new array<Man>;
			g_Game.GetPlayers(players);
			foreach (Man man : players)
			{
				PlayerBase other = PlayerBase.Cast(man);
				if (!other || other == player || !other.GetIdentity())
					continue;
				names.Insert(other.GetIdentity().GetName());
				ids.Insert(other.GetIdentity().GetPlainId());
			}
		}
		rpc.Write(m_Cfg.AllowTransfers);
		rpc.Write(m_Cfg.TransferFeePercent);
		rpc.Write(names);
		rpc.Write(ids);
		rpc.Send(null, BC_RPC_BANK_DATA, true, player.GetIdentity());
	}
}
