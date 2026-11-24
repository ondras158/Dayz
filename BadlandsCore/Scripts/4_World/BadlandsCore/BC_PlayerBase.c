modded class PlayerBase
{
	// stav jen na serveru
	bool m_BC_InSafeZone;
	bool m_BC_SafeZoneGod;
	bool m_BC_AdminGod;
	bool m_BC_SpawnProtect;

	//! Hráč je nesmrtelný, pokud platí aspoň jeden důvod
	void BC_RefreshDamage()
	{
		if (!g_Game.IsServer())
			return;

		bool protectedNow = m_BC_SafeZoneGod || m_BC_AdminGod || m_BC_SpawnProtect;
		SetAllowDamage(!protectedNow);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (g_Game.IsServer() && BC_Server.Get())
			BC_Server.Get().OnPlayerKilled(this, killer);
	}
}
