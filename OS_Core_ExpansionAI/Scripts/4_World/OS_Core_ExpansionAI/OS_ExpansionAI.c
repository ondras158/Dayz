// OS_Core + Expansion AI: opravy výhledu, hlídky, útoky a stráže airdropů.
// Nastavení: $profile:OS_Core/ExpansionAI.json

class OS_AIPatrol
{
	string Name;
	string Faction = "East";          // West, East, Raiders, Mercenaries, Guards, Civilian, Passive
	string Loadout = "";              // soubor výbavy Expansion ("" = výchozí výbava frakce)
	int Count = 3;
	string Behaviour = "LOOP";        // LOOP (dokola), ALTERNATE (tam a zpět), HALT (stojí - stráž)
	string Speed = "WALK";            // WALK, JOG, SPRINT
	float RespawnMinutes = 30;        // -1 = po zničení se neobnoví
	ref array<vector> Waypoints = new array<vector>;   // body trasy [x, 0, z]

	void OS_AIPatrol(string name = "", string faction = "East", int count = 3, string behaviour = "LOOP")
	{
		Name = name;
		Faction = faction;
		Count = count;
		Behaviour = behaviour;
	}
}

class OS_ExpansionAIConfig
{
	bool Enabled = true;

	// výhled (opravy Expansion AI)
	bool StrictLineOfSight = true;     // nevidí a nestřílí přes zdi, stěny základny, skály a auta
	float DayVisionMax = 350;          // max. vzdálenost, na kterou AI uvidí hráče ve dne (m)
	float NightVisionMax = 60;         // v noci, když hráč nesvítí
	float NightLitVisionMax = 150;     // v noci, když hráč svítí (baterka, čelovka)
	float CrouchVisionFactor = 0.7;    // přikrčený hráč je vidět na 70 % vzdálenosti
	float ProneVisionFactor = 0.4;     // ležící hráč na 40 %
	float AccuracyFalloffDistance = 300; // na téhle vzdálenosti má AI poloviční přesnost

	// hlídky (spawnují se, když je hráč poblíž, a zmizí, když odejde)
	float PatrolSpawnDistance = 600;   // hlídka se objeví, když je hráč blíž než tohle
	ref array<ref OS_AIPatrol> Patrols = new array<ref OS_AIPatrol>;

	// útoky na hráče (hlavně v jejich base)
	bool Attacks = true;
	float AttackIntervalMinutes = 60;  // jak často zkusit útok (náhodně ±30 %)
	float AttackChance = 0.5;          // šance, že se útok opravdu stane
	int AttackMinPlayers = 2;          // minimální počet hráčů online
	bool AttackOnlyBases = true;       // útočit jen na hráče, kteří jsou ve své base
	int AttackCountMin = 3;
	int AttackCountMax = 5;
	float AttackSpawnDistance = 300;   // útočníci se objeví v téhle vzdálenosti a jdou k hráči
	string AttackFaction = "Raiders";
	string AttackLoadout = "";

	// stráže airdropů vojáky Expansion AI (místo nakažených)
	bool AirdropGuards = true;
	string AirdropGuardFaction = "Guards";
	string AirdropGuardLoadout = "";

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď trasami na své mapě !!!
		OS_AIPatrol patrol = new OS_AIPatrol("Priklad - Hlidka letiste", "East", 3, "LOOP");
		patrol.Waypoints.Insert(Vector(4400, 0, 9400));
		patrol.Waypoints.Insert(Vector(4700, 0, 9450));
		patrol.Waypoints.Insert(Vector(4650, 0, 9700));
		patrol.Waypoints.Insert(Vector(4350, 0, 9650));
		Patrols.Insert(patrol);

		OS_AIPatrol guards = new OS_AIPatrol("Priklad - Straz zakladny", "Guards", 4, "HALT");
		guards.Waypoints.Insert(Vector(7000, 0, 3000));
		guards.RespawnMinutes = 45;
		Patrols.Insert(guards);
	}
}

class OS_ExpansionAI
{
	static ref OS_ExpansionAIConfig s_Cfg;

