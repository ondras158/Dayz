// Smrt: tělo s věcmi po zabití hráčem / AI, věci zpět po jiné smrti, stejná postava po respawnu.

class OS_ServerDeath
{
	protected static const string CFG_FILE = "Death.json";
	protected static const string DIR = "Deaths/";
	protected static const string TYPES_FILE = "Characters.json";

	ref OS_DeathConfig m_Cfg;
	protected ref OS_CharacterTypes m_Types;
	protected ref OS_BedsData m_Beds;
	protected ref map<string, float> m_BedUsed = new map<string, float>;

	void OS_ServerDeath()
	{
		m_Cfg = OS_ConfigFile<OS_DeathConfig>.Load(CFG_FILE, new OS_DeathConfig(), new OS_DeathConfig());
		MakeDirectory(OS_DIR + DIR);
		m_Types = new OS_CharacterTypes();
		string err;
		string path = OS_DIR + TYPES_FILE;
		if (FileExist(path) && !JsonFileLoader<OS_CharacterTypes>.LoadFile(path, m_Types, err))
			m_Types = new OS_CharacterTypes();
		if (!m_Types.Types)
			m_Types.Types = new map<string, string>;

		m_Beds = new OS_BedsData();
		string bedPath = OS_DIR + "Beds.json";
		if (FileExist(bedPath) && !JsonFileLoader<OS_BedsData>.LoadFile(bedPath, m_Beds, err))
			m_Beds = new OS_BedsData();
		if (!m_Beds.Beds)
			m_Beds.Beds = new map<string, vector>;
	}

	// ------------------------------------------------------------ spací místo (stan / přístřešek)

	static bool IsBed(Object obj)
	{
		TentBase tent = TentBase.Cast(obj);
		return tent && tent.GetState() == TentBase.PITCHED;
	}

	void SetBed(PlayerBase player, TentBase tent)
	{
		if (!m_Cfg.Enabled || !m_Cfg.BedSpawn || !player || !IsBed(tent))
			return;
		m_Beds.Beds.Set(OS_Util.SteamId(player), tent.GetPosition());
		string err;
		JsonFileLoader<OS_BedsData>.SaveFile(OS_DIR + "Beds.json", m_Beds, err);
		OS_Util.Notify(player, OS_T("Spawn"), OS_T("Spaci misto nastaveno - po smrti se tu muzes objevit."), 6);
	}

	//! platné spací místo (stan pořád stojí), jinak false
	bool GetBed(string steamId, out vector pos)
	{
		if (!m_Cfg.Enabled || !m_Cfg.BedSpawn)
			return false;
		vector saved;
		if (!m_Beds.Beds.Find(steamId, saved))
			return false;
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(saved, 2.5, objects, null);
		foreach (Object obj : objects)
		{
			if (IsBed(obj))
			{
				pos = obj.GetPosition();
				return true;
			}
		}
		return false;
	}

	//! zbývající sekundy do dalšího spawnu na spacím místě (0 = jde)
	int BedCooldown(string steamId)
	{
		float used;
		if (!m_BedUsed.Find(steamId, used))
			return 0;
		float left = m_Cfg.BedCooldownMinutes * 60 - (g_Game.GetTickTime() - used);
		if (left <= 0)
			return 0;
		int seconds = Math.Ceil(left);
		return seconds;
	}

	bool SpawnAtBed(PlayerBase player)
	{
		string id = OS_Util.SteamId(player);
		vector pos;
		if (!GetBed(id, pos) || BedCooldown(id) > 0)
			return false;
		m_BedUsed.Set(id, g_Game.GetTickTime());
		vector spot = OS_Util.GroundPos(pos[0] + Math.RandomFloatInclusive(-1.5, 1.5), pos[2] + Math.RandomFloatInclusive(-1.5, 1.5));
		OS_Util.Teleport(player, spot);
		OS_Util.Notify(player, OS_T("Spawn"), OS_T("Probudil ses na svem spacim miste."), 6);
		return true;
	}

	// ------------------------------------------------------------ postava

	//! typ postavy, se kterou má hráč pokračovat ("" = jak si vybral)
	string CharacterType(string steamId)
	{
		if (!m_Cfg.Enabled || !m_Cfg.KeepCharacter)
			return "";
		string type;
		if (m_Types.Types.Find(steamId, type))
			return type;
		return "";
	}

	void RememberCharacter(PlayerBase player)
	{
		if (!m_Cfg.Enabled || !m_Cfg.KeepCharacter || !player || !player.GetIdentity())
			return;
		string id = OS_Util.SteamId(player);
		string type = player.GetType();
		string old;
		if (m_Types.Types.Find(id, old) && old == type)
			return;
		m_Types.Types.Set(id, type);
		string err;
		JsonFileLoader<OS_CharacterTypes>.SaveFile(OS_DIR + TYPES_FILE, m_Types, err);
	}

	// ------------------------------------------------------------ smrt

