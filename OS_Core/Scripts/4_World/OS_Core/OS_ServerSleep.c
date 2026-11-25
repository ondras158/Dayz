// Únava a spánek na serveru (+ oznámení emocí v okolí).

class OS_SleepData
{
	ref map<string, float> Fatigue = new map<string, float>;
}

class OS_ServerSleep
{
	protected static const string CFG_FILE = "Sleep.json";
	protected static const string DATA_FILE = "SleepData.json";

	ref OS_SleepConfig m_Cfg;
	protected ref map<string, float> m_Fatigue = new map<string, float>;      // Steam ID -> 0..100
	protected ref map<string, vector> m_SleepPos = new map<string, vector>;   // spící hráči -> místo usnutí
	protected ref map<string, float> m_SleepHealth = new map<string, float>;
	protected ref map<string, int> m_Warned = new map<string, int>;           // 0 nic, 1 unavený, 2 vyčerpaný
	protected ref map<string, vector> m_LastPos = new map<string, vector>;
	protected float m_SaveTimer;

	void OS_ServerSleep()
	{
		m_Cfg = OS_ConfigFile<OS_SleepConfig>.Load(CFG_FILE, new OS_SleepConfig(), new OS_SleepConfig());
		LoadData();
	}

	protected void LoadData()
	{
		string path = OS_DIR + DATA_FILE;
		string err;
		OS_SleepData data = new OS_SleepData();
		if (FileExist(path) && !JsonFileLoader<OS_SleepData>.LoadFile(path, data, err))
		{
			Print(OS_TAG + "CHYBA v " + path + ": " + err);
			data = new OS_SleepData();
		}
		m_Fatigue = data.Fatigue;
		if (!m_Fatigue)
			m_Fatigue = new map<string, float>;
	}

	protected void SaveData()
	{
		string err;
		OS_SleepData data = new OS_SleepData();
		data.Fatigue = m_Fatigue;
		JsonFileLoader<OS_SleepData>.SaveFile(OS_DIR + DATA_FILE, data, err);
	}

	float Fatigue(string steamId)
	{
		float value;
		if (m_Fatigue.Find(steamId, value))
			return value;
		return 0;
	}

	bool IsSleeping(string steamId)
	{
		return m_SleepPos.Contains(steamId);
	}

	//! nová postava (po smrti) je odpočatá
	void OnNewCharacter(PlayerBase player)
	{
		string id = OS_Util.SteamId(player);
		m_Fatigue.Set(id, 0);
		m_SleepPos.Remove(id);
		m_Warned.Remove(id);
	}

	void OnDisconnect(string steamId)
	{
		m_SleepPos.Remove(steamId);
		m_LastPos.Remove(steamId);
		SaveData();
	}

