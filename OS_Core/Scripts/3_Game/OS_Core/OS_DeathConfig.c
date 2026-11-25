// Smrt: zabití hráčem / AI = tělo s věcmi zůstane, jiná smrt = věci zůstanou po respawnu.
// Postava (vzhled) a staty zůstávají. Nastavení: $profile:OS_Core/Death.json

class OS_DeathConfig
{
	bool Enabled = true;
	float PvPCorpseMinutes = 120;       // zabil tě hráč nebo AI: tělo s věcmi tu leží tolik minut, vybrat ho může kdokoli
	bool AICountsAsPlayer = true;       // zabití vojákem AI = jako zabití hráčem (věci zůstanou na těle)
	bool SuicideDropsItems = true;      // sebevražda = věci zůstanou na těle (aby se nedalo zneužít k teleportu s věcmi)
	bool KeepInventoryOnOtherDeath = true;   // zombík, zvíře, pád, hlad, nemoc...: věci máš i po respawnu
	bool KeepCharacter = true;          // nová postava má stejný vzhled jako předchozí
	bool KeepFatigue = true;            // únava se po smrti nenuluje

	// spací místo: postavený stan nebo improvizovaný přístřešek -> akce "Nastavit spawn zde",
	// po smrti se objeví v menu spawnu jako první možnost
	bool BedSpawn = true;
	float BedCooldownMinutes = 10;      // jak často jde na spacím místě respawnout
}

class OS_BedsData
{
	ref map<string, vector> Beds = new map<string, vector>;   // Steam ID -> poloha stanu / přístřešku
}

class OS_SavedItem
{
	string Type;
	string Slot;                 // slot přílohy ("" = v nákladu)
	float Quantity = -1;
	float Health = -1;
	int Ammo = -1;
	int Liquid = -1;
	ref array<ref OS_SavedItem> Items = new array<ref OS_SavedItem>;   // přílohy a obsah
}

class OS_SavedInventory
{
	ref array<ref OS_SavedItem> Items = new array<ref OS_SavedItem>;   // oblečení a věci na postavě
	ref OS_SavedItem Hands;
}

class OS_CharacterTypes
{
	ref map<string, string> Types = new map<string, string>;   // Steam ID -> classname postavy
}
