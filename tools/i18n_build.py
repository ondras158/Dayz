#!/usr/bin/env python3
"""Překlady OS_Core: posbírá texty ze skriptů a z překladů v lang/*.json vygeneruje OS_LangData.c.

Použití:  python3 tools/i18n_build.py

  - klíče = české texty z OS_T("...") / OS_F("...") ve skriptech, z layoutů (OS_LayoutTexts.c)
    a výchozí názvy z configů (kategorie, obchodníci, zóny, zvířata, zprávy...),
  - lang/cs.json se přepíše seznamem všech klíčů (hodnota = klíč),
  - lang/en.json, de.json, pl.json: chybějící klíče se doplní s prázdnou hodnotou a vypíšou,
    nepoužívané klíče se odstraní,
  - výstup OS_Core/Scripts/3_Game/OS_Core/OS_LangData.c (vestavěné texty v PBO).
Jazyky na serveru pak jdou upravit v $profile:OS_Core/Lang/<jazyk>.json bez přebalení.
"""
import json
import os
import re

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
LANG_DIR = os.path.join(ROOT, "lang")
OUT = os.path.join(ROOT, "OS_Core", "Scripts", "3_Game", "OS_Core", "OS_LangData.c")
SCRIPT_DIRS = [os.path.join(ROOT, "OS_Core", "Scripts"), os.path.join(ROOT, "OS_Core_Expansion", "Scripts")]
LANGS = ["cs", "en", "de", "pl"]

STR = r'"((?:\\.|[^"\\])*)"'
CODE_PATTERNS = [re.compile(r"OS_[TF]\(" + STR)]
LAYOUT_PATTERN = re.compile(r"Set\(root, " + STR + ", " + STR + r"\)")
CONFIG_PATTERNS = [
    re.compile(r"new OS_CategoryRule\(" + STR),
    re.compile(r"new OS_TraderLocation\(" + STR),
    re.compile(r"new OS_SpawnZone\(" + STR),
    re.compile(r"new OS_SafeZone\(" + STR),
    re.compile(r"new OS_PvPZone\(" + STR),
    re.compile(r"new OS_CookRecipe\(" + STR),
    re.compile(r"new OS_VehicleLocation\(" + STR),
    re.compile(r"new OS_FarmAnimalType\(" + STR + ", " + STR),
    re.compile(r"new OS_FarmBuilding\(" + STR + ", " + STR),
    re.compile(r"AddPlant\(" + STR + ", " + STR),
    re.compile(r"Broadcasts\.Insert\(" + STR),
    re.compile(r"string (?:WelcomeTitle|WelcomeText|EnterMessage|LeaveMessage|TimerMessage|BanMessage) = " + STR),
]
CONFIG_FILES = ["OS_Config.c", "OS_VehicleConfig.c", "OS_FarmConfig.c", "OS_PvPConfig.c", "OS_CookingConfig.c"]


def unescape(s):
    return s.replace('\\"', '"').replace("\\\\", "\\")


def escape(s):
    return s.replace("\\", "\\\\").replace('"', '\\"')


def collect_keys():
    keys = []

    def add(k):
        k = unescape(k)
        if k and re.search(r"[A-Za-zÀ-ž]", k) and k not in keys:
            keys.append(k)

    for base in SCRIPT_DIRS:
        for dirpath, _, files in sorted(os.walk(base)):
            for name in sorted(files):
                if not name.endswith(".c") or name == "OS_LangData.c":
                    continue
                text = open(os.path.join(dirpath, name), encoding="utf-8").read()
                for p in CODE_PATTERNS:
                    for m in p.finditer(text):
                        add(m.group(1))
                if name == "OS_LayoutTexts.c":
                    for m in LAYOUT_PATTERN.finditer(text):
                        add(m.group(2))
                if name in CONFIG_FILES:
                    for p in CONFIG_PATTERNS:
                        for m in p.finditer(text):
                            add(m.group(m.lastindex))   # poslední skupina = zobrazovaný název
    return keys


def load(lang):
    path = os.path.join(LANG_DIR, lang + ".json")
    if not os.path.exists(path):
        return {}
    return json.load(open(path, encoding="utf-8"))


def save(lang, data):
    os.makedirs(LANG_DIR, exist_ok=True)
    with open(os.path.join(LANG_DIR, lang + ".json"), "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=1)
        f.write("\n")


def write_data(keys, tables):
    lines = ["// VYGENEROVÁNO nástrojem tools/i18n_build.py z lang/*.json - neupravuj ručně.",
             "// Vestavěné texty; na serveru se dají přepsat v $profile:OS_Core/Lang/<jazyk>.json.",
             "",
             "class OS_LangData",
             "{",
             "\tstatic array<string> Languages()",
             "\t{",
             "\t\tarray<string> langs = {" + ", ".join(f'"{l}"' for l in LANGS) + "};",
             "\t\treturn langs;",
             "\t}",
             "",
             "\tstatic void Fill(string lang, map<string, string> m)",
             "\t{"]
    chunk = 150
    funcs = []
    for lang in LANGS:
        table = tables[lang]
        pairs = [(k, table.get(k, "")) for k in keys if table.get(k, "")]
        parts = [pairs[i:i + chunk] for i in range(0, len(pairs), chunk)] or [[]]
        names = []
        for idx, part in enumerate(parts):
            fname = f"Fill_{lang}_{idx}"
            names.append(fname)
            body = [f"\tprotected static void {fname}(map<string, string> m)", "\t{"]
            for k, v in part:
                body.append(f'\t\tm.Set("{escape(k)}", "{escape(v)}");')
            body.append("\t}")
            funcs.append("\n".join(body))
        lines.append(f'\t\tif (lang == "{lang}")')
        lines.append("\t\t{")
        for n in names:
            lines.append(f"\t\t\t{n}(m);")
        lines.append("\t\t}")
    lines.append("\t}")
    lines.append("")
    lines.append("\n\n".join(funcs))
    lines.append("}")
    lines.append("")
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))


def main():
    keys = collect_keys()
    tables = {"cs": {k: k for k in keys}}
    save("cs", tables["cs"])
    for lang in LANGS[1:]:
        old = load(lang)
        table = {k: old.get(k, "") for k in keys}
        missing = [k for k in keys if not table[k]]
        removed = [k for k in old if k not in table]
        save(lang, table)
        tables[lang] = table
        print(f"{lang}: {len(keys) - len(missing)}/{len(keys)} přeloženo, chybí {len(missing)}, odstraněno {len(removed)}")
        for k in missing[:20]:
            print("   chybí:", k)
    write_data(keys, tables)
    print("OK:", os.path.relpath(OUT, ROOT), "-", len(keys), "textů")


if __name__ == "__main__":
    main()
