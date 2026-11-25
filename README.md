# OS_Core – serverový mód pro DayZ

Velký serverový mód pro DayZ 1.30: obchody s automatickým katalogem všech předmětů, peníze jako bankovky
a bankomaty, auta, helikoptéry a lodě s vlastnictvím bez klíčů, garáže, skupiny, farmy, base u vlajky
s kódovými zámky, PvP zóny na mapě, admin nástroje a překlady do 4 jazyků.

Funguje na **jakékoli mapě** (Chernarus, Livonia, Sakhal, Nasdara, komunitní mapy). Vše, co závisí na mapě
(souřadnice obchodníků, safe a PvP zón, spawnů, bankomatů, autocenter), je v konfiguračních JSON souborech
serveru – mód se kvůli jiné mapě nemusí znovu balit.

| Složka | Co to je |
|---|---|
| `OS_Core/` | **Mód** (zdroj pro `.pbo`) – všechno popsané níže |
| `OS_Core_Expansion/` | Volitelný doplněk pro **Expansion BaseBuilding** (skupiny = členové území), samostatné `.pbo` |
| `OS_Core_ExpansionAI/` | Volitelný doplněk pro **Expansion AI** (opravy výhledu, hlídky, útoky, stráže airdropů), samostatné `.pbo` |
| `lang/` | Překlady (`cs`, `en`, `de`, `pl`) – zdroj pro vestavěné texty módu |
| `economy/`, `tools/tune_economy.py` | Úprava spawnování zbraní a aut (types.xml, events.xml) |
| `spawnpoints/`, `tools/generate_spawnpoints.py` | Generátor vanilla spawn bodů (`cfgplayerspawnpoints.xml`) z CSV |
| `money_images/`, `tools/*banknote*.py`, `tools/*money*.py` | Potisky bankovek (vlastní obrázky, generátory, přebarvení) |
| `tools/generate_layouts.py` | Přegeneruje vzhled menu (`.layout`) a seznam textů menu k překladu |
| `tools/i18n_build.py` | Posbírá texty ze skriptů a z `lang/*.json` vygeneruje vestavěné překlady |

> ⚠️ Všechny souřadnice v módu i v CSV jsou zatím **ukázkové** (např. 2000/2000), ne skutečná místa na mapě.
> Před ostrým provozem je nahraď.

### Použití na jiné mapě
1. Nainstaluj mód (viz níže) a jednou spusť server – v profilové složce se vytvoří `OS_Core/*.json`.
2. V JSON souborech nahraď ukázkové souřadnice místy z tvé mapy:
   `SpawnZones.json`, `SafeZones.json`, `PvP.json` (`Zones`), `Trader.json` (`Traders`),
   `Vehicles.json` (`Locations`), `Bank.json` (`Atms`), `Farms.json` (`AnimalTraders`).
   Souřadnice zjistíš na interaktivní mapě (např. iZurvive) nebo ve hře (admin menu → teleport na X Z,
   PvP zóny jdou přidat přímo ve hře).
3. Restartuj server. Obchod se sám naplní předměty, které na serveru jsou (vanilla + nahrané módy).

Každý server (mapa) má vlastní profilovou složku, takže můžeš mít víc serverů s různým nastavením a jedním módem.

---

## Mód OS_Core

### Klávesy
| Klávesa | Co dělá |
|---|---|
| **B** | Obchod / bankomat / autocentrum, servis a garáž / Farmář – podle toho, u čeho stojíš |
| **J** | Skupina |
| **K** | Osobní garáž |
| **U** | Farma |
| **L** | Emoce a spánek (sednout, lehnout, mávat, salutovat, tanec… a *Spát*) |
| **H** | Skrýt / zobrazit HUD |
| **O** | Dovednosti (úroveň, zkušenosti, vylepšování statů) |
| **M** | Mapa (i bez mapy v inventáři) – PvP zóny, safe zóny, base, spoluhráči, garáž, farma |
| **F** (při pohledu) | Zamknout / odemknout vozidlo; u kódového zámku *Odemknout / Zamknout (skupina)*; u vlajky *Base*; u ohně *Vařit podle receptu* |
| **Y / N** | Uložit / zrušit místo spawnu vozidla |
| **Home** | Admin menu (jen admini) |
| **End** | Admin freecam (jen admini) |

Klávesy lze změnit ve hře: Nastavení → Ovládání → sekce **OS Core**.

### Funkce
| Funkce | Popis | Konfig |
|---|---|---|
| Startovní výbava | Nová postava dostane náhodnou sadu oblečení a věcí (podle váhy) | `Loadouts.json` |
| Výběr spawnu | Po vytvoření postavy se otevře menu se seznamem oblastí; během výběru je hráč nesmrtelný | `SpawnZones.json` |
| Safe zóny | V zóně nejde zranit hráče, zombíci se mažou, hláška při vstupu/odchodu, zeleně na mapě | `SafeZones.json` |
| PvP zóny a mapa | Klávesa **M** otevře mapu i bez mapy; PvP jen v PvP zónách (červeně na mapě), časovač po odchodu ze zóny, admin přidává zóny ve hře | `PvP.json` |
| Zprávy | Uvítání po připojení, pravidelná oznámení v chatu, kill feed | `Messages.json` |
| Jazyky | Čeština, angličtina, němčina, polština + vlastní; všechny texty jdou upravit | `Language.json`, `Lang/*.json` |
| Obchod | **Všechny předměty ze hry i z módů** se do obchodu přidají automaticky do kategorií. Zbraně jdou koupit draze, ostatní věci prodat levně. Admin mění ceny přímo v menu. 15 specializovaných obchodníků (zbrojíř, oblečení, potraviny, lékárna, železářství, černý trh, rybář, řezník, lovec, výkupna…) s násobiteli cen | `Trader.json` |
| Peníze a bankomaty | Peníze jsou předmět (bankovky 1–5000, měna EUR/RUB/… podle nastavení) – po smrti zůstanou na těle. V bankomatu je uložíš na účet, převody jiným hráčům | `Bank.json` |
| Autobazar / motorky / helikoptéry / lodě | Koupě kompletního vozidla s plnou nádrží; helikoptéry a lodě z **DayZ Expansion**, vanilla člun a motorky | `Vehicles.json` |
| Servis | Oprava (podle poškození), tankování (za litr), doplnění chybějících dílů, přepsání nalezeného auta na sebe | `Vehicles.json` |
| Garáže | Veřejné garáže, osobní garáž (klávesa K) a sdílená garáž skupiny; místo spawnu vozidla se nastavuje ve hře obdélníkem | `Vehicles.json` |
| Vlastnictví vozidel | Vozidlo patří hráči podle Steam ID, zamyká se klávesou F bez klíče; ve skupinové garáži se sdílí se skupinou | `Vehicles.json` |
| Skupiny | Klávesa **J**: až 4 hráči zdarma, další místa za peníze, role vlastník / admin / člen, společný účet, spoluhráči na mapě a jména nad hlavou, bez zranění ve skupině | `Groups.json` |
| Farmy | Klávesa **U**: zvířata (slepice, prasata, krávy, kozy, ovce), sad a skleník, krmivo a hnojivo, výroba i offline, Farmář a výkupna úrody; farma jen ve vlastní base | `Farms.json` |
| Airdropy | Každých ~45 min spadne na padáku bedna s lootem (vojenský / zdravotnický / zásoby) na náhodné místo, kouř, stráže, oznámení v chatu a značka na mapě; admin může shodit ručně | `Airdrops.json` |
| Hlasový chat | Vanilla hlas + **skupinová vysílačka** (každá skupina má svůj kanál, vysílačky se naladí samy) a v HUDu kdo právě mluví | `Groups.json`, `Hud.json` |
| Únava, spánek a emoce | Únava roste časem a během (menší výdrž, při vyčerpání ubývá zdraví); spánek přes menu **L** – postava si lehne, obrazovka ztmavne, ve stanu nebo v base rychleji; menu 21 emocí s oznámením v okolí; animace u vaření, farmy, bankomatu, obchodu, base a skupiny | `Sleep.json` |
| HUD | Panel v rohu obrazovky: zdraví, krev, jídlo, voda, výdrž, teplo, zóna (bezpečná / PvP / PvE) a odpočet PvP, base, spoluhráči se vzdáleností, souřadnice, směr, čas, počet hráčů; klávesa **H** skryje | `Hud.json` |
| Dovednosti | Klávesa **O**: zkušenosti za zombíky, zvířata, hráče, vaření, farmu a čas na serveru; za úroveň bod do odolnosti, výdrže, regenerace, krve, metabolismu nebo otužilosti; zůstává i po smrti | `Skills.json` |
| Vaření podle receptů | U hořícího ohně nebo vařiče akce **Vařit podle receptu**: 10 jídel (guláš, polévky, rizoto, omeleta…), víc energie a vody, dočasné bonusy (zdraví, krev, teplo, výdrž), prodej v Potravinách | `Cooking.json` |
| Base a kódové zámky | Akce **Base** u vanilla vlajky: založení, zvětšování poloměru za peníze, rozestupy mezi basemi, base na mapě; vanilla kódový zámek – majitel a skupina odemknou bez kódu | `Bases.json` |
| Admin menu | Klávesa **Home**: léčení, zabití, teleporty, godmode, spawn předmětů, peníze, PvP zóny; **Nástroje (COT)**: spawner, teleport z mapy, čas, počasí, ESP, neviditelnost, freecam, auta, kódové zámky, base, kick/ban, přidávání adminů, hlavní admin | `Admins.json` |
| AI vojáci – Expansion AI (doplněk) | Opravy: nevidí a nestřílí přes zdi, vidí jen na rozumnou vzdálenost (den / noc / postoj), přesnost klesá se vzdáleností; hlídky po trasách, útoky banditů na base, stráže airdropů, **konvoje** s nákladem (přepadení, útoky na base) | `ExpansionAI.json`, `Convoys.json` |
| Expansion BaseBuilding (doplněk) | Členové OS skupiny se automaticky stanou členy území vlajky Expansion | `ExpansionTerritory.json` |

