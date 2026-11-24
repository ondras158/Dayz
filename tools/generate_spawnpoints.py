#!/usr/bin/env python3
"""Generuje cfgplayerspawnpoints.xml pro DayZ ze souboru CSV.

CSV formát (oddělovač čárka, řádky začínající # se ignorují):
    skupina,x,z
    Mesto1,1234.5,6789.0

  skupina  - název skupiny (body se stejnou skupinou tvoří jednu spawn oblast)
  x        - souřadnice X (západ -> východ)
  z        - souřadnice Z (jih -> sever; na mapách jako iZurvive je to "Y")

Použití:
    python3 tools/generate_spawnpoints.py spawnpoints/example.csv \
        <mise>/cfgplayerspawnpoints.xml
"""
import argparse
import csv
import sys
from collections import OrderedDict
from xml.sax.saxutils import quoteattr

SECTIONS = {
    # sekce: (min_dist_infected, max_dist_infected, min_dist_player, max_dist_player)
    "fresh": (30, 70, 65, 150),
    "hop": (25, 70, 40, 150),
    "travel": (10, 100, 10, 150),
}


def load(path):
    groups = OrderedDict()
    with open(path, newline="", encoding="utf-8") as f:
        rows = (r for r in csv.reader(f) if r and not r[0].strip().startswith("#"))
        for lineno, row in enumerate(rows, 1):
            if len(row) != 3:
                sys.exit(f"Chyba: řádek {lineno} má {len(row)} sloupců, čekám 3 (skupina,x,z): {row}")
            name, x, z = (c.strip() for c in row)
            if name.lower() == "skupina":  # hlavička
                continue
            try:
                x, z = float(x), float(z)
            except ValueError:
                sys.exit(f"Chyba: neplatné souřadnice na řádku {lineno}: {row}")
            groups.setdefault(name, []).append((x, z))
    if not groups:
        sys.exit("Chyba: CSV neobsahuje žádné body.")
    return groups


def section(tag, params, groups, enable_groups):
    mi, ma, mp, mxp = params
    out = [f"    <{tag}>",
           "        <spawn_params>",
           f"            <min_dist_infected>{mi}</min_dist_infected>",
           f"            <max_dist_infected>{ma}</max_dist_infected>",
           f"            <min_dist_player>{mp}</min_dist_player>",
           f"            <max_dist_player>{mxp}</max_dist_player>",
           "            <min_dist_static>0</min_dist_static>",
           "            <max_dist_static>2</max_dist_static>",
           "        </spawn_params>",
           "        <generator_params>",
           "            <grid_density>4</grid_density>",
           "            <grid_width>200</grid_width>",
           "            <grid_height>200</grid_height>",
           "            <min_dist_static>0</min_dist_static>",
           "            <max_dist_static>2</max_dist_static>",
           "            <min_steepness>-45</min_steepness>",
           "            <max_steepness>45</max_steepness>",
           "        </generator_params>",
           "        <group_params>",
           f"            <enablegroups>{'true' if enable_groups else 'false'}</enablegroups>",
           "            <groups_as_regular>true</groups_as_regular>",
           "            <lifetime>240</lifetime>",
           "            <counter>-1</counter>",
           "        </group_params>",
           "        <generator_posbubbles>"]
    for name, points in groups.items():
        out.append(f"            <group name={quoteattr(name)}>")
        for x, z in points:
            out.append(f'                <pos x="{x:.6f}" z="{z:.6f}" />')
        out.append("            </group>")
    out += ["        </generator_posbubbles>", f"    </{tag}>"]
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv")
    ap.add_argument("output")
    ap.add_argument("--no-groups", action="store_true",
                    help="nastaví enablegroups=false (všechny body v jednom společném poolu)")
    a = ap.parse_args()

    groups = load(a.csv)
    lines = ['<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>',
             "<!-- Vygenerováno nástrojem tools/generate_spawnpoints.py - upravuj CSV, ne tento soubor -->",
             "<playerspawnpoints>"]
    for tag, params in SECTIONS.items():
        lines += section(tag, params, groups, not a.no_groups)
    lines.append("</playerspawnpoints>")

    with open(a.output, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    total = sum(len(p) for p in groups.values())
    print(f"OK: {len(groups)} skupin, {total} bodů -> {a.output}")


if __name__ == "__main__":
    main()
