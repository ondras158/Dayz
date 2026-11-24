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
				label = label + " [mrtvy]";
			if (player.m_OS_AdminGod)
				label = label + " [GOD]";

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
			OS_Util.Notify(admin, "Admin", "Hrac uz neni na serveru.");
			return;
		}

		string targetName = OS_Util.PlayerName(target);

		if (action == OS_AdminAction.HEAL)
		{
			Heal(target);
			OS_Util.Notify(admin, "Admin", "Vyleceno: " + targetName);
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
				OS_Util.Notify(admin, "Admin", "Godmode ZAPNUT");
			else
				OS_Util.Notify(admin, "Admin", "Godmode VYPNUT");
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
			OS_Util.Notify(admin, "Admin", targetName + ": " + amount.ToString() + " " + m_Currency + " (zustatek " + m_Wallets.GetBalance(OS_Util.SteamId(target)).ToString() + ")");
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
			OS_Util.Notify(admin, "Admin", "Neznamy predmet: " + type);
			return;
		}

		for (int i = 0; i < count; i++)
		{
			OS_Util.GiveItem(target, type);
		}

		OS_Util.Notify(admin, "Admin", count.ToString() + "x " + type + " -> " + OS_Util.PlayerName(target));
		Log(admin, "spawn " + count.ToString() + "x " + type + " -> " + OS_Util.PlayerName(target));
	}

	protected void UnlockNearestCar(PlayerBase admin)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(admin.GetPosition(), 15, objects, null);

		CarScript best;
		float bestDist = 999999;
		foreach (Object obj : objects)
		{
			CarScript car = CarScript.Cast(obj);
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
			OS_Util.Notify(admin, "Admin", "Zadne auto do 15 m.");
			return;
		}

		best.OS_SetLocked(false);
		OS_Util.Notify(admin, "Admin", "Auto odemceno (majitel " + best.OS_GetOwnerId() + ").");
		Log(admin, "odemkl auto " + best.GetType() + " majitele " + best.OS_GetOwnerId());
	}

	//! arg = "x z" nebo "x y z"
	protected void TeleportToCoords(PlayerBase admin, string arg)
	{
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
			OS_Util.Notify(admin, "Admin", "Zadej souradnice jako: X Z");
			return;
		}

		OS_Util.Teleport(admin, OS_Util.GroundPos(x, z));
		Log(admin, "teleport na " + x.ToString() + " " + z.ToString());
	}
}
