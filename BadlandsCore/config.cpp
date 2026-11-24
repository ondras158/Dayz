class CfgPatches
{
	class BadlandsCore
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
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