	void OnKilled(PlayerBase victim, Object killer)
	{
		if (!m_Cfg.Enabled || !victim)
			return;
		string id = OS_Util.SteamId(victim);
		if (id == "")
			return;

		PlayerBase killerPlayer = OS_Util.ResolveKillerPlayer(killer);
		bool dropOnBody = false;
		if (killerPlayer && killerPlayer != victim)
		{
			bool isAI = !killerPlayer.GetIdentity();
			dropOnBody = !isAI || m_Cfg.AICountsAsPlayer;
		}
		else if (killerPlayer == victim)
		{
			dropOnBody = m_Cfg.SuicideDropsItems;
		}

		if (dropOnBody || !m_Cfg.KeepInventoryOnOtherDeath)
		{
			// tělo s věcmi zůstane - vybrat ho může kdokoli
			float seconds = Math.Max(60, m_Cfg.PvPCorpseMinutes * 60);
			victim.SetLifetimeMax(seconds);
			victim.SetLifetime(seconds);
			return;
		}

		// jiná smrt: věci se uloží a po respawnu vrátí (z těla zmizí, aby se nezdvojily)
		OS_SavedInventory saved = new OS_SavedInventory();
		SaveChildren(victim, saved.Items);
		EntityAI inHands = victim.GetHumanInventory().GetEntityInHands();
		if (inHands)
			saved.Hands = SaveItem(inHands, "");
		string err;
		if (!JsonFileLoader<OS_SavedInventory>.SaveFile(OS_DIR + DIR + id + ".json", saved, err))
		{
			Print(OS_TAG + "CHYBA pri ukladani veci po smrti " + id + ": " + err);
			return;
		}
		victim.RemoveAllItems();
		Print(OS_TAG + "SMRT " + OS_Util.PlayerName(victim) + " - veci ulozeny pro respawn");
	}

	protected OS_SavedItem SaveItem(EntityAI item, string slot)
	{
		OS_SavedItem saved = new OS_SavedItem();
		saved.Type = item.GetType();
		saved.Slot = slot;
		saved.Health = item.GetHealth("", "");
		ItemBase ib = ItemBase.Cast(item);
		if (ib)
		{
			if (ib.HasQuantity())
				saved.Quantity = ib.GetQuantity();
			if (ib.GetLiquidType() > 0)
				saved.Liquid = ib.GetLiquidType();
		}
		Magazine mag = Magazine.Cast(item);
		if (mag)
			saved.Ammo = mag.GetAmmoCount();
		SaveChildren(item, saved.Items);
		return saved;
	}

	protected void SaveChildren(EntityAI parent, array<ref OS_SavedItem> list)
	{
		if (!parent.GetInventory())
			return;
		for (int i = 0; i < parent.GetInventory().AttachmentCount(); i++)
		{
			EntityAI attachment = parent.GetInventory().GetAttachmentFromIndex(i);
			if (!attachment)
				continue;
			InventoryLocation il = new InventoryLocation();
			string slotName = "";
			if (attachment.GetInventory().GetCurrentInventoryLocation(il))
				slotName = InventorySlots.GetSlotName(il.GetSlot());
			list.Insert(SaveItem(attachment, slotName));
		}
		CargoBase cargo = parent.GetInventory().GetCargo();
		if (cargo)
		{
			for (int c = 0; c < cargo.GetItemCount(); c++)
			{
				EntityAI cargoItem = cargo.GetItem(c);
				if (cargoItem)
					list.Insert(SaveItem(cargoItem, ""));
			}
		}
	}

	// ------------------------------------------------------------ respawn

	//! true = věci vráceny (startovní výbava se nedává)
	bool TryRestore(PlayerBase player)
	{
		if (!m_Cfg.Enabled || !player)
			return false;
		string id = OS_Util.SteamId(player);
		string path = OS_DIR + DIR + id + ".json";
		if (!FileExist(path))
			return false;

		OS_SavedInventory saved = new OS_SavedInventory();
		string err;
		bool loaded = JsonFileLoader<OS_SavedInventory>.LoadFile(path, saved, err);
		DeleteFile(path);
		if (!loaded || !saved.Items)
			return false;

		player.RemoveAllItems();
		foreach (OS_SavedItem item : saved.Items)
		{
			RestoreItem(player, item, player);
		}
		if (saved.Hands)
		{
			EntityAI hands = player.GetHumanInventory().CreateInHands(saved.Hands.Type);
			if (hands)
				Apply(hands, saved.Hands, player);
			else
				RestoreItem(player, saved.Hands, player);
		}
		OS_Util.Notify(player, OS_T("Smrt"), OS_T("Zabilo te prostredi, ne hrac - tvoje veci se vratily."), 8);
		return true;
	}

	protected void RestoreItem(EntityAI parent, OS_SavedItem saved, PlayerBase owner)
	{
		if (!saved || saved.Type == "")
			return;
		EntityAI item;
		if (saved.Slot != "")
		{
			int slotId = InventorySlots.GetSlotIdFromString(saved.Slot);
			if (slotId != InventorySlots.INVALID)
				item = parent.GetInventory().CreateAttachmentEx(saved.Type, slotId);
		}
		if (!item)
			item = parent.GetInventory().CreateInInventory(saved.Type);
		if (!item && parent != owner)
			item = owner.GetInventory().CreateInInventory(saved.Type);
		if (!item)
			item = EntityAI.Cast(g_Game.CreateObjectEx(saved.Type, owner.GetPosition(), ECE_PLACE_ON_SURFACE));
		if (item)
			Apply(item, saved, owner);
	}

	protected void Apply(EntityAI item, OS_SavedItem saved, PlayerBase owner)
	{
		if (saved.Health >= 0)
			item.SetHealth("", "", saved.Health);
		ItemBase ib = ItemBase.Cast(item);
		if (ib)
		{
			if (saved.Liquid > 0)
				ib.SetLiquidType(saved.Liquid);
			if (saved.Quantity >= 0 && ib.HasQuantity())
				ib.SetQuantity(saved.Quantity);
		}
		Magazine mag = Magazine.Cast(item);
		if (mag && saved.Ammo >= 0)
			mag.ServerSetAmmoCount(saved.Ammo);
		foreach (OS_SavedItem child : saved.Items)
		{
			RestoreItem(item, child, owner);
		}
	}
}
