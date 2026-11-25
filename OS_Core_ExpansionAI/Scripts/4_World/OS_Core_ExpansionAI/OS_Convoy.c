// Konvoje: vozidlo s posádkou vojáků Expansion AI jede po trase, v kufru má loot.
// Přepadení (výstřel do auta, zabitý voják, hráč blízko) -> zastaví, vojáci vystoupí a bojují.
// Projíždí-li kolem base s hráči, zastaví a zaútočí na ni.
// Auto jede skriptem po bodech trasy (spolehlivé), ne řízením AI. Nastavení v ExpansionAI.json.

class OS_ConvoyRoute
{
	string Name;
	ref array<vector> Waypoints = new array<vector>;   // body trasy po silnici [x, 0, z], hustě (zatáčky)

	void OS_ConvoyRoute(string name = "")
	{
		Name = name;
	}
}

class OS_ConvoyConfig
{
	bool Enabled = true;
	float IntervalMinutes = 50;          // jak často vyjede konvoj (náhodně ±30 %)
	int MinPlayers = 1;
	string Vehicles = "Truck_01_Covered|OffroadHatchback|CivilianSedan";   // náhodně jedno
	float SpeedKmh = 35;
	int Crew = 4;                        // vojáků (řidič + spolujezdci, podle míst ve vozidle)
	string Faction = "Raiders";
	string Loadout = "";
	string LootTable = "Vojensky";       // tabulka z Airdrops.json ("" = náhodná)
	int LootRolls = 2;                   // kolikrát se tabulka do kufru vylosuje
	float AmbushDistance = 35;           // hráč blíž než tohle (a vidí ho) = přepadení
	bool AttackBases = true;             // projíždí-li kolem base s hráčem, zaútočí
	float AttackBaseRadius = 250;
	bool Announce = true;                // oznámení v chatu (vyjel, přepaden)
	bool ShowOnMap = true;               // poloha konvoje na mapě
	float LifetimeMinutes = 30;          // po zastavení / v cíli konvoj (auto a zbylí vojáci) zmizí
	ref array<ref OS_ConvoyRoute> Routes = new array<ref OS_ConvoyRoute>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÁ TRASA - nahraď body po silnici na své mapě (aspoň každých 50-100 m, v zatáčkách hustěji) !!!
		OS_ConvoyRoute route = new OS_ConvoyRoute("Priklad - Silnice sever-jih");
		route.Waypoints.Insert(Vector(5000, 0, 11000));
		route.Waypoints.Insert(Vector(5050, 0, 10800));
		route.Waypoints.Insert(Vector(5100, 0, 10600));
		route.Waypoints.Insert(Vector(5120, 0, 10400));
		route.Waypoints.Insert(Vector(5150, 0, 10200));
		Routes.Insert(route);
	}
}

class OS_Convoy
{
	Transport Vehicle;
	string RouteName;
	ref array<vector> Points = new array<vector>;
	int Segment;
	float SegmentProgress;     // metry od začátku úseku
	eAIDynamicPatrol Patrol;
	bool Seated;
	bool Stopped;
	float StoppedAt;
	float SpawnedAt;
	float VehicleHealth;
}

class OS_ConvoyDirector
{
	ref OS_ConvoyConfig m_Cfg;
	protected ref array<ref OS_Convoy> m_Convoys = new array<ref OS_Convoy>;
	protected float m_NextConvoy;
	protected float m_Timer;
	protected float m_MapTimer;

	void OS_ConvoyDirector()
	{
		OS_ConvoyConfig defaults = new OS_ConvoyConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_ConvoyConfig>.Load("Convoys.json", defaults, new OS_ConvoyConfig());
		if (!m_Cfg.Routes)
			m_Cfg.Routes = new array<ref OS_ConvoyRoute>;
		Schedule();
	}

	protected void Schedule()
	{
		float minutes = Math.Max(5, m_Cfg.IntervalMinutes) * Math.RandomFloatInclusive(0.7, 1.3);
		m_NextConvoy = g_Game.GetTickTime() + minutes * 60;
	}

