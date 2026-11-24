modded class ZombieBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (g_Game.IsServer() && OS_Server.Get())
			OS_Server.Get().OnZombieKilled(this, killer);
	}
}
