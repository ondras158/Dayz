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

	//! spoluhráči ze skupiny si neublíží (Groups.json: FriendlyFire)
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (g_Game.IsServer() && OS_Server.Get() && OS_Server.Get().m_Groups && OS_Server.Get().m_Groups.BlocksDamage(this, source))
			return false;

		return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (g_Game.IsServer() && OS_Server.Get())
			OS_Server.Get().OnPlayerKilled(this, killer);
	}
}
