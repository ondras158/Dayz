#!/usr/bin/env python3
"""Hromadná úprava spawnování v centrální ekonomice DayZ (types.xml a events.xml).

Nastavení je v economy/economy_tuning.json. Původní soubory se nepřepisují -
výsledek se uloží vedle s příponou .tuned.xml (nebo do --out-dir).

Příklady:
    python3 tools/tune_economy.py --types db/types.xml --events db/events.xml
    python3 tools/tune_economy.py --types db/types.xml --dry-run     # jen vypíše změny

Pořadí pravidel pro types.xml (použije se první, které sedí):
    1. "overrides"          - přesné hodnoty pro konkrétní classname
    2. "patternMultipliers" - násobitel podle regulárního výrazu jména
    3. "categoryMultipliers"- násobitel podle <category name="...">
Násobí se <nominal> a <min> (min nikdy nepřesáhne nominal).

events.xml: "vehicleMultiplier" násobí nominal/min/max všech eventů "Vehicle*",
"vehicleOverrides" nastaví přesné hodnoty konkrétnímu eventu.
"""
import argparse
import json
import os
import re
import sys
import xml.etree.ElementTree as ET

DEFAULT_CONFIG = os.path.join(os.path.dirname(__file__), "..", "economy", "economy_tuning.json")


def scaled(value, mult):
    if value <= 0:
        return value
    return max(1, int(value * mult + 0.5))


def get_int(el, tag):
    node = el.find(tag)
    if node is None or node.text is None:
        return None
    try:
        return int(node.text.strip())
    except ValueError:
        return None


def set_int(el, tag, value):
    node = el.find(tag)
    if node is not None:
        node.text = str(value)


def tune_types(tree, cfg, log):
    tcfg = cfg.get("types", {})
    overrides = {k.lower(): v for k, v in tcfg.get("overrides", {}).items() if not k.startswith("_")}
    patterns = [(re.compile(p, re.I), m) for p, m in tcfg.get("patternMultipliers", {}).items() if not p.startswith("_")]
    categories = {k.lower(): v for k, v in tcfg.get("categoryMultipliers", {}).items() if not k.startswith("_")}
    changed = 0

    for t in tree.getroot().findall("type"):
        name = t.get("name", "")
        nominal, minimum = get_int(t, "nominal"), get_int(t, "min")
        if nominal is None or minimum is None:
            continue

        new_nom, new_min, rule = nominal, minimum, None
        if name.lower() in overrides:
            o = overrides[name.lower()]
            new_nom = int(o.get("nominal", nominal))
            new_min = int(o.get("min", minimum))
            rule = "override"
        else:
            mult = None
            for rx, m in patterns:
                if rx.search(name):
                    mult, rule = m, f"vzor {rx.pattern}"
                    break
            if mult is None:
                cat = t.find("category")
                cat_name = (cat.get("name", "") if cat is not None else "").lower()
                if cat_name in categories:
                    mult, rule = categories[cat_name], f"kategorie {cat_name}"
            if mult is None or mult == 1:
                continue
            new_nom, new_min = scaled(nominal, mult), scaled(minimum, mult)

        new_min = min(new_min, new_nom)
        if (new_nom, new_min) != (nominal, minimum):
            set_int(t, "nominal", new_nom)
            set_int(t, "min", new_min)
            changed += 1
            log.append(f"  {name:32} nominal {nominal:>3} -> {new_nom:<3} min {minimum:>3} -> {new_min:<3} ({rule})")
    return changed


def tune_events(tree, cfg, log):
    ecfg = cfg.get("events", {})
    mult = float(ecfg.get("vehicleMultiplier", 1))
    overrides = {k: v for k, v in ecfg.get("vehicleOverrides", {}).items() if not k.startswith("_")}
    changed = 0

    for ev in tree.getroot().findall("event"):
        name = ev.get("name", "")
        if not name.startswith("Vehicle") and name not in overrides:
            continue
        old = {k: get_int(ev, k) for k in ("nominal", "min", "max")}
        if None in old.values():
            continue

        if name in overrides:
            new = {k: int(overrides[name].get(k, old[k])) for k in old}
            rule = "override"
        elif mult != 1:
            new = {k: scaled(v, mult) for k, v in old.items()}
            rule = f"x{mult}"
        else:
            continue

        new["min"] = min(new["min"], new["nominal"])
        new["max"] = max(new["max"], new["nominal"])
        if new != old:
            for k, v in new.items():
                set_int(ev, k, v)
            changed += 1
            log.append(f"  {name:32} nominal {old['nominal']}->{new['nominal']}  min {old['min']}->{new['min']}  max {old['max']}->{new['max']} ({rule})")
    return changed


def write(tree, src, out_dir):
    base = os.path.basename(src)
    out = os.path.join(out_dir or os.path.dirname(src), base.replace(".xml", ".tuned.xml"))
    ET.indent(tree, space="    ")
    tree.write(out, encoding="UTF-8", xml_declaration=True)
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--types", help="cesta k types.xml")
    ap.add_argument("--events", help="cesta k events.xml")
    ap.add_argument("--config", default=DEFAULT_CONFIG)
    ap.add_argument("--out-dir", help="kam uložit výsledek (výchozí: vedle originálu)")
    ap.add_argument("--dry-run", action="store_true", help="jen vypsat změny")
    a = ap.parse_args()

    if not a.types and not a.events:
        ap.error("zadej --types a/nebo --events")

    with open(a.config, encoding="utf-8") as f:
        cfg = json.load(f)

    for path, fn, label in ((a.types, tune_types, "types.xml"), (a.events, tune_events, "events.xml")):
        if not path:
            continue
        try:
            tree = ET.parse(path, parser=ET.XMLParser(target=ET.TreeBuilder(insert_comments=True)))
        except (ET.ParseError, OSError) as e:
            sys.exit(f"Chyba při čtení {path}: {e}")
        log = []
        n = fn(tree, cfg, log)
        print(f"{label}: změněno {n} položek")
        print("\n".join(log))
        if not a.dry_run and n:
            print("Uloženo:", write(tree, path, a.out_dir))


if __name__ == "__main__":
    main()
