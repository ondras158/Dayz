// Konvoje: vozidlo s posádkou vojáků Expansion AI jede po trase, v kufru má loot.
// Přepadení (výstřel do auta, zabitý voják, hráč blízko) -> zastaví, vojáci vystoupí a bojují.
// Projíždí-li kolem base s hráči, zastaví a zaútočí na ni.
//
// Hybridní jízda:
//  - hráč blízko (PhysicsDistance): auto opravdu řídí voják za volantem (vlastní řidič OS - drží trasu,
//    zpomaluje do zatáček, řadí, brzdí před překážkou, couvne, když se zasekne),
//  - nikdo blízko: auto se posouvá po trase skriptem (spolehlivé, nezatěžuje server),
//  - zaseknuté nebo převrácené auto, na které se nikdo nedívá, se vrátí na trasu.
// Nastavení: $profile:OS_Core/Convoys.json

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
	float SpeedKmh = 35;                 // cestovní rychlost
	float MinCornerSpeedKmh = 12;        // nejnižší rychlost v ostré zatáčce
	int Crew = 4;                        // vojáků (řidič + spolujezdci, podle míst ve vozidle)
	string Faction = "Raiders";
	string Loadout = "";
	string LootTable = "Vojensky";       // tabulka z Airdrops.json ("" = náhodná)
	int LootRolls = 2;                   // kolikrát se tabulka do kufru vylosuje
	float AmbushDistance = 35;           // hráč blíž než tohle = přepadení
	bool AttackBases = true;             // projíždí-li kolem base s hráčem, zaútočí
	float AttackBaseRadius = 250;
	bool Announce = true;                // oznámení v chatu (vyjel, přepaden)
	bool ShowOnMap = true;               // poloha konvoje na mapě
	float LifetimeMinutes = 30;          // po zastavení / v cíli konvoj (auto a zbylí vojáci) zmizí

	// skutečné řízení
	bool RealDriving = true;             // false = vždy jen skriptová jízda
	float PhysicsDistance = 400;         // hráč blíž než tohle = auto řídí voják
	float UnseenDistance = 200;          // nikdo blíž než tohle = zaseknuté auto se smí vrátit na trasu
	float StuckSeconds = 4;              // po kolika sekundách bez pohybu couvnout
	float RecoverSeconds = 20;           // po kolika sekundách zaseknutí vrátit na trasu (když se nikdo nedívá)

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
	ref array<float> Cum = new array<float>;   // vzdálenost od začátku trasy k bodu i
	float Length;
	float S;                   // poloha na trase (m od začátku)
	eAIDynamicPatrol Patrol;
	bool Seated;
	bool Stopped;
	float StoppedAt;
	float SpawnedAt;
	float VehicleHealth;

	// řidič
	bool Physics;              // true = řídí voják
	vector LastPos;
	float StuckTime;
	float ReverseUntil;
	float ReverseSteer;

	vector PointAt(float s)
	{
		if (s <= 0)
			return Points[0];
		if (s >= Length)
			return Points[Points.Count() - 1];
		for (int i = 0; i < Points.Count() - 1; i++)
		{
			if (s <= Cum[i + 1])
			{
				float seg = Cum[i + 1] - Cum[i];
				float t = 0;
				if (seg > 0.01)
					t = (s - Cum[i]) / seg;
				return Points[i] + (Points[i + 1] - Points[i]) * t;
			}
		}
		return Points[Points.Count() - 1];
	}

	//! směr trasy v bodě s
	vector DirAt(float s)
	{
		vector a = PointAt(s - 2);
		vector b = PointAt(s + 2);
		vector dir = vector.Direction(a, b);
		dir[1] = 0;
		dir.Normalize();
		return dir;
	}

	//! nejbližší místo na trase k pozici (hledá kolem aktuálního S)
	float Project(vector pos)
	{
		float best = S;
		float bestDist = 1000000;
		for (int i = 0; i < Points.Count() - 1; i++)
		{
			if (Cum[i + 1] < S - 60 || Cum[i] > S + 120)
				continue;
			vector a = Points[i];
			vector b = Points[i + 1];
			vector ab = b - a;
			ab[1] = 0;
			vector ap = pos - a;
			ap[1] = 0;
			float lenSq = ab.LengthSq();
			float t = 0;
			if (lenSq > 0.01)
				t = Math.Clamp(vector.Dot(ap, ab) / lenSq, 0, 1);
			vector closest = a + ab * t;
			float dist = OS_Util.Distance2D(pos, closest[0], closest[2]);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = Cum[i] + (Cum[i + 1] - Cum[i]) * t;
			}
		}
		return best;
	}

	//! jak ostrá je zatáčka v úseku s .. s+ahead (úhel ve stupních)
	float TurnAngle(float s, float ahead)
	{
		vector d1 = DirAt(s + 2);
		vector d2 = DirAt(s + ahead);
		float dot = Math.Clamp(vector.Dot(d1, d2), -1, 1);
		return Math.Acos(dot) * Math.RAD2DEG;
	}
}

