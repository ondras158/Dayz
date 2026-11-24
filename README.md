# OS_Core – serverový mód pro DayZ

Funguje na **jakékoli mapě** (Chernarus, Livonia, Sakhal, Nasdara, komunitní mapy). Vše, co závisí na mapě
(souřadnice obchodníků, safe zón, spawnů, bankomatů, autocenter), je v konfiguračních JSON souborech serveru –
mód se kvůli jiné mapě nemusí znovu balit.

| Složka | Co to je |
|---|---|
| `OS_Core/` | **Mód** (zdroj pro `.pbo`): výbava, výběr spawnu, safe zóny, zprávy, admin menu, obchod, auta |
| `economy/`, `tools/tune_economy.py` | Úprava spawnování zbraní a aut (types.xml, events.xml) |
| `spawnpoints/`, `tools/generate_spawnpoints.py` | Generátor vanilla spawn bodů (`cfgplayerspawnpoints.xml`) z CSV |
| `tools/generate_money_textures.py` | Vygeneruje potisky bankovek pro zvolenou měnu |
| `tools/generate_layouts.py` | Přegeneruje vzhled menu (`.layout`) |

> ⚠️ Všechny souřadnice v módu i v CSV jsou zatím **ukázkové** (např. 1000/1000), ne skutečná místa na mapě.
> Před ostrým provozem je nahraď.

### Použití na jiné mapě
1. Nainstaluj mód (viz níže) a jednou spusť server – v profilové složce se vytvoří `OS_Core/*.json`.
2. V JSON souborech nahraď ukázkové souřadnice místy z tvé mapy:
   `SpawnZones.json`, `SafeZones.json`, `Trader.json` (`Traders`), `Vehicles.json` (`Locations`), `Bank.json` (`Atms`).
   Souřadnice zjistíš na interaktivní mapě (např. iZurvive) nebo ve hře v admin menu (teleport na X Z).
3. Restartuj server. Obchod se sám naplní předměty, které na serveru jsou (vanilla + nahrané módy).

Každý server (mapa) má vlastní profilovou složku, takže můžeš mít víc serverů s různým nastavením a jedním módem.

---

## Mód OS_Core

### Funkce
| Funkce | Popis | Konfig |
|---|---|---|
| Startovní výbava | Nová postava dostane náhodnou sadu oblečení a věcí (podle váhy) | `Loadouts.json` |
| Výběr spawnu | Po vytvoření postavy se otevře menu se seznamem oblastí; během výběru je hráč nesmrtelný | `SpawnZones.json` |
| Safe zóny | V zóně nejde zranit hráče, zombíci se mažou, hláška při vstupu/odchodu | `SafeZones.json` |
| Zprávy | Uvítání po připojení, pravidelná oznámení v chatu, kill feed | `Messages.json` |
| Admin menu | Klávesa **Home**: léčení, zabití, teleporty, godmode, spawn předmětů, peníze | `Admins.json` |
| Obchod | Klávesa **B** u obchodníka. **Všechny předměty ze hry i z módů** se do obchodu přidají automaticky. Zbraně jdou koupit draze, ostatní věci prodat levně. Admin mění ceny přímo v menu | `Trader.json` |
| Autobazar / motorky / helikoptéry / lodě | Klávesa **B** v autocentru, na heliportu nebo v přístavu: koupě kompletního auta s plnou nádrží, postaví se na parkoviště | `Vehicles.json` |
| Servis | Oprava (platí se podle poškození), tankování (za litr), doplnění chybějících dílů | `Vehicles.json` |
| Garáže | Uložení auta (bez věcí v kufru) a vyzvednutí v libovolné garáži, max. počet aut na hráče | `Vehicles.json` |
| Peníze a bankomaty | Peníze jsou předmět (bankovky) – po smrti zůstanou na těle. V bankomatu je uložíš na účet, kde jsou v bezpečí | `Bank.json` |
| Skupiny | Klávesa **J**: založení skupiny, pozvánky, až 4 hráči zdarma, další místa za peníze, společný účet v bankomatu | `Groups.json` |
| Vlastnictví aut | Auto patří hráči podle Steam ID, zamyká se klávesou F bez klíče. Ve skupinové garáži se sdílí se skupinou | `Vehicles.json` |

