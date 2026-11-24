class CfgPatches
{
	class BadlandsCore
	{
		units[] = {"BC_CarKey", "BC_Money_10", "BC_Money_50", "BC_Money_100", "BC_Money_500", "BC_Money_1000"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Tools", "DZ_Gear_Consumables"};
	};
};

class CfgMods
{
	class BadlandsCore
	{
		dir = "BadlandsCore";
		name = "Badlands Core";
		author = "ondras158";
		type = "mod";
		inputs = "BadlandsCore/Scripts/Inputs.xml";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"BadlandsCore/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"BadlandsCore/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"BadlandsCore/Scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class HandcuffKeys;
	class Paper;

	// Hotovost - bankovky. Model papíru, vlastní potisk, stohovatelné (počet kusů = počet bankovek).
	// Chování ze skriptu BC_Money, hodnota jedné bankovky je v bcValue.
	class BC_Money : Paper
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
	class BC_Money_10 : BC_Money
	{
		scope = 2;
		displayName = "Bankovka 10 Kr";
		bcValue = 10;
		hiddenSelectionsTextures[] = {"BadlandsCore\data\money_10_co.paa"};
	};
	class BC_Money_50 : BC_Money
	{
		scope = 2;
		displayName = "Bankovka 50 Kr";
		bcValue = 50;
		hiddenSelectionsTextures[] = {"BadlandsCore\data\money_50_co.paa"};
	};
	class BC_Money_100 : BC_Money
	{
		scope = 2;
		displayName = "Bankovka 100 Kr";
		bcValue = 100;
		hiddenSelectionsTextures[] = {"BadlandsCore\data\money_100_co.paa"};
	};
	class BC_Money_500 : BC_Money
	{
		scope = 2;
		displayName = "Bankovka 500 Kr";
		bcValue = 500;
		hiddenSelectionsTextures[] = {"BadlandsCore\data\money_500_co.paa"};
	};
	class BC_Money_1000 : BC_Money
	{
		scope = 2;
		displayName = "Bankovka 1000 Kr";
		bcValue = 1000;
		hiddenSelectionsTextures[] = {"BadlandsCore\data\money_1000_co.paa"};
	};

	// Klíč od auta - model vanilla klíčků od pout, chování ze skriptu BC_CarKey
	class BC_CarKey : HandcuffKeys
	{
		scope = 2;
		displayName = "Klic od auta";
		descriptionShort = "Klic k autu. Vezmi ho do ruky, podivej se na auto a podrz leve tlacitko pro zamknuti nebo odemknuti.";
	};
};