	void FillMap(array<string> names, array<float> xs, array<float> zs)
	{
		if (!m_Cfg.Enabled || !m_Cfg.ShowOnMap)
			return;
		foreach (OS_Convoy convoy : m_Convoys)
		{
			if (!convoy || !convoy.Vehicle)
				continue;
			vector pos = convoy.Vehicle.GetPosition();
			names.Insert(OS_T("Konvoj"));
			xs.Insert(pos[0]);
			zs.Insert(pos[2]);
		}
	}

	// ------------------------------------------------------------ vyslání

	bool Launch()
	{
		if (m_Cfg.Routes.Count() == 0 || !OS_Server.Get() || !OS_Server.Get().m_Vehicles)
			return false;
		OS_ConvoyRoute route = m_Cfg.Routes.GetRandomElement();
		if (!route || route.Waypoints.Count() < 2)
			return false;

		OS_Convoy convoy = new OS_Convoy();
		convoy.RouteName = OS_T(route.Name);
		foreach (vector wp : route.Waypoints)
		{
			convoy.Points.Insert(OS_Util.GroundPos(wp[0], wp[2]));
		}

		array<string> types = new array<string>;
		m_Cfg.Vehicles.Split("|", types);
		string type = types.GetRandomElement();
		type.TrimInPlace();
		vector start = convoy.Points[0];
		vector dir = vector.Direction(start, convoy.Points[1]);
		vector startAngles = dir.VectorToAngles();
		float yaw = startAngles[0];
		convoy.Vehicle = OS_Server.Get().m_Vehicles.AdminSpawn(type, start, yaw);
		if (!convoy.Vehicle)
		{
			Print(OS_TAG + "KONVOJ: nepodarilo se vytvorit vozidlo " + type);
			return false;
		}
		dBodyDynamic(convoy.Vehicle, false);   // jede skriptem, ne fyzikou
		convoy.VehicleHealth = convoy.Vehicle.GetHealth("", "");
		convoy.SpawnedAt = g_Game.GetTickTime();

		// loot do kufru
		if (OS_Server.Get().m_Airdrops)
		{
			for (int i = 0; i < Math.Max(1, m_Cfg.LootRolls); i++)
			{
				OS_Server.Get().m_Airdrops.FillLootFrom(convoy.Vehicle, m_Cfg.LootTable);
			}
		}

		// posádka: skupina Expansion AI u auta, po objevení se posadí
		int seats = convoy.Vehicle.CrewSize();
		int crew = Math.Min(m_Cfg.Crew, Math.Max(1, seats));
		array<vector> waypoints = new array<vector>;
		waypoints.Insert(start + Vector(3, 0, 3));
		convoy.Patrol = OS_ExpansionAI.Spawn(OS_T("Konvoj"), m_Cfg.Faction, m_Cfg.Loadout, crew, "HALT", "JOG", waypoints, -1, m_Cfg.LifetimeMinutes * 60 + 3600, 20000);

		m_Convoys.Insert(convoy);
		if (m_Cfg.Announce)
			OS_Util.ChatAll(OS_F("Ozbrojeny konvoj vyjel: %1. Ma naklad - prepadnete ho!", convoy.RouteName));
		Print(OS_TAG + "KONVOJ " + type + " na trase " + convoy.RouteName);
		RefreshMaps();
		return true;
	}

	protected void RefreshMaps()
	{
		if (OS_Server.Get() && OS_Server.Get().m_PvP)
			OS_Server.Get().m_PvP.BroadcastZones();
	}

	//! vojáci skupiny konvoje
	protected array<eAIBase> Crew(OS_Convoy convoy)
	{
		array<eAIBase> crew = new array<eAIBase>;
		if (!convoy.Patrol || !convoy.Patrol.m_Group)
			return crew;
		for (int i = 0; i < convoy.Patrol.m_Group.Count(); i++)
		{
			eAIBase ai = eAIBase.Cast(convoy.Patrol.m_Group.GetMember(i));
			if (ai && ai.IsAlive())
				crew.Insert(ai);
		}
		return crew;
	}

