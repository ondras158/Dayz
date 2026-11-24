// Obchod s virtuální měnou. Katalog se automaticky doplňuje o všechny předměty ve hře,
// admin může ceny měnit přímo v menu obchodu.

class OS_ServerTrader
{
	protected static const int MAX_LIST_ITEMS = 1500;
	protected static const int MAX_SEARCH_ITEMS = 300;

	ref OS_TraderConfig m_Cfg;
	protected OS_Payments m_Payments;
	protected OS_ServerAdmin m_Admin;
	protected string m_Currency;
	protected ref map<string, OS_TraderItem> m_ByClass = new map<string, OS_TraderItem>;          // malými písmeny
	protected ref map<string, OS_TraderItem> m_InheritedCache = new map<string, OS_TraderItem>;

	void OS_ServerTrader(OS_Payments payments, OS_TraderConfig cfg, OS_ServerAdmin admin, string currency)
	{
		m_Currency = currency;
		m_Payments = payments;
		m_Cfg = cfg;
		m_Admin = admin;

		if (m_Cfg.AutoAddItems && OS_TraderScanner.AddNewItems(m_Cfg) > 0)
			Save();

		RebuildIndex();
	}

	static OS_TraderConfig LoadConfig()
	{
		OS_TraderConfig defaults = new OS_TraderConfig();
		defaults.FillDefaults();
		return OS_ConfigFile<OS_TraderConfig>.Load("Trader.json", defaults, new OS_TraderConfig());
	}

	protected void Save()
	{
		string err;
		if (!JsonFileLoader<OS_TraderConfig>.SaveFile(OS_DIR + "Trader.json", m_Cfg, err))
			Print(OS_TAG + "CHYBA pri ukladani Trader.json: " + err);
	}

	protected void RebuildIndex()
	{
		m_ByClass.Clear();
		m_InheritedCache.Clear();

		foreach (OS_TraderCategory category : m_Cfg.Categories)
		{
			if (!category || !category.Items)
				continue;

			foreach (OS_TraderItem item : category.Items)
			{
				if (item)
					m_ByClass.Set(OS_TraderScanner.Lower(item.ClassName), item);
			}
		}
	}

	//! Přesná shoda classname, jinak varianta (např. M4A1_Black -> M4A1)
	protected OS_TraderItem FindPriced(string className)
	{
		string key = OS_TraderScanner.Lower(className);

		OS_TraderItem item;
		if (m_ByClass.Find(key, item))
			return item;
		if (m_InheritedCache.Find(key, item))
			return item;

		item = null;
		foreach (string pricedKey, OS_TraderItem priced : m_ByClass)
		{
			if (priced.Sell >= 0 && g_Game.IsKindOf(className, priced.ClassName))
			{
				item = priced;
				break;
			}
		}

		m_InheritedCache.Set(key, item);
		return item;
	}

	protected static bool IsHidden(OS_TraderItem item)
	{
		return item.Buy < 0 && item.Sell < 0;
	}

	OS_TraderLocation NearTrader(PlayerBase player)
	{
		if (!player || !m_Cfg.Traders)
			return null;

		foreach (OS_TraderLocation trader : m_Cfg.Traders)
		{
			if (trader && OS_Util.Distance2D(player.GetPosition(), trader.X, trader.Z) <= trader.Radius)
				return trader;
		}
		return null;
	}

	// ------------------------------------------------------------ odměny

	void RewardZombieKill(PlayerBase killer)
	{
		if (m_Cfg.Enabled && killer && m_Cfg.ZombieKillReward != 0)
			m_Payments.Pay(killer, m_Cfg.ZombieKillReward);
	}

	void RewardPlayerKill(PlayerBase killer)
	{
		if (m_Cfg.Enabled && killer && m_Cfg.PlayerKillReward != 0)
		{
			m_Payments.Pay(killer, m_Cfg.PlayerKillReward);
			OS_Util.Notify(killer, "Odmena", "+" + m_Cfg.PlayerKillReward.ToString() + " " + m_Currency);
		}
	}

	// ------------------------------------------------------------ RPC

