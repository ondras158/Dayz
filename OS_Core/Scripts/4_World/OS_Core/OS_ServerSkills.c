// Dovednosti na serveru: zkušenosti, úrovně, body a jejich účinky.

class OS_ServerSkills
{
	protected static const string CFG_FILE = "Skills.json";
	protected static const string DATA_DIR = "Skills/";

	ref OS_SkillsConfig m_Cfg;
	protected OS_Payments m_Payments;
	protected string m_Currency;
	protected ref map<string, ref OS_PlayerSkills> m_Cache = new map<string, ref OS_PlayerSkills>;
	protected ref map<string, float> m_LastEnergy = new map<string, float>;
	protected ref map<string, float> m_LastWater = new map<string, float>;
	protected ref map<string, float> m_StaminaSet = new map<string, float>;
	protected float m_MinuteTimer;
	protected float m_StaminaTimer;

	void OS_ServerSkills(OS_Payments payments, string currency)
	{
		m_Payments = payments;
		m_Currency = currency;
		OS_SkillsConfig defaults = new OS_SkillsConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_SkillsConfig>.Load(CFG_FILE, defaults, new OS_SkillsConfig());
		if (!m_Cfg.Skills)
			m_Cfg.Skills = new array<ref OS_SkillDef>;
		MakeDirectory(OS_DIR + DATA_DIR);
	}

	// ------------------------------------------------------------ data

	OS_PlayerSkills Get(string steamId)
	{
		OS_PlayerSkills data;
		if (m_Cache.Find(steamId, data))
			return data;

		data = new OS_PlayerSkills();
		string path = OS_DIR + DATA_DIR + steamId + ".json";
		string err;
		if (FileExist(path) && !JsonFileLoader<OS_PlayerSkills>.LoadFile(path, data, err))
		{
			Print(OS_TAG + "CHYBA v " + path + ": " + err);
			data = new OS_PlayerSkills();
		}
		if (!data.Ranks)
			data.Ranks = new map<string, int>;
		m_Cache.Set(steamId, data);
		return data;
	}

	protected void Save(string steamId)
	{
		OS_PlayerSkills data;
		if (!m_Cache.Find(steamId, data))
			return;
		string err;
		if (!JsonFileLoader<OS_PlayerSkills>.SaveFile(OS_DIR + DATA_DIR + steamId + ".json", data, err))
			Print(OS_TAG + "CHYBA při ukládání dovedností " + steamId + ": " + err);
	}

	void Forget(string steamId)
	{
		Save(steamId);
		m_Cache.Remove(steamId);
		m_LastEnergy.Remove(steamId);
		m_LastWater.Remove(steamId);
		m_StaminaSet.Remove(steamId);
	}

	float Effect(PlayerBase player, string skillId)
	{
		OS_SkillDef def = m_Cfg.Find(skillId);
		if (!def || !player)
			return 0;
		return Get(OS_Util.SteamId(player)).Rank(skillId) * def.PerRank;
	}

	// ------------------------------------------------------------ zkušenosti

	void AddXp(PlayerBase player, int amount, bool notify = false)
	{
		if (!m_Cfg.Enabled || !player || amount <= 0)
			return;
		string id = OS_Util.SteamId(player);
		if (id == "")
			return;

		OS_PlayerSkills data = Get(id);
		data.TotalXp = data.TotalXp + amount;
		if (data.Level >= m_Cfg.MaxLevel)
		{
			Save(id);
			return;
		}

		data.Xp = data.Xp + amount;
		bool leveled = false;
		while (data.Level < m_Cfg.MaxLevel && data.Xp >= m_Cfg.XpForNextLevel(data.Level))
		{
			data.Xp = data.Xp - m_Cfg.XpForNextLevel(data.Level);
			data.Level = data.Level + 1;
			data.Points = data.Points + m_Cfg.PointsPerLevel;
			leveled = true;
		}
		Save(id);

		if (leveled)
			OS_Util.Notify(player, OS_T("Dovednosti"), OS_F("Uroven %1! Mas %2 bodu k rozdeleni (klavesa O).", data.Level.ToString(), data.Points.ToString()), 8);
		else if (notify)
			OS_Util.Notify(player, OS_T("Dovednosti"), OS_F("+%1 zkusenosti", amount.ToString()), 3);
	}

	void OnZombieKilled(PlayerBase killer)
	{
		AddXp(killer, m_Cfg.XpZombieKill);
	}

	void OnAnimalKilled(PlayerBase killer)
	{
		AddXp(killer, m_Cfg.XpAnimalKill, true);
	}

	void OnPlayerKilled(PlayerBase killer)
	{
		AddXp(killer, m_Cfg.XpPlayerKill, true);
	}