Klávesy lze změnit ve hře: Nastavení → Ovládání → sekce **OS Core**.

### Struktura
```
OS_Core/
├── config.cpp                     definice módu
├── GUI/layouts/*.layout           vzhled menu (lze upravit v DayZ Tools → Workbench)
└── Scripts/
    ├── Inputs.xml                 klávesy
    ├── 3_Game/   OS_Defines.c     RPC ID, konstanty
    │             OS_Config.c      třídy konfigurace + výchozí hodnoty
    ├── 4_World/  OS_Server*.c     serverová logika (každá funkce zvlášť)
    │             OS_PlayerBase.c  nesmrtelnost, kill feed
    │             OS_ZombieBase.c  odměna za zombíka
    └── 5_Mission/OS_Mission*.c    napojení na server/klienta
                  OS_Client.c      příjem dat ze serveru
                  GUI/*.c          menu (spawn, admin, obchod)
```

### 1. Zabalení do .pbo
1. Nainstaluj ze Steamu **DayZ Tools**.
2. Spusť **Addon Builder**:
   - *Addon source directory*: složka `OS_Core` z tohoto repozitáře
   - *Destination*: např. `C:\DayZMods\@OS_Core\Addons`
   - V *Options* nastav **Prefix** na `OS_Core`.
   - Zaškrtni *Binarize* (config.cpp) a nech `*.c;*.layout;*.xml` v seznamu souborů ke zkopírování.
   - Pro podpis: *Sign output* + tvůj `.biprivatekey` (vytvoříš v DS Utils → DSCreateKey).
3. Výsledek:
   ```
   @OS_Core/
   ├── Addons/OS_Core.pbo
   ├── Addons/OS_Core.pbo.<jméno>.bisign
   └── Keys/<jméno>.bikey
   ```

### 2. Instalace na server
1. Zkopíruj `@OS_Core` do složky serveru.
2. `<jméno>.bikey` zkopíruj do složky `keys` serveru.
3. Spouštěcí parametr: `-mod=@OS_Core`
   (ne `-servermod` – mód má menu, takže ho musí mít i hráči).
4. Hráči si mód stáhnou (Workshop nebo ručně) a spouští hru se stejným `-mod`.

### 3. Konfigurace
Při prvním spuštění se v profilové složce serveru (parametr `-profiles=`) vytvoří:
```
<profiles>/OS_Core/
├── Loadouts.json   SpawnZones.json   SafeZones.json
├── Messages.json   Admins.json       Trader.json     Vehicles.json   Bank.json
├── Wallets/<SteamID>.json            bankovní účty hráčů
└── Garages/<SteamID>.json            auta v garáži
```
Uprav je a **restartuj server**. Chybu v JSON najdeš ve script logu pod `[OS_Core]`
(server pak použije výchozí hodnoty – soubor nepřepíše).

**Nastavení admina** – do `Admins.json` dej svoje Steam64 ID (17 číslic):
```json
{ "AdminSteamIds": ["76561198000000000"], "LogActions": true }
```

**Spawn zóna** – body jsou `[x, 0, z]` (výška se dopočítá):
```json
{ "Name": "Pobrezi - jih", "Points": [[1234.5, 0, 2345.6], [1250, 0, 2360]] }
```

**Výbava** – `"A|B|C"` znamená náhodně jedno z A, B, C. `Weight` = jak často se sada vybírá.

**Obchod** (`Trader.json`)
- **Automatický katalog:** při každém startu server projde všechny předměty v configu hry
  (`CfgVehicles`, `CfgWeapons`, `CfgMagazines`, tedy vanilla i všechny nahrané módy) a ty, které v obchodě
  ještě nejsou, přidá do kategorie podle `CategoryRules`. Přidáš mód s novými věcmi → po restartu jsou v obchodě.
  Ceny, které už v souboru jsou (i upravené adminem), se nikdy nepřepíšou. Vypnutí: `"AutoAddItems": false`.