modded class CarScript
{
	OS_Convoy m_OS_Convoy;   // auto konvoje (nastaví OS_ConvoyDirector)

	//! auto konvoje řídí řidič OS (ne řízení Expansion AI)
	override void OnInput(float dt)
	{
		if (!m_OS_Convoy || !g_Game.IsServer())
		{
			super.OnInput(dt);
			return;
		}
		OS_ConvoyDirector.DriveInput(this, m_OS_Convoy, dt);
	}
}

class OS_ConvoyDirector
{
	static OS_ConvoyDirector s_Instance;

	ref OS_ConvoyConfig m_Cfg;
	protected ref array<ref OS_Convoy> m_Convoys = new array<ref OS_Convoy>;
	protected float m_NextConvoy;

	void OS_ConvoyDirector()
	{
		s_Instance = this;
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

	protected void RefreshMaps()
	{
		if (OS_Server.Get() && OS_Server.Get().m_PvP)
			OS_Server.Get().m_PvP.BroadcastZones();
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
		float total = 0;
		vector prev;
		for (int p = 0; p < route.Waypoints.Count(); p++)
		{
			vector wp = route.Waypoints[p];
			vector ground = OS_Util.GroundPos(wp[0], wp[2]);
			if (p > 0)
				total += vector.Distance(prev, ground);
			convoy.Points.Insert(ground);
			convoy.Cum.Insert(total);
			prev = ground;
		}
		convoy.Length = total;

		array<string> types = new array<string>;
		m_Cfg.Vehicles.Split("|", types);
		string type = types.GetRandomElement();
		type.TrimInPlace();
		vector start = convoy.Points[0];
		vector startDir = convoy.DirAt(0);
		vector startAngles = startDir.VectorToAngles();
		convoy.Vehicle = OS_Server.Get().m_Vehicles.AdminSpawn(type, start, startAngles[0]);
		if (!convoy.Vehicle)
		{
			Print(OS_TAG + "KONVOJ: nepodarilo se vytvorit vozidlo " + type);
			return false;
		}
		CarScript car = CarScript.Cast(convoy.Vehicle);
		if (car)
			car.m_OS_Convoy = convoy;
		dBodyDynamic(convoy.Vehicle, false);   // dokud se posádka neposadí, jede skriptem
		convoy.VehicleHealth = convoy.Vehicle.GetHealth("", "");
		convoy.SpawnedAt = g_Game.GetTickTime();
		convoy.LastPos = start;

		if (OS_Server.Get().m_Airdrops)
		{
			for (int i = 0; i < Math.Max(1, m_Cfg.LootRolls); i++)
			{
				OS_Server.Get().m_Airdrops.FillLootFrom(convoy.Vehicle, m_Cfg.LootTable);
			}
		}

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

	//! živý voják na místě řidiče
	static bool HasDriver(OS_Convoy convoy)
	{
		eAIBase driver = eAIBase.Cast(convoy.Vehicle.CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER));
		return driver && driver.IsAlive() && !driver.IsUnconscious();
	}

	protected void Stop(OS_Convoy convoy, string reason, vector attackPos = "0 0 0")
	{
		if (convoy.Stopped)
			return;
		convoy.Stopped = true;
		convoy.StoppedAt = g_Game.GetTickTime();
		dBodyDynamic(convoy.Vehicle, true);
		convoy.Physics = true;   // OnInput drží brzdy

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

	protected void Remove(OS_Convoy convoy, int index)
	{
		if (convoy.Patrol)
			convoy.Patrol.Despawn();
		if (convoy.Vehicle)
			g_Game.ObjectDelete(convoy.Vehicle);
		m_Convoys.Remove(index);
		RefreshMaps();
	}

	// ------------------------------------------------------------ skutečné řízení (OnInput auta, server)

	static void DriveInput(CarScript car, OS_Convoy convoy, float dt)
	{
		if (!s_Instance)
			return;
		OS_ConvoyConfig cfg = s_Instance.m_Cfg;

		// stojí / skriptová jízda / bez řidiče: brzdy
		if (convoy.Stopped || !convoy.Physics || !HasDriver(convoy))
		{
			car.SetThrottle(0);
			car.SetBrake(1);
			car.SetHandbrake(1);
			return;
		}

		if (!car.EngineIsOn())
		{
			car.EngineStart();
			car.SetBrake(1);
			return;
		}
		car.SetHandbrake(0);

		vector pos = car.GetPosition();
		vector forward = car.GetDirection();
		forward[1] = 0;
		forward.Normalize();
		float speed = car.GetSpeedometerAbsolute();   // km/h
		float now = g_Game.GetTickTime();

		// couvání po zaseknutí
		if (now < convoy.ReverseUntil)
		{
			ShiftForward(car, false);
			car.SetSteering(convoy.ReverseSteer);
			car.SetThrottle(0.5);
			car.SetBrake(0);
			return;
		}

		// bod kousek před autem na trase (čím rychleji, tím dál)
		float lookAhead = 7 + speed * 0.35;
		vector target = convoy.PointAt(convoy.S + lookAhead);
		vector toTarget = target - pos;
		toTarget[1] = 0;
		toTarget.Normalize();

		// řízení: úhel k cíli, znaménko podle strany
		float dot = Math.Clamp(vector.Dot(forward, toTarget), -1, 1);
		float angle = Math.Acos(dot) * Math.RAD2DEG;
		// kladné = cíl vpravo (osa y vektorového součinu)
		float side = forward[2] * toTarget[0] - forward[0] * toTarget[2];
		if (side < 0)
			angle = -angle;
		float steering = Math.Clamp(angle / 35.0, -1, 1);

		// rychlost: do zatáčky zpomalit, na konci trasy zastavit
		float turn = convoy.TurnAngle(convoy.S, 25 + speed * 0.5);
		float targetSpeed = cfg.SpeedKmh;
		if (turn > 15)
			targetSpeed = Math.Max(cfg.MinCornerSpeedKmh, cfg.SpeedKmh * (1 - (turn - 15) / 90.0));
		float remaining = convoy.Length - convoy.S;
		if (remaining < 40)
			targetSpeed = Math.Min(targetSpeed, Math.Max(5, remaining));

		// překážka přímo před autem (auto, člověk, zeď)
		if (ObstacleAhead(car, forward, Math.Max(8, speed * 0.6)))
			targetSpeed = 0;

		float throttle = 0;
		float brake = 0;
		if (speed < targetSpeed)
			throttle = Math.Clamp((targetSpeed - speed) / 12.0, 0.15, 0.8);
		else if (speed > targetSpeed + 4)
			brake = Math.Clamp((speed - targetSpeed) / 15.0, 0.2, 1);
		if (targetSpeed <= 0)
		{
			throttle = 0;
			brake = 1;
		}

		ShiftForward(car, true);
		car.SetSteering(steering);
		car.SetThrottle(throttle);
		car.SetBrake(brake);
	}

	//! zařadit dopředu / dozadu; manuál řadí podle otáček
	static void ShiftForward(CarScript car, bool forward)
	{
		int gear = car.GetGear();
		if (!forward)
		{
			if (gear != CarGear.REVERSE)
				car.ShiftTo(CarGear.REVERSE);
			return;
		}
		if (gear == CarGear.REVERSE || gear == CarGear.NEUTRAL)
		{
			car.ShiftTo(CarGear.FIRST);
			return;
		}
		if (car.GearboxGetType() == CarGearboxType.AUTOMATIC)
			return;

		float redline = car.EngineGetRPMRedline();
		if (redline <= 0)
			return;
		float rpm = car.EngineGetRPM();
		if (rpm > redline * 0.85 && gear < car.GetGearCount() - 1)
			car.ShiftUp();
		else if (rpm < redline * 0.35 && gear > CarGear.FIRST)
			car.ShiftDown();
	}

	static bool ObstacleAhead(CarScript car, vector forward, float distance)
	{
		vector from = car.GetPosition() + Vector(0, 1, 0) + forward * 2.5;
		vector to = from + forward * distance;
		vector contactPos;
		vector contactDir;
		int contactComponent;
		set<Object> results = new set<Object>;
		if (!DayZPhysics.RaycastRV(from, to, contactPos, contactDir, contactComponent, results, null, car, true, false, ObjIntersectGeom, 0.6))
			return false;
		foreach (Object obj : results)
		{
			if (!obj || obj.IsTree() || obj.IsBush())
				continue;
			EntityAI entity = EntityAI.Cast(obj);
			if (entity && entity.GetHierarchyRoot() == car)
				continue;
			if (obj.IsMan() || obj.IsTransport() || obj.IsBuilding())
				return true;
		}
		return false;
	}

	// ------------------------------------------------------------ jízda skriptem (každý snímek)

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

			if (!convoy.Seated)
			{
				SeatCrew(convoy);
				if (!convoy.Seated && g_Game.GetTickTime() - convoy.SpawnedAt < 30)
					continue;
				convoy.Seated = true;
			}

			if (convoy.S >= convoy.Length - 3)
			{
				Stop(convoy, OS_F("Konvoj dorazil do cile (%1).", convoy.RouteName));
				continue;
			}

			if (convoy.Physics)
			{
				// řídí voják - jen sledovat polohu na trase
				convoy.S = Math.Max(convoy.S, convoy.Project(convoy.Vehicle.GetPosition()));
				continue;
			}

			convoy.S += m_Cfg.SpeedKmh / 3.6 * timeslice;
			PlaceOnRoute(convoy, convoy.S);
		}
	}

