// Airdropy: bedna s lootem spadne z nebe na náhodné místo. Nastavení: $profile:OS_Core/Airdrops.json

class OS_AirdropLocation
{
	string Name;
	float X;
	float Z;
	float Radius = 150;       // bedna dopadne náhodně do tohoto okruhu

	void OS_AirdropLocation(string name = "", float x = 0, float z = 0, float radius = 150)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class OS_AirdropItem
{
	string Items;             // classname, víc možností oddělených | (náhodně jedna)
	float Chance = 1;         // šance 0..1
	int Count = 1;
	ref array<string> Attachments = new array<string>;   // např. zásobník a optika ke zbrani

	void OS_AirdropItem(string items = "", float chance = 1, int count = 1)
	{
		Items = items;
		Chance = chance;
		Count = count;
	}
}

class OS_AirdropLoot
{
	string Name;
	int Weight = 1;           // jak často se tahle sada vybere
	ref array<ref OS_AirdropItem> Items = new array<ref OS_AirdropItem>;

	void OS_AirdropLoot(string name = "", int weight = 1)
	{
		Name = name;
		Weight = weight;
	}

	OS_AirdropItem Add(string items, float chance = 1, int count = 1)
	{
		OS_AirdropItem item = new OS_AirdropItem(items, chance, count);
		Items.Insert(item);
		return item;
	}
}

class OS_AirdropConfig
{
	bool Enabled = true;
	float IntervalMinutes = 45;        // jak často padá airdrop (náhodně ±25 %)
	int MinPlayers = 1;                // minimální počet hráčů online
	float DropHeight = 150;            // výška, ze které bedna padá (m)
	float FallSpeed = 6;               // rychlost pádu na padáku (m/s)
	float LifetimeMinutes = 40;        // po dopadu bedna zmizí (i s tím, co v ní zbylo)
	string ContainerClass = "SeaChest";
	string SmokeClass = "M18SmokeGrenade_Red";   // kouř po dopadu ("" = žádný)
	bool AnnounceInChat = true;
	bool ShowOnMap = true;
	int GuardCount = 4;                // stráže kolem dopadu (vojenští nakažení), 0 = žádné
	string GuardClasses = "ZmbM_PatrolNormal_Autumn|ZmbM_PatrolNormal_Flat|ZmbM_SoldierNormal|ZmbM_usSoldier_normal_Woodland";
	ref array<ref OS_AirdropLocation> Locations = new array<ref OS_AirdropLocation>;
	ref array<ref OS_AirdropLoot> LootTables = new array<ref OS_AirdropLoot>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď místy na své mapě !!!
		Locations.Insert(new OS_AirdropLocation("Priklad - Letiste", 4500, 9500, 250));
		Locations.Insert(new OS_AirdropLocation("Priklad - Vojenska zakladna", 7000, 3000, 200));
		Locations.Insert(new OS_AirdropLocation("Priklad - Pole na severu", 6000, 11000, 400));

		OS_AirdropLoot military = new OS_AirdropLoot("Vojensky", 3);
		OS_AirdropItem rifle = military.Add("M4A1|AKM|SVD|FAL", 1);
		rifle.Attachments.Insert("ACOGOptic");
		military.Add("Mag_STANAG_30Rnd|Mag_AKM_30Rnd|Mag_SVD_10Rnd|Mag_FAL_20Rnd", 1, 2);
		military.Add("AmmoBox_556x45_20Rnd|AmmoBox_762x39_20Rnd|AmmoBox_762x54_20Rnd|AmmoBox_308Win_20Rnd", 1, 3);
		military.Add("PlateCarrierVest|PressVest_Blue", 0.6);
		military.Add("BallisticHelmet_Green|Mich2001Helmet", 0.5);
		military.Add("M67Grenade|RGD5Grenade", 0.6, 2);
		military.Add("NVGoggles", 0.15);
		LootTables.Insert(military);

		OS_AirdropLoot medical = new OS_AirdropLoot("Zdravotnicky", 2);
		medical.Add("FirstAidKit", 1, 2);
		medical.Add("BandageDressing", 1, 4);
		medical.Add("TetracyclineAntibiotics", 1, 2);
		medical.Add("CharcoalTablets", 0.8, 2);
		medical.Add("VitaminBottle", 0.8, 2);
		medical.Add("Morphine|Epinephrine", 0.7, 2);
		medical.Add("SalineBagIV|BloodBagIV", 0.6);
		medical.Add("DisinfectantSpray", 0.8);
		LootTables.Insert(medical);

		OS_AirdropLoot supply = new OS_AirdropLoot("Zasoby", 2);
		supply.Add("TacticalBaconCan|SpaghettiCan|BakedBeansCan|PorkCan", 1, 6);
		supply.Add("WaterBottle|Canteen", 1, 3);
		supply.Add("Rice|PowderedMilk", 0.7, 2);
		supply.Add("Battery9V", 1, 4);
		supply.Add("PersonalRadio", 0.6);
		supply.Add("NailBox", 0.8, 2);
		supply.Add("CodeLock|DigitalCodeLock", 0.3);
		LootTables.Insert(supply);
	}
}
