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
		if (rpcType != BC_RPC_BANK_ACTION)
			return false;

		PlayerBase player = BC_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		if (!IsNearAtm(player))
		{
			BC_Util.Notify(player, "Bankomat", "Musis stat u bankomatu.");
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

		if (!BC_Money.TakeCash(player, amount))
			return;

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
		if (amount <= 0)
		{
			BC_Util.Notify(player, "Bankomat", "Na uctu nic neni.");
			return;
		}

		if (!m_Wallets.TrySpend(steamId, amount))
		{
			BC_Util.Notify(player, "Bankomat", "Na uctu nemas dost penez.");
			return;
		}

		BC_Money.GiveCash(player, amount);
		BC_Util.Notify(player, "Bankomat", "Vybrano " + amount.ToString() + " " + m_Currency);
		Print(BC_TAG + "BANKA " + BC_Util.PlayerName(player) + " vyber " + amount.ToString());
	}

	protected void SendData(PlayerBase player)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Currency);
		rpc.Write(BC_Money.CountCash(player));
		rpc.Write(m_Wallets.GetBalance(BC_Util.SteamId(player)));
		rpc.Write(m_Cfg.DepositFeePercent);
		rpc.Send(null, BC_RPC_BANK_DATA, true, player.GetIdentity());
	}
}