	protected void SeatCrew(OS_Convoy convoy)
	{
		array<eAIBase> crew = Crew(convoy);
		if (crew.Count() == 0)
			return;
		int seats = convoy.Vehicle.CrewSize();
		for (int i = 0; i < crew.Count() && i < seats; i++)
		{
			crew[i].Notify_Transport(convoy.Vehicle, i);
		}
		convoy.Seated = true;
	}

	//! zastavit: auto zpět na fyziku, vojáci vystoupí a hlídají / útočí na cíl
	protected void Stop(OS_Convoy convoy, string reason, vector attackPos = "0 0 0")
	{
		if (convoy.Stopped)
			return;
		convoy.Stopped = true;
		convoy.StoppedAt = g_Game.GetTickTime();
		dBodyDynamic(convoy.Vehicle, true);

		array<eAIBase> crew = Crew(convoy);
		foreach (eAIBase ai : crew)
		{
			HumanCommandVehicle vehCmd = ai.GetCommand_Vehicle();
			if (vehCmd)
				vehCmd.GetOutVehicle();
		}

		if (convoy.Patrol && convoy.Patrol.m_Group)
		{
			convoy.Patrol.m_Group.ClearWaypoints();
			if (attackPos != vector.Zero)
			{
				convoy.Patrol.m_Group.AddWaypoint(attackPos);
				convoy.Patrol.m_Group.SetWaypointBehaviour(eAIWaypointBehavior.ONCE);
			}
			else
			{
				convoy.Patrol.m_Group.AddWaypoint(convoy.Vehicle.GetPosition());
				convoy.Patrol.m_Group.SetWaypointBehaviour(eAIWaypointBehavior.HALT);
			}
		}

		if (m_Cfg.Announce && reason != "")
			OS_Util.ChatAll(reason);
		RefreshMaps();
	}

	// ------------------------------------------------------------ jízda

	void OnUpdate(float timeslice)
	{
		for (int i = m_Convoys.Count() - 1; i >= 0; i--)
		{
			OS_Convoy convoy = m_Convoys[i];
			if (!convoy || !convoy.Vehicle)
			{
				m_Convoys.Remove(i);
				RefreshMaps();
				continue;
			}

			if (convoy.Stopped)
			{
				if (g_Game.GetTickTime() - convoy.StoppedAt > m_Cfg.LifetimeMinutes * 60)
					Remove(convoy, i);
				continue;
			}

			// posádka se posadí, jakmile ji Expansion vytvoří (max. 30 s čekání, jinak jede bez ní)
			if (!convoy.Seated)
			{
				SeatCrew(convoy);
				if (!convoy.Seated && g_Game.GetTickTime() - convoy.SpawnedAt < 30)
					continue;
				convoy.Seated = true;
			}

			Drive(convoy, timeslice, i);
		}
	}

