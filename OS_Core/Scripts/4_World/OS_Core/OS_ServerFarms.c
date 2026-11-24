// Farmy: výroba běží na serveru každou minutu (i když je majitel offline) a ukládá se do FarmsData.json.

class OS_ServerFarms
{
	ref OS_FarmConfig m_Cfg;
	protected ref OS_FarmsData m_Data;
	protected OS_Payments m_Payments;
	protected OS_ServerGroups m_Groups;
	protected string m_Currency;
	protected float m_TickTimer;

	protected static const string DATA_FILE = "FarmsData.json";
	protected static const int MAX_COLLECT = 50;   // max kusů najednou (aby se nezahltil inventář)

	void OS_ServerFarms(OS_Payments payments, OS_ServerGroups groups, string currency)
	{
		m_Payments = payments;
		m_Groups = groups;
		m_Currency = currency;

		OS_FarmConfig defaults = new OS_FarmConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_FarmConfig>.Load("Farms.json", defaults, new OS_FarmConfig());

		m_Data = new OS_FarmsData();
		string path = OS_DIR + DATA_FILE;
		string err;
		if (FileExist(path) && !JsonFileLoader<OS_FarmsData>.LoadFile(path, m_Data, err))
		{
			Print(OS_TAG + "CHYBA pri nacitani " + path + ": " + err);
			m_Data = new OS_FarmsData();
		}
		if (!m_Data.Farms)
			m_Data.Farms = new array<ref OS_Farm>;
	}

	protected void Save()
	{
		string err;
		if (!JsonFileLoader<OS_FarmsData>.SaveFile(OS_DIR + DATA_FILE, m_Data, err))
			Print(OS_TAG + "CHYBA pri ukladani farem: " + err);
	}

	protected OS_Farm GetFarmOf(string ownerId)
	{
		foreach (OS_Farm farm : m_Data.Farms)
		{
			if (farm && farm.OwnerId == ownerId)
				return farm;
		}
		return null;
	}

	//! farma, kterou hráč u sebe smí používat (vlastní, nebo farma spoluhráče ze skupiny)
	protected OS_Farm UsableFarmNear(PlayerBase player)
	{
		string id = OS_Util.SteamId(player);
		foreach (OS_Farm farm : m_Data.Farms)
		{
			if (!farm || OS_Util.Distance2D(player.GetPosition(), farm.X, farm.Z) > m_Cfg.Radius)
				continue;
			if (farm.OwnerId == id)
				return farm;
			if (m_Cfg.GroupCanUse && m_Groups && m_Groups.SameGroup(id, farm.OwnerId))
				return farm;
		}
		return null;
	}

	//! Farmář, u kterého hráč stojí (nebo null)
	OS_TraderLocation NearAnimalTrader(PlayerBase player)
	{
		if (!m_Cfg.Enabled || !player || !m_Cfg.AnimalTraders)
			return null;
		foreach (OS_TraderLocation trader : m_Cfg.AnimalTraders)
		{
			if (trader && OS_Util.Distance2D(player.GetPosition(), trader.X, trader.Z) <= trader.Radius)
				return trader;
		}
		return null;
	}

	//! klávesa obchodu u Farmáře -> menu farmy v režimu nákupu
	bool TryOpenTrader(PlayerBase player)
	{
		if (!NearAnimalTrader(player))
			return false;
		SendData(player);
		return true;
	}

	// ------------------------------------------------------------ výroba

	void OnTick(float deltaTime)
	{
		if (!m_Cfg.Enabled)
			return;

		m_TickTimer += deltaTime;
		if (m_TickTimer < 60)
			return;
		float minutes = m_TickTimer / 60.0;
		m_TickTimer = 0;

		bool changed = false;
		foreach (OS_Farm farm : m_Data.Farms)
		{
			if (farm && Produce(farm, minutes))
				changed = true;
		}
		if (changed)
			Save();
	}