### Struktura
```
OS_Core/
├── config.cpp                     definice módu, bankovky (OS_Money_*), vejce (OS_ChickenEgg), jídla (OS_Dish_*)
├── stringtable.csv                názvy předmětů módu podle jazyka hry (cs, en, de, pl, ru)
├── data/                          potisky bankovek (money_*_co.paa)
├── GUI/layouts/*.layout           vzhled menu (generuje tools/generate_layouts.py)
└── Scripts/
    ├── Inputs.xml                 klávesy
    ├── 3_Game/   OS_Defines.c     RPC ID, akce, konstanty
    │             OS_*Config.c     třídy konfigurace + výchozí hodnoty (obchod, auta, banka, skupiny,
    │                              farmy, PvP, base)
    │             OS_Lang.c        překlady (OS_T / OS_F), OS_LangData.c = vestavěné texty z lang/*.json
    │             OS_Transport.c   vlastnictví a zamykání vozidel
    ├── 4_World/  OS_Server*.c     serverová logika (každá funkce zvlášť: obchod, banka, auta, skupiny,
    │                              farmy, PvP, base, admin…)
    │             OS_TraderScanner.c  automatický katalog předmětů
    │             OS_CarActions.c  akce F u vozidel; OS_CodeLock.c = kódový zámek a akce Base u vlajky
    │             OS_PlayerBase.c  nesmrtelnost, blokování zranění (skupina, PvP), neviditelnost, kill feed
    │             OS_ZombieBase.c  odměna za zombíka
    └── 5_Mission/OS_Mission*.c    napojení na server/klienta, klávesy, mapa bez předmětu
                  OS_Client.c      příjem dat ze serveru
                  OS_NameTags.c    jména nad hlavou, značky a kruhy zón na mapě
                  OS_SpawnPlacer.c obdélník pro nastavení místa vozidla
                  GUI/*.c          menu (spawn, admin, admin nástroje, obchod, vozidla, bankomat,
                                   skupina, garáž, farma, base)
OS_Core_Expansion/                 doplněk pro Expansion BaseBuilding (vlastní .pbo)
```

### 1. Zabalení do .pbo
1. Nainstaluj ze Steamu **DayZ Tools**.
2. Spusť **Addon Builder**:
   - *Addon source directory*: složka `OS_Core` z tohoto repozitáře
   - *Destination*: např. `C:\DayZMods\@OS_Core\Addons`
   - V *Options* nastav **Prefix** na `OS_Core`.
   - Zaškrtni *Binarize* (config.cpp) a nech `*.c;*.layout;*.xml;*.paa` v seznamu souborů ke zkopírování.
   - Pro podpis: *Sign output* + tvůj `.biprivatekey` (vytvoříš v DS Utils → DSCreateKey).
3. Výsledek:
   ```
   @OS_Core/
   ├── Addons/OS_Core.pbo
   ├── Addons/OS_Core.pbo.<jméno>.bisign
   └── Keys/<jméno>.bikey
   ```
4. Pokud máš **Expansion AI**, zabal stejně i složku `OS_Core_ExpansionAI` (prefix `OS_Core_ExpansionAI`).
5. Pokud máš **Expansion BaseBuilding**, zabal stejně i složku `OS_Core_Expansion`
   (prefix `OS_Core_Expansion`) do `@OS_Core\Addons`. Bez Expansionu ho vynech (viz níže).

### 2. Instalace na server
1. Zkopíruj `@OS_Core` do složky serveru.
2. `<jméno>.bikey` zkopíruj do složky `keys` serveru.
3. Spouštěcí parametr: `-mod=@OS_Core` (s Expansion: `-mod=@CF;@DayZ-Expansion-...;@OS_Core` – OS_Core až za Expansion)
   (ne `-servermod` – mód má menu, takže ho musí mít i hráči).
4. Hráči si mód stáhnou (Workshop nebo ručně) a spouští hru se stejným `-mod`.

### 3. Konfigurace
Při prvním spuštění se v profilové složce serveru (parametr `-profiles=`) vytvoří:
```
<profiles>/OS_Core/
├── Language.json   Lang/cs.json  Lang/en.json  Lang/de.json  Lang/pl.json     jazyk a texty
├── Admins.json                                                               admini, hlavní admini, bany
├── Loadouts.json   SpawnZones.json   SafeZones.json   PvP.json   Messages.json
├── Trader.json                                                               obchod, ceny, obchodníci
├── Bank.json       Wallets/<SteamID>.json                                    měna, bankomaty, účty hráčů
├── Vehicles.json   Garages/…   PersonalGarages.json                          vozidla, garáže
├── Groups.json     GroupsData.json                                           skupiny
├── Farms.json      FarmsData.json                                            farmy
├── Bases.json      BasesData.json                                            base u vlajky
├── Cooking.json                                                              recepty a bonusy z jídla
├── Skills.json     Skills/<SteamID>.json                                     dovednosti, úrovně hráčů
├── Hud.json                                                                  HUD v rohu obrazovky
├── Sleep.json      SleepData.json                                            únava a spánek, emoce
├── Airdrops.json                                                             airdropy, místa, loot
├── ExpansionAI.json   Convoys.json                                           jen s doplňkem pro Expansion AI
└── ExpansionTerritory.json   ExpansionTerritoryData.json                     jen s doplňkem pro Expansion
```
Uprav je a **restartuj server**. Chybu v JSON najdeš ve script logu pod `[OS_Core]`
(server pak použije výchozí hodnoty – soubor nepřepíše).

> **Aktualizace módu:** existující JSON soubory se při aktualizaci **nepřepisují a nové položky se do nich samy
> nedoplní** (platí pro ně výchozí hodnoty ze skriptu). Chceš-li nové volby nebo nové výchozí obchodníky
> a kategorie vidět v souboru, soubor smaž (vytvoří se znovu s výchozím obsahem) a své úpravy přenes zpět,
> nebo nové položky dopiš ručně podle tohoto README.