	protected bool InShelter(PlayerBase player)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(player.GetPosition(), 4, objects, null);
		foreach (Object obj : objects)
		{
			if (TentBase.Cast(obj))
				return true;
		}
		if (OS_Server.Get() && OS_Server.Get().m_Bases && OS_Server.Get().m_Bases.IsInOwnBase(player))
			return true;
		return false;
	}

	// ------------------------------------------------------------ RPC

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_SLEEP && rpcType != OS_RPC_EMOTE_ANNOUNCE)
			return false;
		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		if (rpcType == OS_RPC_EMOTE_ANNOUNCE)
		{
			string emoteText;
			if (ctx.Read(emoteText))
				Announce(player, emoteText);
			return true;
		}

		bool start;
		if (!ctx.Read(start))
			return true;
		if (start)
			StartSleep(player);
		else
			StopSleep(player, false);
		return true;
	}

	protected void StartSleep(PlayerBase player)
	{
		string id = OS_Util.SteamId(player);
		if (!m_Cfg.Enabled || IsSleeping(id))
			return;
		if (Fatigue(id) < m_Cfg.MinFatigueToSleep)
		{
			OS_Util.Notify(player, OS_T("Spanek"), OS_T("Nejsi unaveny, nemuzes usnout."));
			SendState(player, false);
			return;
		}
		if (player.GetCommand_Vehicle())
		{
			SendState(player, false);
			return;
		}
		if (m_Cfg.NoSleepInPvP && OS_Server.Get() && OS_Server.Get().m_PvP && OS_Server.Get().m_PvP.m_Cfg.Enabled && OS_Server.Get().m_PvP.m_Cfg.PvPOnlyInZones && OS_Server.Get().m_PvP.IsPvPActive(player))
		{
			OS_Util.Notify(player, OS_T("Spanek"), OS_T("V PvP zone se neda usnout."));
			SendState(player, false);
			return;
		}
		m_SleepPos.Set(id, player.GetPosition());
		m_SleepHealth.Set(id, player.GetHealth("", "Health"));
		if (InShelter(player))
			OS_Util.Notify(player, OS_T("Spanek"), OS_T("Usinas v ukrytu - vyspis se rychleji. Pohni se pro probuzeni."), 5);
		else
			OS_Util.Notify(player, OS_T("Spanek"), OS_T("Usinas. Pohni se pro probuzeni."), 5);
		SendState(player, true);
	}

	protected void StopSleep(PlayerBase player, bool rested)
	{
		string id = OS_Util.SteamId(player);
		if (!IsSleeping(id))
			return;
		m_SleepPos.Remove(id);
		if (rested)
			OS_Util.Notify(player, OS_T("Spanek"), OS_T("Jsi vyspany."), 4);
		SendState(player, false);
	}

	//! klient: ztmavit obrazovku / probudit
	protected void SendState(PlayerBase player, bool sleeping)
	{
		if (!player.GetIdentity())
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(sleeping);
		rpc.Send(null, OS_RPC_SLEEP, true, player.GetIdentity());
	}

	protected void Announce(PlayerBase player, string emoteText)
	{
		if (m_Cfg.EmoteAnnounceRadius <= 0 || emoteText == "" || emoteText.Length() > 60)
			return;
		string text = "* " + OS_Util.PlayerName(player) + " " + emoteText;
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase other = PlayerBase.Cast(man);
			if (other && vector.Distance(other.GetPosition(), player.GetPosition()) <= m_Cfg.EmoteAnnounceRadius)
				OS_Util.Chat(other, text);
		}
	}

	// ------------------------------------------------------------ únava

	void OnTick(array<Man> players, float delta)
	{
		if (!m_Cfg.Enabled)
			return;

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || !player.GetIdentity())
				continue;
			string id = OS_Util.SteamId(player);
			float fatigue = Fatigue(id);

			if (IsSleeping(id))
			{
				// probuzení: pohyb nebo zranění
				vector startPos = m_SleepPos.Get(id);
				float lastHealth = m_SleepHealth.Get(id);
				float health = player.GetHealth("", "Health");
				if (vector.Distance(startPos, player.GetPosition()) > 1.5 || health < lastHealth - 0.5)
				{
					StopSleep(player, false);
					continue;
				}
				float recovery = m_Cfg.SleepRecoveryPerMinute / 60 * delta;
				if (InShelter(player))
					recovery *= m_Cfg.ShelterMultiplier;
				fatigue = Math.Max(0, fatigue - recovery);
				player.AddHealth("", "", m_Cfg.SleepHealthPerSecond * delta);
				m_SleepHealth.Set(id, player.GetHealth("", "Health"));
				m_Fatigue.Set(id, fatigue);
				if (fatigue <= 0)
					StopSleep(player, true);
				m_Warned.Set(id, 0);
				continue;
			}

			// bdění: únava roste, při běhu rychleji
			vector pos = player.GetPosition();
			float speed = 0;
			vector lastPos;
			if (m_LastPos.Find(id, lastPos) && delta > 0)
				speed = OS_Util.Distance2D(pos, lastPos[0], lastPos[2]) / delta;
			m_LastPos.Set(id, pos);
			if (speed > 15)
				speed = 0;   // teleport, vozidlo
			float mult = 1;
			if (speed > 5)
				mult = m_Cfg.SprintMultiplier;
			else if (speed > 2.5)
				mult = m_Cfg.JogMultiplier;
			fatigue = Math.Min(100, fatigue + m_Cfg.FatiguePerMinute / 60 * delta * mult);
			m_Fatigue.Set(id, fatigue);

			ApplyEffects(player, id, fatigue, delta);
		}

		m_SaveTimer += delta;
		if (m_SaveTimer >= 300)
		{
			m_SaveTimer = 0;
			SaveData();
		}
	}

	protected void ApplyEffects(PlayerBase player, string id, float fatigue, float delta)
	{
		int level = 0;
		if (fatigue >= m_Cfg.ExhaustedAt)
			level = 2;
		else if (fatigue >= m_Cfg.TiredAt)
			level = 1;

		int warned = 0;
		m_Warned.Find(id, warned);
		if (level > warned)
		{
			if (level == 2)
				OS_Util.Notify(player, OS_T("Unava"), OS_T("Jsi vycerpany! Ztracis zdravi - vyspi se (klavesa L -> Spat)."), 8);
			else
				OS_Util.Notify(player, OS_T("Unava"), OS_T("Jsi unaveny - mene vydrze. Vyspi se (klavesa L -> Spat)."), 8);
		}
		m_Warned.Set(id, level);

		if (level == 0)
			return;

		StaminaHandler stamina = player.GetStaminaHandler();
		if (stamina)
		{
			float capFraction = 0.6;
			if (level == 2)
				capFraction = 0.3;
			float limit = stamina.GetStaminaCap() * capFraction;
			if (stamina.GetStamina() > limit)
				stamina.SetStamina(limit);
		}
		if (level == 2)
			player.AddHealth("", "", -m_Cfg.ExhaustedHealthLossPerSecond * delta);
	}
}