	//! true = něco se změnilo
	protected bool Produce(OS_Farm farm, float minutes)
	{
		bool changed = false;
		foreach (OS_FarmStock stock : farm.Stock)
		{
			if (!stock || stock.Count <= 0)
				continue;

			OS_FarmAnimalType type = m_Cfg.FindAnimal(stock.Id);
			if (!type || type.CycleMinutes <= 0)
				continue;

			stock.Progress = stock.Progress + minutes;
			while (stock.Progress >= type.CycleMinutes)
			{
				int need = type.FeedPerCycle * stock.Count;
				if (farm.Feed < need)
				{
					// bez krmení se nevyrábí - čeká se na krmivo
					stock.Progress = type.CycleMinutes;
					break;
				}

				farm.Feed = farm.Feed - need;
				stock.Progress = stock.Progress - type.CycleMinutes;
				changed = true;

				foreach (OS_FarmProduct product : type.Products)
				{
					if (!product)
						continue;
					OS_FarmStored stored = farm.GetStored(product.ClassName, true);
					for (int animal = 0; animal < stock.Count; animal++)
					{
						if (stored.Count >= m_Cfg.MaxStoredPerProduct)
							break;
						if (Math.RandomFloat01() <= product.Chance)
							stored.Count = Math.Min(m_Cfg.MaxStoredPerProduct, stored.Count + product.Amount);
					}
				}
			}
		}
		return changed;
	}

	//! krmivo za hodinu při současném počtu zvířat
	protected int FeedPerHour(OS_Farm farm)
	{
		float total = 0;
		foreach (OS_FarmStock stock : farm.Stock)
		{
			if (!stock || stock.Count <= 0)
				continue;
			OS_FarmAnimalType type = m_Cfg.FindAnimal(stock.Id);
			if (type && type.CycleMinutes > 0)
				total += type.FeedPerCycle * stock.Count * 60.0 / type.CycleMinutes;
		}
		return Math.Ceil(total);
	}