	protected void Drive(OS_Convoy convoy, float timeslice, int index)
	{
		if (convoy.Segment >= convoy.Points.Count() - 1)
		{
			// cíl: zastaví a posádka zůstane u auta
			Stop(convoy, OS_F("Konvoj dorazil do cile (%1).", convoy.RouteName));
			return;
		}

		float speed = m_Cfg.SpeedKmh / 3.6;
		convoy.SegmentProgress += speed * timeslice;
		vector a = convoy.Points[convoy.Segment];
		vector b = convoy.Points[convoy.Segment + 1];
		float length = vector.Distance(a, b);
		while (convoy.SegmentProgress >= length && convoy.Segment < convoy.Points.Count() - 1)
		{
			convoy.SegmentProgress -= length;
			convoy.Segment++;
			if (convoy.Segment >= convoy.Points.Count() - 1)
				return;
			a = convoy.Points[convoy.Segment];
			b = convoy.Points[convoy.Segment + 1];
			length = vector.Distance(a, b);
		}

		float t = 0;
		if (length > 0.01)
			t = convoy.SegmentProgress / length;
		vector pos = a + (b - a) * t;
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]) + 0.2;
		vector dir = vector.Direction(a, b);
		vector angles = dir.VectorToAngles();
		convoy.Vehicle.SetPosition(pos);
		convoy.Vehicle.SetOrientation(Vector(angles[0], 0, 0));
		convoy.Vehicle.PlaceOnSurface();
	}

	protected void Remove(OS_Convoy convoy, int index)
	{
		if (convoy.Patrol)
			convoy.Patrol.Despawn();
		if (convoy.Vehicle)
			g_Game.ObjectDelete(convoy.Vehicle);
		m_Convoys.Remove(index);
		RefreshMaps();
	}

	// ------------------------------------------------------------ přepadení a útoky (1× za sekundu)

	void OnTick(array<Man> players, float delta)
	{
		if (m_Cfg.Enabled && g_Game.GetTickTime() >= m_NextConvoy)
		{
			Schedule();
			if (players.Count() >= m_Cfg.MinPlayers)
				Launch();
		}

		foreach (OS_Convoy convoy : m_Convoys)
		{
			if (!convoy || !convoy.Vehicle || convoy.Stopped)
				continue;
			vector pos = convoy.Vehicle.GetPosition();

			// výstřel do auta nebo zabitý voják
			float health = convoy.Vehicle.GetHealth("", "");
			bool hit = health < convoy.VehicleHealth - 1;
			convoy.VehicleHealth = health;
			if (!hit && convoy.Patrol && convoy.Patrol.m_Group && Crew(convoy).Count() < convoy.Patrol.m_Group.Count())
				hit = true;
			if (hit)
			{
				Stop(convoy, OS_F("Konvoj byl prepaden (%1)!", convoy.RouteName));
				continue;
			}

			foreach (Man man : players)
			{
				PlayerBase player = PlayerBase.Cast(man);
				if (!player || !player.IsAlive() || player.m_OS_InSafeZone)
					continue;
				float dist = vector.Distance(pos, player.GetPosition());

				// hráč těsně u konvoje
				if (dist < m_Cfg.AmbushDistance)
				{
					Stop(convoy, OS_F("Konvoj byl prepaden (%1)!", convoy.RouteName));
					break;
				}

				// base s hráčem poblíž trasy -> útok
				if (m_Cfg.AttackBases && dist < m_Cfg.AttackBaseRadius && OS_Server.Get() && OS_Server.Get().m_Bases && OS_Server.Get().m_Bases.IsInOwnBase(player))
				{
					Stop(convoy, "", player.GetPosition());
					OS_Util.Notify(player, OS_T("Pozor"), OS_T("Konvoj zastavil u tve base - vojaci utoci!"), 8);
					break;
				}
			}
		}
	}
}

modded class OS_Server
{
	protected ref OS_ConvoyDirector m_Convoys;
	protected float m_ConvoyTimer;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		if (!m_Convoys)
			m_Convoys = new OS_ConvoyDirector();
		m_Convoys.OnUpdate(timeslice);

		m_ConvoyTimer += timeslice;
		if (m_ConvoyTimer < 1)
			return;
		float delta = m_ConvoyTimer;
		m_ConvoyTimer = 0;
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		m_Convoys.OnTick(players, delta);
	}

	OS_ConvoyDirector GetConvoys()
	{
		return m_Convoys;
	}

	override bool AdminLaunchConvoy()
	{
		if (!m_Convoys)
			m_Convoys = new OS_ConvoyDirector();
		return m_Convoys.Launch();
	}
}

//! konvoje na mapě (k airdropům)
modded class OS_ServerAirdrops
{
	override void FillMap(array<string> names, array<float> xs, array<float> zs)
	{
		super.FillMap(names, xs, zs);
		if (OS_Server.Get() && OS_Server.Get().GetConvoys())
			OS_Server.Get().GetConvoys().FillMap(names, xs, zs);
	}
}
