class OS_Util
{
	//! Najde postavu hráče podle identity (pro RPC od klienta)
	static PlayerBase FindPlayer(PlayerIdentity identity)
	{
		if (!identity)
			return null;

		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			if (man && man.GetIdentity() && man.GetIdentity().GetId() == identity.GetId())
				return PlayerBase.Cast(man);
		}
		return null;
	}

	//! Najde hráče podle Steam64 ID
	static PlayerBase FindPlayerBySteamId(string steamId)
	{
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			if (man && man.GetIdentity() && man.GetIdentity().GetPlainId() == steamId)
				return PlayerBase.Cast(man);
		}
		return null;
	}

	static string PlayerName(PlayerBase player)
	{
		if (player && player.GetIdentity())
			return player.GetIdentity().GetName();
		return "Neznamy hrac";
	}

	static string SteamId(PlayerBase player)
	{
		if (player && player.GetIdentity())
			return player.GetIdentity().GetPlainId();
		return "";
	}

	//! Kdo doopravdy zabíjel (zbraň v rukou -> hráč, který ji drží)
	static PlayerBase ResolveKillerPlayer(Object killer)
	{
		PlayerBase direct = PlayerBase.Cast(killer);
		if (direct)
			return direct;

		EntityAI ent = EntityAI.Cast(killer);
		if (ent)
			return PlayerBase.Cast(ent.GetHierarchyRootPlayer());

		return null;
	}

	//! Notifikace v pravém horním rohu
	static void Notify(PlayerBase player, string title, string text, float seconds = 6)
	{
		if (player && player.GetIdentity())
			NotificationSystem.SendNotificationToPlayerExtended(player, seconds, title, text, "");
	}

	static void Chat(PlayerBase player, string text)
	{
		if (player)
			g_Game.ChatMP(player, text, "colorStatusChannel");
	}

	static void ChatAll(string text)
	{
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			if (man)
				g_Game.ChatMP(man, text, "colorStatusChannel");
		}
	}

	//! Pozice na terénu
	static vector GroundPos(float x, float z)
	{
		return Vector(x, g_Game.SurfaceY(x, z), z);
	}

	static float Distance2D(vector a, float x, float z)
	{
		float dx = a[0] - x;
		float dz = a[2] - z;
		return Math.Sqrt(dx * dx + dz * dz);
	}

	//! "A|B|C" -> náhodně jedno z A, B, C
	static string PickVariant(string entry)
	{
		array<string> parts = new array<string>;
		entry.Split("|", parts);

		array<string> clean = new array<string>;
		foreach (string part : parts)
		{
			string trimmed = part.Trim();
			if (trimmed != "")
				clean.Insert(trimmed);
		}

		if (clean.Count() == 0)
			return "";
		return clean.GetRandomElement();
	}

	//! Vytvoří předmět v inventáři hráče, když se nevejde, položí ho na zem
	static EntityAI GiveItem(PlayerBase player, string type)
	{
		if (!player || type == "")
			return null;

		EntityAI item = player.GetInventory().CreateInInventory(type);
		if (!item)
			item = EntityAI.Cast(g_Game.CreateObjectEx(type, player.GetPosition(), ECE_PLACE_ON_SURFACE));

		if (!item)
			Print(OS_TAG + "Nepodarilo se vytvorit predmet '" + type + "' (spatny classname?)");

		return item;
	}

	//! Je classname platný předmět? (hledá v CfgVehicles, CfgWeapons, CfgMagazines)
	static bool IsValidClass(string type)
	{
		if (type == "")
			return false;
		return g_Game.ConfigIsExisting("CfgVehicles " + type) || g_Game.ConfigIsExisting("CfgWeapons " + type) || g_Game.ConfigIsExisting("CfgMagazines " + type);
	}

	//! Vydá hráči klíč s daným kódem (do inventáře, jinak na zem)
	static OS_CarKey GiveCarKey(PlayerBase player, int keyCode)
	{
		OS_CarKey key = OS_CarKey.Cast(GiveItem(player, "OS_CarKey"));
		if (key)
			key.OS_SetKeyCode(keyCode);
		return key;
	}

	static void Teleport(PlayerBase player, vector pos)
	{
		if (player)
			DeveloperTeleport.SetPlayerPosition(player, pos);
	}
}
