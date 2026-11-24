// Obchod s virtuální měnou. Katalog se automaticky doplňuje o všechny předměty ve hře,
// admin může ceny měnit přímo v menu obchodu.

class BC_ServerTrader
{
	protected static const int MAX_LIST_ITEMS = 1500;
	protected static const int MAX_SEARCH_ITEMS = 300;

	ref BC_TraderConfig m_Cfg;
	protected BC_Wallets m_Wallets;
	protected BC_ServerAdmin m_Admin;
	protected ref map<string, BC_TraderItem> m_ByClass = new map<string, BC_TraderItem>;          // malými písmeny
	protected ref map<string, BC_TraderItem> m_InheritedCache = new map<string, BC_TraderItem>;

	void BC_ServerTrader(BC_Wallets wallets, BC_TraderConfig cfg, BC_ServerAdmin admin)
	{
		m_Wallets = wallets;
		m_Cfg = cfg;
		m_Admin = admin;

		if (m_Cfg.AutoAddItems && BC_TraderScanner.AddNewItems(m_Cfg) > 0)
			Save();

		RebuildIndex();
	}

	static BC_TraderConfig LoadConfig()
	{
		BC_TraderConfig defaults = new BC_TraderConfig();
		defaults.FillDefaults();
		return BC_ConfigFile<BC_TraderConfig>.Load("Trader.json", defaults, new BC_TraderConfig());
	}

	protected void Save()
	{
		string err;
		if (!JsonFileLoader<BC_TraderConfig>.SaveFile(BC_DIR + "Trader.json", m_Cfg, err))
			Print(BC_TAG + "CHYBA pri ukladani Trader.json: " + err);
	}

	protected void RebuildIndex()
	{
		m_ByClass.Clear();
		m_InheritedCache.Clear();

		foreach (BC_TraderCategory category : m_Cfg.Categories)
		{
			if (!category || !category.Items)
				continue;

			foreach (BC_TraderItem item : category.Items)
			{
				if (item)
					m_ByClass.Set(BC_TraderScanner.Lower(item.ClassName), item);
			}
		}
	}