	protected void PlaceOnRoute(OS_Convoy convoy, float s)
	{
		vector pos = convoy.PointAt(s);
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]) + 0.2;
		vector dir = convoy.DirAt(s);
		vector angles = dir.VectorToAngles();
		convoy.Vehicle.SetPosition(pos);
		convoy.Vehicle.SetOrientation(Vector(angles[0], 0, 0));
		convoy.Vehicle.PlaceOnSurface();
	}

	protected float NearestPlayer(vector pos, array<Man> players)
	{
		float best = 1000000;
		foreach (Man man : players)
		{
			if (!man || !man.IsAlive())
				continue;
			float dist = vector.Distance(pos, man.GetPosition());
			if (dist < best)
				best = dist;
		}
		return best;
	}

	// ------------------------------------------------------------ přepadení, útoky, přepínání řízení (1× za sekundu)

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
			if (!convoy || !convoy.Vehicle || convoy.Stopped || !convoy.Seated)
				continue;
			vector pos = convoy.Vehicle.GetPosition();
			float nearest = NearestPlayer(pos, players);

			// přepínání: voják řídí, když je hráč blízko
			bool wantPhysics = m_Cfg.RealDriving && nearest < m_Cfg.PhysicsDistance && HasDriver(convoy);
			if (wantPhysics && !convoy.Physics)
			{
				convoy.Physics = true;
				convoy.StuckTime = 0;
				convoy.LastPos = pos;
				dBodyDynamic(convoy.Vehicle, true);
			}
			else if (!wantPhysics && convoy.Physics && (nearest > m_Cfg.PhysicsDistance + 100 || !HasDriver(convoy)))
			{
				convoy.Physics = false;
				dBodyDynamic(convoy.Vehicle, false);
				convoy.S = convoy.Project(pos);
			}

			// zaseknutí / převrácení při skutečné jízdě
			if (convoy.Physics)
			{
				float moved = vector.Distance(pos, convoy.LastPos);
				convoy.LastPos = pos;
				if (moved < 1 && g_Game.GetTickTime() > convoy.ReverseUntil)
					convoy.StuckTime += delta;
				else if (moved >= 1)
					convoy.StuckTime = 0;

				vector up = convoy.Vehicle.GetTransformAxis(1);
				bool flipped = up[1] < 0.3;
				if (convoy.StuckTime >= m_Cfg.StuckSeconds && convoy.StuckTime < m_Cfg.RecoverSeconds && g_Game.GetTickTime() > convoy.ReverseUntil + 2)
				{
					convoy.ReverseUntil = g_Game.GetTickTime() + 2.5;
					convoy.ReverseSteer = Math.RandomFloatInclusive(-1, 1);
				}
				if ((convoy.StuckTime >= m_Cfg.RecoverSeconds || flipped) && nearest > m_Cfg.UnseenDistance)
				{
					// nikdo se nedívá - zpátky na trasu
					convoy.S = convoy.Project(pos) + 5;
					dBodyDynamic(convoy.Vehicle, false);
					PlaceOnRoute(convoy, convoy.S);
					dBodyDynamic(convoy.Vehicle, true);
					convoy.StuckTime = 0;
					convoy.LastPos = convoy.Vehicle.GetPosition();
				}
			}

			// přepadení: zásah auta nebo zabitý voják (i řidič)
			float health = convoy.Vehicle.GetHealth("", "");
			bool hit = health < convoy.VehicleHealth - 1;
			convoy.VehicleHealth = health;
			if (!hit && convoy.Patrol && convoy.Patrol.m_Group && Crew(convoy).Count() < convoy.Patrol.m_Group.Count())
				hit = true;
			if (!hit && convoy.Physics && !HasDriver(convoy))
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
				if (dist < m_Cfg.AmbushDistance)
				{
					Stop(convoy, OS_F("Konvoj byl prepaden (%1)!", convoy.RouteName));
					break;
				}
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