**Jazyk** – v `Language.json` nastav jazyk celého módu (menu, hlášky, názvy kategorií, zvířat, zón...):
```json
{ "Language": "en" }
```
`cs` = čeština, `en` = English, `de` = Deutsch, `pl` = Polski. Hráči dostanou jazyk od serveru při připojení.
- **Úprava textů:** `Lang/<jazyk>.json` obsahuje všechny texty jako `"český text": "překlad"`. Změň hodnotu
  (vpravo), klíč (vlevo) nech být. `%1`, `%2`... jsou čísla a jména doplněná za běhu – musí v textu zůstat.
  Upravené texty server posílá i hráčům, takže se změní i v menu. Prázdná hodnota = použije se vestavěný text.
- **Vlastní jazyk:** zkopíruj třeba `Lang/en.json` na `Lang/ru.json`, přelož hodnoty a dej `"Language": "ru"`.
- Soubory `Lang/*.json` se vytvoří jen když chybí. Po aktualizaci módu s novými texty se nové texty
  zobrazí ve vestavěném překladu; chceš-li je mít v souboru k úpravě, smaž ho a nech vytvořit znovu.
- Názvy z configů (kategorie, obchodníci, zóny, zvířata, uvítací zprávy) se překládají, jen pokud
  mají výchozí hodnotu. Když si je v configu přejmenuješ, zobrazí se tak, jak je napíšeš.
- Vývoj: nové texty ve skriptech se píšou jako `OS_T("text")` / `OS_F("text %1", hodnota)`,
  překlady jsou v `lang/*.json` a `python3 tools/i18n_build.py` z nich vygeneruje `OS_LangData.c`.

**Nastavení admina** – do `Admins.json` dej svoje Steam64 ID (17 číslic). Prvního admina je potřeba zapsat
ručně, další už jde přidat ve hře (admin menu → Nástroje → *Přidat admina*):
```json
{
  "AdminSteamIds": ["76561198000000001"],
  "HeadAdminSteamIds": ["76561198000000000"],
  "OnlyHeadAdminsManageAdmins": false,
  "LogActions": true,
  "BannedSteamIds": [],
  "BanMessage": "Jsi zabanovan na tomto serveru."
}
```
- `HeadAdminSteamIds` = hlavní admini (viz Admin nástroje), `BannedSteamIds` plní tlačítko *Ban* ve hře.
- **Admin menu** (klávesa **Home**): seznam hráčů online (`[ADMIN]`, `[HLAVNI ADMIN]`, `[GOD]`, `[mrtvy]`),
  léčení, zabití, teleport k hráči / hráče k sobě, godmode, předmět do inventáře (`classname počet`), peníze na účet,
  teleport na `X Z`, odemknutí nejbližšího auta, PvP zóna zde / smazat PvP zónu a tlačítko **Nástroje (COT)**.
  Bez vybraného hráče se akce týká admina samotného. Všechny akce se zapisují do logu (`LogActions`).

**Spawn zóna** – body jsou `[x, 0, z]` (výška se dopočítá):
```json
{ "Name": "Pobrezi - jih", "Points": [[1234.5, 0, 2345.6], [1250, 0, 2360]] }
```

**Výbava** – `"A|B|C"` znamená náhodně jedno z A, B, C. `Weight` = jak často se sada vybírá.

**Safe zóny** (`SafeZones.json`) – kruh (`Name`, `X`, `Z`, `Radius`). `Godmode` = hráči v zóně nedostávají poškození,
`RemoveZombies` = zombíci se v zóně mažou (každých `ZombieCleanupSeconds`), hlášky `EnterMessage` / `LeaveMessage`.
Na mapě jsou zeleně (`PvP.json` → `ShowSafeZonesOnMap`).

**Zprávy** (`Messages.json`) – uvítání po připojení (`WelcomeTitle`, `WelcomeText`), oznámení v chatu
(`Broadcasts` každých `BroadcastIntervalMinutes`) a kill feed (`KillFeed`).

**Obchod** (`Trader.json`)
- **Automatický katalog:** při každém startu server projde všechny předměty v configu hry
  (`CfgVehicles`, `CfgWeapons`, `CfgMagazines`, tedy vanilla i všechny nahrané módy) a ty, které v obchodě
  ještě nejsou, přidá do kategorie podle `CategoryRules`. Přidáš mód s novými věcmi → po restartu jsou v obchodě.
  Ceny, které už v souboru jsou (i upravené adminem), se nikdy nepřepíšou. Vypnutí: `"AutoAddItems": false`.
- **Pravidla kategorií** (`CategoryRules`, první shoda vyhrává): podle předka v configu (`BaseClasses`, např.
  `Weapon_Base`), části názvu (`NameContains`, např. `Bandage`) nebo slotu (`Slots`, např. `Back` = batohy).
  `NameExcludes` = názvy, na které se pravidlo nepoužije (např. `Can` u ryb, aby konzervy zůstaly v jídle).
  `DefaultBuy`/`DefaultSell` = cena pro nově přidané věci. Poslední pravidlo bez podmínek bere vše ostatní.
  `ExcludeContains` = co se nikdy nepřidá (testovací a základní třídy).
- **Výchozí ceny:** zbraně koupit za 4× výkupní cenu (např. AKM: výkup 350, nákup 1400),
  ostatní věci se vykupují zhruba za 1/5 nákupní ceny. Koupit a hned prodat se nikdy nevyplatí.
- **Prodej:** předmět musí být prázdný, bez příslušenství (zbraň bez zásobníku) a nezničený.
  Opotřebovaný má 50–100 % ceny, načatý (krabička nábojů, jídlo) poměrnou cenu.
  Barevné varianty se vykupují za cenu základní věci (`M4A1_Black` → `M4A1`).
- **Vojenské věci** nemají vlastní kategorii: vojenské oblečení (uniformy, vesty, helmy, sumky, pouzdra, noktovizor)
  je v *Obleceni*, a co patří ke zbraním (krabice s municí, bajonety, sada na čištění zbraní) je u *Zbrojire*.
- **Další nastavení obchodníka:** `PlayersCanBuy` (prodává hráčům), `PlayersCanSell` (vykupuje od hráčů),
  `Items` (konkrétní classname navíc ke kategoriím), `FarmProduce` (vykupuje výrobky farem).
- **Násobitele cen obchodníka:** v `Traders` má každý obchodník `BuyMultiplier` (nákup) a `SellMultiplier` (výkup),
  výchozí 1. Černý trh: nákup **1,5×**, výkup **1,3×**. Hráč vidí v menu už upravené ceny a v titulku procenta.
  `SellMultiplier` drž pod poměrem nákup/výkup (zbraně 4×, ostatní ~5×), jinak by se vyplatilo koupit u jednoho
  obchodníka a prodat u druhého. Admin v panelu obchodu vždy upravuje **základní** cenu z katalogu.
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
  - *Potraviny* – `Jidlo a piti` (včetně alkoholu z módů – pivo, vodka, whisky, víno, brandy, tequila…),
  - *Cerny trh* – `Drogy` (lysohlávka a drogy z módů – tráva, kokain, heroin, pervitin, LSD, extáze, opium)
    a zbraně: `Pistole`, `Zbrane`, `Munice`, `Zasobniky`, `Prislusenstvi zbrani`, `Vybusniny`,
  - *Lekarna* – `Medicina`,
  - *Zelezarstvi* – `Nastroje a ostatni`, `Stavba zakladny` (hřebíky, prkna, plechy, dráty, sady na plot/věž/vlajku,
    zámky včetně kódového zámku, stany, maskovací sítě, bedny, truhly, sudy, skrýše, nábytek, trezory, vlajky) a `Elektronika`
    (vysílačky, megafon, GPS, dálkoměr, 9V baterie, nabíječka, žárovky, svítilny, sada na opravu elektroniky,
    budík, Geigerův počítač, generátor, reflektor, kabel),
  - *Zahradnictvi* – `Zahrada` (semínka a balíčky semínek, zahradní vápno, motyka, hnojiva a konve z módů;
    semínka konopí jsou na černém trhu),
  - *Vykupna* – jen **vykupuje úrodu z farem** (vše, co v `Farms.json` vyrábějí zvířata a rostliny: vejce, ovoce,
    zelenina, mléko, maso, sádlo, peří, kůže, konopí) o 20 % lépe (`SellMultiplier: 1.2`), nic neprodává,
  - *Rybar* – `Ryby`: vykupuje ryby (kapr, sumec, makrela, sardinky, treska, pstruh, hořavky, krevety, kaviár, filety)
    a prodává rybářské vybavení (pruty, sítě, pasti na ryby, háčky, návnady, červi). Konzervy zůstávají v Potravinách,
  - *Reznik* – `Maso` (maso ze zvěře i z farmy – zvěřina, vepřové, hovězí, kozí, skopové, kuřecí – a sádlo),
  - *Lovec* – `Lov` (pasti a oka na zvěř, kostěný a lovecký nůž) a `Zvireci suroviny` (kůže, vnitřnosti, peří, kosti),
  - *Autodily* – `Dily aut` (kola, dveře, kapoty, baterie, svíčky, chladiče, světla, díly helikoptér Expansion).

  Názvy musí odpovídat kategoriím v `Categories` (včetně těch, které vytvořila automatika). Obchodníka můžeš
  přidat kolik chceš, např. vojenský obchod: `{ "Name": "Vojak", "X": ..., "Z": ..., "Radius": 10, "Categories": ["Zbrane", "Munice", "Obleceni"] }`.