- **Pravidla kategorií** (`CategoryRules`, první shoda vyhrává): podle předka v configu (`BaseClasses`, např.
  `Weapon_Base`), části názvu (`NameContains`, např. `Bandage`) nebo slotu (`Slots`, např. `Back` = batohy).
  `DefaultBuy`/`DefaultSell` = cena pro nově přidané věci. Poslední pravidlo bez podmínek bere vše ostatní.
  `ExcludeContains` = co se nikdy nepřidá (testovací a základní třídy).
- **Výchozí ceny:** zbraně koupit za 4× výkupní cenu (např. AKM: výkup 350, nákup 1400),
  ostatní věci se vykupují zhruba za 1/5 nákupní ceny. Koupit a hned prodat se nikdy nevyplatí.
- **Prodej:** předmět musí být prázdný, bez příslušenství (zbraň bez zásobníku) a nezničený.
  Opotřebovaný má 50–100 % ceny, načatý (krabička nábojů, jídlo) poměrnou cenu.
  Barevné varianty se vykupují za cenu základní věci (`M4A1_Black` → `M4A1`).
- **Úpravy v menu (admin):** admin vidí dole panel:
  - vybere předmět → upraví *Nákup* / *Výkup* → **Uložit cenu** (`-1` = nejde koupit/prodat, obojí `-1` = předmět
    se hráčům skryje, admin ho dál vidí s popiskem SKRYTO),
  - **Násobit** = vynásobí ceny celé vybrané kategorie (např. `1.5` zdraží o 50 %, `0.8` zlevní o 20 %).
  Změny se hned uloží do `Trader.json` a platí bez restartu.
- **Hledání:** pole nad seznamem hledá v classname (min. 2 znaky).
- **Specializovaní obchodníci:** každý obchodník v `Traders` má `Categories` = seznam kategorií, které prodává
  **i vykupuje** (prázdné = všechno). Výchozí:
  - *Trziste* – všechno,
  - *Zbrojir* – `Pistole`, `Zbrane`, `Munice`, `Zasobniky`, `Prislusenstvi zbrani`, `Vybusniny` (granáty, kouřové a
    zábleskové granáty, plastická a improvizovaná výbušnina, Claymore, nášlapná mina, dálkový ovladač),
  - *Obleceni* – `Obleceni`, `Batohy`,
  - *Potraviny* – `Jidlo a piti`,
  - *Lekarna* – `Medicina`,
  - *Zelezarstvi* – `Nastroje a ostatni`, `Stavba zakladny` (hřebíky, prkna, plechy, dráty, sady na plot/věž/vlajku,
    zámky, stany, maskovací sítě, bedny, truhly, sudy, skrýše, nábytek, trezory, vlajky) a `Elektronika`
    (vysílačky, megafon, GPS, dálkoměr, 9V baterie, nabíječka, žárovky, svítilny, sada na opravu elektroniky,
    budík, Geigerův počítač, generátor, reflektor, kabel),
  - *Lovec* – `Lov` (pasti, oka, pruty, háčky, návnady, kostěný a lovecký nůž) a `Zvireci suroviny`
    (kůže, maso ze zvěře, rybí filety, sádlo, vnitřnosti, peří, kosti),
  - *Autodily* – `Dily aut` (kola, dveře, kapoty, baterie, svíčky, chladiče, světla, díly helikoptér Expansion).

  Názvy musí odpovídat kategoriím v `Categories` (včetně těch, které vytvořila automatika). Obchodníka můžeš
  přidat kolik chceš, např. vojenský obchod: `{ "Name": "Vojak", "X": ..., "Z": ..., "Radius": 10, "Categories": ["Zbrane", "Munice", "Obleceni"] }`.
- Obchodník je jen místo (`Traders`: X, Z, Radius) – dej ho do safe zóny a postav tam něco viditelného.

