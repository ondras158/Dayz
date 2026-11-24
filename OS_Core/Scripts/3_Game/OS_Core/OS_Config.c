// Datové třídy konfigurace. Server je načítá z $profile:OS_Core/*.json
// Pokud soubor neexistuje, vytvoří se s výchozími hodnotami.

class OS_ConfigFile<Class T>
{
	//! defaults = objekt s výchozími hodnotami (uloží se, pokud soubor chybí)
	//! fresh    = prázdný objekt, do kterého se načte existující soubor
	static T Load(string fileName, T defaults, T fresh)
	{
		string path = OS_DIR + fileName;
		string err;

		MakeDirectory(OS_DIR);

		if (!FileExist(path))
		{
			if (JsonFileLoader<T>.SaveFile(path, defaults, err))
				Print(OS_TAG + "Vytvořen výchozí konfig: " + path);
			else
				Print(OS_TAG + "CHYBA při ukládání " + path + ": " + err);
			return defaults;
		}

		if (!JsonFileLoader<T>.LoadFile(path, fresh, err) || !fresh)
		{
			Print(OS_TAG + "CHYBA v " + path + " - používám výchozí hodnoty. " + err);
			return defaults;
		}

		Print(OS_TAG + "Načteno: " + path);
		return fresh;
	}
}

// ---------------------------------------------------------------- Startovní výbava

class OS_Loadout
{
	string Name;
	int Weight = 1;                                  // čím vyšší, tím častěji se sada vybere
	ref array<string> Clothes = new array<string>;   // "A|B|C" = náhodně jedna z možností
	ref array<string> Items = new array<string>;

	void OS_Loadout(string name = "", int weight = 1)
	{
		Name = name;
		Weight = weight;
	}
}

class OS_LoadoutConfig
{
	bool Enabled = true;
	bool ReplaceVanillaGear = true;   // smaže vanilla oblečení/věci a dá jen tuto výbavu
	ref array<ref OS_Loadout> Loadouts = new array<ref OS_Loadout>;

	void FillDefaults()
	{
		OS_Loadout a = new OS_Loadout("Poutnik", 3);
		a.Clothes.Insert("TShirt_Beige|TShirt_Grey|TShirt_Green");
		a.Clothes.Insert("Jeans_Blue|Jeans_Brown|Jeans_Grey");
		a.Clothes.Insert("AthleticShoes_Brown|AthleticShoes_Black");
		a.Items.Insert("BandageDressing");
		a.Items.Insert("WaterBottle");
		a.Items.Insert("Apple|Pear|Plum");
		a.Items.Insert("Chemlight_White");
		Loadouts.Insert(a);

		OS_Loadout b = new OS_Loadout("Delnik", 2);
		b.Clothes.Insert("Shirt_BlueCheck|Shirt_RedCheck|Shirt_GreenCheck");
		b.Clothes.Insert("CargoPants_Beige|CargoPants_Grey|CargoPants_Green");
		b.Clothes.Insert("WorkingBoots_Brown|WorkingBoots_Grey");
		b.Items.Insert("BandageDressing");
		b.Items.Insert("Canteen");
		b.Items.Insert("BakedBeansCan");
		b.Items.Insert("StoneKnife");
		Loadouts.Insert(b);

		OS_Loadout c = new OS_Loadout("Turista", 1);
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

class OS_SpawnZone
{
	string Name;
	ref array<vector> Points = new array<vector>;   // [x, 0, z] - výška se dopočítá

	void OS_SpawnZone(string name = "")
	{
		Name = name;
	}
}

class OS_SpawnConfig
{
	bool Enabled = true;
	int MenuTimeoutSeconds = 60;       // po této době menu zmizí a hráč zůstane na vanilla spawnu
	bool ProtectWhileChoosing = true;  // nesmrtelnost, dokud má hráč otevřené menu
	ref array<ref OS_SpawnZone> Zones = new array<ref OS_SpawnZone>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečnými místy z tvé mapy !!!
		OS_SpawnZone a = new OS_SpawnZone("Priklad - Jih");
		a.Points.Insert(Vector(1000, 0, 1000));
		a.Points.Insert(Vector(1050, 0, 1020));
		a.Points.Insert(Vector(1100, 0, 980));
		Zones.Insert(a);

		OS_SpawnZone b = new OS_SpawnZone("Priklad - Vychod");
		b.Points.Insert(Vector(3000, 0, 1500));
		b.Points.Insert(Vector(3060, 0, 1540));
		b.Points.Insert(Vector(2950, 0, 1480));
		Zones.Insert(b);
	}
}

// ---------------------------------------------------------------- Safe zóny

class OS_SafeZone
{
	string Name;
	float X;
	float Z;
	float Radius = 100;