- Obchodník je jen místo (`Traders`: X, Z, Radius) – dej ho do safe zóny a postav tam něco viditelného.

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

**Auta** (`Vehicles.json`) – `Locations` = místa (autocentra). Každé má `Dealer`/`Service`/`Garage` (true/false),
kde hráč stojí (`X`, `Z`, `Radius`) a kam se postaví auto (`SpawnX`, `SpawnZ`, `SpawnYaw`).
Parkovací místo musí být volné (bez auta v okruhu 5 m).
`Offers` = auta na prodej s cenou a seznamem dílů (stejný seznam používá servis pro doplnění dílů).
Ve výchozím nastavení: Ada, Olga, Sarka, Gunter, M1025, V3S (vanilla auta – pokud má tvoje mapa vlastní auta,
přidej jejich classname a díly).
- Garáž ukládá typ auta, díly, jejich stav a palivo. **Věci v kufru se neukládají** – auto musí být prázdné.

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

**Skupiny** (`Groups.json`, data skupin v `GroupsData.json`)
- Menu skupiny: klávesa **J**. Kdo není ve skupině, může ji založit (název 3–24 znaků) nebo přijmout pozvánku.
- **Velikost:** `BaseMaxMembers` (4) zdarma, další místo kupuje vlastník nebo admin: první za `SlotPrice` (1000),
  každé další o `SlotPriceIncrease` (500) dražší, maximálně `MaxMembersLimit` (10) hráčů.
  Místo se platí ze **skupinového účtu**, a když na něm není dost, z hotovosti toho, kdo kupuje.
- **Pozvánky:** vlastník a admini (nebo všichni, pokud `MembersCanInvite: true`) zve hráče online, kteří nejsou v jiné skupině.
  Pozvaný dostane oznámení a pozvánku přijme/odmítne v menu skupiny (platí `InviteValidSeconds`).
- **Role:**
  | Role | Co smí |
  |---|---|
  | **Vlastník** | vše: zvát, vyhazovat kohokoli, kupovat místa, vybírat z účtu, jmenovat/odvolat adminy (*Admin / člen*), předat vlastnictví, rozpustit skupinu |
  | **Admin** | úplně totéž co vlastník (může vyhodit jiného admina, jmenovat adminy, kupovat místa, vybírat z účtu) – jen **nemůže vyhodit vlastníka, předat vlastnictví ani rozpustit skupinu** |
  | **Člen** | zvát a vybírat z účtu jen podle `MembersCanInvite` / `MembersCanWithdraw` |

  Vlastník se od admina liší tím, že jen on předává vlastnictví a rozpouští skupinu a jeho nejde vyhodit ani odvolat tlačítkem *Admin / člen*.
  Při předání vlastnictví zůstane původní vlastník adminem. Při rozpuštění jde zůstatek účtu vlastníkovi. Když vlastník odejde, převezme skupinu
  první admin, a když žádný není, první člen. Starší skupiny fungují dál, dosavadní vedoucí je vlastník.
- **Společný účet:** v bankomatu *Vložit do skupiny* / *Vybrat ze skupiny* (částka z pole Částka, prázdné = vše).
  `MembersCanWithdraw: false` = vybírat smí jen vlastník a admini. Při rozpuštění nebo odchodu posledního člena
  jde zůstatek na jeho osobní účet. Všechny vklady a výběry se zapisují do logu.

**Spoluhráči na mapě a jména nad hlavou** (nastavení v `Groups.json`)
- Server každé `PositionUpdateSeconds` (3 s) pošle členům skupiny polohy ostatních online členů.
- **Mapa** (`ShowMapMarkers`): spoluhráči jsou na mapě jako modré značky se jménem, i na druhém konci mapy.
  Vlastní značky z mapy zůstávají.
- **Jména nad hlavou** (`ShowNameTags`): jméno + vzdálenost (`NameTagShowDistance`) do `NameTagMaxDistance` (1000 m).
  Blízký spoluhráč – jméno přesně nad hlavou a hýbe se s ním. Vzdálený (mimo dosah sítě) – jméno v jeho poslední
  známé poloze. Daleko = průhlednější. Při otevřeném inventáři/mapě se jména skryjí.
- Vidí to jen členové stejné skupiny. Po odchodu ze skupiny značky zmizí.

**Bez zranění ve skupině** (`Groups.json` → `FriendlyFire: false`, výchozí)
- Členové stejné skupiny si navzájem neublíží střelbou, noži ani pěstmi – zásah se vůbec neprovede (ani krvácení).
- Výbuchy (granáty, miny) a přejetí autem se nepočítají jako útok hráče a zraňují dál.
- `FriendlyFire: true` = zranění mezi spoluhráči zapnout.

**Farmy** (`Farms.json`, data v `FarmsData.json`) – menu klávesa **U**
- Hráč si postaví farmu tam, kde stojí (`BuildPrice`, jedna na hráče, min. `MinDistance` od jiné farmy).
  Ve výchozím nastavení jen **ve své base** nebo base skupiny (`Bases.json` → `FarmRequiresBase`).
  Na mapě je zelená značka „Moje farma“.
- **Zvířata se kupují u Farmáře** (`AnimalTraders` v `Farms.json`, klávesa **B** u něj) a dovezou se na tvoji farmu.
  `AnimalsOnlyAtTrader: false` = jdou koupit i přímo na farmě.
- **Krmivo** (`FeedPrice` za kus, zásobník `MaxFeed`) jde koupit u Farmáře i na farmě (`FeedAtFarm: false` = jen u Farmáře).
- Zvířata nejsou ve světě – jsou jen v menu, takže nezmizí po restartu a nikdo je nezabije.
- Každé zvíře vyrábí jednou za `CycleMinutes` a spotřebuje `FeedPerCycle` krmiva. **Bez krmiva výroba stojí.**
  Výroba běží i když je majitel offline (jen když běží server).

  | Zvíře | Cena | Cyklus | Krmivo | Vyrábí |
  |---|---|---|---|---|
  | Slepice (max 20) | 300 | 30 min | 1 | vejce, peří (50 %), kuřecí maso (10 %) |
  | Prase (max 10) | 1 500 | 120 min | 3 | 2× vepřové, sádlo (50 %), kůže (5 %) |
  | Kráva (max 5) | 3 000 | 120 min | 4 | sušené mléko, 2× hovězí (30 %), kůže (5 %) |
  | Koza (max 10) | 1 200 | 90 min | 2 | sušené mléko (70 %), maso (40 %), kůže (5 %) |
  | Ovce (max 10) | 1 200 | 90 min | 2 | maso (50 %), kůže (10 %) |

- Výrobky jsou ve **skladu** farmy (max `MaxStoredPerProduct` od každé suroviny), vybírají se v menu
  (*Vybrat vybrané* / *Vybrat vše*, max 50 kusů najednou, co se nevejde, padne na zem).
