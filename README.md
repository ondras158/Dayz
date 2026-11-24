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
| Obchod | Klávesa **B** u obchodníka. **Peníze jen za prodej nalezených zbraní**, za ně se kupuje jídlo, medicína, nářadí, munice | `Trader.json` |
| Autobazar | Klávesa **B** v autocentru: koupě kompletního auta s plnou nádrží, postaví se na parkoviště | `Vehicles.json` |
| Servis | Oprava (platí se podle poškození), tankování (za litr), doplnění chybějících dílů | `Vehicles.json` |
| Garáže | Uložení auta (bez věcí v kufru) a vyzvednutí v libovolné garáži, max. počet aut na hráče | `Vehicles.json` |

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
├── Messages.json   Admins.json       Trader.json     Vehicles.json
├── Wallets/<SteamID>.json            peníze hráčů
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

**Obchod** – `Buy`/`Sell` = cena, `-1` = nelze koupit/prodat. Obchodník je jen místo
(`Traders`: X, Z, Radius) – dej ho do safe zóny a postav tam něco viditelného.
- Ve výchozím nastavení mají výkupní cenu **jen zbraně** a zbraně **nejdou koupit** – peníze tedy vzniknou
  jen z nalezených zbraní (nejde koupit a hned prodat). Za zombíky ani hráče peníze nejsou (`ZombieKillReward: 0`).
- Zbraň musí být **bez zásobníku a příslušenství** a nesmí být zničená. Opotřebovaná zbraň má 50–100 % ceny
  (`ScaleSellPriceByCondition`).
- Barevné varianty (např. `M4A1_Black`) se vykupují za cenu základní zbraně (`M4A1`).

**Auta** (`Vehicles.json`) – `Locations` = místa (autocentra). Každé má `Dealer`/`Service`/`Garage` (true/false),
kde hráč stojí (`X`, `Z`, `Radius`) a kam se postaví auto (`SpawnX`, `SpawnZ`, `SpawnYaw`).
Parkovací místo musí být volné (bez auta v okruhu 5 m).
`Offers` = auta na prodej s cenou a seznamem dílů (stejný seznam používá servis pro doplnění dílů).
Ve výchozím nastavení: Ada, Olga, Sarka, Gunter, M1025, V3S (vanilla auta – pokud má Nasdara vlastní auta,
přidej jejich classname a díly).
- Garáž ukládá typ auta, díly, jejich stav a palivo. **Věci v kufru se neukládají** – auto musí být prázdné.
- Vanilla DayZ nemá vlastnictví aut: uložit může kdokoli jakékoli auto, které stojí u garáže.

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