	void OS_SafeZone(string name = "", float x = 0, float z = 0, float radius = 100)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class OS_SafeZoneConfig
{
	bool Enabled = true;
	bool Godmode = true;               // hráči v zóně nedostávají poškození
	bool RemoveZombies = true;         // zombíci v zóně jsou pravidelně odstraněni
	int ZombieCleanupSeconds = 10;
	string EnterMessage = "Vstoupil jsi do bezpečné zóny";
	string LeaveMessage = "Opustil jsi bezpečnou zónu";
	ref array<ref OS_SafeZone> Zones = new array<ref OS_SafeZone>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečným místem (např. kolem obchodníka) !!!
		Zones.Insert(new OS_SafeZone("Priklad - Trziste", 2000, 2000, 150));
	}
}

// ---------------------------------------------------------------- Zprávy

class OS_MessagesConfig
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

class OS_AdminConfig
{
	// Steam64 ID adminů (17 číslic, např. 76561198000000000)
	ref array<string> AdminSteamIds = new array<string>;
	bool LogActions = true;
}

// ---------------------------------------------------------------- Obchod

class OS_TraderItem
{
	string ClassName;
	int Buy = -1;    // cena nákupu, -1 = nelze koupit
	int Sell = -1;   // výkupní cena, -1 = nelze prodat (obojí -1 = skryté, vidí jen admin)

	void OS_TraderItem(string className = "", int buy = -1, int sell = -1)
	{
		ClassName = className;
		Buy = buy;
		Sell = sell;
	}
}

class OS_TraderCategory
{
	string Name;
	ref array<ref OS_TraderItem> Items = new array<ref OS_TraderItem>;

	void OS_TraderCategory(string name = "")
	{
		Name = name;
	}

	void Add(string className, int buy, int sell)
	{
		Items.Insert(new OS_TraderItem(className, buy, sell));
	}

	//! zbraň: nákup za 4x výkupní cenu
	void AddWeapon(string className, int sell)
	{
		Items.Insert(new OS_TraderItem(className, sell * 4, sell));
	}
}

class OS_TraderLocation
{
	string Name;
	float X;
	float Z;
	float Radius = 15;   // jak blízko musí hráč být, aby mohl otevřít obchod
	// kategorie, které obchodník prodává i vykupuje (názvy z Categories); prázdné = všechno
	ref array<string> Categories = new array<string>;

	void OS_TraderLocation(string name = "", float x = 0, float z = 0, float radius = 15)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class OS_CategoryRule
{
	string Category;
	int DefaultBuy = 50;     // cena pro automaticky přidané předměty
	int DefaultSell = 10;
	ref array<string> BaseClasses = new array<string>;   // předek v configu (např. Weapon_Base)
	ref array<string> NameContains = new array<string>;  // část classname (např. Bandage)
	ref array<string> Slots = new array<string>;         // inventorySlot přesně (např. Back)
	ref array<string> SlotContains = new array<string>;  // část inventorySlot (např. weapon)

	void OS_CategoryRule(string category = "", int buy = 50, int sell = 10)
	{
		Category = category;
		DefaultBuy = buy;
		DefaultSell = sell;
	}
}

class OS_TraderConfig
{
	bool Enabled = true;
	int StartingBalance = 100;
	int ZombieKillReward = 0;
	int PlayerKillReward = 0;
	bool ScaleSellPriceByCondition = true;  // opotřebovaný předmět = nižší výkupní cena (50-100 %)