	static OS_ExpansionAIConfig Cfg()
	{
		if (!s_Cfg)
		{
			OS_ExpansionAIConfig defaults = new OS_ExpansionAIConfig();
			defaults.FillDefaults();
			s_Cfg = OS_ConfigFile<OS_ExpansionAIConfig>.Load("ExpansionAI.json", defaults, new OS_ExpansionAIConfig());
			if (!s_Cfg.Patrols)
				s_Cfg.Patrols = new array<ref OS_AIPatrol>;
		}
		return s_Cfg;
	}

	//! skupina vojáků jako dynamická hlídka Expansion
	static eAIDynamicPatrol Spawn(string name, string faction, string loadout, int count, string behaviour, string speed, array<vector> waypoints, float respawnSeconds, float despawnSeconds, float spawnDistance)
	{
		if (!waypoints || waypoints.Count() == 0)
			return null;

		ExpansionAIPatrol config = new ExpansionAIPatrol(count, speed, "SPRINT", behaviour, faction, loadout, true, 1, 1.0, -2, spawnDistance, respawnSeconds, 0, null);
		config.Name = name;
		config.Persist = false;
		config.Waypoints = new TVectorArray;
		foreach (vector wp : waypoints)
		{
			config.Waypoints.Insert(OS_Util.GroundPos(wp[0], wp[2]));
		}
		config.UseRandomWaypointAsStartPoint = false;
		config.DespawnTime = despawnSeconds;
		config.DespawnRadius = spawnDistance * 1.2;
		return eAIDynamicPatrol.CreateEx(config, config.Waypoints[0], true);
	}

	// ------------------------------------------------------------ výhled

	//! true = mezi očima AI a hráčem je pevná překážka (zeď, budova, skála, auto...)
	static bool IsBlocked(eAIBase ai, PlayerBase target)
	{
		vector eye = ai.GetBonePositionWS(ai.GetBoneIndexByName("Head"));
		array<string> bones = {"Head", "Spine3", "Pelvis"};
		foreach (string bone : bones)
		{
			int index = target.GetBoneIndexByName(bone);
			vector point;
			if (index >= 0)
				point = target.GetBonePositionWS(index);
			else
				point = target.GetPosition() + Vector(0, 1, 0);

			if (!PointBlocked(ai, eye, point, target))
				return false;   // aspoň část těla je vidět
		}
		return true;
	}

	protected static bool PointBlocked(eAIBase ai, vector from, vector to, PlayerBase target)
	{
		vector dir = vector.Direction(from, to);
		float length = dir.Length();
		if (length < 0.1)
			return false;
		dir.Normalize();
		vector end = from + dir * (length + 0.3);

		vector contactPos;
		vector contactDir;
		int contactComponent;
		set<Object> results = new set<Object>;
		if (!DayZPhysics.RaycastRV(from, end, contactPos, contactDir, contactComponent, results, null, ai, true, false, ObjIntersectView, 0))
			return false;   // nic v cestě

		if (results.Count() == 0)
			return false;

		Object hit = results[0];
		EntityAI hitEntity = EntityAI.Cast(hit);
		if (hitEntity)
		{
			EntityAI root = hitEntity.GetHierarchyRoot();
			if (root == target || root == target.GetParent())
				return false;
			// jiná postava (AI, hráč, nakažený) nebo zvíře v cestě - vidět skrz ni není, ale není to zeď
			if (root.IsMan() || root.IsDayZCreature())
				return true;
		}

		// stromy a keře nechává na Expansion (průhledné listí)
		if (hit.IsTree() || hit.IsBush())
			return false;

		// zásah těsně u cíle = cíl
		if (vector.Distance(contactPos, to) < 0.4)
			return false;

		return true;   // zeď, budova, stěna základny, skála, auto
	}
}