- **Sad a skleník:** stavby na farmě (menu farmy → *Stavby na farmě*): **Sad** (4 000) a **Skleník** (6 000).
  Stromky a záhony se kupují u Farmáře (jako zvířata) a místo krmiva spotřebovávají **hnojivo**
  (`FertilizerPrice` za kus, zásobník `MaxFertilizer`). Bez hnojiva nerodí.

  | Rostlina | Stavba | Cena | Cyklus | Úroda |
  |---|---|---|---|---|
  | Jabloň | Sad | 400 | 120 min | 3 jablka |
  | Hrušeň | Sad | 400 | 120 min | 2 hrušky |
  | Švestka | Sad | 400 | 120 min | 3 švestky |
  | Rajčata | Skleník | 200 | 60 min | 2 rajčata |
  | Papriky | Skleník | 200 | 60 min | 2 papriky |
  | Cukety | Skleník | 250 | 90 min | 1 cuketa |
  | Dýně | Skleník | 300 | 120 min | 1 dýně |
  | Brambory | Skleník | 150 | 60 min | 2 brambory |
  | Konopí (max 5) | Skleník | 800 | 120 min | 1 konopí (prodej na černém trhu) |

  Stavby (`Buildings`) i rostliny (`Animals` s `"Resource": "fertilizer"` a `"RequiresBuilding"`) jdou v `Farms.json`
  přidávat a měnit – třeba další stromy nebo jiný skleník.
- `GroupCanUse: true` = členové skupiny majitele můžou krmit, kupovat zvířata a vybírat sklad.
- **Vejce** (`OS_ChickenEgg`) je nový předmět – jí se a vaří jako brambora a **zatím má i model brambory**.
  Model jde změnit v `config.cpp` (`model = "..."` u třídy `OS_ChickenEgg`, cestu najdeš na disku P: v DayZ Tools).
  Vejce jdou prodat v Potravinách. Mléko = vanilla sušené mléko.
- Druhy zvířat, ceny, cykly i výrobky (libovolné classname, i z módů) se nastavují v `Farms.json` → `Animals`.

**Airdropy** (`Airdrops.json`)
- Každých `IntervalMinutes` (45 min, náhodně ±25 %, jen když je online aspoň `MinPlayers`) spadne bedna
  (`ContainerClass`, výchozí `SeaChest`) na náhodné místo z `Locations` (náhodně v okruhu `Radius`).
  Padá pomalu z výšky `DropHeight` (150 m) rychlostí `FallSpeed` (6 m/s), jako na padáku.
- Při shození oznámení v chatu (`AnnounceInChat`) s názvem místa a čtvercem mapy, na mapě oranžová značka
  „Airdrop“ (`ShowOnMap`). Po dopadu červený kouř (`SmokeClass`) a `GuardCount` stráží – vojenští nakažení
  (`GuardClasses`).
- **Loot** (`LootTables`): náhodně jedna sada podle `Weight` – výchozí *Vojenský* (zbraň s optikou, zásobníky,
  munice, vesta, helma, granáty, noktovizor), *Zdravotnický*, *Zásoby*. Položka:
  `{ "Items": "M4A1|AKM", "Chance": 1, "Count": 1, "Attachments": ["ACOGOptic"] }` (`|` = náhodně jedno).
- Po `LifetimeMinutes` (40 min) od dopadu bedna zmizí i se zbylým lootem.
- Admin nástroje: *Airdrop u cíle*, *Airdrop náhodně*.

**Hlasový chat a skupinová vysílačka**
- Hlas přenáší **vanilla hlasový chat** DayZ (mluvení v okolí, šepot / normálně / křik a vysílačky) – vlastní
  zvukový kanál mód vytvořit nemůže, staví nad vanillou:
- **Skupinová vysílačka** (`Groups.json` → `GroupRadio`): každá skupina má vlastní kanál vysílačky
  (`RadioChannels`, kanál 0 zůstává veřejný). Menu skupiny (J) → *Naladit vysílačku skupiny* naladí všechny
  vysílačky (Personal Radio) v inventáři na kanál skupiny. `AutoTuneOnJoin` = naladí se samy po založení
  skupiny, vstupu do ní a po připojení na server. Vysílačku je potřeba zapnout (baterie) a pro vysílání držet v ruce.
- **Kdo mluví** (HUD, `Hud.json` → `ShowVoice`): seznam hráčů, kteří právě mluví – spoluhráči modře se vzdáleností
  a popiskem „skupina“, ostatní se vzdáleností, pokud jsou blízko.

**Únava a spánek** (`Sleep.json`, únava hráčů v `SleepData.json`)
- **Únava** (0–100 %, pruh v HUDu) roste o `FatiguePerMinute` (1 %) za minutu, při běhu ×`JogMultiplier` (1,5),
  při sprintu ×`SprintMultiplier` (2,5). Nová postava po smrti začíná odpočatá.
- Od `TiredAt` (70 %) je hráč **unavený** – výdrž nejvýš 60 %. Od `ExhaustedAt` (90 %) **vyčerpaný** – výdrž
  nejvýš 30 % a ubývá zdraví (`ExhaustedHealthLossPerSecond`). Při každém stupni přijde upozornění.
- **Spaní:** klávesa **L** → *Spát* (od `MinFatigueToSleep`, 20 %). Postava si lehne, obrazovka ztmavne a ukazuje
  únavu. Únava klesá o `SleepRecoveryPerMinute` (8 %) za minutu, ve **stanu** (do 4 m) nebo ve **vlastní base**
  ×`ShelterMultiplier` (2) a doplňuje se zdraví (`SleepHealthPerSecond`). Hráč se probudí pohybem, zraněním
  nebo když je vyspaný. Ve PvP zóně a s běžícím PvP časovačem usnout nejde (`NoSleepInPvP`).

**Emoce a animace**
- Klávesa **L** otevře menu emocí: spát, sednout, sednout do turecka, lehnout, sednout k ohni, mávat, salutovat,
  tleskat, palec nahoru/dolů, ukázat, pojď sem, jdeme, kývnout, zavrtět hlavou, pokrčit rameny, vzdát se, ticho,
  srdce, facepalm, posměšek, tanec (vanilla gesta, dvojklik = přehrát, pohyb emoci ukončí).
  Některé se oznámí v chatu hráčům do `EmoteAnnounceRadius` (30 m), např. „* Pepa mává“.
- **Animace u akcí módu:** při vaření si postava sedne k ohni (menu se zavře), při spánku si lehne;
  po vybrání z farmy palec nahoru, po vkladu / výběru v bankomatu a nákupu kývnutí, po založení a vylepšení base
  salutování, po přijetí do skupiny mávnutí (přehrají se po zavření menu).

**HUD** (`Hud.json`) – panel v pravém horním rohu (`Position`: `right` / `left`), hráč ho skryje klávesou **H**
- **Stav** (`ShowStatus`): zóna barevně – *BEZPEČNÁ ZÓNA* (zelená), *PvP ZÓNA* (červená), *PvP ještě X s*
  (oranžová, časovač po odchodu ze zóny), *PvE* (šedá); base, ve které stojíš (moje / skupiny / cizí);
  pruhy zdraví, krve, jídla, vody, výdrže a tepla (zima / chladno / OK / teplo / horko).
- **Skupina** (`ShowGroup`): spoluhráči online se vzdáleností (max 6).
- **Zdravotní stav** (`ShowDiseases`): nemoci, které se projevily, červeně s radou, čím je léčit
  (např. *Cholera → Tetracyklin, pít čistou vodu*), a zeleně léky, které právě působí.
  `ShowEarlyInfection: true` = ukáže i nákazu ještě před příznaky (výchozí vypnuto).
- **Info** (`ShowInfo`): souřadnice X / Z (`ShowCoordinates`), směr pohledu (S, SV, V…) a stupně,
  čas ve hře a počet hráčů online.
- Stav posílá server každých `UpdateSeconds` (1,5 s). Při otevřeném menu, inventáři nebo mapě se HUD skryje.
  Vanilla ikonky zůstávají. Vypnutí pro všechny: `Enabled: false`.

**Nemoci a léky ve hře** (vanilla DayZ – mód je nemění, jen je ukazuje v HUDu)

