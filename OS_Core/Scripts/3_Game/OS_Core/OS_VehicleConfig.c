// Konfigurace aut: autobazar, servis, garáže. Soubor $profile:OS_Core/Vehicles.json

class OS_VehicleOffer
{
	string ClassName;
	int Price;
	string Category = "car";                       // "car" = auta, "bike" = motorky, "heli" = helikoptéry, "boat" = lodě
	ref array<string> Parts = new array<string>;   // díly, které vozidlo dostane (a které servis doplní)

	void OS_VehicleOffer(string className = "", int price = 0, string category = "car")
	{
		ClassName = className;
		Price = price;
		Category = category;
	}

	void AddParts(string part, int count = 1)
	{
		for (int i = 0; i < count; i++)
		{
			Parts.Insert(part);
		}
	}
}

class OS_VehicleLocation
{
	string Name;
	float X;
	float Z;
	float Radius = 30;      // kde musí hráč stát
	bool Dealer;            // prodej vozidel
	string DealerCategory = "car";   // co prodává: "car", "bike", "heli", "boat" - víc kategorií čárkou, např. "car,bike"
	bool SpawnOnWater = false;       // vozidlo se postaví na hladinu moře (přístaviště)
	float SpawnClearRadius = 5;      // místo pro nové vozidlo musí být volné v tomto okruhu (heliport ~12)
	bool Service;           // oprava, tankování, doplnění dílů
	bool Garage;            // uložení / vyzvednutí auta
	float SpawnX;           // kam se postaví koupené / vyzvednuté auto
	float SpawnZ;
	float SpawnYaw;         // natočení auta ve stupních (0 = sever)

	void OS_VehicleLocation(string name = "", float x = 0, float z = 0)
	{
		Name = name;
		X = x;
		Z = z;
	}
}

class OS_VehicleConfig
{
	bool Enabled = true;
	float CarSearchRadius = 20;        // jak daleko od hráče se hledá auto pro servis/garáž
	int RepairFullPrice = 400;         // cena opravy úplně zničeného auta (oprava se platí podle poškození)
	float RefuelPricePerLiter = 2;
	int PartPrice = 40;                // cena za každý doplněný chybějící díl
	int GarageStoreFee = 20;
	int GarageMaxVehicles = 3;
	int ClaimPrice = 500;              // přepsat nalezené auto bez majitele na sebe
	bool OnlyOwnerCanStore = true;     // auto s majitelem může do garáže uložit jen majitel (nebo člen jeho skupiny)
	bool SharedGroupGarage = true;     // členové skupiny mají společnou garáž
	int GroupGarageMaxVehicles = 6;
	ref array<ref OS_VehicleLocation> Locations = new array<ref OS_VehicleLocation>;
	ref array<ref OS_VehicleOffer> Offers = new array<ref OS_VehicleOffer>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečnými místy z tvé mapy !!!
		OS_VehicleLocation loc = new OS_VehicleLocation("Priklad - Autocentrum", 2100, 2000);
		loc.Dealer = true;
		loc.DealerCategory = "car,bike";   // auta i motorky
		loc.Service = true;
		loc.Garage = true;
		loc.SpawnX = 2120;
		loc.SpawnZ = 2000;
		loc.SpawnYaw = 90;
		Locations.Insert(loc);

		// heliport - classname helikoptér doplň podle módu s helikoptérami (vanilla žádné nemá)
		OS_VehicleLocation heli = new OS_VehicleLocation("Priklad - Heliport", 2300, 2000);
		heli.Radius = 40;
		heli.Dealer = true;
		heli.DealerCategory = "heli";
		heli.Service = true;
		heli.Garage = true;
		heli.SpawnX = 2330;
		heli.SpawnZ = 2000;
		heli.SpawnYaw = 0;
		heli.SpawnClearRadius = 12;
		Locations.Insert(heli);

		// přístaviště - SpawnX/SpawnZ musí být na moři
		OS_VehicleLocation harbor = new OS_VehicleLocation("Priklad - Pristav", 2500, 1800);
		harbor.Radius = 40;
		harbor.Dealer = true;
		harbor.DealerCategory = "boat";
		harbor.Service = true;
		harbor.Garage = true;
		harbor.SpawnX = 2500;
		harbor.SpawnZ = 1760;
		harbor.SpawnYaw = 180;
		harbor.SpawnClearRadius = 8;
		harbor.SpawnOnWater = true;
		Locations.Insert(harbor);

		OS_VehicleOffer ada = new OS_VehicleOffer("OffroadHatchback", 2500);
		ada.AddParts("HatchbackWheel", 4);
		ada.AddParts("HatchbackDoors_Driver");
		ada.AddParts("HatchbackDoors_CoDriver");
		ada.AddParts("HatchbackHood");
		ada.AddParts("HatchbackTrunk");
		ada.AddParts("CarBattery");
		ada.AddParts("CarRadiator");
		ada.AddParts("SparkPlug");
		ada.AddParts("HeadlightH7", 2);
		Offers.Insert(ada);

