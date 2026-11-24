// Jednotné placení: hotovost (předmět BC_Money) a/nebo bankovní účet (peněženka).

class BC_Payments
{
	protected BC_Wallets m_Wallets;
	protected BC_BankConfig m_Cfg;

	void BC_Payments(BC_Wallets wallets, BC_BankConfig cfg)
	{
		m_Wallets = wallets;
		m_Cfg = cfg;
	}

	int Cash(PlayerBase player)
	{
		return BC_Money.CountCash(player);
	}

	int Bank(PlayerBase player)
	{
		return m_Wallets.GetBalance(BC_Util.SteamId(player));
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

		string steamId = BC_Util.SteamId(player);
		if (!m_Cfg.UseCash)
			return m_Wallets.TrySpend(steamId, amount);

		int cash = Cash(player);
		if (cash >= amount)
			return BC_Money.TakeCash(player, amount);

		if (!m_Cfg.ShopsAcceptBank || cash + Bank(player) < amount)
			return false;

		// zbytek z účtu
		if (!m_Wallets.TrySpend(steamId, amount - cash))
			return false;
		BC_Money.TakeCash(player, cash);
		return true;
	}

	//! Výplata (prodej, odměna, vrácení peněz)
	void Pay(PlayerBase player, int amount)
	{
		if (amount <= 0 || !player)
			return;

		if (m_Cfg.UseCash)
			BC_Money.GiveCash(player, amount);
		else
			m_Wallets.Add(BC_Util.SteamId(player), amount);
	}
}