**Auta** (`Vehicles.json`) – `Locations` = místa (autocentra). Každé má `Dealer`/`Service`/`Garage` (true/false),
kde hráč stojí (`X`, `Z`, `Radius`) a kam se postaví auto (`SpawnX`, `SpawnZ`, `SpawnYaw`).
Parkovací místo musí být volné (bez auta v okruhu 5 m).
`Offers` = auta na prodej s cenou a seznamem dílů (stejný seznam používá servis pro doplnění dílů).
Ve výchozím nastavení: Ada, Olga, Sarka, Gunter, M1025, V3S (vanilla auta – pokud má tvoje mapa vlastní auta,
přidej jejich classname a díly).
- Garáž ukládá typ auta, díly, jejich stav a palivo. **Věci v kufru se neukládají** – auto musí být prázdné.

**Peníze a bankomaty** (`Bank.json`)
- **Měna** se nastavuje v `Bank.json`: `CurrencyName` (např. `"EUR"`, `"RUB"`, `"USD"`, `"Kc"`) a `Denominations`
  = které bankovky se vydávají. Mód obsahuje bankovky v hodnotách **1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000
  a 5000** (`OS_Money_<hodnota>`), server použije jen ty z `Denominations`. Příklady:
  - euro (výchozí): `"CurrencyName": "EUR", "Denominations": [5000, 2000, 1000, 500, 200, 100, 50, 20, 10, 5]`
  - rubl: `"CurrencyName": "RUB", "Denominations": [5000, 1000, 500, 100, 50]`
  - dolar: `"CurrencyName": "USD", "Denominations": [100, 50, 20, 10, 5, 1]`
  - koruna: `"CurrencyName": "Kc", "Denominations": [2000, 1000, 500, 200, 100]`

  Ceny v obchodě a autech jsou jen čísla – při změně měny je uprav (např. násobením kategorie v admin menu obchodu).
  Název bankovky ve hře se přizpůsobí měně ze serveru („Bankovka 20 EUR“).
- **Realistický potisk** (výchozí, rytinový styl s pouští, gilošové rozety, vodoznak, ochranný proužek,
  hologram, mikrotext, sériová čísla):
  ```bash
  python3 tools/generate_money_realistic.py --currency EUR --title "OS CORE" --preview 5,10,20,50,100,200,500
  ```
  Bankovka je na šířku uprostřed čtvercové textury (`--layout center`). Když se ve hře zobrazí špatně
  (malá nebo zdeformovaná), zkus `--layout full`. `--size 1024` (výchozí) / `2048` pro ostřejší.
- **AI bankovky přes Gemini API** (vyžaduje API klíč s **placeným tarifem** – obrázky nejsou ve free tieru):
  ```bash
  export GEMINI_API_KEY=...            # klíč nikdy neukládej do repozitáře
  python3 tools/gemini_banknotes.py --values 5,10,20,50,100,200,500   # podklady bez textu -> money_images/raw/
  python3 tools/finish_banknotes.py --currency EUR --title "OS CORE"  # dopíše přesné nápisy -> money_images/
  python3 tools/import_money_images.py --mode fill                    # textury módu
  ```
  Model jde změnit `--model` (`gemini-3.1-flash-image` výchozí, `gemini-3-pro-image` kvalitnější a dražší).
- **Přebarvení bankovky** na novou hodnotu (stejný design, hologram zůstane):
  `python3 tools/recolor_banknote.py --src money_images/raw/10.jpg --value 2000 --ink 18,92,98`
  (`--ink` = nejtmavší tón inkoustu R,G,B). Pak `finish_banknotes.py` a `import_money_images.py`.
- **Jednoduchý potisk** (původní, čtvercový):
  ```bash
  python3 tools/generate_money_textures.py --currency EUR --title "OS CORE" --preview 5,10,20,50,100,200,500
  python3 tools/generate_money_textures.py --currency RUB --label "BANKOVKA" --title "MUJ SERVER"
  ```
  Vznikne `OS_Core/data/money_<hodnota>_co.png` pro všech 12 hodnot. Potisk je součást módu – změna měny na potisku
  vyžaduje přebalit mód (název měny ve hře a v menu se mění jen v `Bank.json`).
- **Vlastní vzhled bankovek:** dej obrázky do `money_images/` pojmenované podle hodnoty (`20.png`, `50.jpg` …)
  a spusť `python3 tools/import_money_images.py`. Obrázek může mít libovolnou velikost a poměr stran.
  Volby: `--mode fit|fill|stretch` (vejít / oříznout / roztáhnout), `--rotate 90` (když je ve hře bokem),
  `--size 1024` (ostřejší), `--mark SPECIMEN` (nápis přes bankovku). Pak převést na .paa a přebalit mód.
