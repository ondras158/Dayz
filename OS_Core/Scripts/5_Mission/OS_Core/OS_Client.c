// Klientská část: přijímá data ze serveru a otevírá menu.

class OS_Client
{
	protected static ref OS_Client s_Instance;

	protected ref OS_SpawnMenu m_SpawnMenu;
	protected ref OS_AdminMenu m_AdminMenu;
	protected ref OS_TraderMenu m_TraderMenu;
	protected ref OS_VehicleMenu m_VehicleMenu;
	protected ref OS_AtmMenu m_AtmMenu;
	protected ref OS_GroupMenu m_GroupMenu;
	protected ref OS_NameTags m_NameTags = new OS_NameTags();

	static OS_Client Get()
	{
		return s_Instance;
	}

	static void Start()
	{
		if (!s_Instance)
			s_Instance = new OS_Client();
	}

	void OS_Client()
	{
		DayZGame.Event_OnRPC.Insert(OnRPC);
	}

	//! každý snímek z MissionGameplay
	void OnFrame()
	{
		m_NameTags.Update();
	}

	void ~OS_Client()
	{
		if (m_NameTags)
			m_NameTags.Destroy();
		DayZGame.Event_OnRPC.Remove(OnRPC);
	}

	static void SendToServer(int rpcType)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(null, rpcType, true, null);
	}

	protected bool IsOpen(UIScriptedMenu menu)
	{
		return menu && g_Game.GetUIManager().GetMenu() == menu;
	}

	protected void Show(UIScriptedMenu menu)
	{
		if (g_Game.GetUIManager().GetMenu())
			g_Game.GetUIManager().CloseAll();
		g_Game.GetUIManager().ShowScriptedMenu(menu, null);
	}

	void RequestTrader()
	{
		SendToServer(OS_RPC_TRADER_OPEN);
	}

	void RequestGroup()
	{
		SendToServer(OS_RPC_GROUP_OPEN);
	}

	void RequestAdmin()
	{
		SendToServer(OS_RPC_ADMIN_OPEN);
	}

	void OnRPC(PlayerIdentity sender, Object target, int rpcType, ParamsReadContext ctx)
	{
		if (g_Game.IsDedicatedServer())
			return;

		switch (rpcType)
		{
			case OS_RPC_SPAWN_MENU:
				OnSpawnMenu(ctx);
				break;
			case OS_RPC_ADMIN_DATA:
				OnAdminData(ctx);
				break;
			case OS_RPC_TRADER_DATA:
				OnTraderData(ctx);
				break;
			case OS_RPC_TRADER_ITEMS:
				OnTraderItems(ctx);
				break;
			case OS_RPC_TRADER_UPDATE:
				OnTraderUpdate(ctx);
				break;
			case OS_RPC_VEHICLE_DATA:
				OnVehicleData(ctx);
				break;
			case OS_RPC_BANK_DATA:
				OnBankData(ctx);
				break;
			case OS_RPC_SETTINGS:
				OnSettings(ctx);
				break;
			case OS_RPC_GROUP_DATA:
				OnGroupData(ctx);
				break;
			case OS_RPC_GROUP_POSITIONS:
				OS_ClientGroup.Read(ctx);
				break;
			case OS_RPC_GROUP_ID:
				OnGroupId(ctx);
				break;
		}
	}

	protected void OnSpawnMenu(ParamsReadContext ctx)
	{
		array<string> zones = new array<string>;
		int timeout;
		if (!ctx.Read(zones) || !ctx.Read(timeout))
			return;

		// postava ještě není načtená - server to zkusí znovu
		if (!g_Game.GetPlayer())
			return;

		if (!IsOpen(m_SpawnMenu))
		{
			m_SpawnMenu = new OS_SpawnMenu();
			m_SpawnMenu.SetData(zones, timeout);
			Show(m_SpawnMenu);
		}

		SendToServer(OS_RPC_SPAWN_ACK);
	}

	protected void OnAdminData(ParamsReadContext ctx)
	{
		array<string> names = new array<string>;
		array<string> ids = new array<string>;
		bool god;
		if (!ctx.Read(names) || !ctx.Read(ids) || !ctx.Read(god))
			return;

		if (IsOpen(m_AdminMenu))
		{
			m_AdminMenu.SetData(names, ids, god);
			return;
		}

		m_AdminMenu = new OS_AdminMenu();
		m_AdminMenu.SetData(names, ids, god);
		Show(m_AdminMenu);
	}

	protected void OnTraderData(ParamsReadContext ctx)
	{
		string traderName;
		string currency;
		bool isAdmin;
		array<string> categories = new array<string>;
		array<int> counts = new array<int>;
		array<int> categoryIndexes = new array<int>;
		if (!ctx.Read(traderName) || !ctx.Read(currency) || !ctx.Read(isAdmin) || !ctx.Read(categories) || !ctx.Read(counts) || !ctx.Read(categoryIndexes))
			return;

		if (IsOpen(m_TraderMenu))
		{
			m_TraderMenu.SetHeader(traderName, currency, isAdmin, categories, counts, categoryIndexes);
			return;
		}

		m_TraderMenu = new OS_TraderMenu();
		m_TraderMenu.SetHeader(traderName, currency, isAdmin, categories, counts, categoryIndexes);
		Show(m_TraderMenu);
	}

	protected void OnTraderItems(ParamsReadContext ctx)
	{
		int viewCategory;
		array<int> cats = new array<int>;
		array<int> indexes = new array<int>;
		array<string> classes = new array<string>;
		array<int> buys = new array<int>;
		array<int> sells = new array<int>;
		if (!ctx.Read(viewCategory) || !ctx.Read(cats) || !ctx.Read(indexes) || !ctx.Read(classes) || !ctx.Read(buys) || !ctx.Read(sells))
			return;

		if (m_TraderMenu)
			m_TraderMenu.SetItems(viewCategory, cats, indexes, classes, buys, sells);
	}

	protected void OnTraderUpdate(ParamsReadContext ctx)
	{
		int balance;
		int cash;
		int bank;
		array<string> classes = new array<string>;
		array<int> counts = new array<int>;
		array<int> prices = new array<int>;
		if (!ctx.Read(balance) || !ctx.Read(cash) || !ctx.Read(bank) || !ctx.Read(classes) || !ctx.Read(counts) || !ctx.Read(prices))
			return;

		if (m_TraderMenu)
			m_TraderMenu.SetPlayerState(balance, cash, bank, classes, counts, prices);
	}

	protected void OnVehicleData(ParamsReadContext ctx)
	{
		OS_VehicleMenuData data = new OS_VehicleMenuData();
		if (!data.Read(ctx))
			return;

		if (IsOpen(m_VehicleMenu))
		{
			m_VehicleMenu.SetData(data);
			return;
		}

		m_VehicleMenu = new OS_VehicleMenu();
		m_VehicleMenu.SetData(data);
		Show(m_VehicleMenu);
	}

	protected void OnGroupData(ParamsReadContext ctx)
	{
		OS_GroupData data = new OS_GroupData();
		if (!data.Read(ctx))
			return;

		if (IsOpen(m_GroupMenu))
		{
			m_GroupMenu.SetData(data);
			return;
		}
		m_GroupMenu = new OS_GroupMenu();
		m_GroupMenu.SetData(data);
		Show(m_GroupMenu);
	}

	protected void OnGroupId(ParamsReadContext ctx)
	{
		int groupId;
		if (ctx.Read(groupId))
			OS_ClientSettings.GroupId = groupId;
	}

	protected void OnSettings(ParamsReadContext ctx)
	{
		string currency;
		if (ctx.Read(currency))
			OS_ClientSettings.Currency = currency;
	}

	protected void OnBankData(ParamsReadContext ctx)
	{
		OS_AtmData data = new OS_AtmData();
		if (!data.Read(ctx))
			return;

		if (!IsOpen(m_AtmMenu))
		{
			m_AtmMenu = new OS_AtmMenu();
			m_AtmMenu.SetData(data);
			Show(m_AtmMenu);
			return;
		}
		m_AtmMenu.SetData(data);
	}

	//! Čitelný název předmětu z configu hry
	static string DisplayName(string className)
	{
		string name;
		if (g_Game.ConfigGetText("CfgVehicles " + className + " displayName", name) && name != "")
			return name;
		if (g_Game.ConfigGetText("CfgWeapons " + className + " displayName", name) && name != "")
			return name;
		if (g_Game.ConfigGetText("CfgMagazines " + className + " displayName", name) && name != "")
			return name;
		return className;
	}
}
