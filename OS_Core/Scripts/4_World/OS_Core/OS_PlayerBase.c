modded class PlayerBase
{
	// stav jen na serveru
	bool m_OS_InSafeZone;
	bool m_OS_SafeZoneGod;
	bool m_OS_AdminGod;
	bool m_OS_SpawnProtect;

	//! Hráč je nesmrtelný, pokud platí aspoň jeden důvod
	void OS_RefreshDamage()
	{
		if (!g_Game.IsServer())
			return;

		bool protectedNow = m_OS_SafeZoneGod || m_OS_AdminGod || m_OS_SpawnProtect;
		SetAllowDamage(!protectedNow);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (g_Game.IsServer() && OS_Server.Get())
			OS_Server.Get().OnPlayerKilled(this, killer);
	}
}
