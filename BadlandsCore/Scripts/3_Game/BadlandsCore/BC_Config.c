// Datové třídy konfigurace. Server je načítá z $profile:BadlandsCore/*.json
// Pokud soubor neexistuje, vytvoří se s výchozími hodnotami.

class BC_ConfigFile<Class T>
{
	//! defaults = objekt s výchozími hodnotami (uloží se, pokud soubor chybí)
	//! fresh    = prázdný objekt, do kterého se načte existující soubor
	static T Load(string fileName, T defaults, T fresh)
	{
		string path = BC_DIR + fileName;
		string err;

		MakeDirectory(BC_DIR);

		if (!FileExist(path))
		{
			if (JsonFileLoader<T>.SaveFile(path, defaults, err))
				Print(BC_TAG + "Vytvořen výchozí konfig: " + path);
			else
				Print(BC_TAG + "CHYBA při ukládání " + path + ": " + err);
			return defaults;
		}

		if (!JsonFileLoader<T>.LoadFile(path, fresh, err) || !fresh)
		{
			Print(BC_TAG + "CHYBA v " + path + " - používám výchozí hodnoty. " + err);
			return defaults;
		}

		Print(BC_TAG + "Načteno: " + path);
		return fresh;
	}
}

// ---------------------------------------------------------------- Startovní výbava

class BC_Loadout
{
	string Name;
	int Weight = 1;                                  // čím vyšší, tím častěji se sada vybere
	ref array<string> Clothes = new array<string>;   // "A|B|C" = náhodně jedna z možností
	ref array<string> Items = new array<string>;

	void BC_Loadout(string name = "", int weight = 1)
	{
		Name = name;
		Weight = weight;
	}
}

class BC_LoadoutConfig
{
	bool Enabled = true;
	bool ReplaceVanillaGear = true;   // smaže vanilla oblečení/věci a dá jen tuto výbavu
	ref array<ref BC_Loadout> Loadouts = new array<ref BC_Loadout>;

	void FillDefaults()
	{
		BC_Loadout a = new BC_Loadout("Poutnik", 3);
		a.Clothes.Insert("TShirt_Beige|TShirt_Grey|TShirt_Green");
		a.Clothes.Insert("Jeans_Blue|Jeans_Brown|Jeans_Grey");
		a.Clothes.Insert("AthleticShoes_Brown|AthleticShoes_Black");
		a.Items.Insert("BandageDressing");
		a.Items.Insert("WaterBottle");
		a.Items.Insert("Apple|Pear|Plum");
		a.Items.Insert("Chemlight_White");
		Loadouts.Insert(a);

		BC_Loadout b = new BC_Loadout("Delnik", 2);
		b.Clothes.Insert("Shirt_BlueCheck|Shirt_RedCheck|Shirt_GreenCheck");
		b.Clothes.Insert("CargoPants_Beige|CargoPants_Grey|CargoPants_Green");
		b.Clothes.Insert("WorkingBoots_Brown|WorkingBoots_Grey");
		b.Items.Insert("BandageDressing");
		b.Items.Insert("Canteen");
		b.Items.Insert("BakedBeansCan");
		b.Items.Insert("StoneKnife");
		Loadouts.Insert(b);

		BC_Loadout c = new BC_Loadout("Turista", 1);
		c.Clothes.Insert("Hoodie_Brown|Hoodie_Grey|Hoodie_Green");
		c.Clothes.Insert("CargoPants_Black|CargoPants_Blue");
		c.Clothes.Insert("HikingBootsLow_Black|HikingBootsLow_Grey");
		c.Items.Insert("BandageDressing");
		c.Items.Insert("WaterBottle");
		c.Items.Insert("TacticalBaconCan");
		c.Items.Insert("Flashlight");
		c.Items.Insert("Battery9V");
		Loadouts.Insert(c);
	}
}

// ---------------------------------------------------------------- Výběr spawnu

class BC_SpawnZone
{
	string Name;
	ref array<vector> Points = new array<vector>;   // [x, 0, z] - výška se dopočítá

	void BC_SpawnZone(string name = "")
	{
		Name = name;
	}
}

class BC_SpawnConfig
{
	bool Enabled = true;
	int MenuTimeoutSeconds = 60;       // po této době menu zmizí a hráč zůstane na vanilla spawnu
	bool ProtectWhileChoosing = true;  // nesmrtelnost, dokud má hráč otevřené menu
	ref array<ref BC_SpawnZone> Zones = new array<ref BC_SpawnZone>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečnými místy z mapy Nasdara !!!
		BC_SpawnZone a = new BC_SpawnZone("Priklad - Jih");
		a.Points.Insert(Vector(1000, 0, 1000));
		a.Points.Insert(Vector(1050, 0, 1020));
		a.Points.Insert(Vector(1100, 0, 980));
		Zones.Insert(a);

		BC_SpawnZone b = new BC_SpawnZone("Priklad - Vychod");
		b.Points.Insert(Vector(3000, 0, 1500));
		b.Points.Insert(Vector(3060, 0, 1540));
		b.Points.Insert(Vector(2950, 0, 1480));
		Zones.Insert(b);
	}
}

// ---------------------------------------------------------------- Safe zóny

class BC_SafeZone
{
	string Name;
	float X;
	float Z;
	float Radius = 100;