| Nemoc | Kde ji chytíš | Příznaky | Léčba |
|---|---|---|---|
| Cholera | Neupravená voda (rybník, řeka) | zvracení, žízeň | **Tetracyklin**, hodně pít čistou vodu |
| Salmonela | Syrové / shnilé maso, špinavé ruce | bolest břicha, zvracení | **Aktivní uhlí** |
| Otrava jídlem | Shnilé jídlo | zvracení | Aktivní uhlí, voda, počkat |
| Nachlazení → Chřipka → Zápal plic | Zima, mokré oblečení, nakažení hráči | kašel, kýchání | **Vitamíny**, teplo; chřipka a zápal plic **Tetracyklin** |
| Infekce rány | Neošetřená rána, špinavý obvaz | horečka, slabost | **Dezinfekční sprej / alkohol** na ránu, Tetracyklin |
| Kuru | Lidské maso | smích, třes | nevyléčitelné, časem přejde |
| Otrava těžkými kovy | Kontaminované prostředí a voda (hlavně Sakhal) | slabost | **Chelační tablety** |
| Chemická otrava | Toxická zóna (plyn) | kašel, zranění | **PO-X antidotum**, plynová maska |

- **Léky:** Tetracyklin (antibiotika, 5 min), Aktivní uhlí (salmonela, 5 min), Vitamíny (posílí imunitu),
  Léky proti bolesti / Morfin (bolest, zlomeniny), Adrenalin (probuzení z bezvědomí), Dezinfekce (rány),
  Chelační tablety (těžké kovy), PO-X (chemie), Fyziologický roztok (krev). Prodává je **Lékárna**.
- Prevence: pít převařenou nebo dezinfikovanou vodu (čisticí tablety, jód), vařit maso, mýt si ruce,
  být v suchu a teple, dezinfikovat obvazy.

**Dovednosti** (`Skills.json`, data hráčů v `Skills/<SteamID>.json`) – menu klávesa **O**
- **Zkušenosti** dostáváš za: zabitého zombíka (`XpZombieKill`, 10), zvíře (`XpAnimalKill`, 20), hráče
  (`XpPlayerKill`, 40), uvařené jídlo (`XpCookDish`, 15), každý vybraný kus z farmy (`XpFarmItem`, 1)
  a každou minutu naživu na serveru (`XpPerSurvivalMinute`, 2).
- **Úrovně:** na další úroveň je potřeba `XpFirstLevel + úroveň × XpLevelIncrease` (100, 150, 200…),
  maximálně `MaxLevel` (60). Za každou úroveň `PointsPerLevel` (1) bod.
- **Body** dáš v menu do dovedností (každá max. 10 úrovní, účinek za úroveň = `PerRank`):

  | Dovednost | Účinek za úroveň | Na maximu |
  |---|---|---|
  | Odolnost | −3 % zranění a ztráty krve | −30 % |
  | Výdrž | −5 % spotřeby výdrže | −50 % |
  | Regenerace | +0,6 zdraví za minutu | +6 / min |
  | Krev | +30 krve za minutu | +300 / min |
  | Metabolismus | −5 % úbytku jídla a vody | −50 % |
  | Otužilost | +12 % tepla za minutu | — |

- Vylepšení patří hráči (Steam ID) a **zůstává i po smrti**. *Přerozdělit body* vrátí všechny body
  (`ResetPrice`, výchozí 5 000; `0` = zdarma, `-1` = nejde).
- Dovednosti, jejich názvy, maximum a sílu jde změnit v `Skills` (účinek podle `Id`: `toughness`, `stamina`,
  `regen`, `blood`, `metabolism`, `warmth`).
- Admin nástroje: *Dát zkušenosti cíli* (počet napiš do pole).

**Vaření podle receptů** (`Cooking.json`)
- U **hořícího ohniště** nebo **zapnutého plynového vařiče** se podívej na oheň a dej akci **Vařit podle receptu**.
  Menu ukáže recepty (u těch, na které máš vše, je „[lze uvařit]“), suroviny, nádobí, čas a bonus.
- Suroviny a nádobí musíš mít u sebe (nebo hrnec/pánev na ohništi). Shnilé a spálené suroviny se nepočítají.
  Suroviny se spotřebují hned, jídlo je hotové po `CookSeconds` – **zůstaň u ohně**, jinak se spálí.
- Výchozí recepty:

  | Jídlo | Suroviny | Nádobí | Bonus |
  |---|---|---|---|
  | Guláš | 2× maso, brambora, paprika nebo rajče | hrnec | teplo 30 min |
  | Masová polévka | maso (i kuřecí), 2× brambora / cuketa / rajče | hrnec | zdraví 20 min |
  | Zeleninová polévka | 3× zelenina | hrnec | výdrž 15 min |
  | Rybí polévka | ryba nebo filet, brambora | hrnec | zdraví 20 min |
  | Houbová polévka | 2× houby (ne lysohlávky a muchomůrky), brambora | hrnec | teplo 20 min |
  | Kuře na paprice | kuřecí maso, paprika, sušené mléko | hrnec | zdraví 30 min |
  | Rizoto | rýže, maso nebo houby | hrnec | krev 15 min |
  | Ovocný kompot | 3× jablko / hruška / švestka | hrnec | výdrž 10 min |
  | Pečené maso s bramborami | maso, 2× brambora | pánev | krev 20 min |
  | Omeleta | 2× vejce z farmy, rajče nebo paprika | pánev | výdrž 20 min |

- **Hotová jídla** jsou nové předměty `OS_Dish_*` (zatím s modelem otevřené konzervy) – zasytí víc než suroviny
  a po snědení dají bonus. Bonus se nesčítá, nový ho přepíše. Síla bonusů: `RegenHealthPerSecond`,
  `RegenBloodPerSecond`, `WarmthPerSecond`, `StaminaPerSecond`.
- Recepty jdou upravovat a přidávat v `Recipes`: `Name`, `Result` (classname, i vanilla jídlo), `ResultCount`,
  `Tool` (`pot` / `pan` / `""`), `CookSeconds`, `Buff` (`regen` / `blood` / `warmth` / `stamina` / `""`), `BuffMinutes`
  a `Ingredients` – `{ "Items": "Potato|Tomato", "Count": 2 }`, kde `*text` = classname obsahuje text,
  `-text` = vyloučit. Co je hrnec a pánev: `PotItems`, `PanItems`.
- Hotová jídla prodává a vykupuje **Potraviny** (kategorie *Hotova jidla*, nákup 500, výkup 150).
- Názvy jídel a vejce ve hře jsou podle jazyka hry hráče (`stringtable.csv`).

**Base u vlajky** (`Bases.json`, data v `BasesData.json`)
- Postav vanilla vlajku (Territory Flag Kit) aspoň se stožárem (`RequirePole`), podívej se na ni a dej akci **Base**.
  V menu *Založit base* (`FoundPrice`), *Zvětšit poloměr* a *Zrušit base* (jen majitel, vlajka zůstane).
- Base patří zakladateli, spravovat a vylepšovat ji může i jeho skupina. `MaxBasesPerPlayer` (výchozí 1).
- **Poloměr podle úrovně** – `Levels`: `[{ "Radius": 30, "Price": 0 }, { "Radius": 50, "Price": 5000 }, ...]`
  (první úroveň dostaneš založením, každá další se kupuje).
- **Rozestupy:** vlajky dvou basí musí být aspoň `MinDistanceBetweenBases` (150 m) od sebe a jejich kruhy se
  nesmí překrývat (mezera `GapBetweenBases`, 20 m) – kontroluje se při založení i při zvětšování.
- `FarmRequiresBase: true` = farmu jde postavit jen stojíš-li ve své base (nebo base skupiny).
- Vlastní base a base skupiny jsou na mapě modrým kruhem (`ShowOnMap`, barva `MapColor`).
- Když vlajka zmizí (zbouraná, rozebraná), base do minuty zanikne.
- Admin nástroje: *Smazat base* (nejbližší do 300 m od cíle).

**Kódový zámek** – používá vanilla **DigitalCodeLock** (DayZ 1.30): dá se nasadit na bránu plotu i na dveře,
potřebuje 9V baterii a kód se zadává v jeho menu. Prodává se v kategorii *Stavba základny* (Železářství), baterie v *Elektronice*.
- Kdo kód nastaví jako první, je **majitel**. Majitel a jeho skupina mají u zámku akce **Odemknout (skupina)**
  a **Zamknout (skupina)** – bez zadávání kódu (zamknout jde jen zavřená brána). Ostatní musí znát kód.
