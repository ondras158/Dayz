// Konfigurace aut: autobazar, servis, garáže. Soubor $profile:OS_Core/Vehicles.json

class OS_VehicleOffer
{
	string ClassName;
	int Price;
	ref array<string> Parts = new array<string>;   // díly, které auto dostane (a které servis doplní)

	void OS_VehicleOffer(string className = "", int price = 0)
	{
		ClassName = className;
		Price = price;
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
	bool Dealer;            // prodej aut
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
	int ClaimPrice = 500;              // přepsat nalezené auto bez majitele na sebe (dostaneš klíč)
	int KeyCopyPrice = 100;            // kopie klíče (jen majitel)
	int ChangeLockPrice = 200;         // nový zámek - staré klíče přestanou fungovat (jen majitel)
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
		loc.Service = true;
		loc.Garage = true;
		loc.SpawnX = 2120;
		loc.SpawnZ = 2000;
		loc.SpawnYaw = 90;
		Locations.Insert(loc);

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
	int KeyCode;
	string OwnerId;
	bool Locked;
}

class OS_GarageData
{
	ref array<ref OS_StoredVehicle> Vehicles = new array<ref OS_StoredVehicle>;
}