	// Při startu serveru projde všechny předměty ve hře (i z módů) a chybějící přidá do obchodu.
	// Ceny, které už v souboru jsou (i upravené adminem), se nemění.
	bool AutoAddItems = true;
	ref array<string> ExcludeContains = new array<string>;     // classname obsahující tento text se nepřidá
	ref array<ref OS_CategoryRule> CategoryRules = new array<ref OS_CategoryRule>; // první shoda vyhrává

	ref array<ref OS_TraderLocation> Traders = new array<ref OS_TraderLocation>;
	ref array<ref OS_TraderCategory> Categories = new array<ref OS_TraderCategory>;

	OS_TraderCategory GetOrCreateCategory(string name)
	{
		foreach (OS_TraderCategory existing : Categories)
		{
			if (existing && existing.Name == name)
				return existing;
		}
		OS_TraderCategory created = new OS_TraderCategory(name);
		Categories.Insert(created);
		return created;
	}

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - stejné místo jako ukázková safe zóna !!!
		Traders.Insert(new OS_TraderLocation("Priklad - Trziste", 2000, 2000, 15));   // prodává všechno

		OS_TraderLocation gunsmith = new OS_TraderLocation("Priklad - Zbrojir", 2040, 2000, 10);
		gunsmith.Categories.Insert("Pistole");
		gunsmith.Categories.Insert("Zbrane");
		gunsmith.Categories.Insert("Munice");
		gunsmith.Categories.Insert("Zasobniky");
		gunsmith.Categories.Insert("Prislusenstvi zbrani");
		gunsmith.Categories.Insert("Vybusniny");
		Traders.Insert(gunsmith);

		OS_TraderLocation tailor = new OS_TraderLocation("Priklad - Obleceni", 1960, 2000, 10);
		tailor.Categories.Insert("Obleceni");
		tailor.Categories.Insert("Batohy");
		Traders.Insert(tailor);

		OS_TraderLocation grocer = new OS_TraderLocation("Priklad - Potraviny", 2000, 2040, 10);
		grocer.Categories.Insert("Jidlo a piti");
		Traders.Insert(grocer);

		// černý trh - drogy
		OS_TraderLocation blackMarket = new OS_TraderLocation("Priklad - Cerny trh", 1900, 1900, 8);
		blackMarket.Categories.Insert("Drogy");
		Traders.Insert(blackMarket);

		OS_TraderLocation pharmacy = new OS_TraderLocation("Priklad - Lekarna", 2000, 1960, 10);
		pharmacy.Categories.Insert("Medicina");
		Traders.Insert(pharmacy);

		OS_TraderLocation hardware = new OS_TraderLocation("Priklad - Zelezarstvi", 1960, 2040, 10);
		hardware.Categories.Insert("Nastroje a ostatni");
		hardware.Categories.Insert("Stavba zakladny");
		hardware.Categories.Insert("Elektronika");
		Traders.Insert(hardware);

		OS_TraderLocation hunter = new OS_TraderLocation("Priklad - Lovec", 1960, 1960, 10);
		hunter.Categories.Insert("Lov");
		hunter.Categories.Insert("Zvireci suroviny");
		Traders.Insert(hunter);

		// autodíly - ideálně u autocentra (servis a garáž jsou ve Vehicles.json)
		OS_TraderLocation autoparts = new OS_TraderLocation("Priklad - Autodily", 2100, 2030, 10);
		autoparts.Categories.Insert("Dily aut");
		Traders.Insert(autoparts);

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
		ExcludeContains.Insert("OS_");   // klíče a peníze módu se v obchodě neprodávají

		// --- pravidla kategorií (pořadí je důležité) ---
		OS_CategoryRule r;

