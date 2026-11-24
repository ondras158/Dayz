#!/usr/bin/env python3
"""Generuje .layout soubory menu módu BadlandsCore (jednotný vzhled).

Použití:  python3 tools/generate_layouts.py
Výstup:   BadlandsCore/GUI/layouts/*.layout
Pozice a velikosti jsou v pixelech vůči oknu menu (okno je vycentrované).
"""
import os

OUT = os.path.join(os.path.dirname(__file__), "..", "BadlandsCore", "GUI", "layouts")


def props(x, y, w, h):
    return f""" position {x} {y}
 size {w} {h}
 hexactpos 1
 vexactpos 1
 hexactsize 1
 vexactsize 1
"""


def text(name, x, y, w, h, txt, font="gui/fonts/metron", align="left"):
    return f"""TextWidgetClass {name} {{
{props(x, y, w, h)} ignorepointer 1
 text "{txt}"
 font "{font}"
 "text halign" {align}
 "text valign" center
}}"""


def button(name, x, y, w, h, txt):
    return f"""ButtonWidgetClass {name} {{
 color 0.25 0.22 0.18 1
{props(x, y, w, h)} style Default
 text "{txt}"
 font "gui/fonts/metron"
}}"""


def listbox(name, x, y, w, h):
    return f"""PanelWidgetClass {name}Bg {{
 color 0 0 0 0.6
{props(x, y, w, h)} ignorepointer 1
 style rover_sim_colorable
}}
TextListboxWidgetClass {name} {{
{props(x + 4, y + 4, w - 8, h - 8)} font "gui/fonts/metron"
}}"""


def edit(name, x, y, w, h):
    return f"""PanelWidgetClass {name}Bg {{
 color 0 0 0 0.7
{props(x, y, w, h)} ignorepointer 1
 style rover_sim_colorable
}}
EditBoxWidgetClass {name} {{
{props(x + 6, y + 2, w - 12, h - 4)} font "gui/fonts/metron"
}}"""


def indent(block, n):
    return "\n".join((" " * n + l) if l else l for l in block.split("\n"))


def layout(root, w, h, children):
    inner = "\n".join(children)
    return f"""FrameWidgetClass {root} {{
 position 0 0
 size 1 1
 hexactpos 1
 vexactpos 1
 hexactsize 0
 vexactsize 0
 {{
  PanelWidgetClass Window {{
   color 0.08 0.07 0.06 0.92
   position 0 0
   size {w} {h}
   halign center_ref
   valign center_ref
   hexactpos 1
   vexactpos 1
   hexactsize 1
   vexactsize 1
   style rover_sim_colorable
   {{
{indent(inner, 4)}
   }}
  }}
 }}
}}
"""


LAYOUTS = {
    "spawn_menu.layout": layout("BC_SpawnMenu", 520, 500, [
        text("Title", 20, 12, 480, 36, "Vyber misto spawnu", "gui/fonts/metron28", "center"),
        text("Countdown", 20, 50, 480, 26, "", align="center"),
        listbox("ZoneList", 20, 85, 480, 300),
        button("BtnSpawn", 20, 400, 230, 50, "Spawnout zde"),
        button("BtnStay", 270, 400, 230, 50, "Zustat tady"),
        text("Status", 20, 460, 480, 26, "", align="center"),
    ]),
    "admin_menu.layout": layout("BC_AdminMenu", 760, 600, [
        text("Title", 20, 12, 720, 36, "Admin menu", "gui/fonts/metron28", "center"),
        text("PlayersLabel", 20, 52, 340, 24, "Hraci online"),
        listbox("PlayerList", 20, 80, 340, 420),
        button("BtnRefresh", 20, 510, 340, 38, "Obnovit seznam"),
        text("Target", 380, 52, 360, 24, "Cil:"),
        button("BtnHeal", 380, 80, 175, 38, "Vylecit"),
        button("BtnKill", 565, 80, 175, 38, "Zabit"),
        button("BtnTpTo", 380, 125, 175, 38, "Teleport k nemu"),
        button("BtnTpHere", 565, 125, 175, 38, "Teleport ke mne"),
        button("BtnGod", 380, 170, 360, 38, "Godmode"),
        text("ItemLabel", 380, 222, 360, 24, "Predmet: classname [pocet]  napr. Apple 5"),
        edit("EditItem", 380, 250, 360, 34),
        button("BtnSpawn", 380, 292, 360, 38, "Dat predmet cili"),
        text("ArgLabel", 380, 344, 360, 24, "Castka penez / souradnice X Z"),
        edit("EditArg", 380, 372, 360, 34),
        button("BtnMoney", 380, 414, 175, 38, "Dat penize cili"),
        button("BtnTpCoords", 565, 414, 175, 38, "Teleport na X Z"),
        button("BtnClose", 380, 510, 360, 38, "Zavrit"),
        text("Status", 20, 558, 720, 26, "", align="center"),
    ]),
    "trader_menu.layout": layout("BC_TraderMenu", 960, 600, [
        text("Title", 20, 12, 600, 36, "Obchod", "gui/fonts/metron28"),
        text("Balance", 620, 12, 320, 36, "Penize: 0", "gui/fonts/metron28", "right"),
        text("CatLabel", 20, 56, 220, 24, "Kategorie"),
        listbox("CategoryList", 20, 84, 220, 416),
        text("ItemLabel", 255, 56, 430, 24, "Nabidka (cena nakupu / vykupu)"),
        listbox("ItemList", 255, 84, 430, 416),
        button("BtnBuy", 255, 510, 430, 40, "Koupit vybrane"),
        text("SellLabel", 700, 56, 240, 24, "Tvoje zbrane k prodeji"),
        listbox("SellList", 700, 84, 240, 416),
        button("BtnSell", 700, 510, 240, 40, "Prodat vybrane"),
        button("BtnClose", 20, 510, 220, 40, "Zavrit"),
        text("Status", 20, 560, 920, 26, "", align="center"),
    ]),
    "vehicle_menu.layout": layout("BC_VehicleMenu", 960, 600, [
        text("Title", 20, 12, 600, 36, "Autocentrum", "gui/fonts/metron28"),
        text("Balance", 620, 12, 320, 36, "Penize: 0", "gui/fonts/metron28", "right"),
        text("DealerLabel", 20, 56, 300, 24, "Autobazar"),
        listbox("OfferList", 20, 84, 300, 410),
        button("BtnBuy", 20, 504, 300, 40, "Koupit auto"),
        text("ServiceLabel", 340, 56, 280, 24, "Servis"),
        text("CarName", 340, 84, 280, 26, ""),
        text("CarHealth", 340, 112, 280, 26, ""),
        text("CarFuel", 340, 140, 280, 26, ""),
        button("BtnRepair", 340, 180, 280, 40, "Opravit"),
        button("BtnRefuel", 340, 228, 280, 40, "Natankovat"),
        button("BtnParts", 340, 276, 280, 40, "Doplnit dily"),
        button("BtnStore", 340, 344, 280, 40, "Ulozit do garaze"),
        button("BtnClose", 340, 504, 280, 40, "Zavrit"),
        text("GarageLabel", 640, 56, 300, 24, "Garaz"),
        listbox("GarageList", 640, 84, 300, 410),
        button("BtnRetrieve", 640, 504, 300, 40, "Vyzvednout"),
        text("Status", 20, 560, 920, 26, "", align="center"),
    ]),
}

if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, content in LAYOUTS.items():
        with open(os.path.join(OUT, name), "w", newline="\r\n") as f:
            f.write(content)
        print("OK:", name)
