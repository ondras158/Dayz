# DayZ – Badlands (Nasdara) server

| Složka | Co to je |
|---|---|
| `BadlandsCore/` | **Mód** (zdroj pro `.pbo`): výbava, výběr spawnu, safe zóny, zprávy, admin menu, obchod, auta |
| `economy/`, `tools/tune_economy.py` | Úprava spawnování zbraní a aut (types.xml, events.xml) |
| `mpmissions/dayzOffline.nasdara/` | Vanilla spawn body (`cfgplayerspawnpoints.xml`) |
| `nasdara-spawns/`, `tools/generate_spawnpoints.py` | Generátor vanilla spawn bodů z CSV |
| `tools/generate_layouts.py` | Přegeneruje vzhled menu (`.layout`) |

> ⚠️ Všechny souřadnice v módu i v CSV jsou zatím **ukázkové** (např. 1000/1000), ne skutečná místa na Nasdaře.
> Před ostrým provozem je nahraď.

---

## Mód BadlandsCore

### Funkce
| Funkce | Popis | Konfig |
|---|---|---|
| Startovní výbava | Nová postava dostane náhodnou sadu oblečení a věcí (podle váhy) | `Loadouts.json` |
| Výběr spawnu | Po vytvoření postavy se otevře menu se seznamem oblastí; během výběru je hráč nesmrtelný | `SpawnZones.json` |
| Safe zóny | V zóně nejde zranit hráče, zombíci se mažou, hláška při vstupu/odchodu | `SafeZones.json` |
| Zprávy | Uvítání po připojení, pravidelná oznámení v chatu, kill feed | `Messages.json` |
| Admin menu | Klávesa **Home**: léčení, zabití, teleporty, godmode, spawn předmětů, peníze | `Admins.json` |
| Obchod | Klávesa **B** u obchodníka. **Všechny předměty ze hry i z módů** se do obchodu přidají automaticky. Zbraně jdou koupit draze, ostatní věci prodat levně. Admin mění ceny přímo v menu | `Trader.json` |
| Autobazar | Klávesa **B** v autocentru: koupě kompletního auta s plnou nádrží, postaví se na parkoviště | `Vehicles.json` |
| Servis | Oprava (platí se podle poškození), tankování (za litr), doplnění chybějících dílů | `Vehicles.json` |
| Garáže | Uložení auta (bez věcí v kufru) a vyzvednutí v libovolné garáži, max. počet aut na hráče | `Vehicles.json` |
| Peníze a bankomaty | Peníze jsou předmět (bankovky) – po smrti zůstanou na těle. V bankomatu je uložíš na účet, kde jsou v bezpečí | `Bank.json` |
| Vlastnictví a klíče | Koupené auto dostane zámek a klíč. Zamčené auto: nejde nastoupit, otevřít dveře, do kufru ani sundat díly | `Vehicles.json` |

Klávesy lze změnit ve hře: Nastavení → Ovládání → sekce **Badlands Core**.

### Struktura
```
BadlandsCore/
├── config.cpp                     definice módu
├── GUI/layouts/*.layout           vzhled menu (lze upravit v DayZ Tools → Workbench)
└── Scripts/
    ├── Inputs.xml                 klávesy
    ├── 3_Game/   BC_Defines.c     RPC ID, konstanty
    │             BC_Config.c      třídy konfigurace + výchozí hodnoty
    ├── 4_World/  BC_Server*.c     serverová logika (každá funkce zvlášť)
    │             BC_PlayerBase.c  nesmrtelnost, kill feed
    │             BC_ZombieBase.c  odměna za zombíka
    └── 5_Mission/BC_Mission*.c    napojení na server/klienta
                  BC_Client.c      příjem dat ze serveru
                  GUI/*.c          menu (spawn, admin, obchod)
```

### 1. Zabalení do .pbo
1. Nainstaluj ze Steamu **DayZ Tools**.
2. Spusť **Addon Builder**:
   - *Addon source directory*: složka `BadlandsCore` z tohoto repozitáře
   - *Destination*: např. `C:\DayZMods\@BadlandsCore\Addons`
   - V *Options* nastav **Prefix** na `BadlandsCore`.
   - Zaškrtni *Binarize* (config.cpp) a nech `*.c;*.layout;*.xml` v seznamu souborů ke zkopírování.
   - Pro podpis: *Sign output* + tvůj `.biprivatekey` (vytvoříš v DS Utils → DSCreateKey).
3. Výsledek:
   ```
   @BadlandsCore/
   ├── Addons/BadlandsCore.pbo
   ├── Addons/BadlandsCore.pbo.<jméno>.bisign
   └── Keys/<jméno>.bikey
   ```

### 2. Instalace na server
1. Zkopíruj `@BadlandsCore` do složky serveru.
2. `<jméno>.bikey` zkopíruj do složky `keys` serveru.
3. Spouštěcí parametr: `-mod=@BadlandsCore`
   (ne `-servermod` – mód má menu, takže ho musí mít i hráči).
4. Hráči si mód stáhnou (Workshop nebo ručně) a spouští hru se stejným `-mod`.

### 3. Konfigurace
Při prvním spuštění se v profilové složce serveru (parametr `-profiles=`) vytvoří:
```
<profiles>/BadlandsCore/
├── Loadouts.json   SpawnZones.json   SafeZones.json
├── Messages.json   Admins.json       Trader.json     Vehicles.json   Bank.json
├── Wallets/<SteamID>.json            bankovní účty hráčů
└── Garages/<SteamID>.json            auta v garáži
```
Uprav je a **restartuj server**. Chybu v JSON najdeš ve script logu pod `[BadlandsCore]`
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
- Obchodník je jen místo (`Traders`: X, Z, Radius) – dej ho do safe zóny a postav tam něco viditelného.

