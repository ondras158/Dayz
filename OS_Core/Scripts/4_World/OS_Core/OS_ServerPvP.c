// PvP zóny na serveru: kdo komu smí ublížit, hlášky při vstupu/odchodu, úpravy z admin menu.

class OS_ServerPvP
{
	protected static const string CFG_FILE = "PvP.json";

	ref OS_PvPConfig m_Cfg;
	protected OS_ServerSafeZones m_SafeZones;

	void OS_ServerPvP(OS_ServerSafeZones safeZones)
	{
		m_SafeZones = safeZones;
		OS_PvPConfig defaults = new OS_PvPConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_PvPConfig>.Load(CFG_FILE, defaults, new OS_PvPConfig());
		if (!m_Cfg.Zones)
			m_Cfg.Zones = new array<ref OS_PvPZone>;
	}

	OS_PvPZone FindZone(vector pos)
	{
		foreach (OS_PvPZone zone : m_Cfg.Zones)
		{
			if (zone && OS_Util.Distance2D(pos, zone.X, zone.Z) <= zone.Radius)
				return zone;
		}
		return null;
	}

	//! smí útočník zranit oběť? (oba jsou hráči)
	bool AllowsPvP(PlayerBase victim, PlayerBase attacker)
	{
		if (!m_Cfg.Enabled || !m_Cfg.PvPOnlyInZones || !victim || !attacker || victim == attacker)
			return true;

		if (!FindZone(victim.GetPosition()))
			return false;
		if (m_Cfg.BothPlayersInZone && !FindZone(attacker.GetPosition()))
			return false;
		return true;
	}

	//! zranění od jiného hráče mimo PvP zónu se neprovede
	bool BlocksDamage(PlayerBase victim, EntityAI source)
	{
		if (!m_Cfg.Enabled || !m_Cfg.PvPOnlyInZones || !victim || !source)
			return false;

		PlayerBase attacker = OS_Util.ResolveKillerPlayer(source);
		if (!attacker || attacker == victim)
			return false;

		return !AllowsPvP(victim, attacker);
	}

	void OnTick(array<Man> players)
	{
		if (!m_Cfg.Enabled || !m_Cfg.NotifyOnEnter)
			return;

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive())
				continue;

			OS_PvPZone zone = FindZone(player.GetPosition());
			bool inside = false;
			if (zone)
				inside = true;

			if (inside == player.m_OS_InPvPZone)
				continue;
			player.m_OS_InPvPZone = inside;

			// když PvP platí všude, hláška o bezpečí by lhala
			if (!m_Cfg.PvPOnlyInZones)
				continue;