	// ------------------------------------------------------------ RPC

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_FARM_OPEN && rpcType != OS_RPC_FARM_ACTION)
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		if (rpcType == OS_RPC_FARM_ACTION)
		{
			int action;
			string arg;
			int amount;
			if (ctx.Read(action) && ctx.Read(arg) && ctx.Read(amount))
				HandleAction(player, action, arg, amount);
		}

		SendData(player);
		return true;
	}

	protected void HandleAction(PlayerBase player, int action, string arg, int amount)
	{
		if (!m_Cfg.Enabled)
			return;

		string id = OS_Util.SteamId(player);

		if (action == OS_FarmAction.BUILD)
		{
			Build(player, id);
			return;
		}
		if (action == OS_FarmAction.REMOVE)
		{
			OS_Farm own = GetFarmOf(id);
			if (own)
			{
				m_Data.Farms.RemoveItem(own);
				Save();
				OS_Util.Notify(player, "Farma", "Farma zrusena.");
			}
			return;
		}

		// u Farmáře: nákup zvířat a krmiva pro vlastní farmu (dovezou se)
		bool atTrader = NearAnimalTrader(player) != null;
		if (atTrader && (action == OS_FarmAction.BUY_ANIMAL || action == OS_FarmAction.BUY_FEED))
		{
			OS_Farm ownFarm = GetFarmOf(id);
			if (!ownFarm)
			{
				OS_Util.Notify(player, "Farmar", "Nejdriv si postav farmu (klavesa U).");
				return;
			}
			if (action == OS_FarmAction.BUY_ANIMAL)
				BuyAnimal(player, ownFarm, arg);
			else
				BuyFeed(player, ownFarm, amount);
			return;
		}

		OS_Farm farm = UsableFarmNear(player);
		if (!farm)
		{
			OS_Util.Notify(player, "Farma", "Musis stat u sve farmy (nebo farmy spoluhrace ze skupiny) nebo u Farmare.");
			return;
		}

		if (action == OS_FarmAction.BUY_ANIMAL)
		{
			if (m_Cfg.AnimalsOnlyAtTrader)
				OS_Util.Notify(player, "Farma", "Zvirata se kupuji u Farmare.");
			else
				BuyAnimal(player, farm, arg);
		}
		else if (action == OS_FarmAction.BUY_FEED)
		{
			if (m_Cfg.FeedAtFarm)
				BuyFeed(player, farm, amount);
			else
				OS_Util.Notify(player, "Farma", "Krmivo se kupuje u Farmare.");
		}
		else if (action == OS_FarmAction.COLLECT)
			Collect(player, farm, arg, amount);
		else if (action == OS_FarmAction.COLLECT_ALL)
			CollectAll(player, farm);
	}

	protected void Build(PlayerBase player, string id)
	{
		if (GetFarmOf(id))
		{
			OS_Util.Notify(player, "Farma", "Uz mas farmu. Nejdriv ji zrus.");
			return;
		}

		foreach (OS_Farm other : m_Data.Farms)
		{
			if (other && OS_Util.Distance2D(player.GetPosition(), other.X, other.Z) < m_Cfg.MinDistance)
			{
				OS_Util.Notify(player, "Farma", "Moc blizko jine farmy.");
				return;
			}
		}

		if (!m_Payments.TrySpend(player, m_Cfg.BuildPrice))
		{
			OS_Util.Notify(player, "Farma", "Stavba farmy stoji " + m_Cfg.BuildPrice.ToString() + " " + m_Currency + ".");
			return;
		}

		vector pos = player.GetPosition();
		OS_Farm farm = new OS_Farm();
		farm.OwnerId = id;
		farm.X = pos[0];
		farm.Z = pos[2];
		m_Data.Farms.Insert(farm);
		Save();
		OS_Util.Notify(player, "Farma", "Farma postavena. Kup zvirata a krmivo.");
		Print(OS_TAG + "FARMA " + OS_Util.PlayerName(player) + " postavil farmu na " + farm.X.ToString() + " " + farm.Z.ToString());
	}

	protected void BuyAnimal(PlayerBase player, OS_Farm farm, string animalId)
	{
		OS_FarmAnimalType type = m_Cfg.FindAnimal(animalId);
		if (!type)
			return;

		OS_FarmStock stock = farm.GetStock(type.Id, true);
		if (stock.Count >= type.MaxCount)
		{
			OS_Util.Notify(player, "Farma", "Maximum: " + type.MaxCount.ToString() + " (" + type.Name + ").");
			return;
		}
		if (!m_Payments.TrySpend(player, type.Price))
		{
			OS_Util.Notify(player, "Farma", type.Name + " stoji " + type.Price.ToString() + " " + m_Currency + ".");
			return;
		}

		stock.Count = stock.Count + 1;
		Save();
		OS_Util.Notify(player, "Farma", "Koupeno: " + type.Name + " (" + stock.Count.ToString() + "/" + type.MaxCount.ToString() + ")");
	}

	protected void BuyFeed(PlayerBase player, OS_Farm farm, int amount)
	{
		int space = m_Cfg.MaxFeed - farm.Feed;
		if (amount > space)
			amount = space;
		if (amount <= 0)
		{
			OS_Util.Notify(player, "Farma", "Zasobnik krmiva je plny.");
			return;
		}

		int price = amount * m_Cfg.FeedPrice;
		if (!m_Payments.TrySpend(player, price))
		{
			OS_Util.Notify(player, "Farma", "Krmivo stoji " + price.ToString() + " " + m_Currency + ".");
			return;
		}

		farm.Feed = farm.Feed + amount;
		Save();
		OS_Util.Notify(player, "Farma", "Koupeno krmiva: " + amount.ToString());
	}

	//! amount -1 = vše (max MAX_COLLECT)
	protected int Collect(PlayerBase player, OS_Farm farm, string className, int amount)
	{
		OS_FarmStored stored = farm.GetStored(className, false);
		if (!stored || stored.Count <= 0)
			return 0;

		if (amount < 0 || amount > stored.Count)
			amount = stored.Count;
		if (amount > MAX_COLLECT)
			amount = MAX_COLLECT;

		int given = 0;
		for (int i = 0; i < amount; i++)
		{
			if (!OS_Util.GiveItem(player, className))
				break;
			given++;
		}

		stored.Count = stored.Count - given;
		Save();
		return given;
	}

	protected void CollectAll(PlayerBase player, OS_Farm farm)
	{
		int total = 0;
		foreach (OS_FarmStored stored : farm.Storage)
		{
			if (!stored || stored.Count <= 0 || total >= MAX_COLLECT)
				continue;
			total += Collect(player, farm, stored.ClassName, MAX_COLLECT - total);
		}

		if (total > 0)
			OS_Util.Notify(player, "Farma", "Vybrano kusu: " + total.ToString() + ". Co se neveslo, lezi na zemi.");
		else
			OS_Util.Notify(player, "Farma", "Sklad je prazdny.");
	}

	// ------------------------------------------------------------ data pro klienta

	void SendData(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string id = OS_Util.SteamId(player);
		OS_Farm own = GetFarmOf(id);
		OS_Farm near = UsableFarmNear(player);
		OS_TraderLocation animalTrader = NearAnimalTrader(player);
		OS_Farm shown = near;
		if (!shown || animalTrader)
			shown = own;   // u Farmáře se vždy nakupuje pro vlastní farmu

		array<string> animalIds = new array<string>;
		array<string> animalNames = new array<string>;
		array<int> animalCounts = new array<int>;
		array<int> animalMax = new array<int>;
		array<int> animalPrices = new array<int>;
		foreach (OS_FarmAnimalType type : m_Cfg.Animals)
		{
			if (!type)
				continue;
			int count = 0;
			if (shown)
			{
				OS_FarmStock stock = shown.GetStock(type.Id, false);
				if (stock)
					count = stock.Count;
			}
			animalIds.Insert(type.Id);
			animalNames.Insert(type.Name);
			animalCounts.Insert(count);
			animalMax.Insert(type.MaxCount);
			animalPrices.Insert(type.Price);
		}

		array<string> storedClasses = new array<string>;
		array<int> storedCounts = new array<int>;
		int feed = 0;
		int feedPerHour = 0;
		float fx = 0;
		float fz = 0;
		if (shown)
		{
			feed = shown.Feed;
			feedPerHour = FeedPerHour(shown);
			fx = shown.X;
			fz = shown.Z;
			foreach (OS_FarmStored stored : shown.Storage)
			{
				if (stored && stored.Count > 0)
				{
					storedClasses.Insert(stored.ClassName);
					storedCounts.Insert(stored.Count);
				}
			}
		}

		bool hasOwn = own != null;
		bool isNear = near != null;
		bool isOwner = shown && shown.OwnerId == id;
		float ownX = 0;
		float ownZ = 0;
		if (own)
		{
			ownX = own.X;
			ownZ = own.Z;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Cfg.Enabled);
		rpc.Write(m_Currency);
		rpc.Write(m_Cfg.BuildPrice);
		rpc.Write(hasOwn);
		rpc.Write(ownX);
		rpc.Write(ownZ);
		rpc.Write(isNear);
		rpc.Write(isOwner);
		rpc.Write(fx);
		rpc.Write(fz);
		rpc.Write(feed);
		rpc.Write(m_Cfg.MaxFeed);
		rpc.Write(m_Cfg.FeedPrice);
		rpc.Write(feedPerHour);
		rpc.Write(animalIds);
		rpc.Write(animalNames);
		rpc.Write(animalCounts);
		rpc.Write(animalMax);
		rpc.Write(animalPrices);
		rpc.Write(storedClasses);
		rpc.Write(storedCounts);

		// režim Farmáře
		bool atTrader = animalTrader != null;
		string traderName = "";
		if (animalTrader)
			traderName = animalTrader.Name;
		rpc.Write(atTrader);
		rpc.Write(traderName);
		rpc.Write(m_Cfg.AnimalsOnlyAtTrader);
		rpc.Write(m_Cfg.FeedAtFarm);
		rpc.Send(null, OS_RPC_FARM_DATA, true, player.GetIdentity());
	}
}
