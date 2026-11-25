// Admin menu - všechny akce se ověřují na serveru podle Steam ID v Admins.json.

class OS_ServerAdmin
{
	ref OS_AdminConfig m_Cfg;
	protected OS_Wallets m_Wallets;
	protected string m_Currency;

	void OS_ServerAdmin(OS_Wallets wallets, string currency)
	{
		m_Wallets = wallets;
		m_Currency = currency;
		m_Cfg = OS_ConfigFile<OS_AdminConfig>.Load("Admins.json", new OS_AdminConfig(), new OS_AdminConfig());
		if (!m_Cfg.AdminSteamIds || m_Cfg.AdminSteamIds.Count() == 0)
			Print(OS_TAG + "Zadny admin neni nastaven - dopln Steam64 ID do " + OS_DIR + "Admins.json");
	}

	bool IsAdmin(PlayerIdentity identity)
	{
		if (!identity || !m_Cfg.AdminSteamIds)
			return false;
		return m_Cfg.AdminSteamIds.Find(identity.GetPlainId()) > -1;
	}

	protected void Log(PlayerBase admin, string text)
	{
		if (m_Cfg.LogActions)
			Print(OS_TAG + "ADMIN " + OS_Util.PlayerName(admin) + " (" + OS_Util.SteamId(admin) + "): " + text);
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_ADMIN_OPEN && rpcType != OS_RPC_ADMIN_ACTION)
			return false;

		if (!IsAdmin(sender))
		{
			if (sender)
				Print(OS_TAG + "Hrac bez opravneni zkusil admin menu: " + sender.GetName() + " (" + sender.GetPlainId() + ")");
			return true;
		}

		PlayerBase admin = OS_Util.FindPlayer(sender);
		if (!admin)
			return true;

		if (rpcType == OS_RPC_ADMIN_ACTION)
			HandleAction(admin, ctx);