- Bankovky stejné hodnoty se stohují (max. 100 ks v balíčku), nic neváží.
- Při placení se bankovky rozmění samy: zaplatíš od největších, a když to nejde přesně, vrátí se drobné v bankovkách.
  Částka menší než nejmenší bankovka nejde vyplatit v hotovosti – připíše se na účet.
- **Textury je potřeba převést na .paa** před zabalením: DayZ Tools → *ImageToPAA* (nebo TexView2 → Save As .paa)
  na všechny `OS_Core/data/money_*_co.png`. Výsledné `money_*_co.paa` nech ve stejné složce a v Addon Builderu
  přidej `*.paa` do souborů ke kopírování.
- Potisk se nanáší na model papíru přes `hiddenSelections[] = {"zbytek"}`. Pokud se bankovky ve hře ukážou
  jako obyčejný papír, má model jiný název oblasti: otevři v Object Builderu `dz\gear\consumables\Paper.p3d`
  (z disku P:), podívej se do *Named Selections* a název přepiš v `config.cpp` u třídy `OS_Money`.
- Obchod, autobazar i servis **vyplácí a berou hotovost**, kterou máš u sebe (i v batohu).
- Hotovost můžeš ztratit – po smrti zůstane na těle a kdokoli ji sebere.
- **Bankomat** (klávesa **B** u bankomatu): vložit / vybrat částku, vložit vše, vybrat vše.
  Peníze na účtu (`Wallets/<SteamID>.json`) jsou v bezpečí i po smrti.
- `Atms` = místa bankomatů (X, Z, Radius). `AtmObjectClasses` = classname objektů na mapě, které mají fungovat
  jako bankomat (pokud mapa nějaké má, např. `"Land_..."` – zjistíš v editoru nebo admin nástrojem).
- **Převod jinému hráči** (v bankomatu): z účtu na účet. Příjemce vybereš ze seznamu hráčů online,
  nebo zadáš jeho Steam64 ID (17 číslic) – pak jde poslat i hráči offline, pokud už na serveru někdy hrál.
  Příjemce online dostane oznámení. `AllowTransfers` (zapnout/vypnout), `TransferFeePercent` (poplatek
  připočtený odesílateli), `MinTransfer`. Každý převod se zapíše do logu serveru.
- `ShopsAcceptBank: true` = když hotovost nestačí, obchod doplatí z účtu (jako kartou).
- `UseCash: false` = vypne hotovost, vše jde přímo přes účet (původní chování).
- `DepositFeePercent` = poplatek za vklad. Startovní peníze (`StartingBalance` v `Trader.json`)
  a peníze od admina (admin menu) jdou **na účet**.

**Skupiny** (`Groups.json`, data skupin v `GroupsData.json`)
- Menu skupiny: klávesa **J**. Kdo není ve skupině, může ji založit (název 3–24 znaků) nebo přijmout pozvánku.
- **Velikost:** `BaseMaxMembers` (4) zdarma, další místo kupuje vedoucí: první za `SlotPrice` (1000),
  každé další o `SlotPriceIncrease` (500) dražší, maximálně `MaxMembersLimit` (10) hráčů.
  Místo se platí ze **skupinového účtu**, a když na něm není dost, z hotovosti vedoucího.
- **Pozvánky:** vedoucí (nebo všichni, pokud `MembersCanInvite: true`) zve hráče online, kteří nejsou v jiné skupině.
  Pozvaný dostane oznámení a pozvánku přijme/odmítne v menu skupiny (platí `InviteValidSeconds`).
- **Vedoucí** může vyhodit člena, předat vedení a rozpustit skupinu. Když vedoucí odejde, vedení převezme další člen.
- **Společný účet:** v bankomatu *Vložit do skupiny* / *Vybrat ze skupiny* (částka z pole Částka, prázdné = vše).
  `MembersCanWithdraw: false` = vybírat smí jen vedoucí. Při rozpuštění nebo odchodu posledního člena
  jde zůstatek na jeho osobní účet. Všechny vklady a výběry se zapisují do logu.