		OS_VehicleOffer olga = new OS_VehicleOffer("CivilianSedan", 3000);
		olga.AddParts("CivSedanWheel", 4);
		olga.AddParts("CivSedanDoors_Driver");
		olga.AddParts("CivSedanDoors_CoDriver");
		olga.AddParts("CivSedanDoors_BackLeft");
		olga.AddParts("CivSedanDoors_BackRight");
		olga.AddParts("CivSedanHood");
		olga.AddParts("CivSedanTrunk");
		olga.AddParts("CarBattery");
		olga.AddParts("CarRadiator");
		olga.AddParts("SparkPlug");
		olga.AddParts("HeadlightH7", 2);
		Offers.Insert(olga);

		OS_VehicleOffer sarka = new OS_VehicleOffer("Sedan_02", 3000);
		sarka.AddParts("Sedan_02_Wheel", 4);
		sarka.AddParts("Sedan_02_Door_1_1");
		sarka.AddParts("Sedan_02_Door_1_2");
		sarka.AddParts("Sedan_02_Door_2_1");
		sarka.AddParts("Sedan_02_Door_2_2");
		sarka.AddParts("Sedan_02_Hood");
		sarka.AddParts("Sedan_02_Trunk");
		sarka.AddParts("CarBattery");
		sarka.AddParts("CarRadiator");
		sarka.AddParts("SparkPlug");
		sarka.AddParts("HeadlightH7", 2);
		Offers.Insert(sarka);

		OS_VehicleOffer gunter = new OS_VehicleOffer("Hatchback_02", 3500);
		gunter.AddParts("Hatchback_02_Wheel", 4);
		gunter.AddParts("Hatchback_02_Door_1_1");
		gunter.AddParts("Hatchback_02_Door_1_2");
		gunter.AddParts("Hatchback_02_Door_2_1");
		gunter.AddParts("Hatchback_02_Door_2_2");
		gunter.AddParts("Hatchback_02_Hood");
		gunter.AddParts("Hatchback_02_Trunk");
		gunter.AddParts("CarBattery");
		gunter.AddParts("CarRadiator");
		gunter.AddParts("SparkPlug");
		gunter.AddParts("HeadlightH7", 2);
		Offers.Insert(gunter);

		OS_VehicleOffer m1025 = new OS_VehicleOffer("Offroad_02", 6000);
		m1025.AddParts("Offroad_02_Wheel", 4);
		m1025.AddParts("Offroad_02_Door_1_1");
		m1025.AddParts("Offroad_02_Door_1_2");
		m1025.AddParts("Offroad_02_Door_2_1");
		m1025.AddParts("Offroad_02_Door_2_2");
		m1025.AddParts("Offroad_02_Hood");
		m1025.AddParts("Offroad_02_Trunk");
		m1025.AddParts("CarBattery");
		m1025.AddParts("GlowPlug");
		m1025.AddParts("HeadlightH7", 2);
		Offers.Insert(m1025);

		// --- helikoptéry z módu DayZ Expansion (Vehicles) - bez módu se koupě nepovede a peníze se vrátí ---
		OS_VehicleOffer gyro = new OS_VehicleOffer("ExpansionGyrocopter", 15000, "heli");
		gyro.AddParts("ExpansionHelicopterBattery");
		gyro.AddParts("SparkPlug");
		gyro.AddParts("HeadlightH7");
		Offers.Insert(gyro);

		OS_VehicleOffer bigGyro = new OS_VehicleOffer("ExpansionBigGyrocopter", 20000, "heli");
		bigGyro.AddParts("ExpansionHelicopterBattery");
		bigGyro.AddParts("SparkPlug");
		bigGyro.AddParts("HeadlightH7");
		Offers.Insert(bigGyro);

		OS_VehicleOffer mh6 = new OS_VehicleOffer("ExpansionMh6", 40000, "heli");
		mh6.AddParts("Expansion_Mh6_Door_1_1");
		mh6.AddParts("Expansion_Mh6_Door_1_2");
		mh6.AddParts("Expansion_Mh6_Door_2_1");
		mh6.AddParts("Expansion_Mh6_Door_2_2");
		mh6.AddParts("ExpansionHydraulicHoses");
		mh6.AddParts("ExpansionIgniterPlug");
		mh6.AddParts("ExpansionHelicopterBattery");
		mh6.AddParts("HeadlightH7");
		Offers.Insert(mh6);

		OS_VehicleOffer uh1h = new OS_VehicleOffer("ExpansionUh1h", 60000, "heli");
		uh1h.AddParts("ExpansionUh1hDoor_1_1");
		uh1h.AddParts("ExpansionUh1hDoor_1_2");
		uh1h.AddParts("ExpansionUh1hDoor_2_1");
		uh1h.AddParts("ExpansionUh1hDoor_2_2");
		uh1h.AddParts("ExpansionHydraulicHoses");
		uh1h.AddParts("ExpansionIgniterPlug");
		uh1h.AddParts("ExpansionHelicopterBattery");
		uh1h.AddParts("HeadlightH7");
		Offers.Insert(uh1h);

