// Farmy: hráč si postaví farmu, koupí zvířata a krmivo, zvířata s časem vyrábějí suroviny.
// Nastavení: $profile:OS_Core/Farms.json, data farem: $profile:OS_Core/FarmsData.json

class OS_FarmProduct
{
	string ClassName;
	int Amount = 1;      // kolik kusů za cyklus na jedno zvíře
	float Chance = 1;    // šance 0-1, že se v cyklu vyrobí

	void OS_FarmProduct(string className = "", int amount = 1, float chance = 1)
	{
		ClassName = className;
		Amount = amount;
		Chance = chance;
	}
}

class OS_FarmAnimalType
{
	string Id;
	string Name;
	int Price;               // cena jednoho zvířete
	int MaxCount = 10;       // kolik jich může farma mít
	float CycleMinutes = 60; // jak často vyrábí
	int FeedPerCycle = 1;    // krmivo / hnojivo za cyklus na jeden kus
	string Resource = "feed";       // "feed" = krmivo (zvířata), "fertilizer" = hnojivo (rostliny)
	string RequiresBuilding = "";   // Id stavby, která je potřeba (např. "orchard" = sad, "greenhouse" = skleník)
	ref array<ref OS_FarmProduct> Products = new array<ref OS_FarmProduct>;

	void OS_FarmAnimalType(string id = "", string name = "", int price = 0, float cycle = 60, int feed = 1)
	{
		Id = id;
		Name = name;
		Price = price;
		CycleMinutes = cycle;
		FeedPerCycle = feed;
	}

	void AddProduct(string className, int amount, float chance)
	{
		Products.Insert(new OS_FarmProduct(className, amount, chance));
	}
}

class OS_FarmBuilding
{
	string Id;
	string Name;
	int Price;

	void OS_FarmBuilding(string id = "", string name = "", int price = 0)
	{
		Id = id;
		Name = name;
		Price = price;
	}
}

class OS_FarmConfig
{
	bool Enabled = true;
	int BuildPrice = 3000;
	float Radius = 20;              // jak blízko musí hráč stát, aby mohl farmu používat
	float MinDistance = 50;         // minimální vzdálenost od jiné farmy
	int FeedPrice = 5;              // cena jednoho krmiva
	int MaxFeed = 2000;             // kolik krmiva se vejde do zásobníku
	int FertilizerPrice = 4;        // cena jednoho hnojiva (pro sad a skleník)
	int MaxFertilizer = 2000;
	ref array<ref OS_FarmBuilding> Buildings = new array<ref OS_FarmBuilding>;   // stavby na farmě (sad, skleník...)
	int MaxStoredPerProduct = 100;  // sklad: max kusů od jedné suroviny (pak výroba té suroviny stojí)
	bool GroupCanUse = true;        // členové skupiny majitele mohou krmit a sbírat
	bool AnimalsOnlyAtTrader = true;  // zvířata se kupují jen u Farmáře (dovezou se na farmu)
	bool FeedAtFarm = true;           // krmivo jde koupit i přímo na farmě (jinak jen u Farmáře)
	ref array<ref OS_TraderLocation> AnimalTraders = new array<ref OS_TraderLocation>;   // Farmáři (Name, X, Z, Radius)
	ref array<ref OS_FarmAnimalType> Animals = new array<ref OS_FarmAnimalType>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečným místem z tvé mapy !!!
		AnimalTraders.Insert(new OS_TraderLocation("Priklad - Farmar", 1900, 2100, 10));

		OS_FarmAnimalType chicken = new OS_FarmAnimalType("chicken", "Slepice", 300, 30, 1);
		chicken.AddProduct("OS_ChickenEgg", 1, 1.0);
		chicken.AddProduct("ChickenFeather", 2, 0.5);
		chicken.AddProduct("ChickenBreastMeat", 1, 0.1);
		chicken.MaxCount = 20;
		Animals.Insert(chicken);

		OS_FarmAnimalType pig = new OS_FarmAnimalType("pig", "Prase", 1500, 120, 3);
		pig.AddProduct("PigSteakMeat", 2, 1.0);
		pig.AddProduct("Lard", 1, 0.5);
		pig.AddProduct("PigPelt", 1, 0.05);
		Animals.Insert(pig);

		OS_FarmAnimalType cow = new OS_FarmAnimalType("cow", "Krava", 3000, 120, 4);
		cow.AddProduct("PowderedMilk", 1, 1.0);
		cow.AddProduct("CowSteakMeat", 2, 0.3);
		cow.AddProduct("CowPelt", 1, 0.05);
		cow.MaxCount = 5;
		Animals.Insert(cow);

