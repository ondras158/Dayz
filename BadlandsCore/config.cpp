class CfgPatches
{
	class BadlandsCore
	{
		units[] = {"BC_CarKey", "BC_Money"};
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

	// Hotovost - model papíru, stohovatelná (jako hadry), chování ze skriptu BC_Money
	class BC_Money : Paper
	{
		scope = 2;
		displayName = "Penize";
		descriptionShort = "Hotovost. Uloz ji v bankomatu - kdyz zemres, zustane na tvem tele.";
		weight = 0;
		canBeSplit = 1;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 10000;
		varStackMax = 10000;
		varQuantityDestroyOnMin = 1;
		quantityBar = 0;
		stackedUnit = "Kr";
	};

	// Klíč od auta - model vanilla klíčků od pout, chování ze skriptu BC_CarKey
	class BC_CarKey : HandcuffKeys
	{
		scope = 2;
		displayName = "Klic od auta";
		descriptionShort = "Klic k autu. Vezmi ho do ruky, podivej se na auto a podrz leve tlacitko pro zamknuti nebo odemknuti.";
	};
};