**Spoluhráči na mapě a jména nad hlavou** (nastavení v `Groups.json`)
- Server každé `PositionUpdateSeconds` (3 s) pošle členům skupiny polohy ostatních online členů.
- **Mapa** (`ShowMapMarkers`): spoluhráči jsou na mapě jako modré značky se jménem, i na druhém konci mapy.
  Vlastní značky z mapy zůstávají.
- **Jména nad hlavou** (`ShowNameTags`): jméno + vzdálenost (`NameTagShowDistance`) do `NameTagMaxDistance` (1000 m).
  Blízký spoluhráč – jméno přesně nad hlavou a hýbe se s ním. Vzdálený (mimo dosah sítě) – jméno v jeho poslední
  známé poloze. Daleko = průhlednější. Při otevřeném inventáři/mapě se jména skryjí.
- Vidí to jen členové stejné skupiny. Po odchodu ze skupiny značky zmizí.

**Místo spawnu vozidel – nastavení ve hře**
- U autocentra, heliportu, přístavu a veřejné garáže má **admin** v menu (klávesa B) tlačítko
  *ADMIN: nastavit místo spawnu*. Hráčům se nezobrazí.
- Po kliknutí se menu zavře a na zemi se ukáže **zelený obdélník ve velikosti vozidla** (auto, motorka, loď,
  helikoptéra) se šipkou ve směru jízdy. Obdélník je vždy před tebou – postav se a otoč, jak má vozidlo stát.
  **Žlutý** obdélník = dosavadní místo.
- **Y** = uložit, **N** = zrušit (klávesy jdou změnit v nastavení ovládání, sekce OS Core).
  Uloží se do `Vehicles.json` (`SpawnX`, `SpawnZ`, `SpawnYaw`) a platí hned, bez restartu.

**Veřejné a osobní garáže**
- **Veřejné garáže** = místa v `Vehicles.json` s `"Garage": true`. Může je použít kdokoli (každý vidí jen svoje
  auta a auta své skupiny). Místo spawnu nastavuje admin.
- **Osobní garáž** (`PersonalGarages: true`): menu klávesa **K** → *Postavit garáž zde* (`PersonalGarageBuildPrice`,
  každý hráč max. jednu). Používá ji jen majitel – u ní klávesa **B** = uložit / vyzvednout vozidlo.
  Místo pro vozidlo si hráč nastaví sám (stejný obdélník, max. `PersonalSpawnMaxDistance` m od garáže)
  v menu K nebo v menu garáže. Garáž je na mapě jako žlutá značka „Moje garáž“.
- Garáž nejde postavit blíž než `PersonalGarageMinDistance` k jiné garáži nebo obchodu.
- *Zrušit garáž* – uložená vozidla zůstanou, vyzvedneš je v jakékoli garáži. Data: `PersonalGarages.json`.

**Bez zranění ve skupině** (`Groups.json` → `FriendlyFire: false`, výchozí)
- Členové stejné skupiny si navzájem neublíží střelbou, noži ani pěstmi – zásah se vůbec neprovede (ani krvácení).
- Výbuchy (granáty, miny) a přejetí autem se nepočítají jako útok hráče a zraňují dál.
- `FriendlyFire: true` = zranění mezi spoluhráči zapnout.

**Sdílená garáž** (`Vehicles.json` → `SharedGroupGarage: true`, výchozí)
- Ve skupině se auto ukládá do **garáže skupiny** (max. `GroupGarageMaxVehicles`, výchozí 6 aut).
  Vyzvednout ho může kterýkoli člen skupiny v kterékoli garáži.
- V seznamu garáže vidíš svoje osobní auta (uložená dřív) i auta skupiny (popisek „skupina“).
- Do garáže jde uložit i auto spoluhráče ze skupiny (ne jen svoje).
- Auto ze skupinové garáže dál patří původnímu majiteli a je sdílené se skupinou (zamyká ho kdokoli ze skupiny).
- Osobní auta uložená před vstupem do skupiny zůstávají v osobní garáži.

