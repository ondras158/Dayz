// Konfigurace aut: autobazar, servis, garáže. Soubor $profile:BadlandsCore/Vehicles.json

class BC_VehicleOffer
{
	string ClassName;
	int Price;
	ref array<string> Parts = new array<string>;   // díly, které auto dostane (a které servis doplní)

	void BC_VehicleOffer(string className = "", int price = 0)
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

class BC_VehicleLocation
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

	void BC_VehicleLocation(string name = "", float x = 0, float z = 0)
	{
		Name = name;
		X = x;
		Z = z;
	}
}

class BC_VehicleConfig
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
	bool OnlyOwnerCanStore = true;     // auto s majitelem může do garáže uložit jen majitel
	ref array<ref BC_VehicleLocation> Locations = new array<ref BC_VehicleLocation>;
	ref array<ref BC_VehicleOffer> Offers = new array<ref BC_VehicleOffer>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečnými místy z mapy Nasdara !!!
		BC_VehicleLocation loc = new BC_VehicleLocation("Priklad - Autocentrum", 2100, 2000);
		loc.Dealer = true;
		loc.Service = true;
		loc.Garage = true;
		loc.SpawnX = 2120;
		loc.SpawnZ = 2000;
		loc.SpawnYaw = 90;
		Locations.Insert(loc);

		BC_VehicleOffer ada = new BC_VehicleOffer("OffroadHatchback", 2500);
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

		BC_VehicleOffer olga = new BC_VehicleOffer("CivilianSedan", 3000);
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

		BC_VehicleOffer sarka = new BC_VehicleOffer("Sedan_02", 3000);
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

		BC_VehicleOffer gunter = new BC_VehicleOffer("Hatchback_02", 3500);
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

		BC_VehicleOffer m1025 = new BC_VehicleOffer("Offroad_02", 6000);
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

		BC_VehicleOffer v3s = new BC_VehicleOffer("Truck_01_Covered", 8000);
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

// Uložená auta v garáži: $profile:BadlandsCore/Garages/<SteamID>.json

class BC_StoredVehicle
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

class BC_GarageData
{
	ref array<ref BC_StoredVehicle> Vehicles = new array<ref BC_StoredVehicle>;
}
