// Admin menu - všechny akce se ověřují na serveru podle Steam ID v Admins.json.

class BC_ServerAdmin
{
	ref BC_AdminConfig m_Cfg;
	protected BC_Wallets m_Wallets;
	protected string m_Currency;

	void BC_ServerAdmin(BC_Wallets wallets, string currency)
	{
		m_Wallets = wallets;
		m_Currency = currency;
		m_Cfg = BC_ConfigFile<BC_AdminConfig>.Load("Admins.json", new BC_AdminConfig(), new BC_AdminConfig());
		if (!m_Cfg.AdminSteamIds || m_Cfg.AdminSteamIds.Count() == 0)
			Print(BC_TAG + "Zadny admin neni nastaven - dopln Steam64 ID do " + BC_DIR + "Admins.json");
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
			Print(BC_TAG + "ADMIN " + BC_Util.PlayerName(admin) + " (" + BC_Util.SteamId(admin) + "): " + text);
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != BC_RPC_ADMIN_OPEN && rpcType != BC_RPC_ADMIN_ACTION)
			return false;

		if (!IsAdmin(sender))
		{
			if (sender)
				Print(BC_TAG + "Hrac bez opravneni zkusil admin menu: " + sender.GetName() + " (" + sender.GetPlainId() + ")");
			return true;
		}

		PlayerBase admin = BC_Util.FindPlayer(sender);
		if (!admin)
			return true;

		if (rpcType == BC_RPC_ADMIN_ACTION)
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
			if (player.m_BC_AdminGod)
				label = label + " [GOD]";

			names.Insert(label);
			ids.Insert(player.GetIdentity().GetPlainId());
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(names);
		rpc.Write(ids);
		rpc.Write(admin.m_BC_AdminGod);
		rpc.Send(null, BC_RPC_ADMIN_DATA, true, admin.GetIdentity());
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
			target = BC_Util.FindPlayerBySteamId(targetId);

		if (!target)
		{
			BC_Util.Notify(admin, "Admin", "Hrac uz neni na serveru.");
			return;
		}

		string targetName = BC_Util.PlayerName(target);

		if (action == BC_AdminAction.HEAL)
		{
			Heal(target);
			BC_Util.Notify(admin, "Admin", "Vyleceno: " + targetName);
			Log(admin, "heal " + targetName);
		}
		else if (action == BC_AdminAction.KILL)
		{
			target.SetHealth("", "", 0);
			Log(admin, "kill " + targetName);
		}
		else if (action == BC_AdminAction.TELEPORT_TO)
		{
			BC_Util.Teleport(admin, target.GetPosition());
			Log(admin, "teleport k " + targetName);
		}
		else if (action == BC_AdminAction.TELEPORT_HERE)
		{
			BC_Util.Teleport(target, admin.GetPosition());
			Log(admin, "teleport " + targetName + " k sobe");
		}
		else if (action == BC_AdminAction.TOGGLE_GOD)
		{
			admin.m_BC_AdminGod = !admin.m_BC_AdminGod;
			admin.BC_RefreshDamage();
			if (admin.m_BC_AdminGod)
				BC_Util.Notify(admin, "Admin", "Godmode ZAPNUT");
			else
				BC_Util.Notify(admin, "Admin", "Godmode VYPNUT");
			Log(admin, "godmode " + admin.m_BC_AdminGod.ToString());
		}
		else if (action == BC_AdminAction.SPAWN_ITEM)
		{
			SpawnItems(admin, target, arg);
		}
		else if (action == BC_AdminAction.GIVE_MONEY)
		{
			int amount = arg.Trim().ToInt();
			m_Wallets.Add(BC_Util.SteamId(target), amount);
			BC_Util.Notify(admin, "Admin", targetName + ": " + amount.ToString() + " " + m_Currency + " (zustatek " + m_Wallets.GetBalance(BC_Util.SteamId(target)).ToString() + ")");
			Log(admin, "penize " + amount.ToString() + " -> " + targetName);
		}
		else if (action == BC_AdminAction.TELEPORT_COORDS)
		{
			TeleportToCoords(admin, arg);
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

		if (!BC_Util.IsValidClass(type))
		{
			BC_Util.Notify(admin, "Admin", "Neznamy predmet: " + type);
			return;
		}

		for (int i = 0; i < count; i++)
		{
			BC_Util.GiveItem(target, type);
		}

		BC_Util.Notify(admin, "Admin", count.ToString() + "x " + type + " -> " + BC_Util.PlayerName(target));
		Log(admin, "spawn " + count.ToString() + "x " + type + " -> " + BC_Util.PlayerName(target));
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
			BC_Util.Notify(admin, "Admin", "Zadej souradnice jako: X Z");
			return;
		}

		BC_Util.Teleport(admin, BC_Util.GroundPos(x, z));
		Log(admin, "teleport na " + x.ToString() + " " + z.ToString());
	}
}