		OS_FarmAnimalType goat = new OS_FarmAnimalType("goat", "Koza", 1200, 90, 2);
		goat.AddProduct("PowderedMilk", 1, 0.7);
		goat.AddProduct("GoatSteakMeat", 1, 0.4);
		goat.AddProduct("GoatPelt", 1, 0.05);
		Animals.Insert(goat);

		OS_FarmAnimalType sheep = new OS_FarmAnimalType("sheep", "Ovce", 1200, 90, 2);
		sheep.AddProduct("SheepSteakMeat", 1, 0.5);
		sheep.AddProduct("SheepPelt", 1, 0.1);
		Animals.Insert(sheep);

		// --- stavby ---
		Buildings.Insert(new OS_FarmBuilding("orchard", "Sad", 4000));
		Buildings.Insert(new OS_FarmBuilding("greenhouse", "Sklenik", 6000));

		// --- sad (stromy, hnojivo) ---
		AddPlant("apple", "Jablon", 400, 120, "orchard", "Apple", 3, 10);
		AddPlant("pear", "Hrusen", 400, 120, "orchard", "Pear", 2, 10);
		AddPlant("plum", "Svestka", 400, 120, "orchard", "Plum", 3, 10);

		// --- skleník (záhony, hnojivo) ---
		AddPlant("tomato", "Rajcata (zahon)", 200, 60, "greenhouse", "Tomato", 2, 10);
		AddPlant("pepper", "Papriky (zahon)", 200, 60, "greenhouse", "GreenBellPepper", 2, 10);
		AddPlant("zucchini", "Cukety (zahon)", 250, 90, "greenhouse", "Zucchini", 1, 10);
		AddPlant("pumpkin", "Dyne (zahon)", 300, 120, "greenhouse", "Pumpkin", 1, 10);
		AddPlant("potato", "Brambory (zahon)", 150, 60, "greenhouse", "Potato", 2, 10);
		AddPlant("cannabis", "Konopi (zahon)", 800, 120, "greenhouse", "Cannabis", 1, 5);
	}

	protected void AddPlant(string id, string name, int price, float cycle, string building, string product, int amount, int maxCount)
	{
		OS_FarmAnimalType plant = new OS_FarmAnimalType(id, name, price, cycle, 1);
		plant.Resource = "fertilizer";
		plant.RequiresBuilding = building;
		plant.MaxCount = maxCount;
		plant.AddProduct(product, amount, 1.0);
		Animals.Insert(plant);
	}

	OS_FarmBuilding FindBuilding(string id)
	{
		foreach (OS_FarmBuilding building : Buildings)
		{
			if (building && building.Id == id)
				return building;
		}
		return null;
	}

	OS_FarmAnimalType FindAnimal(string id)
	{
		foreach (OS_FarmAnimalType type : Animals)
		{
			if (type && type.Id == id)
				return type;
		}
		return null;
	}
}

class OS_FarmStock
{
	string Id;          // druh zvířete
	int Count;
	float Progress;     // minuty do dalšího cyklu
}

class OS_FarmStored
{
	string ClassName;
	int Count;
}

class OS_Farm
{
	string OwnerId;
	float X;
	float Z;
	int Feed;
	int Fertilizer;
	ref array<string> Buildings = new array<string>;
	ref array<ref OS_FarmStock> Stock = new array<ref OS_FarmStock>;
	ref array<ref OS_FarmStored> Storage = new array<ref OS_FarmStored>;

	bool HasBuilding(string id)
	{
		if (id == "")
			return true;
		return Buildings && Buildings.Find(id) > -1;
	}

	OS_FarmStock GetStock(string id, bool create)
	{
		foreach (OS_FarmStock s : Stock)
		{
			if (s && s.Id == id)
				return s;
		}
		if (!create)
			return null;
		OS_FarmStock added = new OS_FarmStock();
		added.Id = id;
		Stock.Insert(added);
		return added;
	}

	OS_FarmStored GetStored(string className, bool create)
	{
		foreach (OS_FarmStored s : Storage)
		{
			if (s && s.ClassName == className)
				return s;
		}
		if (!create)
			return null;
		OS_FarmStored added = new OS_FarmStored();
		added.ClassName = className;
		Storage.Insert(added);
		return added;
	}
}

class OS_FarmsData
{
	ref array<ref OS_Farm> Farms = new array<ref OS_Farm>;
}
