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
		Broadcasts.Insert("Nalezené věci můžeš prodat u obchodníka (klávesa B).");
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
	int Sell = -1;   // výkupní cena, -1 = nelze prodat (obojí -1 = skryté, vidí jen admin)

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

	//! zbraň: nákup za 4x výkupní cenu
	void AddWeapon(string className, int sell)
	{
		Items.Insert(new BC_TraderItem(className, sell * 4, sell));
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

class BC_CategoryRule
{
	string Category;
	int DefaultBuy = 50;     // cena pro automaticky přidané předměty
	int DefaultSell = 10;
	ref array<string> BaseClasses = new array<string>;   // předek v configu (např. Weapon_Base)
	ref array<string> NameContains = new array<string>;  // část classname (např. Bandage)
	ref array<string> Slots = new array<string>;         // inventorySlot přesně (např. Back)
	ref array<string> SlotContains = new array<string>;  // část inventorySlot (např. weapon)

	void BC_CategoryRule(string category = "", int buy = 50, int sell = 10)
	{
		Category = category;
		DefaultBuy = buy;
		DefaultSell = sell;
	}
}

class BC_TraderConfig
{
	bool Enabled = true;
	string CurrencyName = "Kr";
	int StartingBalance = 100;
	int ZombieKillReward = 0;
	int PlayerKillReward = 0;
	bool ScaleSellPriceByCondition = true;  // opotřebovaný předmět = nižší výkupní cena (50-100 %)

	// Při startu serveru projde všechny předměty ve hře (i z módů) a chybějící přidá do obchodu.
	// Ceny, které už v souboru jsou (i upravené adminem), se nemění.
	bool AutoAddItems = true;
	ref array<string> ExcludeContains = new array<string>;     // classname obsahující tento text se nepřidá
	ref array<ref BC_CategoryRule> CategoryRules = new array<ref BC_CategoryRule>; // první shoda vyhrává

	ref array<ref BC_TraderLocation> Traders = new array<ref BC_TraderLocation>;
	ref array<ref BC_TraderCategory> Categories = new array<ref BC_TraderCategory>;

	BC_TraderCategory GetOrCreateCategory(string name)
	{
		foreach (BC_TraderCategory existing : Categories)
		{
			if (existing && existing.Name == name)
				return existing;
		}
		BC_TraderCategory created = new BC_TraderCategory(name);
		Categories.Insert(created);
		return created;
	}

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - stejné místo jako ukázková safe zóna !!!
		Traders.Insert(new BC_TraderLocation("Priklad - Trziste", 2000, 2000, 15));

		ExcludeContains.Insert("_Base");
		ExcludeContains.Insert("Debug");
		ExcludeContains.Insert("Static");
		ExcludeContains.Insert("Proxy");
		ExcludeContains.Insert("Land_");
		ExcludeContains.Insert("Hologram");
		ExcludeContains.Insert("Preview");
		ExcludeContains.Insert("Dummy");
		ExcludeContains.Insert("_TEST");
		ExcludeContains.Insert("Fx");
		ExcludeContains.Insert("BC_");   // klíče a peníze módu se v obchodě neprodávají

		// --- pravidla kategorií (pořadí je důležité) ---
		BC_CategoryRule r;

		r = new BC_CategoryRule("Pistole", 500, 80);
		r.BaseClasses.Insert("Pistol_Base");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Zbrane", 1200, 200);
		r.BaseClasses.Insert("Weapon_Base");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Munice", 60, 10);
		r.BaseClasses.Insert("Ammunition_Base");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Zasobniky", 100, 15);
		r.BaseClasses.Insert("Magazine_Base");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Prislusenstvi zbrani", 250, 40);
		r.BaseClasses.Insert("ItemOptics");
		r.BaseClasses.Insert("ItemSuppressor");
		r.SlotContains.Insert("weapon");
		r.SlotContains.Insert("pistol");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Medicina", 60, 10);
		r.NameContains.Insert("Bandage");
		r.NameContains.Insert("Morphine");
		r.NameContains.Insert("Epinephrine");
		r.NameContains.Insert("Saline");
		r.NameContains.Insert("BloodBag");
		r.NameContains.Insert("BloodTest");
		r.NameContains.Insert("Tetracycline");
		r.NameContains.Insert("Painkiller");
		r.NameContains.Insert("Charcoal");
		r.NameContains.Insert("Vitamin");
		r.NameContains.Insert("Iodine");
		r.NameContains.Insert("Disinfectant");
		r.NameContains.Insert("PurificationTablets");
		r.NameContains.Insert("Splint");
		r.NameContains.Insert("Defibrillator");
		r.NameContains.Insert("Thermometer");
		r.NameContains.Insert("StartKitIV");
		r.NameContains.Insert("Syringe");
		r.NameContains.Insert("Antidote");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Batohy", 200, 30);
		r.Slots.Insert("Back");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Obleceni", 50, 8);
		r.BaseClasses.Insert("Clothing_Base");
		r.BaseClasses.Insert("Clothing");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Jidlo a piti", 30, 5);
		r.BaseClasses.Insert("Edible_Base");
		r.BaseClasses.Insert("Bottle_Base");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Dily aut", 150, 25);
		r.BaseClasses.Insert("CarWheel");
		r.BaseClasses.Insert("CarDoor");
		r.NameContains.Insert("CarBattery");
		r.NameContains.Insert("TruckBattery");
		r.NameContains.Insert("SparkPlug");
		r.NameContains.Insert("GlowPlug");
		r.NameContains.Insert("CarRadiator");
		r.NameContains.Insert("HeadlightH7");
		r.NameContains.Insert("Hood");
		r.NameContains.Insert("Trunk");
		CategoryRules.Insert(r);

		r = new BC_CategoryRule("Stavba zakladny", 100, 15);
		r.BaseClasses.Insert("TentBase");
		r.BaseClasses.Insert("Container_Base");
		r.NameContains.Insert("Nail");
		r.NameContains.Insert("Plank");
		r.NameContains.Insert("WoodenLog");
		r.NameContains.Insert("MetalPlate");
		r.NameContains.Insert("MetalWire");
		r.NameContains.Insert("BarbedWire");
		r.NameContains.Insert("FenceKit");
		r.NameContains.Insert("WatchtowerKit");
		r.NameContains.Insert("TerritoryFlagKit");
		r.NameContains.Insert("ShelterKit");
		r.NameContains.Insert("CodeLock");
		r.NameContains.Insert("CombinationLock");
		CategoryRules.Insert(r);

		// bez podmínek = všechno ostatní
		CategoryRules.Insert(new BC_CategoryRule("Nastroje a ostatni", 40, 6));

		// --- ručně nacenené zbraně (zbytek doplní automatika) ---
		// nákup = 4x výkupní cena, takže koupit a prodat se nevyplatí
		BC_TraderCategory pistols = GetOrCreateCategory("Pistole");
		pistols.AddWeapon("MakarovIJ70", 40);
		pistols.AddWeapon("CZ75", 60);
		pistols.AddWeapon("Glock19", 70);
		pistols.AddWeapon("FNX45", 80);
		pistols.AddWeapon("Colt1911", 70);
		pistols.AddWeapon("MKII", 40);
		pistols.AddWeapon("Deagle", 120);
		pistols.AddWeapon("Magnum", 100);

		BC_TraderCategory guns = GetOrCreateCategory("Zbrane");
		guns.AddWeapon("Izh18", 60);
		guns.AddWeapon("Ruger1022", 70);
		guns.AddWeapon("Izh43Shotgun", 80);
		guns.AddWeapon("Mp133Shotgun", 100);
		guns.AddWeapon("CZ527", 150);
		guns.AddWeapon("Mosin9130", 150);
		guns.AddWeapon("Winchester70", 180);
		guns.AddWeapon("SKS", 180);
		guns.AddWeapon("Repeater", 120);
		guns.AddWeapon("B95", 160);
		guns.AddWeapon("Saiga", 250);
		guns.AddWeapon("Scout", 250);
		guns.AddWeapon("CZ61", 120);
		guns.AddWeapon("MP5K", 200);
		guns.AddWeapon("UMP45", 220);
		guns.AddWeapon("AKS74U", 280);
		guns.AddWeapon("AKM", 350);
		guns.AddWeapon("AK74", 350);
		guns.AddWeapon("AK101", 380);
		guns.AddWeapon("M4A1", 400);
		guns.AddWeapon("M16A2", 380);
		guns.AddWeapon("FAL", 450);
		guns.AddWeapon("SVD", 500);
		guns.AddWeapon("VSS", 500);
	}
}