	void OnCooked(PlayerBase player)
	{
		AddXp(player, m_Cfg.XpCookDish, true);
	}

	void OnFarmCollected(PlayerBase player, int items)
	{
		AddXp(player, m_Cfg.XpFarmItem * items);
	}

	// ------------------------------------------------------------ účinky

	//! odolnost: část zranění se hned vrátí
	void OnHit(PlayerBase player, TotalDamageResult damageResult)
	{
		if (!m_Cfg.Enabled || !damageResult)
			return;
		float reduction = Math.Min(Effect(player, "toughness"), 0.9);
		if (reduction <= 0)
			return;
		float health = damageResult.GetDamage("", "Health");
		float blood = damageResult.GetDamage("", "Blood");
		if (health > 0 && player.IsAlive())
			player.AddHealth("", "", health * reduction);
		if (blood > 0 && player.IsAlive())
			player.AddHealth("", "Blood", blood * reduction);
	}

	void OnTick(array<Man> players, float delta)
	{
		if (!m_Cfg.Enabled)
			return;

		m_MinuteTimer += delta;
		bool minute = m_MinuteTimer >= 60;
		if (minute)
			m_MinuteTimer = 0;

		m_StaminaTimer += delta;
		bool staminaCheck = m_StaminaTimer >= 5;
		if (staminaCheck)
			m_StaminaTimer = 0;

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || !player.GetIdentity())
				continue;
			string id = OS_Util.SteamId(player);

			if (minute)
				AddXp(player, m_Cfg.XpPerSurvivalMinute);

			float regen = Effect(player, "regen");
			if (regen > 0)
				player.AddHealth("", "", regen * delta);

			float blood = Effect(player, "blood");
			if (blood > 0)
				player.AddHealth("", "Blood", blood * delta);

			float warmth = Effect(player, "warmth");
			if (warmth > 0 && player.GetStatHeatBuffer())
			{
				float heatMax = player.GetStatHeatBuffer().GetMax();
				float heat = player.GetStatHeatBuffer().Get() + heatMax * warmth * delta;
				player.GetStatHeatBuffer().Set(Math.Min(heat, heatMax));
			}