**Helikoptéry – DayZ Expansion** (`Vehicles.json`)
- Vanilla DayZ létající helikoptéry nemá. Mód počítá s **DayZ Expansion – Vehicles** (na serveru i u hráčů,
  včetně jeho závislostí `DayZ-Expansion-Core` a `DayZ-Expansion-Licensed`). Mód `@OS_Core` načítej **až za** Expansion.
- Výchozí nabídka heliportu (classname a díly ověřené ze skriptů Expansion 1.9.78):

  | Helikoptéra | Classname | Cena |
  |---|---|---|
  | Gyrokoptéra | `ExpansionGyrocopter` | 15 000 |
  | Velká gyrokoptéra | `ExpansionBigGyrocopter` | 20 000 |
  | MH-6 Little Bird | `ExpansionMh6` | 40 000 |
  | UH-1H Huey | `ExpansionUh1h` | 60 000 |
  | Merlin | `ExpansionMerlin` | 90 000 |

  Každá dostane díly (baterie `ExpansionHelicopterBattery`, `ExpansionIgniterPlug`, `ExpansionHydraulicHoses`,
  dveře / kola podle typu) a plnou nádrž.
- Expansion helikoptéry jsou postavené na `CarScript`, takže funguje i **vlastnictví, zamykání klávesou F, servis,
  garáž a sdílení se skupinou**.
- Heliport: v `Locations` místo s `"Dealer": true`, `"DealerCategory": "heli"` a `"SpawnClearRadius": 12`
  (ve výchozím nastavení ukázkový „Heliport“ – nastav mu souřadnice). Menu se jmenuje *Helikoptéry*.
- Díly helikoptér z Expansion (baterie, zapalovač, hydraulika…) se do obchodu přidají automaticky do kategorie *Dily aut*.
- Expansion má vlastní klíče od vozidel. Aby nekolidovaly s vlastnictvím z OS_Core, nech v nastavení Expansion
  (`VehicleSettings.json`) `VehicleRequireKeyToStart` na `0` a vozidla nepáruj s klíči Expansion.
- Bez Expansion: helikoptéry se v nabídce ukážou, ale koupě selže a peníze se vrátí – smaž je z `Offers`.

**Lodě** (`Vehicles.json`)
- Přístaviště: místo v `Locations` s `"Dealer": true`, `"DealerCategory": "boat"` a `"SpawnOnWater": true`
  (ve výchozím nastavení ukázkový „Pristav“). `SpawnX`/`SpawnZ` musí ležet **na moři** – loď se postaví na hladinu.
  Menu se jmenuje *Lode*.
- Výchozí nabídka:

  | Loď | Classname | Cena | Poznámka |
  |---|---|---|---|
  | Motorový člun (modrý/oranžový/černý/maskovaný) | `Boat_01_Blue` / `_Orange` / `_Black` / `_Camo` | 5 000–5 500 | vanilla |
  | Zodiac | `ExpansionZodiacBoat` | 8 000 | Expansion |
  | Užitková loď | `ExpansionUtilityBoat` | 15 000 | Expansion |
  | Vodnik (obojživelník) | `ExpansionVodnik` | 30 000 | Expansion |

- Všechny lodě (vanilla člun i Expansion) mají vlastnictví, zamykání (F), servis (oprava, tankování,
  přepsání na sebe), garáž a sdílení se skupinou – stejně jako auta.
- Bez Expansion smaž Expansion lodě z `Offers` (koupě by selhala a peníze by se vrátily).

**Motorky** (`Vehicles.json`)
- Výchozí autocentrum prodává auta i motorky: `"DealerCategory": "car,bike"` (víc kategorií oddělených čárkou;
  menu se pak jmenuje *Prodej vozidel*). Samostatný obchod s motorkami = `"DealerCategory": "bike"`.
- Nabídka (vanilla): `Motorbike_01_Blue` / `_Yellow` / `_Red` (1 500) a `Motorbike_02_Red` (2 500), s koly,
  svíčkou, světlem (u Motorbike_02 i kapotáží) a plnou nádrží.
- Vlastnictví, zamykání (F), servis a garáž fungují stejně jako u aut.