		r = new OS_CategoryRule("Pistole", 500, 80);
		r.BaseClasses.Insert("Pistol_Base");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Zbrane", 1200, 200);
		r.BaseClasses.Insert("Weapon_Base");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Munice", 60, 10);
		r.BaseClasses.Insert("Ammunition_Base");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Zasobniky", 100, 15);
		r.BaseClasses.Insert("Magazine_Base");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Prislusenstvi zbrani", 250, 40);
		r.BaseClasses.Insert("ItemOptics");
		r.BaseClasses.Insert("ItemSuppressor");
		r.SlotContains.Insert("weapon");
		r.SlotContains.Insert("pistol");
		CategoryRules.Insert(r);

		// granáty a výbušniny (náboje do granátometu zůstávají v Munice - pravidlo Munice je výš)
		r = new OS_CategoryRule("Vybusniny", 300, 50);
		r.NameContains.Insert("Grenade");       // M67, RGD5, záblesková, kouřové
		r.NameContains.Insert("Explosive");     // plastická a improvizovaná výbušnina
		r.NameContains.Insert("ClaymoreMine");
		r.NameContains.Insert("LandMine");
		r.NameContains.Insert("Detonator");     // dálkový ovladač
		CategoryRules.Insert(r);

		// lov a zvířata - musí být před medicínou a jídlem (maso je jídlo, kosti/peří by jinak spadly do ostatních)
		r = new OS_CategoryRule("Lov", 80, 15);
		r.NameContains.Insert("Trap");          // pasti (na medvědy, králíky, ryby...)
		r.NameContains.Insert("Snare");
		r.NameContains.Insert("Fishing");       // rybářské pruty
		r.NameContains.Insert("Hook");          // háčky (i kostěné)
		r.NameContains.Insert("Bait");
		r.NameContains.Insert("Worm");
		r.NameContains.Insert("BoneKnife");
		r.NameContains.Insert("HuntingKnife");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Zvireci suroviny", 40, 12);
		r.NameContains.Insert("Pelt");          // kůže
		r.NameContains.Insert("SteakMeat");     // maso ze zvěře
		r.NameContains.Insert("BreastMeat");
		r.NameContains.Insert("FilletMeat");    // rybí filety
		r.NameContains.Insert("Lard");          // sádlo
		r.NameContains.Insert("Guts");          // vnitřnosti
		r.NameContains.Insert("Feather");       // peří
		r.NameContains.Insert("Bone");          // kosti
		CategoryRules.Insert(r);

		// drogy (černý trh) - před medicínou a jídlem (lysohlávka je jinak jídlo)
		r = new OS_CategoryRule("Drogy", 500, 120);
		r.NameContains.Insert("Psilocybe");     // lysohlávka (vanilla)
		r.NameContains.Insert("Weed");
		r.NameContains.Insert("Marijuana");
		r.NameContains.Insert("Cannabis");
		r.NameContains.Insert("Cocaine");
		r.NameContains.Insert("Heroin");
		r.NameContains.Insert("Meth");
		r.NameContains.Insert("Pervitin");
		r.NameContains.Insert("LSD");
		r.NameContains.Insert("Ecstasy");
		r.NameContains.Insert("Opium");
		r.NameContains.Insert("Narcotic");
		r.NameContains.Insert("Drug");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Medicina", 60, 10);
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
		r.NameContains.Insert("Tablets");       // všechny tablety (jinak by "Table" spadlo do nábytku)
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Batohy", 200, 30);
		r.Slots.Insert("Back");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Obleceni", 50, 8);
		r.BaseClasses.Insert("Clothing_Base");
		r.BaseClasses.Insert("Clothing");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Jidlo a piti", 30, 5);
		r.BaseClasses.Insert("Edible_Base");
		r.BaseClasses.Insert("Bottle_Base");
		// alkohol (hlavně z módů) - desinfekční alkohol zůstává v medicíně (pravidlo je výš)
		r.NameContains.Insert("Vodka");
		r.NameContains.Insert("Beer");
		r.NameContains.Insert("Whisk");         // Whisky / Whiskey
		r.NameContains.Insert("Wine");
		r.NameContains.Insert("Tequila");
		r.NameContains.Insert("Brandy");
		r.NameContains.Insert("Cognac");
		r.NameContains.Insert("Liquor");
		r.NameContains.Insert("Moonshine");
		r.NameContains.Insert("Slivovice");
		r.NameContains.Insert("Becherovka");
		r.NameContains.Insert("Alcohol");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Dily aut", 150, 25);
		r.BaseClasses.Insert("CarWheel");
		r.BaseClasses.Insert("CarDoor");
		r.NameContains.Insert("CarBattery");
		r.NameContains.Insert("HelicopterBattery");   // Expansion helikoptéry
		r.NameContains.Insert("IgniterPlug");
		r.NameContains.Insert("HydraulicHoses");
		r.NameContains.Insert("TruckBattery");
		r.NameContains.Insert("SparkPlug");
		r.NameContains.Insert("GlowPlug");
		r.NameContains.Insert("CarRadiator");
		r.NameContains.Insert("HeadlightH7");
		r.NameContains.Insert("Hood");
		r.NameContains.Insert("Trunk");
		CategoryRules.Insert(r);