	//! Přesná shoda classname, jinak varianta (např. M4A1_Black -> M4A1)
	protected BC_TraderItem FindPriced(string className)
	{
		string key = BC_TraderScanner.Lower(className);

		BC_TraderItem item;
		if (m_ByClass.Find(key, item))
			return item;
		if (m_InheritedCache.Find(key, item))
			return item;

		item = null;
		foreach (string pricedKey, BC_TraderItem priced : m_ByClass)
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

	protected static bool IsHidden(BC_TraderItem item)
	{
		return item.Buy < 0 && item.Sell < 0;
	}

	BC_TraderLocation NearTrader(PlayerBase player)
	{
		if (!player || !m_Cfg.Traders)
			return null;

		foreach (BC_TraderLocation trader : m_Cfg.Traders)
		{
			if (trader && BC_Util.Distance2D(player.GetPosition(), trader.X, trader.Z) <= trader.Radius)
				return trader;
		}
		return null;
	}

	// ------------------------------------------------------------ odměny

	void RewardZombieKill(PlayerBase killer)
	{
		if (m_Cfg.Enabled && killer && m_Cfg.ZombieKillReward != 0)
			m_Wallets.Add(BC_Util.SteamId(killer), m_Cfg.ZombieKillReward);
	}

	void RewardPlayerKill(PlayerBase killer)
	{
		if (m_Cfg.Enabled && killer && m_Cfg.PlayerKillReward != 0)
		{
			m_Wallets.Add(BC_Util.SteamId(killer), m_Cfg.PlayerKillReward);
			BC_Util.Notify(killer, "Odmena", "+" + m_Cfg.PlayerKillReward.ToString() + " " + m_Cfg.CurrencyName);
		}
	}

	// ------------------------------------------------------------ RPC

	protected bool IsTraderRpc(int rpcType)
	{
		return rpcType == BC_RPC_TRADER_OPEN || rpcType == BC_RPC_TRADER_BUY || rpcType == BC_RPC_TRADER_SELL
			|| rpcType == BC_RPC_TRADER_CATEGORY || rpcType == BC_RPC_TRADER_SEARCH
			|| rpcType == BC_RPC_TRADER_ADMIN_EDIT || rpcType == BC_RPC_TRADER_ADMIN_BULK;
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (!IsTraderRpc(rpcType))
			return false;

		PlayerBase player = BC_Util.FindPlayer(sender);
		if (!player || !player.IsAlive() || !m_Cfg.Enabled)
			return true;

		BC_TraderLocation trader = NearTrader(player);
		if (!trader)
		{
			BC_Util.Notify(player, "Obchod", "Musis stat u obchodnika, autobazaru, servisu nebo garaze.");
			return true;
		}

		bool isAdmin = m_Admin && m_Admin.IsAdmin(sender);

		if (rpcType == BC_RPC_TRADER_OPEN)
		{
			SendHeader(player, trader, isAdmin);
			SendUpdate(player);
		}
		else if (rpcType == BC_RPC_TRADER_CATEGORY)
		{
			int viewCategory;
			if (ctx.Read(viewCategory))
				SendItems(player, viewCategory, "", isAdmin);
		}
		else if (rpcType == BC_RPC_TRADER_SEARCH)
		{
			string searchText;
			if (ctx.Read(searchText))
				SendItems(player, -1, searchText, isAdmin);
		}
		else if (rpcType == BC_RPC_TRADER_BUY)
		{
			int buyCategory;
			int buyIndex;
			if (ctx.Read(buyCategory) && ctx.Read(buyIndex))
				Buy(player, buyCategory, buyIndex);
			SendUpdate(player);
		}
		else if (rpcType == BC_RPC_TRADER_SELL)
		{
			string className;
			if (ctx.Read(className))
				Sell(player, className);
			SendUpdate(player);
		}
		else if (rpcType == BC_RPC_TRADER_ADMIN_EDIT)
		{
			if (isAdmin)
				AdminEdit(player, ctx);
		}
		else if (rpcType == BC_RPC_TRADER_ADMIN_BULK)
		{
			if (isAdmin)
				AdminBulk(player, ctx);
		}
		return true;
	}

	protected BC_TraderItem GetItem(int categoryIndex, int itemIndex)
	{
		if (categoryIndex < 0 || categoryIndex >= m_Cfg.Categories.Count())
			return null;

		BC_TraderCategory category = m_Cfg.Categories[categoryIndex];
		if (!category || !category.Items || itemIndex < 0 || itemIndex >= category.Items.Count())
			return null;

		return category.Items[itemIndex];
	}

	// ------------------------------------------------------------ nákup / prodej

	protected void Buy(PlayerBase player, int categoryIndex, int itemIndex)
	{
		BC_TraderItem priced = GetItem(categoryIndex, itemIndex);
		if (!priced || priced.Buy < 0)
			return;

		string steamId = BC_Util.SteamId(player);
		if (!m_Wallets.TrySpend(steamId, priced.Buy))
		{
			BC_Util.Notify(player, "Obchod", "Nemas dost penez.");
			return;
		}

		EntityAI created = BC_Util.GiveItem(player, priced.ClassName);
		if (!created)
		{
			m_Wallets.Add(steamId, priced.Buy); // vrátit peníze
			BC_Util.Notify(player, "Obchod", "Predmet se nepodarilo vytvorit.");
			return;
		}

		// krabičky nábojů vždy plné
		Magazine pile = Magazine.Cast(created);
		if (pile && created.IsAmmoPile())
			pile.ServerSetAmmoCount(pile.GetAmmoMax());

		Print(BC_TAG + "OBCHOD " + BC_Util.PlayerName(player) + " koupil " + priced.ClassName + " za " + priced.Buy.ToString());
	}

	protected void Sell(PlayerBase player, string className)
	{
		BC_TraderItem priced = FindPriced(className);
		if (!priced || priced.Sell < 0)
			return;

		ItemBase item = FindSellableItem(player, className);
		if (!item)
		{
			BC_Util.Notify(player, "Obchod", "Predmet nelze prodat. Musi byt prazdny, bez prislusenstvi (zbran bez zasobniku) a nesmi byt zniceny.");
			return;
		}

		int price = Math.Round(priced.Sell * SellFraction(item));
		if (price <= 0)
		{
			BC_Util.Notify(player, "Obchod", "Tento kus uz nema zadnou hodnotu.");
			return;
		}

		g_Game.ObjectDelete(item);
		m_Wallets.Add(BC_Util.SteamId(player), price);
		Print(BC_TAG + "OBCHOD " + BC_Util.PlayerName(player) + " prodal " + className + " za " + price.ToString());
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

		BC_TraderItem priced = FindPriced(item.GetType());
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

		BC_TraderItem item = GetItem(categoryIndex, itemIndex);
		if (!item)
			return;

		if (buy < -1)
			buy = -1;
		if (sell < -1)
			sell = -1;

		Print(BC_TAG + "ADMIN " + BC_Util.PlayerName(admin) + " cena " + item.ClassName + ": nakup " + item.Buy.ToString() + "->" + buy.ToString() + ", vykup " + item.Sell.ToString() + "->" + sell.ToString());
		item.Buy = buy;
		item.Sell = sell;
		m_InheritedCache.Clear();
		Save();
		BC_Util.Notify(admin, "Obchod", "Cena ulozena: " + item.ClassName);
	}

	protected void AdminBulk(PlayerBase admin, ParamsReadContext ctx)
	{
		int categoryIndex;
		float factor;
		if (!ctx.Read(categoryIndex) || !ctx.Read(factor))
			return;

		if (categoryIndex < 0 || categoryIndex >= m_Cfg.Categories.Count() || factor <= 0 || factor > 100)
		{
			BC_Util.Notify(admin, "Obchod", "Nasobitel musi byt mezi 0 a 100 (napr. 1.5 nebo 0.8).");
			return;
		}

		BC_TraderCategory category = m_Cfg.Categories[categoryIndex];
		foreach (BC_TraderItem item : category.Items)
		{
			if (item.Buy > 0)
				item.Buy = Math.Max(1, Math.Round(item.Buy * factor));
			if (item.Sell > 0)
				item.Sell = Math.Max(1, Math.Round(item.Sell * factor));
		}

		Save();
		Print(BC_TAG + "ADMIN " + BC_Util.PlayerName(admin) + " nasobil ceny kategorie " + category.Name + " x" + factor.ToString());
		BC_Util.Notify(admin, "Obchod", "Ceny kategorie " + category.Name + " vynasobeny x" + factor.ToString());
	}

	// ------------------------------------------------------------ odesílání dat

	protected void SendHeader(PlayerBase player, BC_TraderLocation trader, bool isAdmin)
	{
		array<string> names = new array<string>;
		array<int> counts = new array<int>;

		foreach (BC_TraderCategory category : m_Cfg.Categories)
		{
			int visible = 0;
			if (category && category.Items)
			{
				foreach (BC_TraderItem item : category.Items)
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
		rpc.Write(m_Cfg.CurrencyName);
		rpc.Write(isAdmin);
		rpc.Write(names);
		rpc.Write(counts);
		rpc.Send(null, BC_RPC_TRADER_DATA, true, player.GetIdentity());
	}

	//! categoryIndex >= 0: obsah kategorie, -1: výsledky hledání
	protected void SendItems(PlayerBase player, int categoryIndex, string searchText, bool isAdmin)
	{
		array<int> cats = new array<int>;
		array<int> indexes = new array<int>;
		array<string> classes = new array<string>;
		array<int> buys = new array<int>;
		array<int> sells = new array<int>;

		string needle = BC_TraderScanner.Lower(searchText.Trim());
		int limit = MAX_LIST_ITEMS;
		if (categoryIndex < 0)
			limit = MAX_SEARCH_ITEMS;

		for (int c = 0; c < m_Cfg.Categories.Count() && classes.Count() < limit; c++)
		{
			if (categoryIndex >= 0 && c != categoryIndex)
				continue;

			BC_TraderCategory category = m_Cfg.Categories[c];
			if (!category || !category.Items)
				continue;

			for (int i = 0; i < category.Items.Count() && classes.Count() < limit; i++)
			{
				BC_TraderItem item = category.Items[i];
				if (!item || (!isAdmin && IsHidden(item)))
					continue;
				if (categoryIndex < 0)
				{
					string lowerClass = BC_TraderScanner.Lower(item.ClassName);
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
		rpc.Send(null, BC_RPC_TRADER_ITEMS, true, player.GetIdentity());
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
		rpc.Write(m_Wallets.GetBalance(BC_Util.SteamId(player)));
		rpc.Write(classes);
		rpc.Write(counts);
		rpc.Write(prices);
		rpc.Send(null, BC_RPC_TRADER_UPDATE, true, player.GetIdentity());
	}
}