			if (inside)
				OS_Util.Notify(player, OS_T(zone.Name), OS_T(m_Cfg.EnterMessage), 8);
			else
				OS_Util.Notify(player, OS_T("PvP zona"), OS_T(m_Cfg.LeaveMessage), 6);
		}
	}

	// ------------------------------------------------------------ admin

	//! arg: "poloměr [název]", např. "400 Letiste"
	void AdminAdd(PlayerBase admin, string arg)
	{
		arg.TrimInPlace();
		float radius = 300;
		string name = "";
		if (arg != "")
		{
			int space = arg.IndexOf(" ");
			if (space > 0)
			{
				radius = arg.Substring(0, space).ToFloat();
				name = arg.Substring(space + 1, arg.Length() - space - 1);
				name.TrimInPlace();
			}
			else
			{
				radius = arg.ToFloat();
			}
		}
		if (radius < 20 || radius > 10000)
		{
			OS_Util.Notify(admin, OS_T("PvP zona"), OS_T("Polomer musi byt 20 az 10000 m. Do pole napis: polomer [nazev], napr. 400 Letiste"));
			return;
		}
		if (name == "")
		{
			int number = m_Cfg.Zones.Count() + 1;
			name = OS_F("PvP zona %1", number.ToString());
		}

		vector pos = admin.GetPosition();
		m_Cfg.Zones.Insert(new OS_PvPZone(name, Math.Round(pos[0]), Math.Round(pos[2]), radius));
		Save();
		OS_Util.Notify(admin, OS_T("PvP zona"), OS_F("Pridana PvP zona %1 (polomer %2 m).", name, radius.ToString()));
		Print(OS_TAG + "ADMIN " + OS_Util.PlayerName(admin) + " pridal PvP zonu " + name + " na " + pos.ToString() + " r=" + radius.ToString());
		BroadcastZones();
	}

	void AdminRemoveNearest(PlayerBase admin)
	{
		int best = -1;
		float bestDist = 1000000;
		vector pos = admin.GetPosition();
		for (int i = 0; i < m_Cfg.Zones.Count(); i++)
		{
			OS_PvPZone zone = m_Cfg.Zones[i];
			if (!zone)
				continue;
			float dist = OS_Util.Distance2D(pos, zone.X, zone.Z);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = i;
			}
		}
		if (best < 0)
		{
			OS_Util.Notify(admin, OS_T("PvP zona"), OS_T("Zadna PvP zona neexistuje."));
			return;
		}

		string removedName = m_Cfg.Zones[best].Name;
		m_Cfg.Zones.Remove(best);
		Save();
		int meters = Math.Round(bestDist);
		OS_Util.Notify(admin, OS_T("PvP zona"), OS_F("Smazana PvP zona %1 (%2 m od tebe).", OS_T(removedName), meters.ToString()));
		Print(OS_TAG + "ADMIN " + OS_Util.PlayerName(admin) + " smazal PvP zonu " + removedName);
		BroadcastZones();
	}

	protected void Save()
	{
		string err;
		if (!JsonFileLoader<OS_PvPConfig>.SaveFile(OS_DIR + CFG_FILE, m_Cfg, err))
			Print(OS_TAG + "CHYBA při ukládání " + CFG_FILE + ": " + err);
	}

	// ------------------------------------------------------------ klienti (mapa)

	void SendZones(PlayerIdentity identity)
	{
		if (!identity)
			return;

		array<string> pvpNames = new array<string>;
		array<float> pvpX = new array<float>;
		array<float> pvpZ = new array<float>;
		array<float> pvpR = new array<float>;
		if (m_Cfg.Enabled && m_Cfg.ShowPvPZonesOnMap)
		{
			foreach (OS_PvPZone zone : m_Cfg.Zones)
			{
				if (!zone)
					continue;
				pvpNames.Insert(OS_T(zone.Name));
				pvpX.Insert(zone.X);
				pvpZ.Insert(zone.Z);
				pvpR.Insert(zone.Radius);
			}
		}

		array<string> safeNames = new array<string>;
		array<float> safeX = new array<float>;
		array<float> safeZ = new array<float>;
		array<float> safeR = new array<float>;
		if (m_Cfg.ShowSafeZonesOnMap && m_SafeZones && m_SafeZones.m_Cfg.Enabled && m_SafeZones.m_Cfg.Zones)
		{
			foreach (OS_SafeZone safe : m_SafeZones.m_Cfg.Zones)
			{
				if (!safe)
					continue;
				safeNames.Insert(OS_T(safe.Name));
				safeX.Insert(safe.X);
				safeZ.Insert(safe.Z);
				safeR.Insert(safe.Radius);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Cfg.MapWithoutItem);
		rpc.Write(m_Cfg.Enabled && m_Cfg.PvPOnlyInZones);
		rpc.Write(OS_PvPConfig.ToARGB(m_Cfg.PvPColor, 220));
		rpc.Write(OS_PvPConfig.ToARGB(m_Cfg.SafeColor, 220));
		rpc.Write(pvpNames);
		rpc.Write(pvpX);
		rpc.Write(pvpZ);
		rpc.Write(pvpR);
		rpc.Write(safeNames);
		rpc.Write(safeX);
		rpc.Write(safeZ);
		rpc.Write(safeR);
		rpc.Send(null, OS_RPC_ZONES, true, identity);
	}

	void BroadcastZones()
	{
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			if (man && man.GetIdentity())
				SendZones(man.GetIdentity());
		}
	}
}