modded class eAIBase
{
	protected float m_OS_LOSCheckTime;
	protected bool m_OS_LOSBlocked;
	protected EntityAI m_OS_LOSTarget;

	//! omezení vzdálenosti, na kterou AI vidí hráče
	protected float OS_VisionCap(EntityAI targetEntity, bool lit)
	{
		OS_ExpansionAIConfig cfg = OS_ExpansionAI.Cfg();
		float cap = cfg.DayVisionMax;
		if (m_Expansion_Visibility <= m_eAI_NightVisibility)
		{
			if (lit)
				cap = cfg.NightLitVisionMax;
			else
				cap = cfg.NightVisionMax;
		}

		PlayerBase player = PlayerBase.Cast(targetEntity);
		if (player)
		{
			if (player.IsPlayerInStance(DayZPlayerConstants.STANCEMASK_PRONE | DayZPlayerConstants.STANCEMASK_RAISEDPRONE))
				cap *= cfg.ProneVisionFactor;
			else if (player.IsPlayerInStance(DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_RAISEDCROUCH))
				cap *= cfg.CrouchVisionFactor;
		}
		return cap;
	}

	override float eAI_GetVisibilityDistance(float distance, eAITarget target)
	{
		float result = super.eAI_GetVisibilityDistance(distance, target);
		if (!OS_ExpansionAI.Cfg().Enabled || !target)
			return result;
		PlayerBase player = PlayerBase.Cast(target.GetEntity());
		if (!player)
			return result;
		return Math.Min(result, OS_VisionCap(player, target.IsLit()));
	}

	override float eAI_GetVisibilityDistanceLimit(PlayerBase player)
	{
		float result = super.eAI_GetVisibilityDistanceLimit(player);
		if (!OS_ExpansionAI.Cfg().Enabled || !player)
			return result;
		return Math.Min(result, OS_VisionCap(player, player.eAI_IsLit()));
	}

	//! přesnost klesá se vzdáleností (visibility násobí přesnost míření)
	override float eAI_GetVisibility(float distance, eAITarget target)
	{
		float result = super.eAI_GetVisibility(distance, target);
		OS_ExpansionAIConfig cfg = OS_ExpansionAI.Cfg();
		if (!cfg.Enabled || !target || !PlayerBase.Cast(target.GetEntity()) || cfg.AccuracyFalloffDistance <= 0)
			return result;
		float factor = Math.Clamp(1 - 0.5 * distance / cfg.AccuracyFalloffDistance, 0.25, 1);
		return result * factor;
	}

	//! přísná kontrola výhledu: přes zeď nevidí a nestřílí
	override bool EnforceLOS(float pDt)
	{
		bool los = super.EnforceLOS(pDt);
		if (!los || !OS_ExpansionAI.Cfg().Enabled || !OS_ExpansionAI.Cfg().StrictLineOfSight)
			return los;

		eAITarget target = GetTarget();
		if (!target)
			return los;
		PlayerBase player = PlayerBase.Cast(target.GetEntity());
		if (!player)
			return los;

		// paprsky jen párkrát za sekundu, jinak výsledek z paměti
		float now = g_Game.GetTickTime();
		if (player != m_OS_LOSTarget || now - m_OS_LOSCheckTime > 0.3)
		{
			m_OS_LOSTarget = player;
			m_OS_LOSCheckTime = now;
			m_OS_LOSBlocked = OS_ExpansionAI.IsBlocked(this, player);
		}

		if (m_OS_LOSBlocked)
		{
			eAITargetInformationState state = target;
			state.m_LOS = false;
			return false;
		}
		return true;
	}
}

// ---------------------------------------------------------------- hlídky a útoky

class OS_AIDirector
{
	protected ref array<eAIDynamicPatrol> m_Patrols = new array<eAIDynamicPatrol>;
	protected bool m_Started;
	protected float m_NextAttack;
	protected float m_Timer;

	void Start()
	{
		if (m_Started)
			return;
		m_Started = true;
		OS_ExpansionAIConfig cfg = OS_ExpansionAI.Cfg();
		if (!cfg.Enabled)
			return;

		foreach (OS_AIPatrol patrol : cfg.Patrols)
		{
			if (!patrol || patrol.Waypoints.Count() == 0)
				continue;
			float respawn = -1;
			if (patrol.RespawnMinutes >= 0)
				respawn = patrol.RespawnMinutes * 60;
			eAIDynamicPatrol spawned = OS_ExpansionAI.Spawn(OS_T(patrol.Name), patrol.Faction, patrol.Loadout, patrol.Count, patrol.Behaviour, patrol.Speed, patrol.Waypoints, respawn, 120, cfg.PatrolSpawnDistance);
			if (spawned)
				m_Patrols.Insert(spawned);
		}
		int patrolCount = m_Patrols.Count();
		Print(OS_TAG + "Expansion AI: hlidek " + patrolCount.ToString());
		ScheduleAttack();
	}

	protected void ScheduleAttack()
	{
		float minutes = Math.Max(5, OS_ExpansionAI.Cfg().AttackIntervalMinutes) * Math.RandomFloatInclusive(0.7, 1.3);
		m_NextAttack = g_Game.GetTickTime() + minutes * 60;
	}

