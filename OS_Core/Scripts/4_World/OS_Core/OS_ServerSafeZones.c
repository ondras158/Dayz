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

			if (inside)
				Heal(player, deltaTime);

			if (inside == player.m_OS_InSafeZone)
				continue;

			player.m_OS_InSafeZone = inside;
			player.m_OS_SafeZoneGod = inside && m_Cfg.Godmode;
			player.OS_RefreshDamage();

			if (inside)
				OS_Util.Notify(player, OS_T(zone.Name), OS_T(m_Cfg.EnterMessage));
			else
				OS_Util.Notify(player, OS_T("Bezpecna zona"), OS_T(m_Cfg.LeaveMessage));
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

	//! zdraví (a krev) se doplní až na HealToPercent
	protected void Heal(PlayerBase player, float deltaTime)
	{
		if (m_Cfg.HealToPercent <= 0)
			return;
		float fraction = Math.Clamp(m_Cfg.HealToPercent / 100, 0, 1);

		float maxHealth = player.GetMaxHealth("", "Health");
		float health = player.GetHealth("", "Health");
		float target = maxHealth * fraction;
		if (health < target)
			player.AddHealth("", "Health", Math.Min(target - health, m_Cfg.HealPerSecond * deltaTime));

		if (!m_Cfg.HealBlood)
			return;
		float maxBlood = player.GetMaxHealth("", "Blood");
		float blood = player.GetHealth("", "Blood");
		float bloodTarget = maxBlood * fraction;
		if (blood < bloodTarget)
		{
			float bloodStep = maxBlood / maxHealth * m_Cfg.HealPerSecond * deltaTime;
			player.AddHealth("", "Blood", Math.Min(bloodTarget - blood, bloodStep));
		}
	}

	//! právě vytvořený zombík / zvíře / loot v zóně zmizí
	void CheckSpawned(EntityAI entity)
	{
		if (!entity || !m_Cfg.Enabled || entity.GetHierarchyParent())
			return;
		if (!FindZone(entity.GetPosition()))
			return;

		if (entity.IsZombie())
		{
			if (m_Cfg.RemoveZombies)
				g_Game.ObjectDelete(entity);
			return;
		}
		if (entity.IsDayZCreature() || AnimalBase.Cast(entity))
		{
			if (m_Cfg.RemoveAnimals)
				g_Game.ObjectDelete(entity);
			return;
		}

		// loot: věc na zemi, u které nestojí žádný hráč (hráčem upuštěné / vyrobené věci zůstávají)
		ItemBase item = ItemBase.Cast(entity);
		if (!item || !m_Cfg.BlockItemSpawns || BaseBuildingBase.Cast(item) || TentBase.Cast(item))
			return;
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			if (man && vector.Distance(man.GetPosition(), item.GetPosition()) < 5)
				return;
		}
		g_Game.ObjectDelete(item);
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
				{
					g_Game.ObjectDelete(zombie);
					continue;
				}
				if (m_Cfg.RemoveAnimals && AnimalBase.Cast(obj))
					g_Game.ObjectDelete(obj);
			}
		}
	}
}