- Admin nástroje: *Odemknout zámek* a *Smazat kód* (nejbližší zámek do 5 m od cíle; po smazání bude majitelem ten,
  kdo nastaví nový kód).
- Majitel se ukládá do zámku. Zámky nasazené před instalací módu majitele nemají (vyřešíš smazáním kódu adminem).

**PvP zóny a mapa** (`PvP.json`)
- **Mapa:** klávesa **M** (dá se změnit v Nastavení → Ovládání → OS Core) otevře mapu i bez mapy v inventáři,
  stejnou klávesou nebo Esc se zavře. Vypnutí: `MapWithoutItem: false` (pak jen s mapou jako ve vanille).
- Na mapě jsou **PvP zóny červeně** a **bezpečné zóny zeleně** (kruh + název). Barvy: `PvPColor`,
  `SafeColor` jako `"R G B"`. Skrytí: `ShowPvPZonesOnMap` / `ShowSafeZonesOnMap: false`.
- `PvPOnlyInZones: true` = hráč může zranit hráče **jen v PvP zóně**, jinde je server PvE (zombíci,
  zvířata, pád… zraňují dál). `false` = PvP všude a zóny jsou jen vyznačené na mapě.
- `BothPlayersInZone: true` = útočník i oběť musí stát v zóně (nejde střílet do zóny zvenku ani ven).
- **Časovač po odchodu:** `LeaveTimerSeconds` (výchozí 30). Kdo vyběhne ze zóny, je ještě tolik sekund
  „v PvP“ – může střílet a jde po něm střílet (i ten, kdo za ním vyběhl, protože i jemu běží časovač).
  Hráč dostane hlášku `TimerMessage` (`%1` = sekundy) a po vypršení `LeaveMessage`. Návrat do zóny časovač zruší,
  `0` = PvP končí hned na hranici zóny.
- Při vstupu a odchodu přijde hláška (`EnterMessage`, `LeaveMessage`, vypnutí `NotifyOnEnter: false`).
- Zóna: `{ "Name": "Letiste", "X": 4500, "Z": 9500, "Radius": 600 }`.
- **Ve hře (admin menu, Home):** do pole *Částka / souřadnice / poloměr* napiš `poloměr [název]`
  (např. `400 Letiste`) a klikni **PvP zóna zde** – zóna vznikne kolem tebe, uloží se do `PvP.json`
  a hráčům se hned ukáže na mapě. **Smazat PvP zónu** smaže tu nejbližší.
- Bezpečná zóna má přednost (v ní je hráč nesmrtelný), skupiny bez friendly fire si neublíží ani v PvP zóně.

**Admin nástroje (jako Community Online Tools)** – admin menu (Home) → **Nástroje (COT)**.
Akce se týkají hráče vybraného v seznamu admin menu (bez výběru tebe). Vše ověřuje server podle `Admins.json`.
- **Teleport z mapy:** otevři mapu (M) a **dvojklikni** – vybraný hráč (nebo ty) se tam přenese.
- **Spawner:** hledání ve všech předmětech a vozidlech ve hře (vanilla i módy, podle názvu nebo classname),
  *Do inventáře cíle* (max 20 ks), *Na zem k cíli* (max 50 ks), *Spawnout vozidlo* (před cíl, s díly
  podle `Vehicles.json`, plné nádrže, bez majitele).
- **Čas:** 6:00 / 12:00 / 18:00 / 0:00 nebo vlastní `hh:mm`. **Počasí:** jasno, zataženo, déšť, bouřka, mlha
  (změna do 30 s, drží 30 minut, pak se počasí vrací k normálnímu vývoji).
- **ESP:** jména a vzdálenost hráčů do 1500 m přes zdi (vidíš jen hráče v dosahu sítě serveru).
- **Neviditelnost:** admin není vidět ostatním hráčům (zapni si k tomu i godmode).
- **Freecam:** klávesa **End** (změna v Ovládání → OS Core) – WASD, myš, Q/E dolů/nahoru, Shift rychle.
  **Shift + End** freecam vypne a teleportuje tě na místo kamery. Hra načítá okolí jen kolem postavy,
  takže daleko od ní kamera neukáže hráče ani předměty.
- **Vozidlo do 15 m od cíle:** opravit (vše včetně dílů a kapalin), zničit, smazat.
- **Hráč:** *Vysvléknout* (odebere všechny věci), *Kick*, *Ban* (důvod z pole, Steam ID se uloží do
  `Admins.json` → `BannedSteamIds`, zprávu změníš v `BanMessage`), *Unban* (Steam ID napiš do pole).
- **Přidat / Odebrat admina:** vyber hráče v seznamu online hráčů v admin menu (admini mají `[ADMIN]`),
  nebo napiš do pole jeho Steam64 ID (funguje i pro hráče offline). Uloží se do `Admins.json`
  a platí hned, bez restartu. Sám sebe odebrat nejde. Prvního admina je pořád potřeba zapsat do `Admins.json` ručně.
- **Hlavní admin:** Steam64 ID dej do `HeadAdminSteamIds` v `Admins.json` (je admin i bez zápisu v `AdminSteamIds`).
  Ve hře ho nikdo neodebere, nevyhodí, nezabanuje, nezabije ani nevysvlékne – kromě jiného hlavního admina.
  V seznamu má `[HLAVNI ADMIN]`. `OnlyHeadAdminsManageAdmins: true` = adminy ve hře přidává a odebírá jen hlavní admin.
  ```json
  { "AdminSteamIds": [], "HeadAdminSteamIds": ["76561198000000000"], "OnlyHeadAdminsManageAdmins": false }
  ```

### Doplněk pro Expansion AI (OS_Core_ExpansionAI)
Vojáky se zbraněmi (chůze, míření, krytí) dělá **DayZ Expansion AI** – vanilla DayZ žádné lidské AI nemá.
Doplněk `OS_Core_ExpansionAI` (vlastní PBO, nahrávej **jen s Expansion AI**) opravuje to, co na Expansion AI
nejvíc vadí, a řídí vojáky z OS_Core. Nastavení `$profile:OS_Core/ExpansionAI.json`:
- **Přes zdi nevidí a nestřílí** (`StrictLineOfSight`): k výhledu Expansion se přidá vlastní kontrola paprsky
  z očí AI na hlavu, hrudník a pánev hráče. Když je ve všech směrech zeď, budova, stěna základny, skála nebo auto,
  AI hráče nevidí a nestřílí. Stromy a keře posuzuje Expansion.
- **Nevidí na kilometr:** ve dne max. `DayVisionMax` (350 m), v noci `NightVisionMax` (60 m), v noci se svítilnou
  `NightLitVisionMax` (150 m). Přikrčený hráč je vidět na `CrouchVisionFactor` (70 %), ležící na `ProneVisionFactor` (40 %).
- **Přesnost se vzdáleností:** na `AccuracyFalloffDistance` (300 m) poloviční, dál ještě horší.
- **Hlídky** (`Patrols`): skupina vojáků (`Faction` – West, East, Raiders, Mercenaries, Guards; `Count`, `Loadout`)
  chodí po bodech `Waypoints` (`Behaviour`: LOOP dokola, ALTERNATE tam a zpět, HALT = stojí na místě jako stráž).
  Objeví se, když je hráč blíž než `PatrolSpawnDistance` (600 m), a po odchodu hráčů zmizí; po zničení se obnoví
  za `RespawnMinutes`.
- **Útoky** (`Attacks`): každých ~`AttackIntervalMinutes` (60 min) se šancí `AttackChance` přijde
  `AttackCountMin`–`AttackCountMax` banditů (`AttackFaction`) z `AttackSpawnDistance` (300 m) k náhodnému hráči
  (`AttackOnlyBases` = jen k hráči ve své base; ne v bezpečné zóně). Hráč a jeho skupina dostanou varování.
- **Stráže airdropů** (`AirdropGuards`): místo nakažených hlídají airdrop vojáci (`AirdropGuardFaction`, výchozí Guards)
  a zmizí s bednou.