**Vlastnictví vozidel (bez klíčů)** – auta, lodě i helikoptéry
- Auto patří hráči podle **Steam ID**: tomu, kdo ho koupí v autobazaru nebo si nalezené auto přepíše v servisu
  (*Přepsat auto na sebe*, `ClaimPrice`). Auta, která spawnuje hra, majitele nemají.
- **Zamknout / odemknout:** podívej se na auto a stiskni **F** (akce „Zamknout auto“ / „Odemknout auto“).
  Může majitel nebo člen skupiny, se kterou je auto sdílené. Zamknout jde jen prázdné auto.
- Zamčené auto: nejde nastoupit, otevřít dveře zvenku, dívat se do kufru, nic do něj dávat ani sundávat díly.
  Kdo je uvnitř, může vystoupit.
- **Sdílení se skupinou:** auto uložené do **skupinové garáže** se automaticky přiřadí skupině – po vyzvednutí ho
  zamyká/odemyká kterýkoli člen. Majitelem zůstává původní hráč. Kdo ze skupiny odejde, přístup ztratí.
  V servisu je u auta vidět „Majitel: ty / tvoje skupina / jiný hráč / nikdo“.
- **Admin menu:** *Odemknout nejbližší auto* (do 15 m).
- Vlastnictví se ukládá přímo do auta (persistence serveru). **Pokud mód ze serveru odebereš,
  persistence aut se může poškodit** – před odebráním módu udělej zálohu `storage_1`.

### Souřadnice
- Interaktivní mapa (např. iZurvive): zobrazuje `X / Y` → v konfigu `X` a `Z`.
- Ve hře: admin menu → pole „souřadnice“ → teleport na `X Z` (ověříš si, kam bod vede).

### Testování a ladění
Mód je napsaný proti vanilla skriptům **DayZ 1.30**, ale **nebyl zatím spuštěn ve hře**.
Po prvním startu zkontroluj:
- `script_*.log` na serveru i klientovi – chyby kompilace / `[OS_Core]` hlášky,
- v Addon Builderu případné chyby při binarizaci configu.

Chybu z logu mi pošli a opravím ji.

---

## Spawnování zbraní a aut (centrální ekonomika)

Kolik zbraní a aut se na mapě objevuje, neurčuje mód, ale soubory mise serveru:
- `mpmissions/<mise>/db/types.xml` – předměty (`nominal` = cílový počet na mapě, `min` = kdy doplnit)
- `mpmissions/<mise>/db/events.xml` – auta (eventy `Vehicle...`)
- `mpmissions/<mise>/cfgeventspawns.xml` – **kde** se auta mohou objevit

Nastavení je v `economy/economy_tuning.json` (násobitel 1.0 = beze změny, 2.0 = dvakrát víc, 0.5 = polovina).
Zbraně jsou rozdělené na skupiny (vojenské, pušky, pistole, zásobníky, munice) a jednotlivé kusy jdou nastavit přesně
v `overrides`.

```bash
# nejdřív jen vypsat, co se změní
python3 tools/tune_economy.py --types db/types.xml --events db/events.xml --dry-run
# vytvoří types.tuned.xml a events.tuned.xml vedle originálů
python3 tools/tune_economy.py --types db/types.xml --events db/events.xml
```
Výsledné `*.tuned.xml` zkontroluj, přejmenuj na původní názvy a restartuj server.
U aut platí: víc aut než míst v `cfgeventspawns.xml` se neobjeví a počet omezuje i `max` u `<child>` v eventu.

---

## Vanilla spawn body (bez módu)

Vanilla spawny serveru určuje `cfgplayerspawnpoints.xml`. Mód je nepotřebuje
(menu teleportuje hráče až po spawnu), ale hodí se jako záloha, když hráč menu zavře.

```bash
python3 tools/generate_spawnpoints.py spawnpoints/example.csv \
    mpmissions/<mise>/cfgplayerspawnpoints.xml
```
CSV formát: `skupina,x,z`. Soubor nahraj do kořene mise (vedle `init.c`).
Pokud se hráči spawnují na `0,0,0`, zkus `--no-groups`.
