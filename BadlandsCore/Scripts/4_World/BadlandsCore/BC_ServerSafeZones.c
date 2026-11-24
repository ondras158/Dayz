// Bezpečné zóny: nesmrtelnost hráčů, odstraňování zombíků, hlášky při vstupu/odchodu.

class BC_ServerSafeZones
{
	ref BC_SafeZoneConfig m_Cfg;
	protected float m_ZombieTimer;

	void BC_ServerSafeZones()
	{
		BC_SafeZoneConfig defaults = new BC_SafeZoneConfig();
		defaults.FillDefaults();
		m_Cfg = BC_ConfigFile<BC_SafeZoneConfig>.Load("SafeZones.json", defaults, new BC_SafeZoneConfig());
	}

	BC_SafeZone FindZone(vector pos)
	{
		if (!m_Cfg.Zones)
			return null;

		foreach (BC_SafeZone zone : m_Cfg.Zones)
		{
			if (zone && BC_Util.Distance2D(pos, zone.X, zone.Z) <= zone.Radius)
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

			BC_SafeZone zone = FindZone(player.GetPosition());
			bool inside = false;
			if (zone)
				inside = true;

			if (inside == player.m_BC_InSafeZone)
				continue;

			player.m_BC_InSafeZone = inside;
			player.m_BC_SafeZoneGod = inside && m_Cfg.Godmode;
			player.BC_RefreshDamage();

			if (inside)
				BC_Util.Notify(player, zone.Name, m_Cfg.EnterMessage);
			else
				BC_Util.Notify(player, "Bezpecna zona", m_Cfg.LeaveMessage);
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
		foreach (BC_SafeZone zone : m_Cfg.Zones)
		{
			if (!zone)
				continue;

			array<Object> objects = new array<Object>;
			g_Game.GetObjectsAtPosition(BC_Util.GroundPos(zone.X, zone.Z), zone.Radius, objects, null);
			foreach (Object obj : objects)
			{
				ZombieBase zombie = ZombieBase.Cast(obj);
				if (zombie)
					g_Game.ObjectDelete(zombie);
			}
		}
	}
}
