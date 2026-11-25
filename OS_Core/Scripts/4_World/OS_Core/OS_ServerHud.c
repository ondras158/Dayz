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
		rpc.Send(null, OS_RPC_HUD, false, player.GetIdentity());
	}
}
