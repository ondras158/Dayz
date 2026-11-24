modded class ZombieBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (g_Game.IsServer() && BC_Server.Get())
			BC_Server.Get().OnZombieKilled(this, killer);
	}
}
