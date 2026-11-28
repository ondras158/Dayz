// Bazar hráčů u obchodníka: věci (i s obsahem) a auta z garáže za cenu prodejce.

class OS_ServerMarket
{
	protected static const string CFG_FILE = "Market.json";
	protected static const string DATA_FILE = "MarketData.json";

	ref OS_MarketConfig m_Cfg;
	protected ref OS_MarketData m_Data;
	protected OS_Payments m_Payments;
	protected OS_Wallets m_Wallets;
	protected string m_Currency;
	protected float m_ExpireTimer;

	void OS_ServerMarket(OS_Payments payments, OS_Wallets wallets, string currency)
	{
		m_Payments = payments;
		m_Wallets = wallets;
		m_Currency = currency;
		m_Cfg = OS_ConfigFile<OS_MarketConfig>.Load(CFG_FILE, new OS_MarketConfig(), new OS_MarketConfig());
		if (!m_Cfg.Traders)
			m_Cfg.Traders = new array<string>;
		m_Data = new OS_MarketData();
		string path = OS_DIR + DATA_FILE;
		string err;
		if (FileExist(path) && !JsonFileLoader<OS_MarketData>.LoadFile(path, m_Data, err))
		{
			Print(OS_TAG + "CHYBA v " + path + ": " + err);
			m_Data = new OS_MarketData();
		}
		if (!m_Data.Listings)
			m_Data.Listings = new array<ref OS_MarketListing>;
		if (!m_Data.Returns)
			m_Data.Returns = new array<ref OS_MarketReturn>;
		ExpireOld();
	}

	protected void Save()
	{
		string err;
		if (!JsonFileLoader<OS_MarketData>.SaveFile(OS_DIR + DATA_FILE, m_Data, err))
			Print(OS_TAG + "CHYBA při ukládání " + DATA_FILE + ": " + err);
	}

