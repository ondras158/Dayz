// Hotovost a bankomaty. Soubor $profile:OS_Core/Bank.json
// Bankovní účet = peněženka hráče (Wallets/<SteamID>.json) - je v bezpečí i po smrti.

class OS_AtmLocation
{
	string Name;
	float X;
	float Z;
	float Radius = 3;

	void OS_AtmLocation(string name = "", float x = 0, float z = 0, float radius = 3)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class OS_BankConfig
{
	// Měna - platí pro celý mód (obchod, auta, bankomat).
	// Textury bankovek s názvem měny vygeneruj: python3 tools/generate_money_textures.py --currency EUR
	string CurrencyName = "EUR";
	// Bankovky, které se vydávají (z hodnot 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000)
	ref array<int> Denominations = new array<int>;

	bool UseCash = true;            // obchod, autobazar i servis platí a vyplácí v hotovosti (předmět OS_Money)
	bool ShopsAcceptBank = false;   // když hotovost nestačí, doplatit z účtu (jako platba kartou)
	float DepositFeePercent = 0;    // poplatek za vklad v %
	bool AllowTransfers = true;     // převod z účtu na účet jiného hráče (v bankomatu)
	float TransferFeePercent = 0;   // poplatek za převod v %
	int MinTransfer = 1;
	ref array<ref OS_AtmLocation> Atms = new array<ref OS_AtmLocation>;
	ref array<string> AtmObjectClasses = new array<string>;   // objekty na mapě, které fungují jako bankomat
	float AtmObjectRadius = 2.5;

	void FillDefaults()
	{
		// euro (+ vyšší bankovky 1000, 2000, 5000)
		Denominations.Insert(5000);
		Denominations.Insert(2000);
		Denominations.Insert(1000);
		Denominations.Insert(500);
		Denominations.Insert(200);
		Denominations.Insert(100);
		Denominations.Insert(50);
		Denominations.Insert(20);
		Denominations.Insert(10);
		Denominations.Insert(5);

		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečnými místy (např. u obchodníka) !!!
		Atms.Insert(new OS_AtmLocation("Priklad - Bankomat Trziste", 2005, 2000, 3));
	}
}
