// Doplněk OS_Core pro DayZ Expansion AI.
// Nahraj ho JEN pokud máš na serveru Expansion AI - jinak se server nespustí.
// Opravy výhledu AI (nestřílí přes zdi, nevidí na kilometr), hlídky, útoky a stráže airdropů z OS_Core.
class CfgPatches
{
	class OS_Core_ExpansionAI
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"OS_Core", "DayZExpansion_AI_Scripts"};
	};
};

class CfgMods
{
	class OS_Core_ExpansionAI
	{
		dir = "OS_Core_ExpansionAI";
		name = "OS Core - Expansion AI";
		author = "ondras158";
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"OS_Core_ExpansionAI/Scripts/4_World"};
			};
		};
	};
};