		OS_VehicleOffer merlin = new OS_VehicleOffer("ExpansionMerlin", 90000, "heli");
		merlin.AddParts("ExpansionMerlinFrontWheel", 2);
		merlin.AddParts("ExpansionMerlinBackWheel", 2);
		merlin.AddParts("ExpansionHydraulicHoses");
		merlin.AddParts("ExpansionIgniterPlug");
		merlin.AddParts("ExpansionHelicopterBattery");
		merlin.AddParts("HeadlightH7");
		Offers.Insert(merlin);

		// --- lodě: vanilla motorový člun ---
		OS_VehicleOffer boatBlue = new OS_VehicleOffer("Boat_01_Blue", 5000, "boat");
		boatBlue.AddParts("SparkPlug");
		Offers.Insert(boatBlue);
		OS_VehicleOffer boatOrange = new OS_VehicleOffer("Boat_01_Orange", 5000, "boat");
		boatOrange.AddParts("SparkPlug");
		Offers.Insert(boatOrange);
		OS_VehicleOffer boatBlack = new OS_VehicleOffer("Boat_01_Black", 5000, "boat");
		boatBlack.AddParts("SparkPlug");
		Offers.Insert(boatBlack);
		OS_VehicleOffer boatCamo = new OS_VehicleOffer("Boat_01_Camo", 5500, "boat");
		boatCamo.AddParts("SparkPlug");
		Offers.Insert(boatCamo);

		// --- lodě z módu DayZ Expansion (Vehicles) ---
		OS_VehicleOffer zodiac = new OS_VehicleOffer("ExpansionZodiacBoat", 8000, "boat");
		zodiac.AddParts("GlowPlug");
		zodiac.AddParts("TruckBattery");
		Offers.Insert(zodiac);

		OS_VehicleOffer utility = new OS_VehicleOffer("ExpansionUtilityBoat", 15000, "boat");
		utility.AddParts("GlowPlug");
		utility.AddParts("TruckBattery");
		utility.AddParts("HeadlightH7", 2);
		Offers.Insert(utility);

		OS_VehicleOffer vodnik = new OS_VehicleOffer("ExpansionVodnik", 30000, "boat");
		vodnik.AddParts("ExpansionVodnikWheel", 4);
		vodnik.AddParts("TruckBattery");
		vodnik.AddParts("SparkPlug");
		vodnik.AddParts("CarRadiator");
		vodnik.AddParts("ExpansionVodnikDoorDriver");
		vodnik.AddParts("ExpansionVodnikDoorCoDriver");
		vodnik.AddParts("HeadlightH7", 2);
		Offers.Insert(vodnik);

		// --- motorky (vanilla) ---
		array<string> bikeColors = {"Blue", "Yellow", "Red"};
		foreach (string bikeColor : bikeColors)
		{
			OS_VehicleOffer bike = new OS_VehicleOffer("Motorbike_01_" + bikeColor, 1500, "bike");
			bike.AddParts("Motorbike_01_Wheel_1");
			bike.AddParts("Motorbike_01_Wheel_2");
			bike.AddParts("SparkPlug");
			bike.AddParts("HeadlightH7");
			Offers.Insert(bike);
		}

		OS_VehicleOffer bike2 = new OS_VehicleOffer("Motorbike_02_Red", 2500, "bike");
		bike2.AddParts("Motorbike_02_Wheel_1");
		bike2.AddParts("Motorbike_02_Wheel_2");
		bike2.AddParts("Motorbike_02_ShieldFront_Red");
		bike2.AddParts("Motorbike_02_ShieldLeft_Red");
		bike2.AddParts("Motorbike_02_ShieldRight_Red");
		bike2.AddParts("SparkPlug");
		bike2.AddParts("HeadlightH7");
		Offers.Insert(bike2);

		OS_VehicleOffer v3s = new OS_VehicleOffer("Truck_01_Covered", 8000);
		v3s.AddParts("Truck_01_Wheel", 2);
		v3s.AddParts("Truck_01_WheelDouble", 4);
		v3s.AddParts("Truck_01_Door_1_1");
		v3s.AddParts("Truck_01_Door_2_1");
		v3s.AddParts("Truck_01_Hood");
		v3s.AddParts("TruckBattery");
		v3s.AddParts("HeadlightH7", 2);
		Offers.Insert(v3s);
	}
}

// Uložená auta v garáži: $profile:OS_Core/Garages/<SteamID>.json

class OS_StoredVehicle
{
	string ClassName;
	float Health = 1;
	ref array<string> Zones = new array<string>;
	ref array<float> ZoneHealth = new array<float>;
	ref array<string> Parts = new array<string>;
	ref array<float> PartHealth = new array<float>;
	float Fuel;
	float Oil;
	float Brake;
	float Coolant;
	string OwnerId;
	int GroupId;
	bool Locked;
}

class OS_GarageData
{
	ref array<ref OS_StoredVehicle> Vehicles = new array<ref OS_StoredVehicle>;
}
