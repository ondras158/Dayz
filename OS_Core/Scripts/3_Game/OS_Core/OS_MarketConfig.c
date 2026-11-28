// Bazar hráčů: věci a auta za cenu prodejce, nákup i když prodejce není online.
// Nastavení: $profile:OS_Core/Market.json, nabídky: $profile:OS_Core/MarketData.json

class OS_MarketConfig
{
	bool Enabled = true;
	bool AtAllTraders = true;              // bazar je u každého obchodníka (tlačítko v obchodě)
	ref array<string> Traders = new array<string>;   // jinak jen u těchto obchodníků (názvy z Trader.json)
	int MaxListingsPerPlayer = 10;
	int MinPrice = 1;
	int MaxPrice = 10000000;
	float FeePercent = 5;                  // poplatek z prodeje (odečte se prodejci)
	float ListingDays = 7;                 // po této době se neprodaná nabídka vrátí prodejci
	bool AllowVehicles = true;             // auta z osobní garáže
}

class OS_MarketListing
{
	int Id;
	string SellerId;
	string SellerName;
	string ClassName;
	bool IsVehicle;
	int Price;
	int Created;                           // unix čas vystavení
	ref OS_SavedItem Item;
	ref OS_StoredVehicle Vehicle;
}

class OS_MarketReturn
{
	string OwnerId;
	ref OS_SavedItem Item;
}

class OS_MarketData
{
	int NextId = 1;
	ref array<ref OS_MarketListing> Listings = new array<ref OS_MarketListing>;
	ref array<ref OS_MarketReturn> Returns = new array<ref OS_MarketReturn>;   // neprodané / stažené věci k vyzvednutí
}
