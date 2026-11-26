// Base u vanilla vlajky: založení, vylepšení poloměru, zrušení, kontrola rozestupů.
// Base patří hráči, který ji založil; spravovat ji může i jeho skupina.

class OS_ServerBases
{
	protected static const string CFG_FILE = "Bases.json";
	protected static const string DATA_FILE = "BasesData.json";
	protected static const float FLAG_REACH = 6;       // jak blízko vlajky musí hráč stát
	protected static const float FLAG_MATCH = 3;       // base patří vlajce v tomhle okruhu

	ref OS_BaseConfig m_Cfg;
	protected ref OS_BasesData m_Data;
	protected OS_Payments m_Payments;
	protected OS_ServerGroups m_Groups;
	protected string m_Currency;
	protected float m_CheckTimer;
	protected float m_StorageTimer;

	void OS_ServerBases(OS_Payments payments, OS_ServerGroups groups, string currency)
	{
		m_Payments = payments;
		m_Groups = groups;
		m_Currency = currency;

		OS_BaseConfig defaults = new OS_BaseConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_BaseConfig>.Load(CFG_FILE, defaults, new OS_BaseConfig());
		if (!m_Cfg.Levels || m_Cfg.Levels.Count() == 0)
		{
			m_Cfg.Levels = new array<ref OS_BaseLevel>;
			m_Cfg.Levels.Insert(new OS_BaseLevel(30, 0));
		}
		LoadData();
	}

	protected void LoadData()
	{
		m_Data = new OS_BasesData();
		string path = OS_DIR + DATA_FILE;
		string err;
		if (FileExist(path) && !JsonFileLoader<OS_BasesData>.LoadFile(path, m_Data, err))
		{
			Print(OS_TAG + "CHYBA v " + path + ": " + err);
			m_Data = new OS_BasesData();
		}
		if (!m_Data.Bases)
			m_Data.Bases = new array<ref OS_Base>;
	}

	protected void Save()
	{
		string err;
		if (!JsonFileLoader<OS_BasesData>.SaveFile(OS_DIR + DATA_FILE, m_Data, err))
			Print(OS_TAG + "CHYBA při ukládání " + DATA_FILE + ": " + err);
	}

	// ------------------------------------------------------------ dotazy

	float Radius(OS_Base base)
	{
		return m_Cfg.RadiusOf(base.Level);
	}

	//! base, do jejíhož poloměru patří místo
	OS_Base BaseAt(vector pos)
	{
		foreach (OS_Base base : m_Data.Bases)
		{
			if (base && OS_Util.Distance2D(pos, base.X, base.Z) <= Radius(base))
				return base;
		}
		return null;
	}

	//! majitel nebo člen jeho skupiny
	bool CanUse(string steamId, OS_Base base)
	{
		if (!base || steamId == "")
			return false;
		if (base.OwnerId == steamId)
			return true;
		return m_Groups && m_Groups.SameGroup(steamId, base.OwnerId);
	}

	//! hráč stojí ve své base nebo v base skupiny
	bool IsInOwnBase(PlayerBase player)
	{
		OS_Base base = BaseAt(player.GetPosition());
		return base && CanUse(OS_Util.SteamId(player), base);
	}

