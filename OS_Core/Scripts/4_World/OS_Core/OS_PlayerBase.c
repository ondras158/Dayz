modded class PlayerBase
{
	// stav jen na serveru
	bool m_OS_InSafeZone;
	bool m_OS_SafeZoneGod;
	bool m_OS_AdminGod;
	bool m_OS_SpawnProtect;
	bool m_OS_InPvPZone;
	string m_OS_BuffType;      // bonus z jídla (regen / blood / warmth / stamina)
	float m_OS_BuffUntil;
	float m_OS_PvPLeftAt;      // čas odchodu z PvP zóny (běží časovač), 0 = neběží

	// neviditelnost admina (synchronizuje se všem klientům)
	protected bool m_OS_Invisible;

	override void Init()
	{
		super.Init();
		RegisterNetSyncVariableBool("m_OS_Invisible");
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		SetInvisible(m_OS_Invisible);
	}

	void OS_SetInvisible(bool state)
	{
		m_OS_Invisible = state;
		SetInvisible(state);
		SetSynchDirty();
	}

	bool OS_IsInvisible()
	{
		return m_OS_Invisible;
	}

	//! Hráč je nesmrtelný, pokud platí aspoň jeden důvod
	void OS_RefreshDamage()
	{
		if (!g_Game.IsServer())
			return;

		bool protectedNow = m_OS_SafeZoneGod || m_OS_AdminGod || m_OS_SpawnProtect;
		SetAllowDamage(!protectedNow);
	}

	//! spoluhráči ze skupiny si neublíží (Groups.json: FriendlyFire), mimo PvP zónu nikdo
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (g_Game.IsServer() && OS_Server.Get() && OS_Server.Get().m_Groups && OS_Server.Get().m_Groups.BlocksDamage(this, source))
			return false;

		// mimo PvP zónu hráč hráči neublíží (PvP.json: PvPOnlyInZones)
		if (g_Game.IsServer() && OS_Server.Get() && OS_Server.Get().m_PvP && OS_Server.Get().m_PvP.BlocksDamage(this, source))
			return false;

		return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}

	//! dovednost Odolnost: část zranění se vrátí
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		if (g_Game.IsServer() && OS_Server.Get() && OS_Server.Get().m_Skills)
			OS_Server.Get().m_Skills.OnHit(this, damageResult);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (g_Game.IsServer() && OS_Server.Get())
			OS_Server.Get().OnPlayerKilled(this, killer);
	}
}
