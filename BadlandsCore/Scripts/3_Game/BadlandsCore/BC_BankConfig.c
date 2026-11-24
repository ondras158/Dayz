// Hotovost a bankomaty. Soubor $profile:BadlandsCore/Bank.json
// Bankovní účet = peněženka hráče (Wallets/<SteamID>.json) - je v bezpečí i po smrti.

class BC_AtmLocation
{
	string Name;
	float X;
	float Z;
	float Radius = 3;

	void BC_AtmLocation(string name = "", float x = 0, float z = 0, float radius = 3)
	{
		Name = name;
		X = x;
		Z = z;
		Radius = radius;
	}
}

class BC_BankConfig
{
	bool UseCash = true;            // obchod, autobazar i servis platí a vyplácí v hotovosti (předmět BC_Money)
	bool ShopsAcceptBank = false;   // když hotovost nestačí, doplatit z účtu (jako platba kartou)
	float DepositFeePercent = 0;    // poplatek za vklad v %
	bool AllowTransfers = true;     // převod z účtu na účet jiného hráče (v bankomatu)
	float TransferFeePercent = 0;   // poplatek za převod v %
	int MinTransfer = 1;
	ref array<ref BC_AtmLocation> Atms = new array<ref BC_AtmLocation>;
	ref array<string> AtmObjectClasses = new array<string>;   // objekty na mapě, které fungují jako bankomat
	float AtmObjectRadius = 2.5;

	void FillDefaults()
	{
		// !!! UKÁZKOVÉ SOUŘADNICE - nahraď skutečnými místy (např. u obchodníka) !!!
		Atms.Insert(new BC_AtmLocation("Priklad - Bankomat Trziste", 2005, 2000, 3));
	}
}