	protected bool IsTraderRpc(int rpcType)
	{
		return rpcType == OS_RPC_TRADER_OPEN || rpcType == OS_RPC_TRADER_BUY || rpcType == OS_RPC_TRADER_SELL
			|| rpcType == OS_RPC_TRADER_CATEGORY || rpcType == OS_RPC_TRADER_SEARCH
			|| rpcType == OS_RPC_TRADER_ADMIN_EDIT || rpcType == OS_RPC_TRADER_ADMIN_BULK;
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (!IsTraderRpc(rpcType))
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive() || !m_Cfg.Enabled)
			return true;

		OS_TraderLocation trader = NearTrader(player);
		if (!trader)
		{
			OS_Util.Notify(player, "Obchod", "Musis stat u obchodnika, bankomatu, autobazaru, servisu nebo garaze.");
			return true;
		}

		bool isAdmin = m_Admin && m_Admin.IsAdmin(sender);

		if (rpcType == OS_RPC_TRADER_OPEN)
		{
			SendHeader(player, trader, isAdmin);
			SendUpdate(player);
		}
		else if (rpcType == OS_RPC_TRADER_CATEGORY)
		{
			int viewCategory;
			if (ctx.Read(viewCategory))
				SendItems(player, viewCategory, "", isAdmin);
		}
		else if (rpcType == OS_RPC_TRADER_SEARCH)
		{
			string searchText;
			if (ctx.Read(searchText))
				SendItems(player, -1, searchText, isAdmin);
		}
		else if (rpcType == OS_RPC_TRADER_BUY)
		{
			int buyCategory;
			int buyIndex;
			if (ctx.Read(buyCategory) && ctx.Read(buyIndex))
				Buy(player, buyCategory, buyIndex);
			SendUpdate(player);
		}
		else if (rpcType == OS_RPC_TRADER_SELL)
		{
			string className;
			if (ctx.Read(className))
				Sell(player, className);
			SendUpdate(player);
		}
		else if (rpcType == OS_RPC_TRADER_ADMIN_EDIT)
		{
			if (isAdmin)
				AdminEdit(player, ctx);
		}
		else if (rpcType == OS_RPC_TRADER_ADMIN_BULK)
		{
			if (isAdmin)
				AdminBulk(player, ctx);
		}
		return true;
	}

	protected OS_TraderItem GetItem(int categoryIndex, int itemIndex)
	{
		if (categoryIndex < 0 || categoryIndex >= m_Cfg.Categories.Count())
			return null;

		OS_TraderCategory category = m_Cfg.Categories[categoryIndex];
		if (!category || !category.Items || itemIndex < 0 || itemIndex >= category.Items.Count())
			return null;

		return category.Items[itemIndex];
	}

	// ------------------------------------------------------------ nákup / prodej

	protected void Buy(PlayerBase player, int categoryIndex, int itemIndex)
	{
		OS_TraderItem priced = GetItem(categoryIndex, itemIndex);
		if (!priced || priced.Buy < 0)
			return;

		string steamId = OS_Util.SteamId(player);
		if (!m_Payments.TrySpend(player, priced.Buy))
		{
			OS_Util.Notify(player, "Obchod", "Nemas dost penez.");
			return;
		}

		EntityAI created = OS_Util.GiveItem(player, priced.ClassName);
		if (!created)
		{
			m_Payments.Pay(player, priced.Buy); // vrátit peníze
			OS_Util.Notify(player, "Obchod", "Predmet se nepodarilo vytvorit.");
			return;
		}

		// krabičky nábojů vždy plné
		Magazine pile = Magazine.Cast(created);
		if (pile && created.IsAmmoPile())
			pile.ServerSetAmmoCount(pile.GetAmmoMax());

		Print(OS_TAG + "OBCHOD " + OS_Util.PlayerName(player) + " koupil " + priced.ClassName + " za " + priced.Buy.ToString());
	}

	protected void Sell(PlayerBase player, string className)
	{
		OS_TraderItem priced = FindPriced(className);
		if (!priced || priced.Sell < 0)
			return;

		ItemBase item = FindSellableItem(player, className);
		if (!item)
		{
			OS_Util.Notify(player, "Obchod", "Predmet nelze prodat. Musi byt prazdny, bez prislusenstvi (zbran bez zasobniku) a nesmi byt zniceny.");
			return;
		}

		int price = Math.Round(priced.Sell * SellFraction(item));
		if (price <= 0)
		{
			OS_Util.Notify(player, "Obchod", "Tento kus uz nema zadnou hodnotu.");
			return;
		}

		g_Game.ObjectDelete(item);
		m_Payments.Pay(player, price);
		Print(OS_TAG + "OBCHOD " + OS_Util.PlayerName(player) + " prodal " + className + " za " + price.ToString());
	}

	//! Částečně použité věci (načatá krabička nábojů, snědené jídlo) a opotřebované věci mají nižší cenu
	protected float SellFraction(ItemBase item)
	{
		float fraction = 1;

		if (item.IsAmmoPile())
		{
			Magazine pile = Magazine.Cast(item);
			if (pile && pile.GetAmmoMax() > 0)
			{
				float ammoCount = pile.GetAmmoCount();
				float ammoMax = pile.GetAmmoMax();
				fraction = ammoCount / ammoMax;
			}
		}
		else if (!Magazine.Cast(item) && !item.IsLiquidContainer() && item.HasQuantity())
		{
			fraction = item.GetQuantityNormalized();
		}

		if (m_Cfg.ScaleSellPriceByCondition)
			fraction = fraction * (0.5 + 0.5 * item.GetHealth01("", ""));

		return fraction;
	}

	protected bool CanSellItem(ItemBase item)
	{
		if (!item || item.IsRuined())
			return false;

		// peníze a klíče módu se neprodávají
		string type = item.GetType();
		if (type.IndexOf("OS_") == 0)
			return false;

		OS_TraderItem priced = FindPriced(type);
		if (!priced || priced.Sell < 0)
			return false;

		if (item.GetInventory().AttachmentCount() > 0)
			return false;

		CargoBase cargo = item.GetInventory().GetCargo();
		if (cargo && cargo.GetItemCount() > 0)
			return false;

		return true;
	}

	protected ItemBase FindSellableItem(PlayerBase player, string className)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI ent : items)
		{
			ItemBase item = ItemBase.Cast(ent);
			if (item && item.GetType() == className && CanSellItem(item))
				return item;
		}
		return null;
	}

	// ------------------------------------------------------------ admin úpravy

	protected void AdminEdit(PlayerBase admin, ParamsReadContext ctx)
	{
		int categoryIndex;
		int itemIndex;
		int buy;
		int sell;
		if (!ctx.Read(categoryIndex) || !ctx.Read(itemIndex) || !ctx.Read(buy) || !ctx.Read(sell))
			return;

		OS_TraderItem item = GetItem(categoryIndex, itemIndex);
		if (!item)
			return;

		if (buy < -1)
			buy = -1;
		if (sell < -1)
			sell = -1;

		Print(OS_TAG + "ADMIN " + OS_Util.PlayerName(admin) + " cena " + item.ClassName + ": nakup " + item.Buy.ToString() + "->" + buy.ToString() + ", vykup " + item.Sell.ToString() + "->" + sell.ToString());
		item.Buy = buy;
		item.Sell = sell;
		m_InheritedCache.Clear();
		Save();
		OS_Util.Notify(admin, "Obchod", "Cena ulozena: " + item.ClassName);
	}

	protected void AdminBulk(PlayerBase admin, ParamsReadContext ctx)
	{
		int categoryIndex;
		float factor;
		if (!ctx.Read(categoryIndex) || !ctx.Read(factor))
			return;

		if (categoryIndex < 0 || categoryIndex >= m_Cfg.Categories.Count() || factor <= 0 || factor > 100)
		{
			OS_Util.Notify(admin, "Obchod", "Nasobitel musi byt mezi 0 a 100 (napr. 1.5 nebo 0.8).");
			return;
		}

		OS_TraderCategory category = m_Cfg.Categories[categoryIndex];
		foreach (OS_TraderItem item : category.Items)
		{
			if (item.Buy > 0)
				item.Buy = Math.Max(1, Math.Round(item.Buy * factor));
			if (item.Sell > 0)
				item.Sell = Math.Max(1, Math.Round(item.Sell * factor));
		}

		Save();
		Print(OS_TAG + "ADMIN " + OS_Util.PlayerName(admin) + " nasobil ceny kategorie " + category.Name + " x" + factor.ToString());
		OS_Util.Notify(admin, "Obchod", "Ceny kategorie " + category.Name + " vynasobeny x" + factor.ToString());
	}

	// ------------------------------------------------------------ odesílání dat

	protected void SendHeader(PlayerBase player, OS_TraderLocation trader, bool isAdmin)
	{
		array<string> names = new array<string>;
		array<int> counts = new array<int>;

		foreach (OS_TraderCategory category : m_Cfg.Categories)
		{
			int visible = 0;
			if (category && category.Items)
			{
				foreach (OS_TraderItem item : category.Items)
				{
					if (isAdmin || !IsHidden(item))
						visible++;
				}
			}

			string name = "?";
			if (category)
				name = category.Name;
			names.Insert(name);
			counts.Insert(visible);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(trader.Name);
		rpc.Write(m_Currency);
		rpc.Write(isAdmin);
		rpc.Write(names);
		rpc.Write(counts);
		rpc.Send(null, OS_RPC_TRADER_DATA, true, player.GetIdentity());
	}

	//! categoryIndex >= 0: obsah kategorie, -1: výsledky hledání
	protected void SendItems(PlayerBase player, int categoryIndex, string searchText, bool isAdmin)
	{
		array<int> cats = new array<int>;
		array<int> indexes = new array<int>;
		array<string> classes = new array<string>;
		array<int> buys = new array<int>;
		array<int> sells = new array<int>;

		string needle = OS_TraderScanner.Lower(searchText.Trim());
		int limit = MAX_LIST_ITEMS;
		if (categoryIndex < 0)
			limit = MAX_SEARCH_ITEMS;

		for (int c = 0; c < m_Cfg.Categories.Count() && classes.Count() < limit; c++)
		{
			if (categoryIndex >= 0 && c != categoryIndex)
				continue;

			OS_TraderCategory category = m_Cfg.Categories[c];
			if (!category || !category.Items)
				continue;

			for (int i = 0; i < category.Items.Count() && classes.Count() < limit; i++)
			{
				OS_TraderItem item = category.Items[i];
				if (!item || (!isAdmin && IsHidden(item)))
					continue;
				if (categoryIndex < 0)
				{
					string lowerClass = OS_TraderScanner.Lower(item.ClassName);
					if (needle == "" || !lowerClass.Contains(needle))
						continue;
				}

				cats.Insert(c);
				indexes.Insert(i);
				classes.Insert(item.ClassName);
				buys.Insert(item.Buy);
				sells.Insert(item.Sell);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(categoryIndex);
		rpc.Write(cats);
		rpc.Write(indexes);
		rpc.Write(classes);
		rpc.Write(buys);
		rpc.Write(sells);
		rpc.Send(null, OS_RPC_TRADER_ITEMS, true, player.GetIdentity());
	}

	protected void SendUpdate(PlayerBase player)
	{
		array<string> classes = new array<string>;
		array<int> counts = new array<int>;
		array<int> prices = new array<int>;

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI ent : items)
		{
			ItemBase item = ItemBase.Cast(ent);
			if (!item || !CanSellItem(item))
				continue;

			string type = item.GetType();
			int index = classes.Find(type);
			if (index == -1)
			{
				classes.Insert(type);
				counts.Insert(1);
				prices.Insert(FindPriced(type).Sell);
			}
			else
			{
				counts[index] = counts[index] + 1;
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Payments.Available(player));
		rpc.Write(m_Payments.Cash(player));
		rpc.Write(m_Payments.Bank(player));
		rpc.Write(classes);
		rpc.Write(counts);
		rpc.Write(prices);
		rpc.Send(null, OS_RPC_TRADER_UPDATE, true, player.GetIdentity());
	}
}
