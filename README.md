# DayZ módy a konfigurace

## Spawn lokace – Nasdara (DayZ Badlands)

Mise: `dayzOffline.nasdara` (ověř název ve složce `mpmissions` na serveru).

### Struktura
| Soubor | Co to je |
|---|---|
| `nasdara-spawns/spawnpoints.csv` | **Tady upravuješ body** (skupina, x, z) |
| `tools/generate_spawnpoints.py` | Převede CSV na XML |
| `mpmissions/dayzOffline.nasdara/cfgplayerspawnpoints.xml` | Hotový soubor pro server |

> ⚠️ Aktuální CSV obsahuje jen **ukázkové souřadnice**, ne skutečná místa na Nasdaře.
> Před nasazením je nahraď.

### 1. Zjištění souřadnic
- **Interaktivní mapa** (např. iZurvive, pokud má Nasdaru): klikni na místo a opiš souřadnice.
  Mapa ukazuje `X / Y` → do CSV piš `x = X`, `z = Y`.
- **Ve hře**: admin nástroj (VPP Admin Tools, COT) ukáže pozici hráče `X, Y(výška), Z` – výšku vynech.
- Vybírej rovný terén mimo budovy, vodu a skály (generátor odmítá svah nad 45°).

### 2. Vygenerování XML
```bash
python3 tools/generate_spawnpoints.py nasdara-spawns/spawnpoints.csv \
    mpmissions/dayzOffline.nasdara/cfgplayerspawnpoints.xml
```
Volba `--no-groups` nastaví `enablegroups=false`. Zkus ji, pokud se hráči spawnují na `0,0,0`.
Tuhle chybu u Nasdary zmiňuje jeden návod pro hostingy, ale nemám ji ověřenou.

### 3. Nasazení
1. Zastav server.
2. Zálohuj původní `mpmissions/dayzOffline.nasdara/cfgplayerspawnpoints.xml`.
3. Nahraj nový soubor do **kořene mise** (vedle `init.c`, ne do `db/`).
4. Spusť server a zkontroluj RPT log, jestli neobsahuje chyby parsování XML.

### Sekce v XML
- `fresh` – nová postava (po smrti / první připojení)
- `hop` – hráč přešel z jiného serveru
- `travel` – přechod mezi servery stejné „skupiny“

Všechny sekce teď používají stejné body. Vzdálenosti od zombíků a hráčů jsou ve funkci `SECTIONS` ve skriptu.
