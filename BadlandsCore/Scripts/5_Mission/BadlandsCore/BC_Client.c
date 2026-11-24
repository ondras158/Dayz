// Klientská část: přijímá data ze serveru a otevírá menu.

class BC_Client
{
	protected static ref BC_Client s_Instance;

	protected ref BC_SpawnMenu m_SpawnMenu;
	protected ref BC_AdminMenu m_AdminMenu;
	protected ref BC_TraderMenu m_TraderMenu;
	protected ref BC_VehicleMenu m_VehicleMenu;

	static BC_Client Get()
	{
		return s_Instance;
	}

	static void Start()
	{
		if (!s_Instance)
			s_Instance = new BC_Client();
	}

	void BC_Client()
	{
		DayZGame.Event_OnRPC.Insert(OnRPC);
	}

	void ~BC_Client()
	{
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
		SendToServer(BC_RPC_TRADER_OPEN);
	}

	void RequestAdmin()
	{
		SendToServer(BC_RPC_ADMIN_OPEN);
	}

	void OnRPC(PlayerIdentity sender, Object target, int rpcType, ParamsReadContext ctx)
	{
		if (g_Game.IsDedicatedServer())
			return;

		switch (rpcType)
		{
			case BC_RPC_SPAWN_MENU:
				OnSpawnMenu(ctx);
				break;
			case BC_RPC_ADMIN_DATA:
				OnAdminData(ctx);
				break;
			case BC_RPC_TRADER_DATA:
				OnTraderData(ctx);
				break;
			case BC_RPC_TRADER_UPDATE:
				OnTraderUpdate(ctx);
				break;
			case BC_RPC_VEHICLE_DATA:
				OnVehicleData(ctx);
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
			m_SpawnMenu = new BC_SpawnMenu();
			m_SpawnMenu.SetData(zones, timeout);
			Show(m_SpawnMenu);
		}

		SendToServer(BC_RPC_SPAWN_ACK);
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

		m_AdminMenu = new BC_AdminMenu();
		m_AdminMenu.SetData(names, ids, god);
		Show(m_AdminMenu);
	}

	protected void OnTraderData(ParamsReadContext ctx)
	{
		string traderName;
		string currency;
		array<string> categories = new array<string>;
		array<int> itemCategories = new array<int>;
		array<string> itemClasses = new array<string>;
		array<int> itemBuy = new array<int>;
		array<int> itemSell = new array<int>;

		if (!ctx.Read(traderName) || !ctx.Read(currency) || !ctx.Read(categories) || !ctx.Read(itemCategories))
			return;
		if (!ctx.Read(itemClasses) || !ctx.Read(itemBuy) || !ctx.Read(itemSell))
			return;

		if (!IsOpen(m_TraderMenu))
		{
			m_TraderMenu = new BC_TraderMenu();
			m_TraderMenu.SetCatalog(traderName, currency, categories, itemCategories, itemClasses, itemBuy, itemSell);
			Show(m_TraderMenu);
		}
		else
		{
			m_TraderMenu.SetCatalog(traderName, currency, categories, itemCategories, itemClasses, itemBuy, itemSell);
		}
	}

	protected void OnTraderUpdate(ParamsReadContext ctx)
	{
		int balance;
		array<string> classes = new array<string>;
		array<int> counts = new array<int>;
		if (!ctx.Read(balance) || !ctx.Read(classes) || !ctx.Read(counts))
			return;

		if (m_TraderMenu)
			m_TraderMenu.SetPlayerState(balance, classes, counts);
	}

	protected void OnVehicleData(ParamsReadContext ctx)
	{
		BC_VehicleMenuData data = new BC_VehicleMenuData();
		if (!data.Read(ctx))
			return;

		if (IsOpen(m_VehicleMenu))
		{
			m_VehicleMenu.SetData(data);
			return;
		}

		m_VehicleMenu = new BC_VehicleMenu();
		m_VehicleMenu.SetData(data);
		Show(m_VehicleMenu);
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
