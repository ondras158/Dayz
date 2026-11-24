// Bankomaty: vklad hotovosti na účet a výběr z účtu.

class OS_ServerBank
{
	ref OS_BankConfig m_Cfg;
	protected OS_Wallets m_Wallets;
	protected OS_ServerGroups m_Groups;
	protected string m_Currency;
	protected ref array<string> m_AtmClassesLower = new array<string>;

	void OS_ServerBank(OS_Wallets wallets, OS_BankConfig cfg, string currency, OS_ServerGroups groups)
	{
		m_Groups = groups;
		m_Wallets = wallets;
		m_Cfg = cfg;
		m_Currency = currency;

		if (m_Cfg.AtmObjectClasses)
		{
			foreach (string cls : m_Cfg.AtmObjectClasses)
			{
				m_AtmClassesLower.Insert(OS_TraderScanner.Lower(cls));
			}
		}
	}

	static OS_BankConfig LoadConfig()
	{
		OS_BankConfig defaults = new OS_BankConfig();
		defaults.FillDefaults();
		return OS_ConfigFile<OS_BankConfig>.Load("Bank.json", defaults, new OS_BankConfig());
	}

	bool IsNearAtm(PlayerBase player)
	{
		if (!player)
			return false;

		if (m_Cfg.Atms)
		{
			foreach (OS_AtmLocation atm : m_Cfg.Atms)
			{
				if (atm && OS_Util.Distance2D(player.GetPosition(), atm.X, atm.Z) <= atm.Radius)
					return true;
			}
		}

		if (m_AtmClassesLower.Count() > 0)
		{
			array<Object> objects = new array<Object>;
			g_Game.GetObjectsAtPosition(player.GetPosition(), m_Cfg.AtmObjectRadius, objects, null);
			foreach (Object obj : objects)
			{
				if (obj && m_AtmClassesLower.Find(OS_TraderScanner.Lower(obj.GetType())) > -1)
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
		if (rpcType != OS_RPC_BANK_ACTION && rpcType != OS_RPC_BANK_TRANSFER)
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		if (!IsNearAtm(player))
		{
			OS_Util.Notify(player, "Bankomat", "Musis stat u bankomatu.");
			return true;
		}

		if (rpcType == OS_RPC_BANK_TRANSFER)
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

		if (action == OS_BankAction.DEPOSIT)
			Deposit(player, amount);
		else if (action == OS_BankAction.WITHDRAW)
			Withdraw(player, amount);
		else if (action == OS_BankAction.GROUP_DEPOSIT)
			GroupDeposit(player, amount);
		else if (action == OS_BankAction.GROUP_WITHDRAW)
			GroupWithdraw(player, amount);

		SendData(player);
		return true;
	}

	//! amount -1 = vše
	protected void Deposit(PlayerBase player, int amount)
	{
		int cash = OS_Money.CountCash(player);
		if (amount < 0)
			amount = cash;
		if (amount <= 0)
		{
			OS_Util.Notify(player, "Bankomat", "Nemas u sebe zadnou hotovost.");
			return;
		}
		if (amount > cash)
		{
			OS_Util.Notify(player, "Bankomat", "Tolik hotovosti u sebe nemas.");
			return;
		}

		int changeLeftover;
		if (!OS_Money.TakeCash(player, amount, changeLeftover))
			return;
		// drobné menší než nejmenší bankovka jdou taky na účet
		if (changeLeftover > 0)
			m_Wallets.Add(OS_Util.SteamId(player), changeLeftover);

		int fee = Math.Floor(amount * m_Cfg.DepositFeePercent / 100);
		int deposited = amount - fee;
		m_Wallets.Add(OS_Util.SteamId(player), deposited);

		string msg = "Vlozeno " + deposited.ToString() + " " + m_Currency;
		if (fee > 0)
			msg = msg + " (poplatek " + fee.ToString() + ")";
		OS_Util.Notify(player, "Bankomat", msg);
		Print(OS_TAG + "BANKA " + OS_Util.PlayerName(player) + " vklad " + amount.ToString());
	}

	protected void Withdraw(PlayerBase player, int amount)
	{
		string steamId = OS_Util.SteamId(player);
		int bank = m_Wallets.GetBalance(steamId);
		if (amount < 0)
			amount = bank;
		// bankomat vydává jen bankovky (násobky nejmenší bankovky)
		int smallest = OS_Money.Smallest();
		amount = amount - amount % smallest;
		if (amount <= 0)
		{
			OS_Util.Notify(player, "Bankomat", "Vybrat jde nejmene " + smallest.ToString() + " " + m_Currency + " (bankomat vydava jen bankovky).");
			return;
		}

		if (!m_Wallets.TrySpend(steamId, amount))
		{
			OS_Util.Notify(player, "Bankomat", "Na uctu nemas dost penez.");
			return;
		}

		int undelivered = OS_Money.GiveCash(player, amount);
		if (undelivered > 0)
			m_Wallets.Add(steamId, undelivered);
		OS_Util.Notify(player, "Bankomat", "Vybrano " + amount.ToString() + " " + m_Currency);
		Print(OS_TAG + "BANKA " + OS_Util.PlayerName(player) + " vyber " + amount.ToString());
	}

	// ------------------------------------------------------------ skupinový účet

	protected OS_Group PlayerGroup(PlayerBase player)
	{
		if (!m_Groups)
			return null;
		OS_Group group = m_Groups.GetGroupOf(OS_Util.SteamId(player));
		if (!group)
			OS_Util.Notify(player, "Bankomat", "Nejsi v zadne skupine.");
		return group;
	}

	//! hotovost -> skupinový účet (amount -1 = vše)
	protected void GroupDeposit(PlayerBase player, int amount)
	{
		OS_Group group = PlayerGroup(player);
		if (!group)
			return;

		int cash = OS_Money.CountCash(player);
		if (amount < 0)
			amount = cash;
		if (amount <= 0 || amount > cash)
		{
			OS_Util.Notify(player, "Bankomat", "Tolik hotovosti u sebe nemas.");
			return;
		}

		int changeLeftover;
		if (!OS_Money.TakeCash(player, amount, changeLeftover))
			return;
		if (changeLeftover > 0)
			m_Wallets.Add(OS_Util.SteamId(player), changeLeftover);

		m_Groups.AddBalance(group, amount);
		OS_Util.Notify(player, "Bankomat", "Vlozeno " + amount.ToString() + " " + m_Currency + " na ucet skupiny " + group.Name);
		Print(OS_TAG + "BANKA " + OS_Util.PlayerName(player) + " vklad na skupinu " + group.Name + ": " + amount.ToString());
	}

	//! skupinový účet -> hotovost (amount -1 = vše)
	protected void GroupWithdraw(PlayerBase player, int amount)
	{
		OS_Group group = PlayerGroup(player);
		if (!group)
			return;

		string steamId = OS_Util.SteamId(player);
		if (!m_Groups.CanWithdraw(group, steamId))
		{
			OS_Util.Notify(player, "Bankomat", "Ze skupinoveho uctu vybira jen vlastnik a admini.");
			return;
		}

		if (amount < 0)
			amount = group.Balance;
		int smallest = OS_Money.Smallest();
		amount = amount - amount % smallest;
		if (amount <= 0 || amount > group.Balance)
		{
			OS_Util.Notify(player, "Bankomat", "Na uctu skupiny neni dost penez.");
			return;
		}

		if (!m_Groups.AddBalance(group, -amount))
			return;

		int undelivered = OS_Money.GiveCash(player, amount);
		if (undelivered > 0)
			m_Wallets.Add(steamId, undelivered);
		OS_Util.Notify(player, "Bankomat", "Vybrano " + amount.ToString() + " " + m_Currency + " z uctu skupiny " + group.Name);
		Print(OS_TAG + "BANKA " + OS_Util.PlayerName(player) + " vyber ze skupiny " + group.Name + ": " + amount.ToString());
	}

	protected void Transfer(PlayerBase player, string targetId, int amount)
	{
		if (!m_Cfg.AllowTransfers)
		{
			OS_Util.Notify(player, "Bankomat", "Prevody jsou vypnute.");
			return;
		}

		targetId.TrimInPlace();
		string senderId = OS_Util.SteamId(player);

		if (!OS_Wallets.IsValidSteamId(targetId))
		{
			OS_Util.Notify(player, "Bankomat", "Neplatne Steam ID (17 cislic).");
			return;
		}
		if (targetId == senderId)
		{
			OS_Util.Notify(player, "Bankomat", "Nemuzes posilat penize sam sobe.");
			return;
		}
		if (amount < m_Cfg.MinTransfer)
		{
			OS_Util.Notify(player, "Bankomat", "Minimalni castka je " + m_Cfg.MinTransfer.ToString() + " " + m_Currency + ".");
			return;
		}

		PlayerBase target = OS_Util.FindPlayerBySteamId(targetId);
		if (!target && !m_Wallets.HasAccount(targetId))
		{
			OS_Util.Notify(player, "Bankomat", "Hrac s timto Steam ID na serveru nikdy nehral.");
			return;
		}

		int fee = Math.Floor(amount * m_Cfg.TransferFeePercent / 100);
		int total = amount + fee;
		if (!m_Wallets.TrySpend(senderId, total))
		{
			OS_Util.Notify(player, "Bankomat", "Na uctu nemas dost penez (" + total.ToString() + " " + m_Currency + ").");
			return;
		}

		m_Wallets.Add(targetId, amount);

		string targetName = targetId;
		if (target)
			targetName = OS_Util.PlayerName(target);

		string msg = "Odeslano " + amount.ToString() + " " + m_Currency + " hraci " + targetName;
		if (fee > 0)
			msg = msg + " (poplatek " + fee.ToString() + ")";
		OS_Util.Notify(player, "Bankomat", msg);

		if (target)
			OS_Util.Notify(target, "Banka", OS_Util.PlayerName(player) + " ti poslal " + amount.ToString() + " " + m_Currency + " na ucet.");

		Print(OS_TAG + "BANKA prevod " + amount.ToString() + " od " + OS_Util.PlayerName(player) + " (" + senderId + ") pro " + targetName + " (" + targetId + ")");
	}

	protected void SendData(PlayerBase player)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Currency);
		rpc.Write(OS_Money.CountCash(player));
		rpc.Write(m_Wallets.GetBalance(OS_Util.SteamId(player)));
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

		// skupinový účet
		string groupName = "";
		int groupBalance = 0;
		bool groupCanWithdraw = false;
		if (m_Groups)
		{
			OS_Group group = m_Groups.GetGroupOf(OS_Util.SteamId(player));
			if (group)
			{
				groupName = group.Name;
				groupBalance = group.Balance;
				groupCanWithdraw = m_Groups.CanWithdraw(group, OS_Util.SteamId(player));
			}
		}
		rpc.Write(groupName);
		rpc.Write(groupBalance);
		rpc.Write(groupCanWithdraw);
		rpc.Send(null, OS_RPC_BANK_DATA, true, player.GetIdentity());
	}
}
