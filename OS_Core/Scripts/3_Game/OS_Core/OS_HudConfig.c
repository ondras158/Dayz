// HUD v rohu obrazovky. Nastavení: $profile:OS_Core/Hud.json (platí pro všechny hráče)
// Hráč si HUD skryje / zobrazí klávesou H.

class OS_HudConfig
{
	bool Enabled = true;
	bool ShowStatus = true;        // zdraví, krev, jídlo, voda, výdrž, teplo + zóna a PvP
	bool ShowGroup = true;         // spoluhráči online se vzdáleností
	bool ShowInfo = true;          // souřadnice, směr, čas ve hře, počet hráčů
	bool ShowDiseases = true;      // nemoci (s radou, čím léčit) a léky, které právě působí
	bool ShowEarlyInfection = false; // ukázat i nákazu před příznaky (jinak až když se nemoc projeví)
	bool ShowCoordinates = true;   // souřadnice X / Z (na některých serverech nechtěné)
	string Position = "right";     // right / left
	float UpdateSeconds = 1.5;     // jak často server posílá stav
}

//! data HUDu na klientovi (posílá server)
class OS_ClientHud
{
	static bool Enabled = true;
	static bool ShowStatus = true;
	static bool ShowGroup = true;
	static bool ShowInfo = true;
	static bool ShowDiseases = true;
	static ref array<string> Diseases = new array<string>;   // "nemoc|čím léčit"
	static ref array<string> Drugs = new array<string>;
	static bool ShowCoordinates = true;
	static bool Left;
	static bool Received;

	static float Health = 1;      // 0..1
	static float Blood = 1;
	static float Food = 1;
	static float Water = 1;
	static float Heat;            // -1 zima .. 0 ok .. 1 horko
	static int ZoneType;          // 0 PvE, 1 bezpečná zóna, 2 PvP zóna, 3 PvP časovač, 4 PvP všude
	static string ZoneName;
	static int PvPSeconds;
	static string BaseText;
	static int PlayersOnline;

	static void Read(ParamsReadContext ctx)
	{
		bool enabled;
		bool showStatus;
		bool showGroup;
		bool showInfo;
		bool showCoords;
		bool left;
		float health;
		float blood;
		float food;
		float water;
		float heat;
		int zoneType;
		string zoneName;
		int pvpSeconds;
		string baseText;
		int players;
		if (!ctx.Read(enabled) || !ctx.Read(showStatus) || !ctx.Read(showGroup) || !ctx.Read(showInfo) || !ctx.Read(showCoords) || !ctx.Read(left))
			return;
		if (!ctx.Read(health) || !ctx.Read(blood) || !ctx.Read(food) || !ctx.Read(water) || !ctx.Read(heat))
			return;
		if (!ctx.Read(zoneType) || !ctx.Read(zoneName) || !ctx.Read(pvpSeconds) || !ctx.Read(baseText) || !ctx.Read(players))
			return;
		bool showDiseases;
		array<string> diseases = new array<string>;
		array<string> drugs = new array<string>;
		if (!ctx.Read(showDiseases) || !ctx.Read(diseases) || !ctx.Read(drugs))
			return;

		Enabled = enabled;
		ShowStatus = showStatus;
		ShowGroup = showGroup;
		ShowInfo = showInfo;
		ShowCoordinates = showCoords;
		Left = left;
		Health = health;
		Blood = blood;
		Food = food;
		Water = water;
		Heat = heat;
		ZoneType = zoneType;
		ZoneName = zoneName;
		PvPSeconds = pvpSeconds;
		BaseText = baseText;
		PlayersOnline = players;
		ShowDiseases = showDiseases;
		Diseases = diseases;
		Drugs = drugs;
		Received = true;
	}
}