**Auta** (`Vehicles.json`) – `Locations` = místa (autocentra). Každé má `Dealer`/`Service`/`Garage` (true/false),
kde hráč stojí (`X`, `Z`, `Radius`) a kam se postaví auto (`SpawnX`, `SpawnZ`, `SpawnYaw`).
Parkovací místo musí být volné (bez auta v okruhu 5 m).
`Offers` = auta na prodej s cenou a seznamem dílů (stejný seznam používá servis pro doplnění dílů).
Ve výchozím nastavení: Ada, Olga, Sarka, Gunter, M1025, V3S (vanilla auta – pokud má Nasdara vlastní auta,
přidej jejich classname a díly).
- Garáž ukládá typ auta, díly, jejich stav a palivo. **Věci v kufru se neukládají** – auto musí být prázdné.

**Peníze a bankomaty** (`Bank.json`)
- Peníze jsou **bankovky 10, 50, 100, 500 a 1000 Kr** (`BC_Money_10` … `BC_Money_1000`), každá s vlastním potiskem
  (`BadlandsCore/data/money_*_co.png`, náhled `preview_bankovky.png`). Bankovky stejné hodnoty se stohují
  (max. 100 ks v balíčku), nic neváží.
- Při placení se bankovky rozmění samy: zaplatíš od největších, a když to nejde přesně, vrátí se drobné v bankovkách.
  Částky pod 10 Kr (drobné) nejdou vyplatit v hotovosti – připíšou se na účet.
- **Textury je potřeba převést na .paa** před zabalením: DayZ Tools → *ImageToPAA* (nebo TexView2 → Save As .paa)
  na všechny `BadlandsCore/data/money_*_co.png`. Výsledné `money_*_co.paa` nech ve stejné složce a v Addon Builderu
  přidej `*.paa` do souborů ke kopírování. Textury znovu vygeneruješ: `python3 tools/generate_money_textures.py`.
- Potisk se nanáší na model papíru přes `hiddenSelections[] = {"zbytek"}`. Pokud se bankovky ve hře ukážou
  jako obyčejný papír, má model jiný název oblasti: otevři v Object Builderu `dz\gear\consumables\Paper.p3d`
  (z disku P:), podívej se do *Named Selections* a název přepiš v `config.cpp` u třídy `BC_Money`.
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

**Vlastnictví a klíče**
- Koupené auto má majitele a zámek, kupující dostane **klíč** (`BC_CarKey`, model klíčků od pout).
- Zamknutí/odemknutí: klíč do ruky → podívej se na auto → **podrž levé tlačítko**.
  Zamknout jde jen prázdné auto. Klíč funguje jen ke svému autu (číslo na konci názvu klíče pomáhá rozlišit klíče).
- Zamčené auto: nejde nastoupit, otevřít dveře zvenku, dívat se do kufru, nic do něj dávat ani sundávat díly.
  Kdo je uvnitř, může vystoupit.
- Klíč se dá předat, ztratit nebo ukrást – kdo má klíč, auto odemkne.
- **Servis:**
  - *Přepsat auto na sebe* (`ClaimPrice`) – nalezené auto bez majitele → stane se tvým a dostaneš klíč,
  - *Kopie klíče* (`KeyCopyPrice`) – jen majitel,
  - *Vyměnit zámek* (`ChangeLockPrice`) – jen majitel, staré klíče přestanou fungovat (při ztrátě/krádeži klíče).
- **Garáž:** auto s majitelem uloží jen majitel (`OnlyOwnerCanStore`). Zámek zůstává, staré klíče platí dál.
- **Admin menu:** *Odemknout nejbližší auto* (do 15 m).
- Auta, která spawnuje hra (CE), majitele nemají a jdou řídit jako ve vanille.
- Vlastnictví se ukládá přímo do auta (persistence serveru). **Pokud mód ze serveru odebereš,
  persistence aut se může poškodit** – před odebráním módu udělej zálohu `storage_1`.

### Souřadnice
- Interaktivní mapa (např. iZurvive): zobrazuje `X / Y` → v konfigu `X` a `Z`.
- Ve hře: admin menu → pole „souřadnice“ → teleport na `X Z` (ověříš si, kam bod vede).

### Testování a ladění
Mód je napsaný proti vanilla skriptům **DayZ 1.30**, ale **nebyl zatím spuštěn ve hře**.
Po prvním startu zkontroluj:
- `script_*.log` na serveru i klientovi – chyby kompilace / `[BadlandsCore]` hlášky,
- v Addon Builderu případné chyby při binarizaci configu.

Chybu z logu mi pošli a opravím ji.

---

## Spawnování zbraní a aut (centrální ekonomika)

Kolik zbraní a aut se na mapě objevuje, neurčuje mód, ale soubory mise serveru:
- `mpmissions/dayzOffline.nasdara/db/types.xml` – předměty (`nominal` = cílový počet na mapě, `min` = kdy doplnit)
- `mpmissions/dayzOffline.nasdara/db/events.xml` – auta (eventy `Vehicle...`)
- `mpmissions/dayzOffline.nasdara/cfgeventspawns.xml` – **kde** se auta mohou objevit

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
python3 tools/generate_spawnpoints.py nasdara-spawns/spawnpoints.csv \
    mpmissions/dayzOffline.nasdara/cfgplayerspawnpoints.xml
```
CSV formát: `skupina,x,z`. Soubor nahraj do kořene mise (vedle `init.c`).
Pokud se hráči spawnují na `0,0,0`, zkus `--no-groups`.
