// Společné konstanty pro klienta i server.

const string OS_DIR = "$profile:OS_Core/";
const string OS_TAG = "[OS_Core] ";
const string OS_LAYOUT_DIR = "OS_Core/GUI/layouts/";

// ID vzdálených volání (RPC). Musí být unikátní - neměň je, pokud nemusíš.
const int OS_RPC_SPAWN_MENU    = 784512001; // server -> klient: otevři menu spawnu
const int OS_RPC_SPAWN_ACK     = 784512002; // klient -> server: menu je otevřené
const int OS_RPC_SPAWN_CHOICE  = 784512003; // klient -> server: vybraná zóna

const int OS_RPC_ADMIN_OPEN    = 784512010; // klient -> server: chci admin menu
const int OS_RPC_ADMIN_DATA    = 784512011; // server -> klient: seznam hráčů
const int OS_RPC_ADMIN_ACTION  = 784512012; // klient -> server: proveď akci

const int OS_RPC_TRADER_OPEN   = 784512020; // klient -> server: chci obchod
const int OS_RPC_TRADER_DATA   = 784512021; // server -> klient: katalog
const int OS_RPC_TRADER_BUY    = 784512022; // klient -> server: koupit
const int OS_RPC_TRADER_SELL   = 784512023; // klient -> server: prodat
const int OS_RPC_TRADER_UPDATE = 784512024; // server -> klient: nový zůstatek + co lze prodat
const int OS_RPC_TRADER_CATEGORY   = 784512025; // klient -> server: předměty kategorie
const int OS_RPC_TRADER_SEARCH     = 784512026; // klient -> server: hledání
const int OS_RPC_TRADER_ITEMS      = 784512027; // server -> klient: seznam předmětů
const int OS_RPC_TRADER_ADMIN_EDIT = 784512028; // klient -> server: admin mění cenu
const int OS_RPC_TRADER_ADMIN_BULK = 784512029; // klient -> server: admin násobí ceny kategorie

const int OS_RPC_VEHICLE_DATA   = 784512030; // server -> klient: autobazar / servis / garáž
const int OS_RPC_VEHICLE_ACTION = 784512031; // klient -> server: koupit, opravit, natankovat...
const int OS_RPC_VEHICLE_SET_SPAWN = 784512032; // klient -> server: nové místo spawnu vozidel (admin / osobní garáž)
const int OS_RPC_PGARAGE_OPEN   = 784512033; // klient -> server: menu osobní garáže
const int OS_RPC_PGARAGE_DATA   = 784512034; // server -> klient: stav osobní garáže
const int OS_RPC_PGARAGE_ACTION = 784512035; // klient -> server: postavit / zrušit osobní garáž

const int OS_RPC_BANK_DATA      = 784512040; // server -> klient: stav hotovosti a účtu
const int OS_RPC_BANK_ACTION    = 784512041; // klient -> server: vložit / vybrat
const int OS_RPC_BANK_TRANSFER  = 784512042; // klient -> server: převod jinému hráči

const int OS_RPC_SETTINGS       = 784512050; // server -> klient: nastavení (měna)

const int OS_RPC_GROUP_OPEN     = 784512060; // klient -> server: chci menu skupiny
const int OS_RPC_GROUP_DATA     = 784512061; // server -> klient: stav skupiny
const int OS_RPC_GROUP_ACTION   = 784512062; // klient -> server: akce ve skupině
const int OS_RPC_GROUP_POSITIONS = 784512063; // server -> klient: polohy spoluhráčů (mapa, jmenovky)
const int OS_RPC_GROUP_ID        = 784512064; // server -> klient: ID skupiny hráče

const int OS_RPC_FIRST = 784512001;
const int OS_RPC_LAST  = 784512099;

enum OS_AdminAction
{
	HEAL,
	KILL,
	TELEPORT_TO,
	TELEPORT_HERE,
	TOGGLE_GOD,
	SPAWN_ITEM,
	GIVE_MONEY,
	TELEPORT_COORDS,
	UNLOCK_CAR
}

enum OS_VehicleAction
{
	BUY,
	REPAIR,
	REFUEL,
	PARTS,
	STORE,
	RETRIEVE,
	CLAIM
}

enum OS_BankAction
{
	DEPOSIT,
	WITHDRAW,
	GROUP_DEPOSIT,
	GROUP_WITHDRAW
}

enum OS_PersonalGarageAction
{
	BUILD,
	REMOVE
}

//! kam se ukládá nastavené místo spawnu
enum OS_SpawnTarget
{
	LOCATION,        // autocentrum / heliport / přístav / veřejná garáž (admin)
	PERSONAL_GARAGE  // vlastní garáž hráče
}

enum OS_GroupAction
{
	CREATE,
	INVITE,
	ACCEPT,
	DECLINE,
	LEAVE,
	KICK,
	PROMOTE,
	BUY_SLOT,
	DISBAND
}
