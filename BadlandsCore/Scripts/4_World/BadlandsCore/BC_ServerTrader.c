// Obchod s virtuální měnou. Peníze se získávají prodejem věcí a zabíjením zombíků.

class BC_ServerTrader
{
	ref BC_TraderConfig m_Cfg;
	protected BC_Wallets m_Wallets;
	protected ref map<string, BC_TraderItem> m_ByClass = new map<string, BC_TraderItem>; // malými písmeny

	void BC_ServerTrader(BC_Wallets wallets, BC_TraderConfig cfg)
	{
		m_Wallets = wallets;
		m_Cfg = cfg;

		foreach (BC_TraderCategory category : m_Cfg.Categories)
		{
			if (!category || !category.Items)
				continue;

			foreach (BC_TraderItem item : category.Items)
			{
				if (!item)
					continue;

				string key = item.ClassName;
				key.ToLower();
				m_ByClass.Set(key, item);
			}
		}
	}

	static BC_TraderConfig LoadConfig()
	{
		BC_TraderConfig defaults = new BC_TraderConfig();
		defaults.FillDefaults();
		return BC_ConfigFile<BC_TraderConfig>.Load("Trader.json", defaults, new BC_TraderConfig());
	}

	protected ref map<string, BC_TraderItem> m_InheritedCache = new map<string, BC_TraderItem>;

	//! Přesná shoda classname, jinak varianta (např. M4A1_Black -> M4A1)
	protected BC_TraderItem FindPriced(string className)
	{
		string key = className;
		key.ToLower();

		BC_TraderItem item;
		if (m_ByClass.Find(key, item))
			return item;
		if (m_InheritedCache.Find(key, item))
			return item;

		item = null;
		foreach (string pricedKey, BC_TraderItem priced : m_ByClass)
		{
			if (g_Game.IsKindOf(className, priced.ClassName))
			{
				item = priced;
				break;
			}
		}

		m_InheritedCache.Set(key, item);
		return item;
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

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != BC_RPC_TRADER_OPEN && rpcType != BC_RPC_TRADER_BUY && rpcType != BC_RPC_TRADER_SELL)
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

		if (rpcType == BC_RPC_TRADER_OPEN)
		{
			SendCatalog(player, trader);
		}
		else if (rpcType == BC_RPC_TRADER_BUY)
		{
			int categoryIndex;
			int itemIndex;
			if (ctx.Read(categoryIndex) && ctx.Read(itemIndex))
				Buy(player, categoryIndex, itemIndex);
			SendUpdate(player);
		}
		else if (rpcType == BC_RPC_TRADER_SELL)
		{
			string className;
			if (ctx.Read(className))
				Sell(player, className);
			SendUpdate(player);
		}
		return true;
	}

	protected void Buy(PlayerBase player, int categoryIndex, int itemIndex)
	{
		if (categoryIndex < 0 || categoryIndex >= m_Cfg.Categories.Count())
			return;

		BC_TraderCategory category = m_Cfg.Categories[categoryIndex];
		if (!category || itemIndex < 0 || itemIndex >= category.Items.Count())
			return;

		BC_TraderItem priced = category.Items[itemIndex];
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
			BC_Util.Notify(player, "Obchod", "Predmet nelze prodat. Zbran musi byt bez zasobniku a prislusenstvi a nesmi byt znicena.");
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

	//! Částečně použité věci (načatá krabička nábojů, snědené jídlo) mají nižší výkupní cenu
	protected float SellFraction(ItemBase item)
	{
		if (item.IsAmmoPile())
		{
			Magazine pile = Magazine.Cast(item);
			if (pile && pile.GetAmmoMax() > 0)
			{
				float ammoCount = pile.GetAmmoCount();
				float ammoMax = pile.GetAmmoMax();
				return ammoCount / ammoMax;
			}
			return 1;
		}

		float fraction = 1;
		if (!Magazine.Cast(item) && !item.IsLiquidContainer() && item.HasQuantity())
			fraction = item.GetQuantityNormalized();

		// stav předmětu: zničený se neprodá, opotřebovaný za 50-100 % ceny
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

	protected void CollectSellable(PlayerBase player, array<string> classes, array<int> counts)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI ent : items)
		{
			ItemBase item = ItemBase.Cast(ent);
			if (!CanSellItem(item))
				continue;

			string type = item.GetType();
			int index = classes.Find(type);
			if (index == -1)
			{
				classes.Insert(type);
				counts.Insert(1);
			}
			else
			{
				counts[index] = counts[index] + 1;
			}
		}
	}

	protected void SendCatalog(PlayerBase player, BC_TraderLocation trader)
	{
		array<string> categoryNames = new array<string>;
		array<int> itemCategories = new array<int>;
		array<string> itemClasses = new array<string>;
		array<int> itemBuy = new array<int>;
		array<int> itemSell = new array<int>;

		for (int c = 0; c < m_Cfg.Categories.Count(); c++)
		{
			BC_TraderCategory category = m_Cfg.Categories[c];
			if (!category)
				continue;

			categoryNames.Insert(category.Name);
			foreach (BC_TraderItem item : category.Items)
			{
				itemCategories.Insert(c);
				itemClasses.Insert(item.ClassName);
				itemBuy.Insert(item.Buy);
				itemSell.Insert(item.Sell);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(trader.Name);
		rpc.Write(m_Cfg.CurrencyName);
		rpc.Write(categoryNames);
		rpc.Write(itemCategories);
		rpc.Write(itemClasses);
		rpc.Write(itemBuy);
		rpc.Write(itemSell);
		rpc.Send(null, BC_RPC_TRADER_DATA, true, player.GetIdentity());

		SendUpdate(player);
	}

	protected void SendUpdate(PlayerBase player)
	{
		array<string> classes = new array<string>;
		array<int> counts = new array<int>;
		CollectSellable(player, classes, counts);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Wallets.GetBalance(BC_Util.SteamId(player)));
		rpc.Write(classes);
		rpc.Write(counts);
		rpc.Send(null, BC_RPC_TRADER_UPDATE, true, player.GetIdentity());
	}
}
