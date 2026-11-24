class CfgPatches
{
	class BadlandsCore
	{
		units[] = {"BC_CarKey"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Tools"};
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

	// Klíč od auta - model vanilla klíčků od pout, chování ze skriptu BC_CarKey
	class BC_CarKey : HandcuffKeys
	{
		scope = 2;
		displayName = "Klic od auta";
		descriptionShort = "Klic k autu. Vezmi ho do ruky, podivej se na auto a podrz leve tlacitko pro zamknuti nebo odemknuti.";
	};
};
