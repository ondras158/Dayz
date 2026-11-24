// Společné konstanty pro klienta i server.

const string BC_DIR = "$profile:BadlandsCore/";
const string BC_TAG = "[BadlandsCore] ";
const string BC_LAYOUT_DIR = "BadlandsCore/GUI/layouts/";

// ID vzdálených volání (RPC). Musí být unikátní - neměň je, pokud nemusíš.
const int BC_RPC_SPAWN_MENU    = 784512001; // server -> klient: otevři menu spawnu
const int BC_RPC_SPAWN_ACK     = 784512002; // klient -> server: menu je otevřené
const int BC_RPC_SPAWN_CHOICE  = 784512003; // klient -> server: vybraná zóna

const int BC_RPC_ADMIN_OPEN    = 784512010; // klient -> server: chci admin menu
const int BC_RPC_ADMIN_DATA    = 784512011; // server -> klient: seznam hráčů
const int BC_RPC_ADMIN_ACTION  = 784512012; // klient -> server: proveď akci

const int BC_RPC_TRADER_OPEN   = 784512020; // klient -> server: chci obchod
const int BC_RPC_TRADER_DATA   = 784512021; // server -> klient: katalog
const int BC_RPC_TRADER_BUY    = 784512022; // klient -> server: koupit
const int BC_RPC_TRADER_SELL   = 784512023; // klient -> server: prodat
const int BC_RPC_TRADER_UPDATE = 784512024; // server -> klient: nový zůstatek + co lze prodat
const int BC_RPC_TRADER_CATEGORY   = 784512025; // klient -> server: předměty kategorie
const int BC_RPC_TRADER_SEARCH     = 784512026; // klient -> server: hledání
const int BC_RPC_TRADER_ITEMS      = 784512027; // server -> klient: seznam předmětů
const int BC_RPC_TRADER_ADMIN_EDIT = 784512028; // klient -> server: admin mění cenu
const int BC_RPC_TRADER_ADMIN_BULK = 784512029; // klient -> server: admin násobí ceny kategorie

const int BC_RPC_VEHICLE_DATA   = 784512030; // server -> klient: autobazar / servis / garáž
const int BC_RPC_VEHICLE_ACTION = 784512031; // klient -> server: koupit, opravit, natankovat...

const int BC_RPC_BANK_DATA      = 784512040; // server -> klient: stav hotovosti a účtu
const int BC_RPC_BANK_ACTION    = 784512041; // klient -> server: vložit / vybrat
const int BC_RPC_BANK_TRANSFER  = 784512042; // klient -> server: převod jinému hráči

const int BC_RPC_FIRST = 784512001;
const int BC_RPC_LAST  = 784512099;

enum BC_AdminAction
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

enum BC_VehicleAction
{
	BUY,
	REPAIR,
	REFUEL,
	PARTS,
	STORE,
	RETRIEVE,
	CLAIM,
	KEY_COPY,
	CHANGE_LOCK
}

enum BC_BankAction
{
	DEPOSIT,
	WITHDRAW
}