	//! přibližný čas v sekundách (UTC, roste monotónně) - stačí na vypršení nabídek
	protected int Now()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDayUTC(year, month, day);
		GetHourMinuteSecondUTC(hour, minute, second);
		int days = (year - 2020) * 372 + month * 31 + day;
		return days * 86400 + hour * 3600 + minute * 60 + second;
	}

	//! je u tohoto obchodníka bazar?
	bool AllowedAt(OS_TraderLocation trader)
	{
		if (!m_Cfg.Enabled || !trader)
			return false;
		if (m_Cfg.AtAllTraders)
			return true;
		return m_Cfg.Traders.Find(trader.Name) > -1;
	}

	protected bool NearMarket(PlayerBase player)
	{
		if (!OS_Server.Get() || !OS_Server.Get().m_Trader)
			return false;
		OS_TraderLocation trader = OS_Server.Get().m_Trader.NearTrader(player);
		return AllowedAt(trader);
	}

	// ------------------------------------------------------------ vypršení

	void OnTick(float delta)
	{
		m_ExpireTimer += delta;
		if (m_ExpireTimer < 600)
			return;
		m_ExpireTimer = 0;
		ExpireOld();
	}

	protected void ExpireOld()
	{
		int limit = Now() - m_Cfg.ListingDays * 86400;
		bool changed = false;
		for (int i = m_Data.Listings.Count() - 1; i >= 0; i--)
		{
			OS_MarketListing listing = m_Data.Listings[i];
			if (!listing || listing.Created > limit)
				continue;
			ReturnToSeller(listing);
			m_Data.Listings.Remove(i);
			changed = true;
		}
		if (changed)
			Save();
	}

	protected void ReturnToSeller(OS_MarketListing listing)
	{
		if (listing.IsVehicle)
		{
			if (OS_Server.Get() && OS_Server.Get().m_Vehicles)
				OS_Server.Get().m_Vehicles.GiveStored(listing.SellerId, listing.Vehicle);
			return;
		}
		OS_MarketReturn ret = new OS_MarketReturn();
		ret.OwnerId = listing.SellerId;
		ret.Item = listing.Item;
		m_Data.Returns.Insert(ret);
	}

	// ------------------------------------------------------------ RPC

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_MARKET_OPEN && rpcType != OS_RPC_MARKET_ACTION)
			return false;
		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive() || !m_Cfg.Enabled)
			return true;
		if (!NearMarket(player))
		{
			OS_Util.Notify(player, OS_T("Bazar"), OS_T("Bazar je u obchodnika."));
			return true;
		}

		if (rpcType == OS_RPC_MARKET_ACTION)
		{
			int action;
			int a;
			int b;
			int price;
			if (ctx.Read(action) && ctx.Read(a) && ctx.Read(b) && ctx.Read(price))
				HandleAction(player, action, a, b, price);
		}
		SendData(player);
		return true;
	}

	protected int CountListings(string steamId)
	{
		int count = 0;
		foreach (OS_MarketListing listing : m_Data.Listings)
		{
			if (listing && listing.SellerId == steamId)
				count++;
		}
		return count;
	}

	protected OS_MarketListing Find(int id)
	{
		foreach (OS_MarketListing listing : m_Data.Listings)
		{
			if (listing && listing.Id == id)
				return listing;
		}
		return null;
	}

	protected void HandleAction(PlayerBase player, int action, int a, int b, int price)
	{
		string id = OS_Util.SteamId(player);

		if (action == OS_MarketAction.LIST_ITEM || action == OS_MarketAction.LIST_VEHICLE)
		{
			if (CountListings(id) >= m_Cfg.MaxListingsPerPlayer)
			{
				OS_Util.Notify(player, OS_T("Bazar"), OS_F("Muzes mit nejvyse %1 nabidek.", m_Cfg.MaxListingsPerPlayer.ToString()));
				return;
			}
			if (price < m_Cfg.MinPrice || price > m_Cfg.MaxPrice)
			{
				OS_Util.Notify(player, OS_T("Bazar"), OS_F("Cena musi byt %1 az %2 %3.", m_Cfg.MinPrice.ToString(), m_Cfg.MaxPrice.ToString(), m_Currency));
				return;
			}
		}

		if (action == OS_MarketAction.LIST_ITEM)
			ListItem(player, id, a, b, price);
		else if (action == OS_MarketAction.LIST_VEHICLE)
			ListVehicle(player, id, a, price);
		else if (action == OS_MarketAction.BUY)
			Buy(player, id, a);
		else if (action == OS_MarketAction.WITHDRAW)
			Withdraw(player, id, a);
		else if (action == OS_MarketAction.CLAIM)
			Claim(player, id);
	}

	protected bool Sellable(PlayerBase player, EntityAI item)
	{
		if (!item || item == player || item.GetHierarchyRootPlayer() != player)
			return false;
		if (item.IsRuined() || OS_Money.Cast(item))
			return false;
		return true;
	}

	protected void ListItem(PlayerBase player, string id, int low, int high, int price)
	{
		EntityAI item = EntityAI.Cast(g_Game.GetObjectByNetworkId(low, high));
		if (!Sellable(player, item))
		{
			OS_Util.Notify(player, OS_T("Bazar"), OS_T("Tuhle vec nejde vystavit."));
			return;
		}
		OS_MarketListing listing = new OS_MarketListing();
		listing.Id = m_Data.NextId;
		m_Data.NextId = m_Data.NextId + 1;
		listing.SellerId = id;
		listing.SellerName = OS_Util.PlayerName(player);
		listing.ClassName = item.GetType();
		listing.Price = price;
		listing.Created = Now();
		listing.Item = OS_ItemStore.SaveItem(item, "");
		g_Game.ObjectDelete(item);
		m_Data.Listings.Insert(listing);
		Save();
		OS_Util.Notify(player, OS_T("Bazar"), OS_F("Vystaveno: %1 za %2 %3.", OS_Util.DisplayNameOf(listing.ClassName), price.ToString(), m_Currency));
		Print(OS_TAG + "BAZAR " + listing.SellerName + " vystavil " + listing.ClassName + " za " + price.ToString());
	}

	protected void ListVehicle(PlayerBase player, string id, int index, int price)
	{
		if (!m_Cfg.AllowVehicles || !OS_Server.Get() || !OS_Server.Get().m_Vehicles)
			return;
		OS_StoredVehicle vehicle = OS_Server.Get().m_Vehicles.TakeStored(id, index);
		if (!vehicle)
		{
			OS_Util.Notify(player, OS_T("Bazar"), OS_T("Auto v garazi nenalezeno."));
			return;
		}
		OS_MarketListing listing = new OS_MarketListing();
		listing.Id = m_Data.NextId;
		m_Data.NextId = m_Data.NextId + 1;
		listing.SellerId = id;
		listing.SellerName = OS_Util.PlayerName(player);
		listing.ClassName = vehicle.ClassName;
		listing.IsVehicle = true;
		listing.Price = price;
		listing.Created = Now();
		listing.Vehicle = vehicle;
		m_Data.Listings.Insert(listing);
		Save();
		OS_Util.Notify(player, OS_T("Bazar"), OS_F("Auto vystaveno: %1 za %2 %3.", OS_Util.DisplayNameOf(listing.ClassName), price.ToString(), m_Currency));
		Print(OS_TAG + "BAZAR " + listing.SellerName + " vystavil auto " + listing.ClassName + " za " + price.ToString());
	}

	protected void Buy(PlayerBase player, string id, int listingId)
	{
		OS_MarketListing listing = Find(listingId);
		if (!listing)
		{
			OS_Util.Notify(player, OS_T("Bazar"), OS_T("Nabidka uz neexistuje."));
			return;
		}
		if (listing.SellerId == id)
		{
			OS_Util.Notify(player, OS_T("Bazar"), OS_T("Vlastni nabidku nekoupis - stahni ji."));
			return;
		}
		if (!m_Payments.TrySpend(player, listing.Price))
		{
			OS_Util.Notify(player, OS_T("Bazar"), OS_F("Nemas dost penez (%1 %2).", listing.Price.ToString(), m_Currency));
			return;
		}

		if (listing.IsVehicle)
		{
			if (!OS_Server.Get() || !OS_Server.Get().m_Vehicles || !OS_Server.Get().m_Vehicles.GiveStored(id, listing.Vehicle))
			{
				m_Payments.Pay(player, listing.Price);
				OS_Util.Notify(player, OS_T("Bazar"), OS_T("Chyba pri predani auta - penize vraceny."));
				return;
			}
			OS_Util.Notify(player, OS_T("Bazar"), OS_F("Koupeno: %1. Auto je v tve garazi (vyzvedni ho v kterekoli garazi).", OS_Util.DisplayNameOf(listing.ClassName)), 8);
		}
		else
		{
			OS_ItemStore.RestoreItem(player, listing.Item, player);
			OS_Util.Notify(player, OS_T("Bazar"), OS_F("Koupeno: %1", OS_Util.DisplayNameOf(listing.ClassName)));
		}

		// prodejce dostane peníze na účet (minus poplatek)
		int fee = Math.Round(listing.Price * m_Cfg.FeePercent / 100);
		int payout = listing.Price - fee;
		m_Wallets.Add(listing.SellerId, payout);
		PlayerBase seller = OS_Util.FindPlayerBySteamId(listing.SellerId);
		if (seller)
			OS_Util.Notify(seller, OS_T("Bazar"), OS_F("Prodano: %1 za %2 %3 (na ucet %4 %3).", OS_Util.DisplayNameOf(listing.ClassName), listing.Price.ToString(), m_Currency, payout.ToString()), 8);
		Print(OS_TAG + "BAZAR " + OS_Util.PlayerName(player) + " koupil " + listing.ClassName + " od " + listing.SellerName + " za " + listing.Price.ToString());

		m_Data.Listings.RemoveItem(listing);
		Save();
	}

	protected void Withdraw(PlayerBase player, string id, int listingId)
	{
		OS_MarketListing listing = Find(listingId);
		if (!listing || listing.SellerId != id)
			return;
		if (listing.IsVehicle)
		{
			if (OS_Server.Get() && OS_Server.Get().m_Vehicles)
				OS_Server.Get().m_Vehicles.GiveStored(id, listing.Vehicle);
			OS_Util.Notify(player, OS_T("Bazar"), OS_T("Nabidka stazena - auto je zpet v garazi."));
		}
		else
		{
			OS_ItemStore.RestoreItem(player, listing.Item, player);
			OS_Util.Notify(player, OS_T("Bazar"), OS_T("Nabidka stazena - vec mas zpet."));
		}
		m_Data.Listings.RemoveItem(listing);
		Save();
	}

	protected void Claim(PlayerBase player, string id)
	{
		int claimed = 0;
		for (int i = m_Data.Returns.Count() - 1; i >= 0; i--)
		{
			OS_MarketReturn ret = m_Data.Returns[i];
			if (!ret || ret.OwnerId != id)
				continue;
			OS_ItemStore.RestoreItem(player, ret.Item, player);
			m_Data.Returns.Remove(i);
			claimed++;
		}
		if (claimed > 0)
		{
			Save();
			OS_Util.Notify(player, OS_T("Bazar"), OS_F("Vyzvednuto veci: %1", claimed.ToString()));
		}
	}

	// ------------------------------------------------------------ data pro klienta

	protected void SendData(PlayerBase player)
	{
		string id = OS_Util.SteamId(player);

		array<int> ids = new array<int>;
		array<string> classes = new array<string>;
		array<int> prices = new array<int>;
		array<string> sellers = new array<string>;
		array<int> vehicles = new array<int>;
		array<int> mine = new array<int>;
		foreach (OS_MarketListing listing : m_Data.Listings)
		{
			if (!listing)
				continue;
			ids.Insert(listing.Id);
			classes.Insert(listing.ClassName);
			prices.Insert(listing.Price);
			sellers.Insert(listing.SellerName);
			if (listing.IsVehicle)
				vehicles.Insert(1);
			else
				vehicles.Insert(0);
			if (listing.SellerId == id)
				mine.Insert(1);
			else
				mine.Insert(0);
		}

		// moje věci (síťové ID) - kromě peněz
		array<string> itemClasses = new array<string>;
		array<int> itemLow = new array<int>;
		array<int> itemHigh = new array<int>;
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI item : items)
		{
			if (!Sellable(player, item))
				continue;
			int low;
			int high;
			item.GetNetworkID(low, high);
			itemClasses.Insert(item.GetType());
			itemLow.Insert(low);
			itemHigh.Insert(high);
		}

		// moje auta v osobní garáži
		array<string> garage = new array<string>;
		if (m_Cfg.AllowVehicles && OS_Server.Get() && OS_Server.Get().m_Vehicles)
		{
			foreach (OS_StoredVehicle stored : OS_Server.Get().m_Vehicles.PersonalStored(id))
			{
				if (stored)
					garage.Insert(stored.ClassName);
			}
		}

		int returns = 0;
		foreach (OS_MarketReturn ret : m_Data.Returns)
		{
			if (ret && ret.OwnerId == id)
				returns++;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Currency);
		rpc.Write(m_Cfg.FeePercent);
		rpc.Write(ids);
		rpc.Write(classes);
		rpc.Write(prices);
		rpc.Write(sellers);
		rpc.Write(vehicles);
		rpc.Write(mine);
		rpc.Write(itemClasses);
		rpc.Write(itemLow);
		rpc.Write(itemHigh);
		rpc.Write(garage);
		rpc.Write(returns);
		rpc.Send(null, OS_RPC_MARKET_DATA, true, player.GetIdentity());
	}
}