		SendData(admin);
		return true;
	}

	protected void SendData(PlayerBase admin)
	{
		array<string> names = new array<string>;
		array<string> ids = new array<string>;

		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.GetIdentity())
				continue;

			string label = player.GetIdentity().GetName();
			if (!player.IsAlive())
				label = OS_F("%1 [mrtvy]", label);
			if (player.m_OS_AdminGod)
				label = OS_F("%1 [GOD]", label);
			if (IsAdmin(player.GetIdentity()))
				label = OS_F("%1 [ADMIN]", label);

			names.Insert(label);
			ids.Insert(player.GetIdentity().GetPlainId());
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(names);
		rpc.Write(ids);
		rpc.Write(admin.m_OS_AdminGod);
		rpc.Send(null, OS_RPC_ADMIN_DATA, true, admin.GetIdentity());
	}

	protected void HandleAction(PlayerBase admin, ParamsReadContext ctx)
	{
		int action;
		string targetId;
		string arg;
		if (!ctx.Read(action) || !ctx.Read(targetId) || !ctx.Read(arg))
			return;

		// bez vybraného hráče se akce týká admina samotného
		PlayerBase target = admin;
		if (targetId != "")
			target = OS_Util.FindPlayerBySteamId(targetId);

		if (!target)
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Hrac uz neni na serveru."));
			return;
		}

		string targetName = OS_Util.PlayerName(target);

		if (action == OS_AdminAction.HEAL)
		{
			Heal(target);
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Vyleceno: %1", targetName));
			Log(admin, "heal " + targetName);
		}
		else if (action == OS_AdminAction.KILL)
		{
			target.SetHealth("", "", 0);
			Log(admin, "kill " + targetName);
		}
		else if (action == OS_AdminAction.TELEPORT_TO)
		{
			OS_Util.Teleport(admin, target.GetPosition());
			Log(admin, "teleport k " + targetName);
		}
		else if (action == OS_AdminAction.TELEPORT_HERE)
		{
			OS_Util.Teleport(target, admin.GetPosition());
			Log(admin, "teleport " + targetName + " k sobe");
		}
		else if (action == OS_AdminAction.TOGGLE_GOD)
		{
			admin.m_OS_AdminGod = !admin.m_OS_AdminGod;
			admin.OS_RefreshDamage();
			if (admin.m_OS_AdminGod)
				OS_Util.Notify(admin, OS_T("Admin"), OS_T("Godmode ZAPNUT"));
			else
				OS_Util.Notify(admin, OS_T("Admin"), OS_T("Godmode VYPNUT"));
			Log(admin, "godmode " + admin.m_OS_AdminGod.ToString());
		}
		else if (action == OS_AdminAction.SPAWN_ITEM)
		{
			SpawnItems(admin, target, arg);
		}
		else if (action == OS_AdminAction.GIVE_MONEY)
		{
			int amount = arg.Trim().ToInt();
			m_Wallets.Add(OS_Util.SteamId(target), amount);
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("%1: %2 %3 (zustatek %4)", targetName, amount.ToString(), m_Currency, m_Wallets.GetBalance(OS_Util.SteamId(target)).ToString()));
			Log(admin, "penize " + amount.ToString() + " -> " + targetName);
		}
		else if (action == OS_AdminAction.TELEPORT_COORDS)
		{
			TeleportToCoords(admin, arg);
		}
		else if (action == OS_AdminAction.UNLOCK_CAR)
		{
			UnlockNearestCar(admin);
		}
		else if (action == OS_AdminAction.PVP_ADD && OS_Server.Get())
		{
			OS_Server.Get().m_PvP.AdminAdd(admin, arg);
			Log(admin, "pvp zona + " + arg);
		}
		else if (action == OS_AdminAction.PVP_REMOVE && OS_Server.Get())
		{
			OS_Server.Get().m_PvP.AdminRemoveNearest(admin);
			Log(admin, "pvp zona -");
		}
		else
		{
			HandleToolAction(admin, target, action, arg);
		}
	}

	protected void Heal(PlayerBase player)
	{
		player.SetHealth("", "", player.GetMaxHealth("", ""));
		player.SetHealth("", "Blood", player.GetMaxHealth("", "Blood"));
		player.SetHealth("", "Shock", player.GetMaxHealth("", "Shock"));
		player.GetStatWater().Set(player.GetStatWater().GetMax());
		player.GetStatEnergy().Set(player.GetStatEnergy().GetMax());
		if (player.GetBleedingManagerServer())
			player.GetBleedingManagerServer().RemoveAllSources();
	}

	//! arg = "ClassName" nebo "ClassName pocet"
	protected void SpawnItems(PlayerBase admin, PlayerBase target, string arg)
	{
		array<string> parts = new array<string>;
		string trimmedArg = arg.Trim();
		trimmedArg.Split(" ", parts);
		if (parts.Count() == 0)
			return;

		string type = parts[0];
		int count = 1;
		if (parts.Count() > 1)
		{
			count = parts[1].ToInt();
			if (count < 1)
				count = 1;
			if (count > 20)
				count = 20;
		}

		if (!OS_Util.IsValidClass(type))
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Neznamy predmet: %1", type));
			return;
		}

		for (int i = 0; i < count; i++)
		{
			OS_Util.GiveItem(target, type);
		}

		OS_Util.Notify(admin, OS_T("Admin"), count.ToString() + "x " + type + " -> " + OS_Util.PlayerName(target));
		Log(admin, "spawn " + count.ToString() + "x " + type + " -> " + OS_Util.PlayerName(target));
	}

	protected void UnlockNearestCar(PlayerBase admin)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(admin.GetPosition(), 15, objects, null);

		Transport best;
		float bestDist = 999999;
		foreach (Object obj : objects)
		{
			Transport car = Transport.Cast(obj);
			if (!car)
				continue;
			float dist = vector.Distance(admin.GetPosition(), car.GetPosition());
			if (dist < bestDist)
			{
				bestDist = dist;
				best = car;
			}
		}

		if (!best)
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Zadne auto do 15 m."));
			return;
		}

		best.OS_SetLocked(false);
		OS_Util.Notify(admin, OS_T("Admin"), OS_F("Auto odemceno (majitel %1).", best.OS_GetOwnerId()));
		Log(admin, "odemkl auto " + best.GetType() + " majitele " + best.OS_GetOwnerId());
	}

	//! arg = "x z" nebo "x y z"
	protected void TeleportToCoords(PlayerBase admin, string arg, PlayerBase target = null)
	{
		if (!target)
			target = admin;

		array<string> raw = new array<string>;
		arg.Replace(",", " ");
		arg.Split(" ", raw);

		array<float> nums = new array<float>;
		foreach (string token : raw)
		{
			string t = token.Trim();
			if (t != "")
				nums.Insert(t.ToFloat());
		}

		float x;
		float z;
		if (nums.Count() == 2)
		{
			x = nums[0];
			z = nums[1];
		}
		else if (nums.Count() == 3)
		{
			x = nums[0];
			z = nums[2];
		}
		else
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Zadej souradnice jako: X Z"));
			return;
		}

		OS_Util.Teleport(target, OS_Util.GroundPos(x, z));
		Log(admin, "teleport " + OS_Util.PlayerName(target) + " na " + x.ToString() + " " + z.ToString());
	}

	// ------------------------------------------------------------ nástroje (jako Community Online Tools)

	bool IsBanned(string steamId)
	{
		return m_Cfg.BannedSteamIds && m_Cfg.BannedSteamIds.Find(steamId) > -1;
	}

	//! zabanovaný hráč se po připojení odpojí
	void CheckBan(PlayerBase player)
	{
		if (!player || !player.GetIdentity() || !IsBanned(OS_Util.SteamId(player)))
			return;
		Print(OS_TAG + "Zabanovany hrac se pokusil pripojit: " + OS_Util.PlayerName(player) + " (" + OS_Util.SteamId(player) + ")");
		Disconnect(player, OS_T(m_Cfg.BanMessage));
	}

	protected void Disconnect(PlayerBase player, string reason)
	{
		if (!player || !player.GetIdentity())
			return;
		OS_Util.Notify(player, OS_T("Admin"), reason, 10);
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DisconnectNow, 3000, false, player.GetIdentity().GetPlainId());
	}

	protected void DisconnectNow(string steamId)
	{
		PlayerBase player = OS_Util.FindPlayerBySteamId(steamId);
		if (player && player.GetIdentity())
			g_Game.DisconnectPlayer(player.GetIdentity(), player.GetIdentity().GetId());
	}

	protected void SaveConfig()
	{
		string err;
		if (!JsonFileLoader<OS_AdminConfig>.SaveFile(OS_DIR + "Admins.json", m_Cfg, err))
			Print(OS_TAG + "CHYBA při ukládání Admins.json: " + err);
	}

	//! místo pár metrů před hráčem (na zemi)
	protected vector InFront(PlayerBase player, float distance)
	{
		vector dir = player.GetDirection();
		dir[1] = 0;
		dir.Normalize();
		vector pos = player.GetPosition() + dir * distance;
		return OS_Util.GroundPos(pos[0], pos[2]);
	}

	protected Transport NearestVehicle(vector pos, float radius)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(pos, radius, objects, null);
		Transport best;
		float bestDist = 999999;
		foreach (Object obj : objects)
		{
			Transport car = Transport.Cast(obj);
			if (!car)
				continue;
			float dist = vector.Distance(pos, car.GetPosition());
			if (dist < bestDist)
			{
				bestDist = dist;
				best = car;
			}
		}
		return best;
	}

	protected void HandleToolAction(PlayerBase admin, PlayerBase target, int action, string arg)
	{
		string targetName = OS_Util.PlayerName(target);

		if (action == OS_AdminAction.TELEPORT_MAP)
		{
			TeleportToCoords(admin, arg, target);
		}
		else if (action == OS_AdminAction.SET_TIME)
		{
			SetTime(admin, arg);
		}
		else if (action == OS_AdminAction.SET_WEATHER)
		{
			SetWeather(admin, arg);
		}
		else if (action == OS_AdminAction.SPAWN_GROUND)
		{
			SpawnGround(admin, target, arg);
		}
		else if (action == OS_AdminAction.SPAWN_VEHICLE)
		{
			SpawnVehicleFor(admin, target, arg);
		}
		else if (action == OS_AdminAction.TOGGLE_INVIS)
		{
			admin.OS_SetInvisible(!admin.OS_IsInvisible());
			if (admin.OS_IsInvisible())
				OS_Util.Notify(admin, OS_T("Admin"), OS_T("Neviditelnost ZAPNUTA"));
			else
				OS_Util.Notify(admin, OS_T("Admin"), OS_T("Neviditelnost VYPNUTA"));
			Log(admin, "neviditelnost " + admin.OS_IsInvisible().ToString());
		}
		else if (action == OS_AdminAction.VEHICLE_REPAIR || action == OS_AdminAction.VEHICLE_DESTROY || action == OS_AdminAction.VEHICLE_DELETE)
		{
			VehicleAction(admin, target, action);
		}
		else if (action == OS_AdminAction.STRIP)
		{
			target.RemoveAllItems();
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Odebrany vsechny veci: %1", targetName));
			Log(admin, "vysvlekl " + targetName);
		}
		else if (action == OS_AdminAction.KICK || action == OS_AdminAction.BAN)
		{
			KickOrBan(admin, target, action == OS_AdminAction.BAN, arg);
		}
		else if (action == OS_AdminAction.ADD_ADMIN || action == OS_AdminAction.REMOVE_ADMIN)
		{
			SetAdminRights(admin, target, action == OS_AdminAction.ADD_ADMIN, arg);
		}
		else if (action == OS_AdminAction.UNBAN)
		{
			string unbanId = arg.Trim();
			if (m_Cfg.BannedSteamIds && m_Cfg.BannedSteamIds.Find(unbanId) > -1)
			{
				m_Cfg.BannedSteamIds.RemoveItem(unbanId);
				SaveConfig();
				OS_Util.Notify(admin, OS_T("Admin"), OS_F("Odbanovan: %1", unbanId));
				Log(admin, "unban " + unbanId);
			}
			else
			{
				OS_Util.Notify(admin, OS_T("Admin"), OS_T("Toto Steam ID neni zabanovane. Napis ho do pole."));
			}
		}
	}

	//! arg "hh:mm" nebo "hh"
	protected void SetTime(PlayerBase admin, string arg)
	{
		arg.Replace(":", " ");
		arg.Replace(".", " ");
		array<string> parts = new array<string>;
		arg.Split(" ", parts);
		if (parts.Count() == 0)
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Zadej cas jako hh:mm, napr. 12:00"));
			return;
		}
		int hour = parts[0].ToInt();
		int minute = 0;
		if (parts.Count() > 1)
			minute = parts[1].ToInt();
		if (hour < 0 || hour > 23)
			hour = 12;
		if (minute < 0 || minute > 59)
			minute = 0;

		int year;
		int month;
		int day;
		int oldHour;
		int oldMinute;
		g_Game.GetWorld().GetDate(year, month, day, oldHour, oldMinute);
		g_Game.GetWorld().SetDate(year, month, day, hour, minute);

		string minuteText = minute.ToString();
		if (minute < 10)
			minuteText = "0" + minuteText;
		OS_Util.Notify(admin, OS_T("Admin"), OS_F("Cas nastaven na %1:%2", hour.ToString(), minuteText));
		Log(admin, "cas " + hour.ToString() + ":" + minuteText);
	}

	//! arg: clear / cloudy / rain / storm / fog
	protected void SetWeather(PlayerBase admin, string preset)
	{
		Weather weather = g_Game.GetWeather();
		if (!weather)
			return;

		float overcast = 0;
		float rain = 0;
		float fog = 0;
		bool storm = false;
		string label = OS_T("Jasno");
		if (preset == "cloudy")
		{
			overcast = 0.6;
			label = OS_T("Zatazeno");
		}
		else if (preset == "rain")
		{
			overcast = 0.85;
			rain = 0.7;
			label = OS_T("Dest");
		}
		else if (preset == "storm")
		{
			overcast = 1;
			rain = 1;
			fog = 0.1;
			storm = true;
			label = OS_T("Bourka");
		}
		else if (preset == "fog")
		{
			overcast = 0.4;
			fog = 0.6;
			label = OS_T("Mlha");
		}

		// změna za 30 s, drží se 30 min (pak se počasí vrací k běžnému vývoji)
		float change = 30;
		float hold = 1800;
		weather.GetOvercast().Set(overcast, change, hold);
		weather.GetRain().Set(rain, change, hold);
		weather.GetFog().Set(fog, change, hold);
		if (storm)
			weather.SetStorm(1, 0.7, 20);
		else
			weather.SetStorm(0, 1, 120);

		OS_Util.Notify(admin, OS_T("Admin"), OS_F("Pocasi: %1 (zmena behem 30 s)", label));
		Log(admin, "pocasi " + preset);
	}

	//! arg "classname počet" - na zem před cíl
	protected void SpawnGround(PlayerBase admin, PlayerBase target, string arg)
	{
		array<string> parts = new array<string>;
		string trimmedArg = arg.Trim();
		trimmedArg.Split(" ", parts);
		if (parts.Count() == 0)
			return;

		string type = parts[0];
		int count = 1;
		if (parts.Count() > 1)
			count = parts[1].ToInt();
		if (count < 1)
			count = 1;
		if (count > 50)
			count = 50;

		if (!OS_Util.IsValidClass(type))
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Neznamy predmet: %1", type));
			return;
		}

		vector pos = InFront(target, 1.5);
		for (int i = 0; i < count; i++)
		{
			g_Game.CreateObjectEx(type, pos, ECE_PLACE_ON_SURFACE);
		}
		OS_Util.Notify(admin, OS_T("Admin"), OS_F("Na zemi: %1x %2", count.ToString(), type));
		Log(admin, "spawn zem " + count.ToString() + "x " + type + " u " + OS_Util.PlayerName(target));
	}

	protected void SpawnVehicleFor(PlayerBase admin, PlayerBase target, string type)
	{
		type = type.Trim();
		if (!OS_Server.Get() || !OS_Server.Get().m_Vehicles)
			return;

		vector rot = target.GetOrientation();
		Transport vehicle = OS_Server.Get().m_Vehicles.AdminSpawn(type, InFront(target, 6), rot[0]);
		if (!vehicle)
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Vozidlo %1 se nepodarilo vytvorit.", type));
			return;
		}
		OS_Util.Notify(admin, OS_T("Admin"), OS_F("Vozidlo %1 je pripravene (bez majitele).", type));
		Log(admin, "spawn vozidlo " + type + " u " + OS_Util.PlayerName(target));
	}

	protected void VehicleAction(PlayerBase admin, PlayerBase target, int action)
	{
		Transport car = NearestVehicle(target.GetPosition(), 15);
		if (!car)
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Zadne auto do 15 m."));
			return;
		}
		string type = car.GetType();
		if (action == OS_AdminAction.VEHICLE_REPAIR)
		{
			if (OS_Server.Get() && OS_Server.Get().m_Vehicles)
				OS_Server.Get().m_Vehicles.AdminRepair(car);
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Opraveno: %1", type));
			Log(admin, "oprava vozidla " + type);
		}
		else if (action == OS_AdminAction.VEHICLE_DESTROY)
		{
			car.SetHealth("", "", 0);
			TStringArray zones = new TStringArray;
			car.GetDamageZones(zones);
			foreach (string zone : zones)
			{
				car.SetHealth(zone, "", 0);
			}
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Zniceno: %1", type));
			Log(admin, "zniceni vozidla " + type);
		}
		else
		{
			g_Game.ObjectDelete(car);
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Smazano: %1", type));
			Log(admin, "smazani vozidla " + type);
		}
	}

	protected void KickOrBan(PlayerBase admin, PlayerBase target, bool ban, string reason)
	{
		if (target == admin)
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Nejdriv vyber hrace v seznamu."));
			return;
		}
		reason = reason.Trim();
		string targetName = OS_Util.PlayerName(target);
		string targetId = OS_Util.SteamId(target);

		string message;
		if (ban)
		{
			if (!m_Cfg.BannedSteamIds)
				m_Cfg.BannedSteamIds = new array<string>;
			if (m_Cfg.BannedSteamIds.Find(targetId) == -1)
				m_Cfg.BannedSteamIds.Insert(targetId);
			SaveConfig();
			message = OS_T(m_Cfg.BanMessage);
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Zabanovan: %1 (%2)", targetName, targetId));
			Log(admin, "ban " + targetName + " (" + targetId + ") " + reason);
		}
		else
		{
			message = OS_T("Byl jsi vyhozen ze serveru.");
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("Vyhozen: %1", targetName));
			Log(admin, "kick " + targetName + " (" + targetId + ") " + reason);
		}
		if (reason != "")
			message = OS_F("%1 Duvod: %2", message, reason);
		Disconnect(target, message);
	}

	//! přidání / odebrání admina ve hře: vybraný hráč, nebo Steam64 ID z pole (i offline hráč)
	protected void SetAdminRights(PlayerBase admin, PlayerBase target, bool add, string arg)
	{
		string steamId = arg.Trim();
		string name = steamId;
		PlayerBase player;
		if (target != admin)
		{
			steamId = OS_Util.SteamId(target);
			name = OS_Util.PlayerName(target);
			player = target;
		}
		else if (steamId != "")
		{
			player = OS_Util.FindPlayerBySteamId(steamId);
			if (player)
				name = OS_Util.PlayerName(player);
		}

		if (!OS_Wallets.IsValidSteamId(steamId))
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Vyber hrace v seznamu, nebo napis do pole jeho Steam64 ID (17 cislic)."));
			return;
		}
		if (!m_Cfg.AdminSteamIds)
			m_Cfg.AdminSteamIds = new array<string>;

		if (add)
		{
			if (m_Cfg.AdminSteamIds.Find(steamId) > -1)
			{
				OS_Util.Notify(admin, OS_T("Admin"), OS_F("%1 uz je admin.", name));
				return;
			}
			m_Cfg.AdminSteamIds.Insert(steamId);
			SaveConfig();
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("%1 je novy admin.", name));
			if (player)
				OS_Util.Notify(player, OS_T("Admin"), OS_T("Jsi admin serveru. Admin menu otevres klavesou Home."), 10);
			Log(admin, "pridal admina " + name + " (" + steamId + ")");
		}
		else
		{
			if (steamId == OS_Util.SteamId(admin))
			{
				OS_Util.Notify(admin, OS_T("Admin"), OS_T("Sam sebe odebrat nemuzes (uprav Admins.json)."));
				return;
			}
			if (m_Cfg.AdminSteamIds.Find(steamId) == -1)
			{
				OS_Util.Notify(admin, OS_T("Admin"), OS_F("%1 neni admin.", name));
				return;
			}
			m_Cfg.AdminSteamIds.RemoveItem(steamId);
			SaveConfig();
			OS_Util.Notify(admin, OS_T("Admin"), OS_F("%1 uz neni admin.", name));
			if (player)
			{
				// vypnout admin výhody
				player.m_OS_AdminGod = false;
				player.OS_RefreshDamage();
				if (player.OS_IsInvisible())
					player.OS_SetInvisible(false);
				OS_Util.Notify(player, OS_T("Admin"), OS_T("Uz nejsi admin serveru."), 10);
			}
			Log(admin, "odebral admina " + name + " (" + steamId + ")");
		}

		if (player)
			SendAdminFlag(player);
	}

	//! klient potřebuje vědět, že je admin (teleport z mapy, ESP, freecam)
	void SendAdminFlag(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(IsAdmin(player.GetIdentity()));
		rpc.Send(null, OS_RPC_ADMIN_FLAG, true, player.GetIdentity());
	}
}