	void OnTick(array<Man> players, float delta)
	{
		OS_ExpansionAIConfig cfg = OS_ExpansionAI.Cfg();
		if (!cfg.Enabled || !cfg.Attacks)
			return;
		if (g_Game.GetTickTime() < m_NextAttack)
			return;
		ScheduleAttack();
		if (players.Count() < cfg.AttackMinPlayers || Math.RandomFloat01() > cfg.AttackChance)
			return;

		// cíl: náhodný hráč (ve své base), ne v bezpečné zóně
		array<PlayerBase> candidates = new array<PlayerBase>;
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || player.m_OS_InSafeZone)
				continue;
			if (cfg.AttackOnlyBases && (!OS_Server.Get() || !OS_Server.Get().m_Bases || !OS_Server.Get().m_Bases.IsInOwnBase(player)))
				continue;
			candidates.Insert(player);
		}
		if (candidates.Count() == 0)
			return;
		Attack(candidates.GetRandomElement());
	}

	void Attack(PlayerBase target)
	{
		OS_ExpansionAIConfig cfg = OS_ExpansionAI.Cfg();
		vector targetPos = target.GetPosition();
		float angle = Math.RandomFloat(0, Math.PI2);
		vector start = Vector(targetPos[0] + Math.Cos(angle) * cfg.AttackSpawnDistance, 0, targetPos[2] + Math.Sin(angle) * cfg.AttackSpawnDistance);

		array<vector> waypoints = new array<vector>;
		waypoints.Insert(start);
		waypoints.Insert(targetPos);
		int count = Math.RandomIntInclusive(cfg.AttackCountMin, Math.Max(cfg.AttackCountMin, cfg.AttackCountMax));
		OS_ExpansionAI.Spawn(OS_T("Utok"), cfg.AttackFaction, cfg.AttackLoadout, count, "ONCE", "JOG", waypoints, -1, 300, cfg.AttackSpawnDistance + 200);

		OS_Util.Notify(target, OS_T("Pozor"), OS_T("Blizi se ozbrojeni banditi!"), 8);
		if (OS_Server.Get() && OS_Server.Get().m_Groups)
		{
			OS_Group group = OS_Server.Get().m_Groups.GetGroupOf(OS_Util.SteamId(target));
			if (group)
			{
				foreach (OS_GroupMember member : group.Members)
				{
					PlayerBase mate = OS_Util.FindPlayerBySteamId(member.Id);
					if (mate && mate != target)
						OS_Util.Notify(mate, OS_T("Pozor"), OS_F("Na %1 utoci banditi!", OS_Util.PlayerName(target)), 8);
				}
			}
		}
		Print(OS_TAG + "Expansion AI: utok " + count.ToString() + " vojaku na " + OS_Util.PlayerName(target));
	}
}

modded class OS_Server
{
	protected ref OS_AIDirector m_AIDirector;
	protected float m_AITimer;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		if (!m_AIDirector)
		{
			m_AIDirector = new OS_AIDirector();
			m_AIDirector.Start();
		}
		m_AITimer += timeslice;
		if (m_AITimer < 5)
			return;
		float delta = m_AITimer;
		m_AITimer = 0;
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		m_AIDirector.OnTick(players, delta);
	}

	OS_AIDirector GetAIDirector()
	{
		return m_AIDirector;
	}
}

//! stráže airdropů jako vojáci Expansion AI
modded class OS_ServerAirdrops
{
	override void SpawnGuards(vector pos, int count)
	{
		OS_ExpansionAIConfig cfg = OS_ExpansionAI.Cfg();
		if (!cfg.Enabled || !cfg.AirdropGuards)
		{
			super.SpawnGuards(pos, count);
			return;
		}
		array<vector> waypoints = new array<vector>;
		waypoints.Insert(Vector(pos[0] + 6, 0, pos[2] + 6));
		eAIDynamicPatrol patrol = OS_ExpansionAI.Spawn(OS_T("Straz airdropu"), cfg.AirdropGuardFaction, cfg.AirdropGuardLoadout, count, "HALT", "WALK", waypoints, -1, m_Cfg.LifetimeMinutes * 60, 800);
		if (!patrol)
			super.SpawnGuards(pos, count);
	}
}