	void BC_SafeZone(string name = "", float x = 0, float z = 0, float radius = 100)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class BC_SafeZoneConfig
{
	bool Enabled = true;
	bool Godmode = true;               // hráči v zóně nedostávají poškození
	bool RemoveZombies = true;         // zombíci v zóně jsou pravidelně odstraněni
	int ZombieCleanupSeconds = 10;
	string EnterMessage = "Vstoupil jsi do bezpečné zóny";
	string LeaveMessage = "Opustil jsi bezpečnou zónu";
	ref array<ref BC_SafeZone> Zones = new array<ref BC_SafeZone>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečným místem (např. kolem obchodníka) !!!
		Zones.Insert(new BC_SafeZone("Priklad - Trziste", 2000, 2000, 150));
	}
}

// ---------------------------------------------------------------- Zprávy

class BC_MessagesConfig
{
	string WelcomeTitle = "Vítej na serveru!";
	string WelcomeText = "Klávesa B = obchod (u obchodníka). Pravidla najdeš na našem Discordu.";
	int WelcomeSeconds = 12;
	int BroadcastIntervalMinutes = 15;     // 0 = vypnuto
	ref array<string> Broadcasts = new array<string>;
	bool KillFeed = true;                  // hláška do chatu při úmrtí hráče

	void FillDefaults()
	{
		Broadcasts.Insert("Nezapomeň: v bezpečné zóně je zakázáno střílet.");
		Broadcasts.Insert("Přidej se na náš Discord!");
		Broadcasts.Insert("Za každého zabitého zombíka dostaneš kredity do obchodu.");
	}
}

// ---------------------------------------------------------------- Admini

class BC_AdminConfig
{
	// Steam64 ID adminů (17 číslic, např. 76561198000000000)
	ref array<string> AdminSteamIds = new array<string>;
	bool LogActions = true;
}

// ---------------------------------------------------------------- Obchod

class BC_TraderItem
{
	string ClassName;
	int Buy = -1;    // cena nákupu, -1 = nelze koupit
	int Sell = -1;   // výkupní cena, -1 = nelze prodat

	void BC_TraderItem(string className = "", int buy = -1, int sell = -1)
	{
		ClassName = className;
		Buy = buy;
		Sell = sell;
	}
}

class BC_TraderCategory
{
	string Name;
	ref array<ref BC_TraderItem> Items = new array<ref BC_TraderItem>;

	void BC_TraderCategory(string name = "")
	{
		Name = name;
	}

	void Add(string className, int buy, int sell)
	{
		Items.Insert(new BC_TraderItem(className, buy, sell));
	}
}

class BC_TraderLocation
{
	string Name;
	float X;
	float Z;
	float Radius = 15;   // jak blízko musí hráč být, aby mohl otevřít obchod

	void BC_TraderLocation(string name = "", float x = 0, float z = 0, float radius = 15)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class BC_TraderConfig
{
	bool Enabled = true;
	string CurrencyName = "Kr";
	int StartingBalance = 100;
	int ZombieKillReward = 5;
	int PlayerKillReward = 0;
	ref array<ref BC_TraderLocation> Traders = new array<ref BC_TraderLocation>;
	ref array<ref BC_TraderCategory> Categories = new array<ref BC_TraderCategory>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - stejné místo jako ukázková safe zóna !!!
		Traders.Insert(new BC_TraderLocation("Priklad - Trziste", 2000, 2000, 15));

		BC_TraderCategory food = new BC_TraderCategory("Jidlo a piti");
		food.Add("WaterBottle", 20, 5);
		food.Add("Canteen", 40, 10);
		food.Add("SodaCan_Cola", 15, 3);
		food.Add("TacticalBaconCan", 30, 8);
		food.Add("BakedBeansCan", 25, 6);
		food.Add("Apple", 8, 2);
		Categories.Insert(food);

		BC_TraderCategory med = new BC_TraderCategory("Medicina");
		med.Add("BandageDressing", 15, 4);
		med.Add("PainkillerTablets", 30, 8);
		med.Add("CharcoalTablets", 30, 8);
		med.Add("TetracyclineAntibiotics", 60, 15);
		med.Add("Morphine", 80, 20);
		med.Add("Epinephrine", 80, 20);
		med.Add("SalineBagIV", 100, 25);
		Categories.Insert(med);

		BC_TraderCategory tools = new BC_TraderCategory("Nastroje");
		tools.Add("Matchbox", 15, 3);
		tools.Add("Flashlight", 40, 10);
		tools.Add("Battery9V", 20, 5);
		tools.Add("Compass", 50, 12);
		tools.Add("Hammer", 40, 10);
		tools.Add("Pliers", 50, 12);
		tools.Add("Hatchet", 60, 15);
		tools.Add("Shovel", 80, 20);
		tools.Add("Binoculars", 100, 25);
		Categories.Insert(tools);

		BC_TraderCategory bags = new BC_TraderCategory("Batohy");
		bags.Add("TaloonBag_Blue", 80, 20);
		bags.Add("HuntingBag", 200, 50);
		bags.Add("MountainBag_Green", 300, 75);
		Categories.Insert(bags);

		BC_TraderCategory guns = new BC_TraderCategory("Zbrane a munice");
		guns.Add("MakarovIJ70", 200, 50);
		guns.Add("Mag_IJ70_8Rnd", 30, 8);
		guns.Add("Ammo_380", 40, 10);
		guns.Add("Mosin9130", 400, 100);
		guns.Add("Ammo_762x54", 60, 15);
		guns.Add("SKS", 500, 125);
		guns.Add("Ammo_762x39", 60, 15);
		Categories.Insert(guns);
	}
}
