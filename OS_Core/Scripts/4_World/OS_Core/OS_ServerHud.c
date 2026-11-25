// HUD: server posílá hráčům jejich stav (zdraví, krev, jídlo, voda, teplo) a zónu.

class OS_ServerHud
{
	ref OS_HudConfig m_Cfg;
	protected float m_Timer;

	void OS_ServerHud()
	{
		m_Cfg = OS_ConfigFile<OS_HudConfig>.Load("Hud.json", new OS_HudConfig(), new OS_HudConfig());
	}

	void OnTick(array<Man> players, float delta)
	{
		m_Timer += delta;
		if (m_Timer < Math.Max(0.5, m_Cfg.UpdateSeconds))
			return;
		m_Timer = 0;

		int online = players.Count();
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (player && player.GetIdentity())
				Send(player, online);
		}
	}

	protected float Fraction(PlayerStat<float> stat)
	{
		if (!stat || stat.GetMax() <= 0)
			return 0;
		return Math.Clamp(stat.Get() / stat.GetMax(), 0, 1);
	}

	protected void Send(PlayerBase player, int online)
	{
		float health = 0;
		float blood = 0;
		if (player.IsAlive())
		{
			health = Math.Clamp(player.GetHealth("", "Health") / Math.Max(1, player.GetMaxHealth("", "Health")), 0, 1);
			blood = Math.Clamp(player.GetHealth("", "Blood") / Math.Max(1, player.GetMaxHealth("", "Blood")), 0, 1);
		}
		float food = Fraction(player.GetStatEnergy());
		float water = Fraction(player.GetStatWater());
		float heat = 0;
		if (player.GetStatHeatComfort())
			heat = Math.Clamp(player.GetStatHeatComfort().Get(), -1, 1);

		// zóna: bezpečná > PvP zóna > PvP časovač > PvE
		int zoneType = 0;
		string zoneName = "";
		int pvpSeconds = 0;
		OS_Server server = OS_Server.Get();
		vector pos = player.GetPosition();
		if (server && server.m_SafeZones && server.m_SafeZones.m_Cfg.Enabled)
		{
			OS_SafeZone safe = server.m_SafeZones.FindZone(pos);
			if (safe)
			{
				zoneType = 1;
				zoneName = OS_T(safe.Name);
			}
		}
		if (zoneType == 0 && server && server.m_PvP && server.m_PvP.m_Cfg.Enabled)
		{
			if (!server.m_PvP.m_Cfg.PvPOnlyInZones)
			{
				zoneType = 4;
			}
			else
			{
				OS_PvPZone pvpZone = server.m_PvP.FindZone(pos);
				if (pvpZone)
				{
					zoneType = 2;
					zoneName = OS_T(pvpZone.Name);
				}
				else
				{
					pvpSeconds = server.m_PvP.PvPSecondsLeft(player);
					if (pvpSeconds > 0)
						zoneType = 3;
				}
			}
		}

		string baseText = "";
		if (server && server.m_Bases && server.m_Bases.m_Cfg.Enabled)
		{
			OS_Base base = server.m_Bases.BaseAt(pos);
			if (base)
			{
				if (base.OwnerId == OS_Util.SteamId(player))
					baseText = OS_T("Moje base");
				else if (server.m_Bases.CanUse(OS_Util.SteamId(player), base))
					baseText = OS_F("Base skupiny (%1)", base.OwnerName);
				else
					baseText = OS_F("Cizi base (%1)", base.OwnerName);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Cfg.Enabled);
		rpc.Write(m_Cfg.ShowStatus);
		rpc.Write(m_Cfg.ShowGroup);
		rpc.Write(m_Cfg.ShowInfo);
		rpc.Write(m_Cfg.ShowCoordinates);
		rpc.Write(m_Cfg.Position == "left");
		rpc.Write(health);
		rpc.Write(blood);
		rpc.Write(food);
		rpc.Write(water);
		rpc.Write(heat);
		rpc.Write(zoneType);
		rpc.Write(zoneName);
		rpc.Write(pvpSeconds);
		rpc.Write(baseText);
		rpc.Write(online);

		array<string> diseases = new array<string>;
		array<string> drugs = new array<string>;
		if (m_Cfg.ShowDiseases && player.IsAlive())
			FillHealth(player, diseases, drugs);
		rpc.Write(m_Cfg.ShowDiseases);
		rpc.Write(diseases);
		rpc.Write(drugs);

		float fatigue = -1;   // -1 = únava vypnutá
		if (server && server.m_Sleep && server.m_Sleep.m_Cfg.Enabled)
			fatigue = server.m_Sleep.Fatigue(OS_Util.SteamId(player)) / 100;
		rpc.Write(fatigue);
		rpc.Send(null, OS_RPC_HUD, false, player.GetIdentity());
	}

	protected bool Active(PlayerBase player, int modifier)
	{
		return player.GetModifiersManager() && player.GetModifiersManager().IsModifierActive(modifier);
	}

	protected void AddDisease(array<string> list, string name, string cure)
	{
		string line = name + "|" + cure;
		if (list.Find(line) == -1)
			list.Insert(line);
	}

	//! nemoci (příznaky) s radou, čím léčit, a léky, které právě působí
	protected void FillHealth(PlayerBase player, array<string> diseases, array<string> drugs)
	{
		if (Active(player, eModifiers.MDF_CHOLERA))
			AddDisease(diseases, OS_T("Cholera"), OS_T("Tetracyklin, pit cistou vodu"));
		if (Active(player, eModifiers.MDF_INFLUENZA))
			AddDisease(diseases, OS_T("Chripka"), OS_T("Tetracyklin, vitaminy, teplo"));
		if (Active(player, eModifiers.MDF_PNEUMONIA))
			AddDisease(diseases, OS_T("Zapal plic"), OS_T("Tetracyklin, teplo"));
		if (Active(player, eModifiers.MDF_COMMON_COLD))
			AddDisease(diseases, OS_T("Nachlazeni"), OS_T("Vitaminy, teplo a suche obleceni"));
		if (Active(player, eModifiers.MDF_SALMONELLA))
			AddDisease(diseases, OS_T("Salmonela"), OS_T("Aktivni uhli"));
		if (player.GetSingleAgentCount(eAgents.FOOD_POISON) > 0)
			AddDisease(diseases, OS_T("Otrava jidlem"), OS_T("Aktivni uhli, voda, nejist zkazene"));
		if (Active(player, eModifiers.MDF_WOUND_INFECTION1))
			AddDisease(diseases, OS_T("Infekce rany"), OS_T("Dezinfekce rany, Tetracyklin"));
		if (Active(player, eModifiers.MDF_WOUND_INFECTION2))
			AddDisease(diseases, OS_T("Tezka infekce rany"), OS_T("Tetracyklin a dezinfekce hned"));
		if (Active(player, eModifiers.MDF_BRAIN))
			AddDisease(diseases, OS_T("Kuru (lidske maso)"), OS_T("Nelecitelne, prejde samo"));
		if (Active(player, eModifiers.MDF_HEAVYMETAL1) || Active(player, eModifiers.MDF_HEAVYMETAL2) || Active(player, eModifiers.MDF_HEAVYMETAL3))
			AddDisease(diseases, OS_T("Otrava tezkymi kovy"), OS_T("Chelacni tablety"));
		if (Active(player, eModifiers.MDF_CONTAMINATION1) || Active(player, eModifiers.MDF_CONTAMINATION2) || Active(player, eModifiers.MDF_CONTAMINATION3))
			AddDisease(diseases, OS_T("Chemicka otrava"), OS_T("PO-X antidotum, plynova maska"));
		if (Active(player, eModifiers.MDF_HEAT_STROKE1) || Active(player, eModifiers.MDF_HEAT_STROKE2) || Active(player, eModifiers.MDF_HEAT_STROKE3) || Active(player, eModifiers.MDF_HEAT_STROKE4))
			AddDisease(diseases, OS_T("Prehrati"), OS_T("Stin, mene obleceni, pit"));
		if (Active(player, eModifiers.MDF_PARTICLES_BREATH))
			AddDisease(diseases, OS_T("Podrazdene plice"), OS_T("Respirator nebo maska"));
		if (Active(player, eModifiers.MDF_PARTICLES_EYES))
			AddDisease(diseases, OS_T("Podrazdene oci"), OS_T("Bryle, odejit z prachu"));

		// nákaza před příznaky
		if (m_Cfg.ShowEarlyInfection)
		{
			if (player.GetSingleAgentCount(eAgents.CHOLERA) > 0 && !Active(player, eModifiers.MDF_CHOLERA))
				AddDisease(diseases, OS_T("Nakaza: cholera"), OS_T("Tetracyklin"));
			if (player.GetSingleAgentCount(eAgents.INFLUENZA) > 0 && !Active(player, eModifiers.MDF_INFLUENZA) && !Active(player, eModifiers.MDF_COMMON_COLD))
				AddDisease(diseases, OS_T("Nakaza: chripka"), OS_T("Vitaminy, teplo"));
			if (player.GetSingleAgentCount(eAgents.SALMONELLA) > 0 && !Active(player, eModifiers.MDF_SALMONELLA))
				AddDisease(diseases, OS_T("Nakaza: salmonela"), OS_T("Aktivni uhli"));
			if (player.GetSingleAgentCount(eAgents.WOUND_AGENT) > 0 && !Active(player, eModifiers.MDF_WOUND_INFECTION1) && !Active(player, eModifiers.MDF_WOUND_INFECTION2))
				AddDisease(diseases, OS_T("Nakaza: infekce rany"), OS_T("Dezinfekce rany"));
		}

		if (Active(player, eModifiers.MDF_ANTIBIOTICS))
			drugs.Insert(OS_T("antibiotika"));
		if (Active(player, eModifiers.MDF_CHARCOAL))
			drugs.Insert(OS_T("aktivni uhli"));
		if (Active(player, eModifiers.MDF_IMMUNITYBOOST))
			drugs.Insert(OS_T("vitaminy"));
		if (Active(player, eModifiers.MDF_PAINKILLERS))
			drugs.Insert(OS_T("proti bolesti"));
		if (Active(player, eModifiers.MDF_MORPHINE))
			drugs.Insert(OS_T("morfin"));
		if (Active(player, eModifiers.MDF_EPINEPHRINE))
			drugs.Insert(OS_T("adrenalin"));
		if (Active(player, eModifiers.MDF_DISINFECTION))
			drugs.Insert(OS_T("dezinfekce"));
		if (Active(player, eModifiers.MDF_CHELATION))
			drugs.Insert(OS_T("chelacni tablety"));
		if (Active(player, eModifiers.MDF_SALINE))
			drugs.Insert(OS_T("fyziologicky roztok"));
	}
}