- **Konvoje** (`Convoys.json`): každých ~`IntervalMinutes` (50 min) vyjede vozidlo (`Vehicles`, náhodně jedno)
  s posádkou `Crew` vojáků (`Faction`) po trase z `Routes` (body po silnici, aspoň každých 50–100 m, v zatáčkách
  hustěji) rychlostí `SpeedKmh` (35 km/h). V kufru má loot z tabulky airdropů (`LootTable`, `LootRolls`×).
  Oznámení v chatu a poloha na mapě (`ShowOnMap`).
  - **Přepadení:** výstřel do auta, zabitý voják nebo hráč blíž než `AmbushDistance` (35 m) – konvoj zastaví,
    vojáci vystoupí a bojují. Kufr jde vybrat, až padnou.
  - **Útok:** projíždí-li konvoj do `AttackBaseRadius` (250 m) od hráče v jeho base, zastaví a vojáci jdou na base.
  - **Hybridní jízda** (`RealDriving`): když je hráč blíž než `PhysicsDistance` (400 m), auto **opravdu řídí voják**
    za volantem – vlastní řidič OS drží trasu (míří na bod před autem), do zatáček zpomaluje až na `MinCornerSpeedKmh`,
    řadí (manuál podle otáček), brzdí před autem / člověkem / zdí a po `StuckSeconds` zaseknutí couvne.
    Když nikdo blízko není, posouvá se po trase skriptem. Zaseknuté (`RecoverSeconds`) nebo převrácené auto se vrátí
    na trasu, jen když nikdo není blíž než `UnseenDistance` (200 m). Zastřelený řidič = konvoj zastaví.
    Po `LifetimeMinutes` od zastavení nebo dojetí konvoj (auto i zbylí vojáci) zmizí.
  - Admin nástroje: *Vyslat konvoj*.
- Přesnost, poškození a další chování dál nastavuješ v nastavení Expansion AI (`AISettings.json`).

### Doplněk pro Expansion BaseBuilding (OS_Core_Expansion)
Mód má vlastní systém base na **vanilla vlajce** (viz *Base u vlajky*), který Expansion nepotřebuje.
Pokud ale na serveru používáš **Expansion BaseBuilding** (stavění jen u územní vlajky Expansion), je tu
samostatný doplněk **`OS_Core_Expansion`**, který propojí OS skupiny s územím Expansion.
Zabal ho do vlastního PBO (prefix `OS_Core_Expansion`) a dej ho do stejného `@OS_Core\Addons`
**jen pokud na serveru máš Expansion BaseBuilding**. Bez Expansionu ho nenahrávej, jinak se server nespustí.

- Kdo je v OS skupině s **majitelem vlajky**, je automaticky **členem jeho území**. Může tam stavět,
  rozebírat a používat garáž území. Funguje to pro všechny vlajky všech členů skupiny.
  Vlastník a admini skupiny jsou v území **moderátoři**, běžní členové **členové**.
- Kdo ze skupiny odejde nebo je vyhozen, je z území do pár sekund odebrán. Odebírá se jen ten,
  koho přidal tenhle doplněk; hráče pozvané ručně přes menu vlajky to nechá být.
- Nastavení `$profile:OS_Core/ExpansionTerritory.json`: `Enabled`, `SyncSeconds` (jak často
  se kontroluje, výchozí 10 s), `RemoveWhenLeftGroup`, `NotifyPlayers`.
- V nastavení Expansion `TerritorySettings.json` dej **`OnlyInviteGroupMember: 0`**. Jinak Expansion
  chce k založení vlajky svoji vlastní skupinu (party) a s naší skupinou by vlajku nešlo založit.
- Kódové zámky Expansion dál fungují na kód. Doplněk mění jen členství v území.
- Hráč odebraný ze skupiny může do odpojení vidět území ve svém menu, server mu ale stavění už nepovolí.

### Souřadnice
- Interaktivní mapa (např. iZurvive): zobrazuje `X / Y` → v konfigu `X` a `Z`.
- Ve hře: admin menu → pole „Částka / souřadnice / poloměr“ → *Teleport na X Z* (ověříš si, kam bod vede),
  nebo v mapě (M) dvojklik jako admin.

### Testování a ladění
Mód je napsaný proti vanilla skriptům **DayZ 1.30**, ale **nebyl zatím spuštěn ve hře**.
Po prvním startu zkontroluj:
- `script_*.log` na serveru i klientovi – chyby kompilace / `[OS_Core]` hlášky,
- v Addon Builderu případné chyby při binarizaci configu.

Chybu z logu mi pošli a opravím ji.

### Přehled přidaných a upravených funkcí
- **Obchod:** automatický katalog všech předmětů (vanilla + módy), úprava cen a násobení kategorií v menu,
  hledání, 15 specializovaných obchodníků (Tržiště, Zbrojíř, Oblečení, Potraviny, Černý trh s násobiteli cen,
  Lékárna, Železářství, Zahradnictví, Výkupna úrody, Rybář, Řezník, Lovec, Autodíly), volby `PlayersCanBuy`,
  `PlayersCanSell`, `Items`, `FarmProduce`; vojenské věci rozdělené mezi oblečení a zbraně, granáty u munice,
  nábytek, stany a elektronika ve stavbě, alkohol v jídle, drogy na černém trhu.
- **Peníze:** bankovky jako předmět (1–5000, realistický potisk s „OS CORE“), měna podle nastavení (EUR výchozí),
  bankomaty s vkladem, výběrem, převody hráčům (i offline) a účtem skupiny.
- **Vozidla:** autobazar, motorky, helikoptéry a lodě (Expansion + vanilla člun), servis, veřejné, osobní
  a skupinové garáže, vlastnictví bez klíčů (F), nastavení místa spawnu ve hře obdélníkem.
- **Skupiny:** až 4 zdarma + dokupování míst, společný účet, role vlastník / admin / člen (admin smí vše kromě
  vyhození vlastníka, předání vlastnictví a rozpuštění), spoluhráči na mapě, jména nad hlavou, bez friendly fire.
- **Farmy:** zvířata, sad a skleník, krmivo a hnojivo, vejce, výroba i offline, Farmář a Výkupna;
  nově jen ve vlastní base.
- **Airdropy:** bedna na padáku s lootem, kouř, stráže, oznámení a mapa, ruční shození adminem.
- **AI vojáci (Expansion AI):** nestřílí přes zdi, vidí jen rozumně daleko, hlídky, útoky na base, stráže airdropů, konvoje s nákladem k přepadení.
- **Hlasový chat:** skupinová vysílačka (vlastní kanál skupiny, automatické naladění) a v HUDu kdo právě mluví.
- **Únava, spánek a emoce:** únava v HUDu, spaní ve stanu / base, menu emocí (L), animace u akcí módu.
- **HUD:** stav hráče, nemoci s radou, čím léčit, působící léky, zóna a PvP odpočet, base, skupina, souřadnice, směr a čas (klávesa H).
- **Dovednosti:** zkušenosti a úrovně, body do odolnosti, výdrže, regenerace, krve, metabolismu a otužilosti (klávesa O).
- **Vaření:** recepty u ohně, 10 hotových jídel s lepší výživou a bonusy, prodej v Potravinách.
- **Base u vanilla vlajky:** založení, zvětšování poloměru po úrovních, rozestupy mezi basemi, zobrazení na mapě.
- **Kódový zámek:** vanilla DigitalCodeLock rozšířený o majitele, odemykání/zamykání skupinou bez kódu a admin odemknutí.
- **PvP:** PvP jen v PvP zónách, časovač po odchodu ze zóny, zóny a safe zóny na mapě, mapa bez předmětu (M).
- **Admin:** admin menu + nástroje jako Community Online Tools (spawner, teleport z mapy, čas, počasí, ESP,
  neviditelnost, freecam, auta, zámky, base, vysvlečení, kick, ban/unban), přidávání adminů ve hře, hlavní admin.
- **Jazyky:** čeština, angličtina, němčina, polština, vlastní jazyk a úprava všech textů v `Lang/*.json`.
- **Expansion BaseBuilding:** doplněk `OS_Core_Expansion` (skupina = členové území).

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
