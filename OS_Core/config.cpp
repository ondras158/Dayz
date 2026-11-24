class CfgPatches
{
	class OS_Core
	{
		units[] = {"OS_ChickenEgg", "OS_Money_1", "OS_Money_2", "OS_Money_5", "OS_Money_10", "OS_Money_20", "OS_Money_50", "OS_Money_100", "OS_Money_200", "OS_Money_500", "OS_Money_1000", "OS_Money_2000", "OS_Money_5000"};
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

	// Vejce z farmy - jídlo (vaření, jedení) i model zatím z brambory
	// (model vajíčka: nastav model = "..." podle P:\dz\gear\... v DayZ Tools)
	class OS_ChickenEgg : Potato
	{
		scope = 2;
		displayName = "Vejce";
		descriptionShort = "Slepici vejce z farmy. Da se uvarit nebo upect.";
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

};