			ApplyMetabolism(player, id);
			if (staminaCheck)
				ApplyStamina(player, id);
		}
	}

	//! metabolismus: část úbytku jídla a vody se vrátí
	protected void ApplyMetabolism(PlayerBase player, string id)
	{
		if (!player.GetStatEnergy() || !player.GetStatWater())
			return;
		float energy = player.GetStatEnergy().Get();
		float water = player.GetStatWater().Get();
		float reduction = Math.Min(Effect(player, "metabolism"), 0.9);

		float lastEnergy;
		if (reduction > 0 && m_LastEnergy.Find(id, lastEnergy) && energy < lastEnergy)
		{
			energy = energy + (lastEnergy - energy) * reduction;
			player.GetStatEnergy().Set(energy);
		}
		float lastWater;
		if (reduction > 0 && m_LastWater.Find(id, lastWater) && water < lastWater)
		{
			water = water + (lastWater - water) * reduction;
			player.GetStatWater().Set(water);
		}
		m_LastEnergy.Set(id, energy);
		m_LastWater.Set(id, water);
	}

	//! výdrž: menší spotřeba (nepřepisuje jiné dočasné úpravy hry)
	protected void ApplyStamina(PlayerBase player, string id)
	{
		StaminaHandler stamina = player.GetStaminaHandler();
		if (!stamina)
			return;
		float wanted = 1 - Math.Min(Effect(player, "stamina"), 0.9);
		float current = stamina.GetDepletionMultiplier();
		float lastSet = 1;
		m_StaminaSet.Find(id, lastSet);
		if (Math.AbsFloat(current - 1) > 0.001 && Math.AbsFloat(current - lastSet) > 0.001)
			return;   // běží jiná úprava hry
		if (Math.AbsFloat(current - wanted) > 0.001)
			stamina.SetDepletionMultiplier(wanted);
		m_StaminaSet.Set(id, wanted);
	}

	// ------------------------------------------------------------ menu

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_SKILLS_OPEN && rpcType != OS_RPC_SKILLS_ACTION)
			return false;
		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !m_Cfg.Enabled)
			return true;

		if (rpcType == OS_RPC_SKILLS_ACTION)
		{
			string skillId;
			if (ctx.Read(skillId))
			{
				if (skillId == "#reset")
					Reset(player);
				else
					Upgrade(player, skillId);
			}
		}
		SendData(player);
		return true;
	}

	protected void Upgrade(PlayerBase player, string skillId)
	{
		OS_SkillDef def = m_Cfg.Find(skillId);
		if (!def)
			return;
		string id = OS_Util.SteamId(player);
		OS_PlayerSkills data = Get(id);
		if (data.Points <= 0)
		{
			OS_Util.Notify(player, OS_T("Dovednosti"), OS_T("Nemas zadne body. Ziskas je za dalsi uroven."));
			return;
		}
		int rank = data.Rank(skillId);
		if (rank >= def.MaxRank)
		{
			OS_Util.Notify(player, OS_T("Dovednosti"), OS_T("Tahle dovednost uz je na maximu."));
			return;
		}
		data.Ranks.Set(skillId, rank + 1);
		data.Points = data.Points - 1;
		Save(id);
		m_StaminaSet.Remove(id);
		ApplyStamina(player, id);
		int newRank = rank + 1;
		OS_Util.Notify(player, OS_T("Dovednosti"), OS_F("%1: uroven %2 / %3", OS_T(def.Name), newRank.ToString(), def.MaxRank.ToString()), 4);
	}

	protected void Reset(PlayerBase player)
	{
		if (m_Cfg.ResetPrice < 0)
			return;
		string id = OS_Util.SteamId(player);
		OS_PlayerSkills data = Get(id);
		int spent = 0;
		foreach (string skill, int rank : data.Ranks)
		{
			spent += rank;
		}
		if (spent == 0)
			return;
		if (m_Cfg.ResetPrice > 0 && !m_Payments.TrySpend(player, m_Cfg.ResetPrice))
		{
			OS_Util.Notify(player, OS_T("Dovednosti"), OS_F("Prerozdeleni bodu stoji %1 %2.", m_Cfg.ResetPrice.ToString(), m_Currency));
			return;
		}
		data.Ranks.Clear();
		data.Points = data.Points + spent;
		Save(id);
		m_StaminaSet.Remove(id);
		ApplyStamina(player, id);
		OS_Util.Notify(player, OS_T("Dovednosti"), OS_F("Body vraceny: %1. Rozdel je znovu.", spent.ToString()));
	}

	protected string EffectText(OS_SkillDef def, int rank)
	{
		float value = rank * def.PerRank;
		if (def.Id == "toughness" || def.Id == "stamina" || def.Id == "metabolism")
		{
			int pct = Math.Round(Math.Min(value, 0.9) * 100);
			return OS_F("-%1 %", pct.ToString());
		}
		if (def.Id == "regen")
		{
			float perMinute = value * 60;
			return OS_F("+%1 zdravi / min", perMinute.ToString());
		}
		if (def.Id == "blood")
		{
			float bloodMinute = value * 60;
			return OS_F("+%1 krve / min", bloodMinute.ToString());
		}
		if (def.Id == "warmth")
		{
			int warmPct = Math.Round(value * 60 * 100);
			return OS_F("+%1 % tepla / min", warmPct.ToString());
		}
		return value.ToString();
	}

	void SendData(PlayerBase player)
	{
		OS_PlayerSkills data = Get(OS_Util.SteamId(player));
		array<string> ids = new array<string>;
		array<string> names = new array<string>;
		array<string> descriptions = new array<string>;
		array<int> ranks = new array<int>;
		array<int> maxRanks = new array<int>;
		array<string> effects = new array<string>;
		array<string> nextEffects = new array<string>;
		foreach (OS_SkillDef def : m_Cfg.Skills)
		{
			if (!def)
				continue;
			int rank = data.Rank(def.Id);
			ids.Insert(def.Id);
			names.Insert(OS_T(def.Name));
			descriptions.Insert(OS_T(def.Description));
			ranks.Insert(rank);
			maxRanks.Insert(def.MaxRank);
			effects.Insert(EffectText(def, rank));
			nextEffects.Insert(EffectText(def, rank + 1));
		}

		int needed = 0;
		if (data.Level < m_Cfg.MaxLevel)
			needed = m_Cfg.XpForNextLevel(data.Level);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data.Level);
		rpc.Write(m_Cfg.MaxLevel);
		rpc.Write(data.Xp);
		rpc.Write(needed);
		rpc.Write(data.Points);
		rpc.Write(m_Cfg.ResetPrice);
		rpc.Write(m_Currency);
		rpc.Write(ids);
		rpc.Write(names);
		rpc.Write(descriptions);
		rpc.Write(ranks);
		rpc.Write(maxRanks);
		rpc.Write(effects);
		rpc.Write(nextEffects);
		rpc.Send(null, OS_RPC_SKILLS_DATA, true, player.GetIdentity());
	}
}

modded class AnimalBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);
		if (g_Game.IsServer() && OS_Server.Get() && OS_Server.Get().m_Skills)
		{
			PlayerBase killerPlayer = OS_Util.ResolveKillerPlayer(killer);
			if (killerPlayer)
				OS_Server.Get().m_Skills.OnAnimalKilled(killerPlayer);
		}
	}
}