	protected TerritoryFlag FindFlag(vector pos, float radius)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(pos, radius, objects, null);
		TerritoryFlag best;
		float bestDist = 999999;
		foreach (Object obj : objects)
		{
			TerritoryFlag flag = TerritoryFlag.Cast(obj);
			if (!flag)
				continue;
			float dist = vector.Distance(pos, flag.GetPosition());
			if (dist < bestDist)
			{
				bestDist = dist;
				best = flag;
			}
		}
		return best;
	}

	protected OS_Base BaseOfFlag(TerritoryFlag flag)
	{
		if (!flag)
			return null;
		vector pos = flag.GetPosition();
		foreach (OS_Base base : m_Data.Bases)
		{
			if (base && OS_Util.Distance2D(pos, base.X, base.Z) <= FLAG_MATCH)
				return base;
		}
		return null;
	}

	protected int CountOwned(string steamId)
	{
		int count = 0;
		foreach (OS_Base base : m_Data.Bases)
		{
			if (base && base.OwnerId == steamId)
				count++;
		}
		return count;
	}

	//! vejde se base s poloměrem radius na místo x,z? (ignore = base, která se zvětšuje)
	protected string CheckSpace(float x, float z, float radius, OS_Base ignore)
	{
		foreach (OS_Base other : m_Data.Bases)
		{
			if (!other || other == ignore)
				continue;
			float dist = OS_Util.Distance2D(Vector(x, 0, z), other.X, other.Z);
			if (dist < m_Cfg.MinDistanceBetweenBases)
			{
				int minDist = m_Cfg.MinDistanceBetweenBases;
				return OS_F("Moc blizko jine base (vlajky musi byt aspon %1 m od sebe).", minDist.ToString());
			}
			if (dist < radius + Radius(other) + m_Cfg.GapBetweenBases)
				return OS_T("Base by zasahovala do jine base - neni tu misto.");
		}
		return "";
	}

	// ------------------------------------------------------------ RPC

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_BASE_OPEN && rpcType != OS_RPC_BASE_ACTION)
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive() || !m_Cfg.Enabled)
			return true;

		TerritoryFlag flag = FindFlag(player.GetPosition(), FLAG_REACH);
		if (!flag)
		{
			OS_Util.Notify(player, OS_T("Base"), OS_T("Musis stat u vlajky."));
			return true;
		}

		if (rpcType == OS_RPC_BASE_ACTION)
		{
			int action;
			if (ctx.Read(action))
				HandleAction(player, flag, action);
		}

		SendData(player, flag);
		return true;
	}

	protected void HandleAction(PlayerBase player, TerritoryFlag flag, int action)
	{
		string id = OS_Util.SteamId(player);
		OS_Base base = BaseOfFlag(flag);

		if (action == OS_BaseAction.FOUND)
		{
			if (base)
			{
				OS_Util.Notify(player, OS_T("Base"), OS_T("U teto vlajky uz base je."));
				return;
			}
			if (m_Cfg.RequirePole && (!flag.GetConstruction() || !flag.GetConstruction().IsPartConstructed("pole")))
			{
				OS_Util.Notify(player, OS_T("Base"), OS_T("Nejdriv postav stozar vlajky."));
				return;
			}
			if (m_Cfg.MaxBasesPerPlayer > 0 && CountOwned(id) >= m_Cfg.MaxBasesPerPlayer)
			{
				OS_Util.Notify(player, OS_T("Base"), OS_F("Muzes mit nejvyse %1 base.", m_Cfg.MaxBasesPerPlayer.ToString()));
				return;
			}
			vector pos = flag.GetPosition();
			string problem = CheckSpace(pos[0], pos[2], m_Cfg.RadiusOf(1), null);
			if (problem != "")
			{
				OS_Util.Notify(player, OS_T("Base"), problem);
				return;
			}
			if (m_Cfg.FoundPrice > 0 && !m_Payments.TrySpend(player, m_Cfg.FoundPrice))
			{
				OS_Util.Notify(player, OS_T("Base"), OS_F("Zalozeni base stoji %1 %2.", m_Cfg.FoundPrice.ToString(), m_Currency));
				return;
			}

			OS_Base created = new OS_Base();
			created.Id = m_Data.NextId;
			m_Data.NextId = m_Data.NextId + 1;
			created.OwnerId = id;
			created.OwnerName = OS_Util.PlayerName(player);
			created.X = pos[0];
			created.Z = pos[2];
			created.Level = 1;
			m_Data.Bases.Insert(created);
			Save();
			int radius = Radius(created);
			OS_Util.Notify(player, OS_T("Base"), OS_F("Base zalozena (polomer %1 m).", radius.ToString()));
			Print(OS_TAG + "BASE " + created.OwnerName + " (" + id + ") zalozil base na " + pos.ToString());
			RefreshMaps();
			return;
		}

		if (!base)
		{
			OS_Util.Notify(player, OS_T("Base"), OS_T("U teto vlajky neni base."));
			return;
		}
		if (!CanUse(id, base))
		{
			OS_Util.Notify(player, OS_T("Base"), OS_T("Tahle base ti nepatri."));
			return;
		}

		if (action == OS_BaseAction.UPGRADE)
		{
			if (base.Level >= m_Cfg.Levels.Count())
			{
				OS_Util.Notify(player, OS_T("Base"), OS_T("Base uz ma nejvetsi polomer."));
				return;
			}
			OS_BaseLevel next = m_Cfg.Levels[base.Level];
			string spaceProblem = CheckSpace(base.X, base.Z, next.Radius, base);
			if (spaceProblem != "")
			{
				OS_Util.Notify(player, OS_T("Base"), spaceProblem);
				return;
			}
			if (next.Price > 0 && !m_Payments.TrySpend(player, next.Price))
			{
				OS_Util.Notify(player, OS_T("Base"), OS_F("Vylepseni stoji %1 %2.", next.Price.ToString(), m_Currency));
				return;
			}
			base.Level = base.Level + 1;
			Save();
			int newRadius = next.Radius;
			OS_Util.Notify(player, OS_T("Base"), OS_F("Base vylepsena na uroven %1 (polomer %2 m).", base.Level.ToString(), newRadius.ToString()));
			RefreshMaps();
		}
		else if (action == OS_BaseAction.REMOVE)
		{
			if (base.OwnerId != id)
			{
				OS_Util.Notify(player, OS_T("Base"), OS_T("Zrusit base muze jen majitel."));
				return;
			}
			m_Data.Bases.RemoveItem(base);
			Save();
			OS_Util.Notify(player, OS_T("Base"), OS_T("Base zrusena. Vlajka zustava."));
			RefreshMaps();
		}
	}

	protected void SendData(PlayerBase player, TerritoryFlag flag)
	{
		OS_Base base = BaseOfFlag(flag);
		string id = OS_Util.SteamId(player);

		bool hasBase = base != null;
		bool canManage = false;
		bool isOwner = false;
		string ownerName = "";
		int level = 0;
		int maxLevel = m_Cfg.Levels.Count();
		float radius = m_Cfg.RadiusOf(1);
		float nextRadius = 0;
		int nextPrice = 0;
		if (base)
		{
			canManage = CanUse(id, base);
			isOwner = base.OwnerId == id;
			ownerName = base.OwnerName;
			level = base.Level;
			radius = Radius(base);
			if (base.Level < maxLevel)
			{
				nextRadius = m_Cfg.Levels[base.Level].Radius;
				nextPrice = m_Cfg.Levels[base.Level].Price;
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Currency);
		rpc.Write(hasBase);
		rpc.Write(canManage);
		rpc.Write(isOwner);
		rpc.Write(ownerName);
		rpc.Write(level);
		rpc.Write(maxLevel);
		rpc.Write(radius);
		rpc.Write(nextRadius);
		rpc.Write(nextPrice);
		rpc.Write(m_Cfg.FoundPrice);
		rpc.Write(m_Cfg.FarmRequiresBase);
		rpc.Send(null, OS_RPC_BASE_DATA, true, player.GetIdentity());
	}

	// ------------------------------------------------------------ údržba a mapa

	//! base bez vlajky (zbouraná, rozebraná) zaniká
	void OnTick(float delta)
	{
		m_CheckTimer += delta;
		if (m_CheckTimer < 60)
			return;
		m_CheckTimer = 0;

		// kódové zámky: skupina majitele se mohla změnit (pro zobrazení akcí u klientů)
		DigitalCodeLock.OS_RefreshAllGroups();

		m_StorageTimer += 60;
		if (m_StorageTimer >= Math.Max(5, m_Cfg.StorageRefreshMinutes) * 60)
		{
			m_StorageTimer = 0;
			RefreshStorage();
		}

		bool changed = false;
		for (int i = m_Data.Bases.Count() - 1; i >= 0; i--)
		{
			OS_Base base = m_Data.Bases[i];
			if (!base)
			{
				m_Data.Bases.Remove(i);
				changed = true;
				continue;
			}
			if (!FindFlag(Vector(base.X, g_Game.SurfaceY(base.X, base.Z), base.Z), FLAG_MATCH + 2))
			{
				Print(OS_TAG + "BASE zanikla (chybi vlajka): " + base.OwnerName + " na " + base.X.ToString() + " " + base.Z.ToString());
				m_Data.Bases.Remove(i);
				changed = true;
			}
		}
		if (changed)
		{
			Save();
			RefreshMaps();
		}
	}

	//! truhly, stavby (a auta) v poloměru basí nezmizí, dokud base stojí
	void RefreshStorage()
	{
		if (!m_Cfg.Enabled || (!m_Cfg.KeepStorageInBase && !m_Cfg.KeepBuildingsInBase && !m_Cfg.KeepVehiclesInBase))
			return;
		int refreshed = 0;
		foreach (OS_Base base : m_Data.Bases)
		{
			if (!base)
				continue;
			array<Object> objects = new array<Object>;
			vector center = Vector(base.X, g_Game.SurfaceY(base.X, base.Z), base.Z);
			g_Game.GetObjectsAtPosition(center, Radius(base), objects, null);
			foreach (Object obj : objects)
			{
				EntityAI entity = EntityAI.Cast(obj);
				if (!entity || entity.GetHierarchyParent() || entity.IsMan() || entity.IsDayZCreature())
					continue;
				if (!KeepEntity(entity))
					continue;
				entity.IncreaseLifetime();
				refreshed++;
			}
		}
		if (refreshed > 0)
			Print(OS_TAG + "BASE: obnovena zivotnost " + refreshed.ToString() + " veci v basich");
	}

	protected bool KeepEntity(EntityAI entity)
	{
		if (Transport.Cast(entity))
			return m_Cfg.KeepVehiclesInBase;
		if (BaseBuildingBase.Cast(entity))
			return m_Cfg.KeepBuildingsInBase;
		if (!m_Cfg.KeepStorageInBase)
			return false;
		if (TentBase.Cast(entity))
			return true;
		// cokoli s úložným prostorem (truhla, bedna, sud, kufr, skrýš...)
		return entity.GetInventory() && entity.GetInventory().GetCargo() != null;
	}

	//! base, které hráč vidí na mapě (svoje a skupiny)
	void FillMapBases(PlayerBase player, array<string> names, array<float> xs, array<float> zs, array<float> rs)
	{
		if (!m_Cfg.Enabled || !m_Cfg.ShowOnMap || !player)
			return;
		string id = OS_Util.SteamId(player);
		foreach (OS_Base base : m_Data.Bases)
		{
			if (!base || !CanUse(id, base))
				continue;
			names.Insert(OS_F("Base: %1", base.OwnerName));
			xs.Insert(base.X);
			zs.Insert(base.Z);
			rs.Insert(Radius(base));
		}
	}

	protected void RefreshMaps()
	{
		if (OS_Server.Get() && OS_Server.Get().m_PvP)
			OS_Server.Get().m_PvP.BroadcastZones();
	}

	//! admin: smazat nejbližší base (vlajka zůstane)
	void AdminRemoveNearest(PlayerBase admin, vector pos)
	{
		OS_Base best;
		float bestDist = 1000000;
		foreach (OS_Base base : m_Data.Bases)
		{
			if (!base)
				continue;
			float dist = OS_Util.Distance2D(pos, base.X, base.Z);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = base;
			}
		}
		if (!best || bestDist > 300)
		{
			OS_Util.Notify(admin, OS_T("Admin"), OS_T("Zadna base do 300 m."));
			return;
		}
		string ownerName = best.OwnerName;
		m_Data.Bases.RemoveItem(best);
		Save();
		OS_Util.Notify(admin, OS_T("Admin"), OS_F("Smazana base hrace %1.", ownerName));
		Print(OS_TAG + "ADMIN " + OS_Util.PlayerName(admin) + " smazal base " + ownerName);
		RefreshMaps();
	}
}
