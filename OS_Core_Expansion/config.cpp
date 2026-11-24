// Doplněk OS_Core pro DayZ Expansion BaseBuilding (územní vlajky).
// Nahraj ho JEN pokud máš na serveru Expansion BaseBuilding - jinak se server nespustí.
// Členové OS skupiny se automaticky stanou členy území (vlajky), které postavil kdokoli z nich.
class CfgPatches
{
	class OS_Core_Expansion
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"OS_Core", "DayZExpansion_BaseBuilding_Scripts"};
	};
};

class CfgMods
{
	class OS_Core_Expansion
	{
		dir = "OS_Core_Expansion";
		name = "OS Core - Expansion BaseBuilding";
		author = "ondras158";
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"OS_Core_Expansion/Scripts/4_World"};
			};
		};
	};
};
