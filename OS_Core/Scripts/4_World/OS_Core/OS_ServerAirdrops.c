// Airdropy: bedna pomalu padá (padák), po dopadu kouř a stráže, oznámení v chatu a značka na mapě.

class OS_ActiveAirdrop
{
	EntityAI Container;
	string Name;
	vector Target;          // místo dopadu
	float Height;           // aktuální výška nad zemí
	bool Landed;
	float LandedAt;
}

class OS_ServerAirdrops
{
	protected static const string CFG_FILE = "Airdrops.json";

	ref OS_AirdropConfig m_Cfg;
	protected ref array<ref OS_ActiveAirdrop> m_Active = new array<ref OS_ActiveAirdrop>;
	protected float m_NextDrop;
	protected float m_Timer;

	void OS_ServerAirdrops()
	{
		OS_AirdropConfig defaults = new OS_AirdropConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_AirdropConfig>.Load(CFG_FILE, defaults, new OS_AirdropConfig());
		if (!m_Cfg.Locations)
			m_Cfg.Locations = new array<ref OS_AirdropLocation>;
		if (!m_Cfg.LootTables)
			m_Cfg.LootTables = new array<ref OS_AirdropLoot>;
		ScheduleNext();
	}

	protected void ScheduleNext()
	{
		float minutes = Math.Max(1, m_Cfg.IntervalMinutes) * Math.RandomFloatInclusive(0.75, 1.25);
		m_NextDrop = g_Game.GetTickTime() + minutes * 60;
	}

	//! pro mapu klientů
	void FillMap(array<string> names, array<float> xs, array<float> zs)
	{
		if (!m_Cfg.Enabled || !m_Cfg.ShowOnMap)
			return;
		foreach (OS_ActiveAirdrop drop : m_Active)
		{
			if (!drop || !drop.Container)
				continue;
			names.Insert(OS_F("Airdrop: %1", OS_T(drop.Name)));
			xs.Insert(drop.Target[0]);
			zs.Insert(drop.Target[2]);
		}
	}

	protected void RefreshMaps()
	{
		if (OS_Server.Get() && OS_Server.Get().m_PvP)
			OS_Server.Get().m_PvP.BroadcastZones();
	}

	// ------------------------------------------------------------ shození

	//! náhodné místo z Locations
	bool DropRandom()
	{
		if (m_Cfg.Locations.Count() == 0)
		{
			Print(OS_TAG + "AIRDROP: v Airdrops.json nejsou zadna mista (Locations)");
			return false;
		}
		OS_AirdropLocation loc = m_Cfg.Locations.GetRandomElement();
		float angle = Math.RandomFloat(0, Math.PI2);
		float dist = Math.RandomFloat(0, loc.Radius);
		float x = loc.X + Math.Cos(angle) * dist;
		float z = loc.Z + Math.Sin(angle) * dist;
		return DropAt(OS_T(loc.Name), x, z);
	}

	bool DropAt(string name, float x, float z)
	{
		vector ground = OS_Util.GroundPos(x, z);
		vector start = ground + Vector(0, m_Cfg.DropHeight, 0);
		EntityAI container = EntityAI.Cast(g_Game.CreateObjectEx(m_Cfg.ContainerClass, start, ECE_KEEPHEIGHT | ECE_NOLIFETIME));
		if (!container)
		{
			Print(OS_TAG + "AIRDROP: nepodarilo se vytvorit bednu '" + m_Cfg.ContainerClass + "'");
			return false;
		}

		OS_ActiveAirdrop drop = new OS_ActiveAirdrop();
		drop.Container = container;
		drop.Name = name;
		drop.Target = ground;
		drop.Height = m_Cfg.DropHeight;
		m_Active.Insert(drop);

		FillLoot(container);

		int gx = Math.Round(x / 100);
		int gz = Math.Round(z / 100);
		if (m_Cfg.AnnounceInChat)
			OS_Util.ChatAll(OS_F("AIRDROP pada: %1 (mapa, ctverec %2 / %3). Pozor na straze!", name, gx.ToString(), gz.ToString()));
		Print(OS_TAG + "AIRDROP " + name + " na " + ground.ToString());
		RefreshMaps();
		return true;
	}

	//! loot z tabulky podle jména ("" = náhodná tabulka) - používají ho i konvoje
	void FillLootFrom(EntityAI container, string tableName)
	{
		OS_AirdropLoot chosen;
		if (tableName != "")
		{
			foreach (OS_AirdropLoot t : m_Cfg.LootTables)
			{
				if (t && t.Name == tableName)
					chosen = t;
			}
		}
		if (!chosen)
			chosen = PickTable();
		FillLootTable(container, chosen);
	}

