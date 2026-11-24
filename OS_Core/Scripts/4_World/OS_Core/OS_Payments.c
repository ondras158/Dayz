// Jednotné placení: hotovost (předmět OS_Money) a/nebo bankovní účet (peněženka).

class OS_Payments
{
	protected OS_Wallets m_Wallets;
	protected OS_BankConfig m_Cfg;
	protected string m_Currency;

	void OS_Payments(OS_Wallets wallets, OS_BankConfig cfg, string currency)
	{
		m_Currency = currency;
		m_Wallets = wallets;
		m_Cfg = cfg;
	}

	int Cash(PlayerBase player)
	{
		return OS_Money.CountCash(player);
	}

	int Bank(PlayerBase player)
	{
		return m_Wallets.GetBalance(OS_Util.SteamId(player));
	}

	//! Kolik může hráč utratit v obchodě
	int Available(PlayerBase player)
	{
		if (!m_Cfg.UseCash)
			return Bank(player);

		int total = Cash(player);
		if (m_Cfg.ShopsAcceptBank)
			total += Bank(player);
		return total;
	}

	bool TrySpend(PlayerBase player, int amount)
	{
		if (amount < 0)
			return false;

		string steamId = OS_Util.SteamId(player);
		if (!m_Cfg.UseCash)
			return m_Wallets.TrySpend(steamId, amount);

		int leftover;
		int cash = Cash(player);
		if (cash >= amount)
		{
			if (!OS_Money.TakeCash(player, amount, leftover))
				return false;
			DepositSmallChange(player, leftover);
			return true;
		}

		if (!m_Cfg.ShopsAcceptBank || cash + Bank(player) < amount)
			return false;

		// zbytek z účtu
		if (!m_Wallets.TrySpend(steamId, amount - cash))
			return false;
		OS_Money.TakeCash(player, cash, leftover);
		DepositSmallChange(player, leftover);
		return true;
	}

	//! Výplata (prodej, odměna, vrácení peněz). Část menší než nejmenší bankovka jde na účet.
	void Pay(PlayerBase player, int amount)
	{
		if (amount <= 0 || !player)
			return;

		if (!m_Cfg.UseCash)
		{
			m_Wallets.Add(OS_Util.SteamId(player), amount);
			return;
		}

		int leftover = OS_Money.GiveCash(player, amount);
		DepositSmallChange(player, leftover);
	}

	//! drobné, které nejde vyplatit bankovkami, připsat na účet
	protected void DepositSmallChange(PlayerBase player, int amount)
	{
		if (amount <= 0)
			return;
		m_Wallets.Add(OS_Util.SteamId(player), amount);
		OS_Util.Notify(player, "Banka", "Drobne " + amount.ToString() + " " + m_Currency + " pripsany na ucet.", 4);
	}
}
