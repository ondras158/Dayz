// Propojení OS skupin s územími (vlajkami) Expansion BaseBuilding.
// Nastavení: $profile:OS_Core/ExpansionTerritory.json
// Evidence přidaných členů: $profile:OS_Core/ExpansionTerritoryData.json
//
// Kdo je ve skupině s majitelem vlajky, je automaticky členem jeho území
// (může stavět, rozebírat, používat garáž území...). Kdo ze skupiny odejde
// nebo je vyhozen, je z území zase odebrán - ale jen pokud ho tam přidal tenhle
// doplněk. Členy pozvané ručně přes menu vlajky to nechá být.

class OS_TerritoryConfig
{
	bool Enabled = true;
	float SyncSeconds = 10;             // jak často se kontroluje skupina vs. území
	bool RemoveWhenLeftGroup = true;    // odebrat z území, když hráč opustí skupinu
	bool NotifyPlayers = true;          // oznámit hráči přidání/odebrání
}

class OS_TerritoryLink
{
	int TerritoryId;
	ref TStringArray Added = new TStringArray;   // Steam ID přidaná tímto doplňkem
}

class OS_TerritoryLinks
{
	ref array<ref OS_TerritoryLink> Links = new array<ref OS_TerritoryLink>;
}

class OS_TerritorySync
{
	protected static const string CFG_FILE = "ExpansionTerritory.json";
	protected static const string DATA_FILE = "ExpansionTerritoryData.json";

	ref OS_TerritoryConfig m_Cfg;
	protected ref OS_TerritoryLinks m_Links;
	protected float m_Timer;

	void OS_TerritorySync()
	{
		m_Cfg = OS_ConfigFile<OS_TerritoryConfig>.Load(CFG_FILE, new OS_TerritoryConfig(), new OS_TerritoryConfig());
		LoadLinks();
		Print(OS_TAG + "Expansion BaseBuilding propojeni se skupinami: " + m_Cfg.Enabled.ToString());
	}

	protected void LoadLinks()
	{
		m_Links = new OS_TerritoryLinks();
		string path = OS_DIR + DATA_FILE;
		string err;
		if (FileExist(path) && !JsonFileLoader<OS_TerritoryLinks>.LoadFile(path, m_Links, err))
		{
			Print(OS_TAG + "CHYBA v " + path + ": " + err);
			m_Links = new OS_TerritoryLinks();
		}
		if (!m_Links.Links)
			m_Links.Links = new array<ref OS_TerritoryLink>;
	}

	protected void SaveLinks()
	{
		string err;
		if (!JsonFileLoader<OS_TerritoryLinks>.SaveFile(OS_DIR + DATA_FILE, m_Links, err))
			Print(OS_TAG + "CHYBA při ukládání " + DATA_FILE + ": " + err);
	}

	protected OS_TerritoryLink GetLink(int territoryId, bool create)
	{
		foreach (OS_TerritoryLink link : m_Links.Links)
		{
			if (link && link.TerritoryId == territoryId)
				return link;
		}
		if (!create)
			return null;

		OS_TerritoryLink newLink = new OS_TerritoryLink();
		newLink.TerritoryId = territoryId;
		m_Links.Links.Insert(newLink);
		return newLink;
	}

	void OnTick(float delta, OS_ServerGroups groups)
	{
		if (!m_Cfg.Enabled || !groups)
			return;

		m_Timer += delta;
		if (m_Timer < m_Cfg.SyncSeconds)
			return;
		m_Timer = 0;

		SyncAll(groups);
	}

	void SyncAll(OS_ServerGroups groups)
	{
		ExpansionTerritoryModule module;
		if (!Class.CastTo(module, CF_ModuleCoreManager.Get(ExpansionTerritoryModule)))
			return;

		map<int, TerritoryFlag> flags = module.GetAllTerritoryFlags();
		if (!flags)
			return;

		bool dirty = false;
		TIntArray alive = new TIntArray;

		foreach (int territoryId, TerritoryFlag flag : flags)
		{
			if (!flag)
				continue;
			ExpansionTerritory territory = flag.GetTerritory();
			if (!territory)
				continue;

			alive.Insert(territoryId);
			if (SyncTerritory(module, territoryId, territory, groups))
				dirty = true;
		}

		// zbourané vlajky už neevidujeme
		for (int i = m_Links.Links.Count() - 1; i >= 0; i--)
		{
			OS_TerritoryLink link = m_Links.Links[i];
			if (!link || alive.Find(link.TerritoryId) == -1)
			{
				m_Links.Links.Remove(i);
				dirty = true;
			}
		}

		if (dirty)
			SaveLinks();
	}

	//! vrací true, když se změnila evidence
	protected bool SyncTerritory(ExpansionTerritoryModule module, int territoryId, ExpansionTerritory territory, OS_ServerGroups groups)
	{
		string ownerId = territory.GetOwnerID();
		OS_Group group = groups.GetGroupOf(ownerId);
		OS_TerritoryLink link = GetLink(territoryId, false);

		bool changedTerritory = false;
		bool changedLinks = false;
		TStringArray wanted = new TStringArray;

		// 1) přidat chybějící členy skupiny
		if (group)
		{
			foreach (OS_GroupMember member : group.Members)
			{
				if (!member || member.Id == "" || member.Id == ownerId)
					continue;

				wanted.Insert(member.Id);
				if (territory.IsMember(member.Id))
					continue;

				territory.AddMember(member.Id, member.Name);
				if (!link)
					link = GetLink(territoryId, true);
				if (link.Added.Find(member.Id) == -1)
					link.Added.Insert(member.Id);
				changedTerritory = true;
				changedLinks = true;

				if (m_Cfg.NotifyPlayers)
					NotifySteamId(member.Id, "Byl jsi pridan do uzemi " + territory.GetTerritoryName() + " (skupina " + group.Name + ").");
			}
		}

		// 2) odebrat ty, které jsme přidali a už ve skupině nejsou
		if (link && m_Cfg.RemoveWhenLeftGroup)
		{
			for (int i = link.Added.Count() - 1; i >= 0; i--)
			{
				string addedId = link.Added[i];
				if (wanted.Find(addedId) != -1)
					continue;

				link.Added.Remove(i);
				changedLinks = true;

				if (addedId == ownerId)
					continue;

				ExpansionTerritoryMember tm = territory.GetMember(addedId);
				if (!tm)
					continue;

				territory.RemoveMember(tm);
				changedTerritory = true;

				if (m_Cfg.NotifyPlayers)
					NotifySteamId(addedId, "Uz nejsi ve skupine majitele, byl jsi odebran z uzemi " + territory.GetTerritoryName() + ".");
			}
		}

		if (changedTerritory)
		{
			// vlajka si území uloží sama (OnStoreSave), stačí poslat novinky členům
			module.UpdateClient(territoryId);
		}

		return changedLinks;
	}

	protected void NotifySteamId(string steamId, string text)
	{
		PlayerBase player = OS_Util.FindPlayerBySteamId(steamId);
		if (player)
			OS_Util.Notify(player, "Uzemi", text);
	}
}

modded class OS_Server
{
	protected ref OS_TerritorySync m_TerritorySync;
	protected float m_TerritoryTimer;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!m_TerritorySync)
			m_TerritorySync = new OS_TerritorySync();

		m_TerritoryTimer += timeslice;
		if (m_TerritoryTimer < 1.0)
			return;

		float delta = m_TerritoryTimer;
		m_TerritoryTimer = 0;
		m_TerritorySync.OnTick(delta, m_Groups);
	}
}