		// elektronika (autobaterie a baterie helikoptér už jsou v Dily aut - pravidlo je výš)
		r = new OS_CategoryRule("Elektronika", 120, 20);
		r.NameContains.Insert("Radio");          // osobní a základnová vysílačka
		r.NameContains.Insert("Megaphone");
		r.NameContains.Insert("GPS");
		r.NameContains.Insert("Rangefinder");
		r.NameContains.Insert("Battery");        // 9V baterie
		r.NameContains.Insert("Charger");        // nabíječka baterií
		r.NameContains.Insert("Bulb");           // žárovky
		r.NameContains.Insert("Flashlight");
		r.NameContains.Insert("Electronic");     // sada na opravu elektroniky
		r.NameContains.Insert("AlarmClock");
		r.NameContains.Insert("KitchenTimer");
		r.NameContains.Insert("Geiger");
		r.NameContains.Insert("Dosimeter");
		r.NameContains.Insert("Generator");      // elektřina na základně
		r.NameContains.Insert("Spotlight");
		r.NameContains.Insert("CableReel");
		CategoryRules.Insert(r);

		r = new OS_CategoryRule("Stavba zakladny", 100, 15);
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
		// stany, úložné věci a nábytek (i z módů) - pravidlo je až za oblečením a jídlem,
		// takže "ChestHolster" zůstane oblečení
		r.NameContains.Insert("Tent");
		r.NameContains.Insert("CamoNet");
		r.NameContains.Insert("Camonet");
		r.NameContains.Insert("Crate");
		r.NameContains.Insert("Chest");
		r.NameContains.Insert("Barrel");
		r.NameContains.Insert("Stash");
		r.NameContains.Insert("Safe");
		r.NameContains.Insert("Locker");
		r.NameContains.Insert("Cabinet");
		r.NameContains.Insert("Shelf");
		r.NameContains.Insert("Table");
		r.NameContains.Insert("Chair");
		r.NameContains.Insert("Bench");
		r.NameContains.Insert("Bed");
		r.NameContains.Insert("Sofa");
		r.NameContains.Insert("Furniture");
		r.NameContains.Insert("Flag_");         // vlajky na stožár
		CategoryRules.Insert(r);

		// bez podmínek = všechno ostatní
		CategoryRules.Insert(new OS_CategoryRule("Nastroje a ostatni", 40, 6));

		// --- ručně nacenené zbraně (zbytek doplní automatika) ---
		// nákup = 4x výkupní cena, takže koupit a prodat se nevyplatí
		OS_TraderCategory pistols = GetOrCreateCategory("Pistole");
		pistols.AddWeapon("MakarovIJ70", 40);
		pistols.AddWeapon("CZ75", 60);
		pistols.AddWeapon("Glock19", 70);
		pistols.AddWeapon("FNX45", 80);
		pistols.AddWeapon("Colt1911", 70);
		pistols.AddWeapon("MKII", 40);
		pistols.AddWeapon("Deagle", 120);
		pistols.AddWeapon("Magnum", 100);

		OS_TraderCategory guns = GetOrCreateCategory("Zbrane");
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
