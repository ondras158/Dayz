// PvP zóny: kde smí hráči zraňovat jiné hráče. Nastavení: $profile:OS_Core/PvP.json
// Zóny jsou vidět na mapě (klávesa M otevře mapu i bez mapy v inventáři).
// Admin je může přidávat a mazat i ve hře (admin menu, klávesa Home).

class OS_PvPZone
{
	string Name;
	float X;
	float Z;
	float Radius = 300;

	void OS_PvPZone(string name = "", float x = 0, float z = 0, float radius = 300)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class OS_PvPConfig
{
	bool Enabled = true;
	bool PvPOnlyInZones = true;       // true = mimo PvP zóny si hráči neublíží (PvE), false = PvP všude, zóny jen na mapě
	bool BothPlayersInZone = true;    // true = útočník i oběť musí stát v PvP zóně (nejde střílet do zóny zvenku)
	int LeaveTimerSeconds = 30;       // po odchodu ze zóny ještě tolik sekund platí PvP (střílet i být zraněn), 0 = hned konec
	bool NotifyOnEnter = true;
	string EnterMessage = "Vstoupil jsi do PvP zony - ostatni hraci te tu mohou zranit!";
	string TimerMessage = "Opustil jsi PvP zonu - jeste %1 s te mohou hraci zranit (a ty je).";
	string LeaveMessage = "Opustil jsi PvP zonu - jsi v bezpeci pred hraci.";

	// mapa
	bool MapWithoutItem = true;       // klávesa (výchozí M) otevře mapu i bez mapy v inventáři
	bool ShowPvPZonesOnMap = true;
	bool ShowSafeZonesOnMap = true;
	string PvPColor = "230 40 40";     // barva kruhu na mapě: R G B (0-255)
	string SafeColor = "60 200 90";

	ref array<ref OS_PvPZone> Zones = new array<ref OS_PvPZone>;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečnými místy (nebo přidej zóny v admin menu) !!!
		Zones.Insert(new OS_PvPZone("Priklad - PvP letiste", 4500, 9500, 600));
	}

	static int ToARGB(string rgb, int alpha)
	{
		array<string> parts = new array<string>;
		rgb.Replace(",", " ");
		rgb.Split(" ", parts);
		if (parts.Count() < 3)
			return ARGB(alpha, 255, 255, 255);
		return ARGB(alpha, parts[0].ToInt(), parts[1].ToInt(), parts[2].ToInt());
	}
}

//! zóny a nastavení mapy na klientovi (posílá server po připojení a po změně)
class OS_ClientZones
{
	static bool MapWithoutItem = true;
	static bool PvPOnlyInZones;
	static int PvPColor = 0xDCE62828;
	static int SafeColor = 0xDC3CC85A;
	static ref array<string> PvPNames = new array<string>;
	static ref array<float> PvPX = new array<float>;
	static ref array<float> PvPZ = new array<float>;
	static ref array<float> PvPR = new array<float>;
	static ref array<string> SafeNames = new array<string>;
	static ref array<float> SafeX = new array<float>;
	static ref array<float> SafeZ = new array<float>;
	static ref array<float> SafeR = new array<float>;
	static int BaseColor = 0xDC468CFF;
	static ref array<string> BaseNames = new array<string>;
	static ref array<float> BaseX = new array<float>;
	static ref array<float> BaseZ = new array<float>;
	static ref array<float> BaseR = new array<float>;

	static void Read(ParamsReadContext ctx)
	{
		bool mapWithoutItem;
		bool pvpOnly;
		int pvpColor;
		int safeColor;
		array<string> pvpNames = new array<string>;
		array<float> pvpX = new array<float>;
		array<float> pvpZ = new array<float>;
		array<float> pvpR = new array<float>;
		array<string> safeNames = new array<string>;
		array<float> safeX = new array<float>;
		array<float> safeZ = new array<float>;
		array<float> safeR = new array<float>;
		if (!ctx.Read(mapWithoutItem) || !ctx.Read(pvpOnly) || !ctx.Read(pvpColor) || !ctx.Read(safeColor))
			return;
		if (!ctx.Read(pvpNames) || !ctx.Read(pvpX) || !ctx.Read(pvpZ) || !ctx.Read(pvpR))
			return;
		if (!ctx.Read(safeNames) || !ctx.Read(safeX) || !ctx.Read(safeZ) || !ctx.Read(safeR))
			return;

		int baseColor;
		array<string> baseNames = new array<string>;
		array<float> baseX = new array<float>;
		array<float> baseZ = new array<float>;
		array<float> baseR = new array<float>;
		if (ctx.Read(baseColor) && ctx.Read(baseNames) && ctx.Read(baseX) && ctx.Read(baseZ) && ctx.Read(baseR))
		{
			BaseColor = baseColor;
			BaseNames = baseNames;
			BaseX = baseX;
			BaseZ = baseZ;
			BaseR = baseR;
		}

		MapWithoutItem = mapWithoutItem;
		PvPOnlyInZones = pvpOnly;
		PvPColor = pvpColor;
		SafeColor = safeColor;
		PvPNames = pvpNames;
		PvPX = pvpX;
		PvPZ = pvpZ;
		PvPR = pvpR;
		SafeNames = safeNames;
		SafeX = safeX;
		SafeZ = safeZ;
		SafeR = safeR;
	}

	static int Count()
	{
		return PvPNames.Count() + SafeNames.Count() + BaseNames.Count();
	}
}