	protected void FillLoot(EntityAI container)
	{
		FillLootTable(container, PickTable());
	}

	protected void FillLootTable(EntityAI container, OS_AirdropLoot table)
	{
		if (!table)
			return;
		foreach (OS_AirdropItem entry : table.Items)
		{
			if (!entry || Math.RandomFloat01() > entry.Chance)
				continue;
			array<string> options = new array<string>;
			entry.Items.Split("|", options);
			if (options.Count() == 0)
				continue;
			for (int i = 0; i < Math.Max(1, entry.Count); i++)
			{
				string type = options.GetRandomElement();
				type.TrimInPlace();
				EntityAI item = container.GetInventory().CreateInInventory(type);
				if (!item)
					continue;
				foreach (string attachment : entry.Attachments)
				{
					item.GetInventory().CreateAttachment(attachment);
				}
			}
		}
	}

	protected OS_AirdropLoot PickTable()
	{
		int total = 0;
		foreach (OS_AirdropLoot t : m_Cfg.LootTables)
		{
			if (t)
				total += Math.Max(0, t.Weight);
		}
		if (total <= 0)
			return null;
		int roll = Math.RandomInt(0, total);
		foreach (OS_AirdropLoot table : m_Cfg.LootTables)
		{
			if (!table)
				continue;
			roll -= Math.Max(0, table.Weight);
			if (roll < 0)
				return table;
		}
		return null;
	}

	protected void OnLanded(OS_ActiveAirdrop drop)
	{
		drop.Landed = true;
		drop.LandedAt = g_Game.GetTickTime();
		drop.Container.SetPosition(drop.Target);
		drop.Container.PlaceOnSurface();

		if (m_Cfg.SmokeClass != "")
		{
			Grenade_Base smoke = Grenade_Base.Cast(g_Game.CreateObjectEx(m_Cfg.SmokeClass, drop.Target + Vector(1.5, 0, 1.5), ECE_PLACE_ON_SURFACE));
			if (smoke)
				smoke.ActivateImmediate();
		}

		if (m_Cfg.GuardCount > 0)
			SpawnGuards(drop.Target, m_Cfg.GuardCount);

		if (m_Cfg.AnnounceInChat)
			OS_Util.ChatAll(OS_F("AIRDROP pristal: %1", drop.Name));
	}

	//! stráže u dopadu - vojenští nakažení (doplněk OS_Core_ExpansionAI je nahradí vojáky Expansion AI)
	void SpawnGuards(vector pos, int count)
	{
		array<string> guards = new array<string>;
		m_Cfg.GuardClasses.Split("|", guards);
		for (int i = 0; i < count && guards.Count() > 0; i++)
		{
			float angle = Math.RandomFloat(0, Math.PI2);
			float dist = Math.RandomFloat(8, 25);
			vector guardPos = OS_Util.GroundPos(pos[0] + Math.Cos(angle) * dist, pos[2] + Math.Sin(angle) * dist);
			string guardType = guards.GetRandomElement();
			guardType.TrimInPlace();
			g_Game.CreateObjectEx(guardType, guardPos, ECE_PLACE_ON_SURFACE | ECE_INITAI);
		}
	}

	// ------------------------------------------------------------ běh

	void OnUpdate(float timeslice, int playersOnline)
	{
		// pád bedny plynule (každý snímek serveru)
		for (int i = m_Active.Count() - 1; i >= 0; i--)
		{
			OS_ActiveAirdrop drop = m_Active[i];
			if (!drop || !drop.Container)
			{
				m_Active.Remove(i);
				RefreshMaps();
				continue;
			}
			if (!drop.Landed)
			{
				drop.Height = drop.Height - m_Cfg.FallSpeed * timeslice;
				if (drop.Height <= 0.3)
				{
					OnLanded(drop);
				}
				else
				{
					drop.Container.SetPosition(drop.Target + Vector(0, drop.Height, 0));
					drop.Container.SetOrientation(Vector(drop.Height * 3, 0, 0));
				}
				continue;
			}
			if (g_Game.GetTickTime() - drop.LandedAt > m_Cfg.LifetimeMinutes * 60)
			{
				g_Game.ObjectDelete(drop.Container);
				m_Active.Remove(i);
				RefreshMaps();
			}
		}

		if (!m_Cfg.Enabled)
			return;
		m_Timer += timeslice;
		if (m_Timer < 5)
			return;
		m_Timer = 0;
		if (g_Game.GetTickTime() < m_NextDrop)
			return;
		ScheduleNext();
		if (playersOnline >= m_Cfg.MinPlayers)
			DropRandom();
	}
}
