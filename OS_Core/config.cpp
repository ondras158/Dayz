class CfgPatches
{
	class OS_Core
	{
		units[] = {"OS_ChickenEgg", "OS_Dish_Gulas", "OS_Dish_MeatSoup", "OS_Dish_VegSoup", "OS_Dish_FishSoup", "OS_Dish_MushroomSoup", "OS_Dish_ChickenPaprika", "OS_Dish_Risotto", "OS_Dish_Compote", "OS_Dish_RoastMeat", "OS_Dish_Omelette", "OS_Money_1", "OS_Money_2", "OS_Money_5", "OS_Money_10", "OS_Money_20", "OS_Money_50", "OS_Money_100", "OS_Money_200", "OS_Money_500", "OS_Money_1000", "OS_Money_2000", "OS_Money_5000"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Consumables", "DZ_Gear_Food"};
	};
};

class CfgMods
{
	class OS_Core
	{
		dir = "OS_Core";
		name = "OS Core";
		author = "ondras158";
		type = "mod";
		inputs = "OS_Core/Scripts/Inputs.xml";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"OS_Core/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"OS_Core/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"OS_Core/Scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class Paper;
	class Potato;
	class BakedBeansCan_Opened;
	class PeachesCan_Opened;
	class PorkCan_Opened;
	class SpaghettiCan_Opened;
	class TacticalBaconCan_Opened;
	class TunaCan_Opened;

	// Vejce z farmy - jídlo (vaření, jedení) i model zatím z brambory
	// (model vajíčka: nastav model = "..." podle P:\dz\gear\... v DayZ Tools)
	class OS_ChickenEgg : Potato
	{
		scope = 2;
		displayName = "$STR_OS_ChickenEgg";
		descriptionShort = "$STR_OS_ChickenEgg_Desc";
	};

	// Hotovost - bankovky. Model papíru, vlastní potisk, stohovatelné (počet kusů = počet bankovek).
	// Chování ze skriptu OS_Money, hodnota jedné bankovky je v bcValue.
	// Které hodnoty server vydává a jak se jmenuje měna, se nastavuje v Bank.json (bez přebalení módu).
	class OS_Money : Paper
	{
		scope = 0;
		displayName = "Bankovka";
		descriptionShort = "Hotovost. Uloz ji v bankomatu - kdyz zemres, zustane na tvem tele.";
		weight = 0;
		canBeSplit = 1;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 100;
		varStackMax = 100;
		varQuantityDestroyOnMin = 1;
		quantityBar = 0;
		stackedUnit = "ks";
		bcValue = 1;
		hiddenSelections[] = {"zbytek"};
	};
	class OS_Money_1 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 1";
		bcValue = 1;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_1_co.paa"};
	};
	class OS_Money_2 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 2";
		bcValue = 2;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_2_co.paa"};
	};
	class OS_Money_5 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 5";
		bcValue = 5;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_5_co.paa"};
	};
	class OS_Money_10 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 10";
		bcValue = 10;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_10_co.paa"};
	};
	class OS_Money_20 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 20";
		bcValue = 20;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_20_co.paa"};
	};
	class OS_Money_50 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 50";
		bcValue = 50;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_50_co.paa"};
	};
	class OS_Money_100 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 100";
		bcValue = 100;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_100_co.paa"};
	};
	class OS_Money_200 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 200";
		bcValue = 200;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_200_co.paa"};
	};
	class OS_Money_500 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 500";
		bcValue = 500;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_500_co.paa"};
	};
	class OS_Money_1000 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 1000";
		bcValue = 1000;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_1000_co.paa"};
	};
	class OS_Money_2000 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 2000";
		bcValue = 2000;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_2000_co.paa"};
	};
	class OS_Money_5000 : OS_Money
	{
		scope = 2;
		displayName = "Bankovka 5000";
		bcValue = 5000;
		hiddenSelectionsTextures[] = {"OS_Core\data\money_5000_co.paa"};
	};


	// Hotová jídla z receptů (vaření u ohně, Cooking.json). Model zatím z otevřené konzervy.
	// Výživa: energy / water = na 100 jednotek množství, celé jídlo má 400 jednotek.
	class OS_Dish_Gulas : PorkCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_Gulas";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 650;
			water = 150;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_MeatSoup : BakedBeansCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_MeatSoup";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 450;
			water = 450;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_VegSoup : BakedBeansCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_VegSoup";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 350;
			water = 500;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_FishSoup : TunaCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_FishSoup";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 420;
			water = 450;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_MushroomSoup : BakedBeansCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_MushroomSoup";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 380;
			water = 450;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_ChickenPaprika : PorkCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_ChickenPaprika";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 600;
			water = 250;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_Risotto : SpaghettiCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_Risotto";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 600;
			water = 150;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_Compote : PeachesCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_Compote";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 250;
			water = 650;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_RoastMeat : TacticalBaconCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_RoastMeat";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 700;
			water = 100;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
	class OS_Dish_Omelette : TacticalBaconCan_Opened
	{
		scope = 2;
		displayName = "$STR_OS_Dish_Omelette";
		descriptionShort = "$STR_OS_Dish_Desc";
		varQuantityInit = 400;
		varQuantityMin = 0;
		varQuantityMax = 400;
		class Nutrition
		{
			fullnessIndex = 1.5;
			energy = 500;
			water = 100;
			nutritionalIndex = 1;
			toxicity = 0;
			digestibility = 2;
			agents = 0;
		};
	};
};
