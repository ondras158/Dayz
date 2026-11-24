// Bezpečné zóny: nesmrtelnost hráčů, odstraňování zombíků, hlášky při vstupu/odchodu.

class OS_ServerSafeZones
{
	ref OS_SafeZoneConfig m_Cfg;
	protected float m_ZombieTimer;

	void OS_ServerSafeZones()
	{
		OS_SafeZoneConfig defaults = new OS_SafeZoneConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_SafeZoneConfig>.Load("SafeZones.json", defaults, new OS_SafeZoneConfig());
	}

	OS_SafeZone FindZone(vector pos)
	{
		if (!m_Cfg.Zones)
			return null;

		foreach (OS_SafeZone zone : m_Cfg.Zones)
		{
			if (zone && OS_Util.Distance2D(pos, zone.X, zone.Z) <= zone.Radius)
				return zone;
		}
		return null;
	}

	void OnTick(array<Man> players, float deltaTime)
	{
		if (!m_Cfg.Enabled)
			return;

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive())
				continue;

			OS_SafeZone zone = FindZone(player.GetPosition());
			bool inside = false;
			if (zone)
				inside = true;

			if (inside == player.m_OS_InSafeZone)
				continue;

			player.m_OS_InSafeZone = inside;
			player.m_OS_SafeZoneGod = inside && m_Cfg.Godmode;
			player.OS_RefreshDamage();

			if (inside)
				OS_Util.Notify(player, zone.Name, m_Cfg.EnterMessage);
			else
				OS_Util.Notify(player, "Bezpecna zona", m_Cfg.LeaveMessage);
		}

		if (m_Cfg.RemoveZombies)
		{
			m_ZombieTimer += deltaTime;
			if (m_ZombieTimer >= m_Cfg.ZombieCleanupSeconds)
			{
				m_ZombieTimer = 0;
				RemoveZombies();
			}
		}
	}

	protected void RemoveZombies()
	{
		foreach (OS_SafeZone zone : m_Cfg.Zones)
		{
			if (!zone)
				continue;

			array<Object> objects = new array<Object>;
			g_Game.GetObjectsAtPosition(OS_Util.GroundPos(zone.X, zone.Z), zone.Radius, objects, null);
			foreach (Object obj : objects)
			{
				ZombieBase zombie = ZombieBase.Cast(obj);
				if (zombie)
					g_Game.ObjectDelete(zombie);
			}
		}
	}
}
