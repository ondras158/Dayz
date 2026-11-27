modded class ZombieBase
{
	//! v bezpečné zóně se zombíci nespawnují
	override void EEInit()
	{
		super.EEInit();
		if (g_Game.IsServer())
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(OS_CheckSafeZone, 500, false);
	}

	void OS_CheckSafeZone()
	{
		if (OS_Server.Get() && OS_Server.Get().m_SafeZones)
			OS_Server.Get().m_SafeZones.CheckSpawned(this);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (g_Game.IsServer() && OS_Server.Get())
			OS_Server.Get().OnZombieKilled(this, killer);
	}
}

modded class AnimalBase
{
	override void EEInit()
	{
		super.EEInit();
		if (g_Game.IsServer())
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(OS_CheckSafeZone, 500, false);
	}

	void OS_CheckSafeZone()
	{
		if (OS_Server.Get() && OS_Server.Get().m_SafeZones)
			OS_Server.Get().m_SafeZones.CheckSpawned(this);
	}
}

//! loot se v bezpečné zóně nespawnuje (kontroluje se jen věc na zemi, ne v inventáři)
modded class ItemBase
{
	override void EEInit()
	{
		super.EEInit();
		// po startu serveru se načítají uložené věci - ty nechat být
		if (g_Game.IsServer() && g_Game.GetTickTime() > 120 && !GetHierarchyParent())
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(OS_CheckSafeZone, 1000, false);
	}

	void OS_CheckSafeZone()
	{
		if (OS_Server.Get() && OS_Server.Get().m_SafeZones && OS_Server.Get().m_SafeZones.m_Cfg.BlockItemSpawns)
			OS_Server.Get().m_SafeZones.CheckSpawned(this);
	}
}
