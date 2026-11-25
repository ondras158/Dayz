#!/usr/bin/env python3
"""Generuje .layout soubory menu módu OS_Core (jednotný vzhled).

Použití:  python3 tools/generate_layouts.py
Výstup:   OS_Core/GUI/layouts/*.layout
Pozice a velikosti jsou v pixelech vůči oknu menu (okno je vycentrované).
"""
import os
import re

OUT = os.path.join(os.path.dirname(__file__), "..", "OS_Core", "GUI", "layouts")


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
    "spawn_menu.layout": layout("OS_SpawnMenu", 520, 500, [
        text("Title", 20, 12, 480, 36, "Vyber misto spawnu", "gui/fonts/metron28", "center"),
        text("Countdown", 20, 50, 480, 26, "", align="center"),
        listbox("ZoneList", 20, 85, 480, 300),
        button("BtnSpawn", 20, 400, 230, 50, "Spawnout zde"),
        button("BtnStay", 270, 400, 230, 50, "Zustat tady"),
        text("Status", 20, 460, 480, 26, "", align="center"),
    ]),
    "admin_menu.layout": layout("OS_AdminMenu", 760, 650, [
        text("Title", 20, 12, 720, 36, "Admin menu", "gui/fonts/metron28", "center"),
        text("PlayersLabel", 20, 52, 340, 24, "Hraci online"),
        listbox("PlayerList", 20, 80, 340, 466),
        button("BtnRefresh", 20, 556, 340, 38, "Obnovit seznam"),
        text("Target", 380, 52, 360, 24, "Cil:"),
        button("BtnHeal", 380, 80, 175, 38, "Vylecit"),
        button("BtnKill", 565, 80, 175, 38, "Zabit"),
        button("BtnTpTo", 380, 125, 175, 38, "Teleport k nemu"),
        button("BtnTpHere", 565, 125, 175, 38, "Teleport ke mne"),
        button("BtnGod", 380, 170, 360, 38, "Godmode"),
        text("ItemLabel", 380, 222, 360, 24, "Predmet: classname [pocet]  napr. Apple 5"),
        edit("EditItem", 380, 250, 360, 34),
        button("BtnSpawn", 380, 292, 360, 38, "Dat predmet cili"),
        text("ArgLabel", 380, 344, 360, 24, "Castka penez / souradnice X Z / polomer [nazev]"),
        edit("EditArg", 380, 372, 360, 34),
        button("BtnMoney", 380, 414, 175, 38, "Dat penize cili"),
        button("BtnTpCoords", 565, 414, 175, 38, "Teleport na X Z"),
        button("BtnUnlockCar", 380, 460, 360, 38, "Odemknout nejblizsi auto"),
        button("BtnPvpAdd", 380, 506, 175, 38, "PvP zona zde"),
        button("BtnPvpRemove", 565, 506, 175, 38, "Smazat PvP zonu"),
        button("BtnTools", 380, 556, 175, 38, "Nastroje (COT)"),
        button("BtnClose", 565, 556, 175, 38, "Zavrit"),
        text("Status", 20, 606, 720, 26, "", align="center"),
    ]),
    "trader_menu.layout": layout("OS_TraderMenu", 960, 690, [
        text("Title", 20, 12, 600, 36, "Obchod", "gui/fonts/metron28"),
        text("Balance", 560, 12, 380, 36, "", align="right"),
        text("CatLabel", 20, 56, 220, 24, "Kategorie"),
        listbox("CategoryList", 20, 90, 220, 412),
        edit("EditSearch", 255, 54, 330, 30),
        button("BtnSearch", 595, 54, 90, 30, "Hledat"),
        listbox("ItemList", 255, 90, 430, 412),
        button("BtnBuy", 255, 512, 430, 40, "Koupit vybrane"),
        text("SellLabel", 700, 56, 240, 24, "Tvoje veci k prodeji"),
        listbox("SellList", 700, 90, 240, 412),
        button("BtnSell", 700, 512, 240, 40, "Prodat vybrane"),
        button("BtnClose", 20, 512, 220, 40, "Zavrit"),
        text("AdminLabel", 20, 566, 920, 24, "ADMIN - cena vybraneho predmetu (-1 = nelze, obe -1 = skryt) / nasobeni cele kategorie"),
        text("BuyLabel", 20, 596, 60, 34, "Nakup"),
        edit("EditBuy", 80, 596, 110, 34),
        text("SellPriceLabel", 200, 596, 60, 34, "Vykup"),
        edit("EditSell", 260, 596, 110, 34),
        button("BtnSavePrice", 380, 596, 200, 34, "Ulozit cenu"),
        text("FactorLabel", 600, 596, 140, 34, "Nasobit kat."),
        edit("EditFactor", 740, 596, 80, 34),
        button("BtnBulk", 830, 596, 110, 34, "Nasobit"),
        text("Status", 20, 646, 920, 26, "", align="center"),
    ]),
    "vehicle_menu.layout": layout("OS_VehicleMenu", 960, 600, [
        text("Title", 20, 12, 600, 36, "Autocentrum", "gui/fonts/metron28"),
        text("Balance", 560, 12, 380, 36, "", align="right"),
        text("DealerLabel", 20, 56, 300, 24, "Autobazar"),
        listbox("OfferList", 20, 84, 300, 410),
        button("BtnBuy", 20, 504, 300, 40, "Koupit auto"),
        text("ServiceLabel", 340, 56, 280, 24, "Servis"),
        text("CarName", 340, 84, 280, 26, ""),
        text("CarOwner", 340, 110, 280, 26, ""),
        text("CarHealth", 340, 136, 280, 26, ""),
        text("CarFuel", 340, 162, 280, 26, ""),
        button("BtnRepair", 340, 196, 280, 40, "Opravit"),
        button("BtnRefuel", 340, 240, 280, 40, "Natankovat"),
        button("BtnParts", 340, 284, 280, 40, "Doplnit dily"),
        button("BtnClaim", 340, 328, 280, 40, "Prepsat auto na sebe"),
        button("BtnStore", 340, 400, 280, 40, "Ulozit do garaze"),
        button("BtnSetSpawn", 340, 452, 280, 40, "Nastavit misto spawnu"),
        button("BtnClose", 340, 504, 280, 40, "Zavrit"),
        text("GarageLabel", 640, 56, 300, 24, "Garaz"),
        listbox("GarageList", 640, 84, 300, 410),
        button("BtnRetrieve", 640, 504, 300, 40, "Vyzvednout"),
        text("Status", 20, 560, 920, 26, "", align="center"),
    ]),
    "atm_menu.layout": layout("OS_AtmMenu", 860, 480, [
        text("Title", 20, 12, 820, 36, "Bankomat", "gui/fonts/metron28", "center"),
        text("CashText", 20, 60, 400, 28, ""),
        text("BankText", 20, 90, 400, 28, ""),
        text("FeeText", 20, 120, 400, 24, ""),
        text("AmountLabel", 20, 160, 100, 34, "Castka"),
        edit("EditAmount", 120, 160, 300, 34),
        button("BtnDeposit", 20, 210, 195, 40, "Vlozit"),
        button("BtnWithdraw", 225, 210, 195, 40, "Vybrat"),
        button("BtnDepositAll", 20, 258, 195, 40, "Vlozit vse"),
        button("BtnWithdrawAll", 225, 258, 195, 40, "Vybrat vse"),
        text("GroupText", 20, 306, 400, 26, ""),
        button("BtnGroupDeposit", 20, 336, 195, 40, "Vlozit do skupiny"),
        button("BtnGroupWithdraw", 225, 336, 195, 40, "Vybrat ze skupiny"),
        button("BtnClose", 20, 400, 400, 40, "Zavrit"),
        text("TransferLabel", 440, 60, 400, 24, "Prevod jinemu hraci (online)"),
        listbox("PlayerList", 440, 88, 400, 130),
        text("TargetIdLabel", 440, 224, 150, 30, "nebo Steam64 ID"),
        edit("EditTargetId", 590, 224, 250, 30),
        text("TransferAmountLabel", 440, 260, 150, 30, "Castka"),
        edit("EditTransferAmount", 590, 260, 250, 30),
        button("BtnTransfer", 440, 298, 400, 40, "Poslat z uctu"),
        text("TransferFeeText", 440, 342, 400, 24, ""),
        text("Status", 20, 446, 820, 26, "", align="center"),
    ]),
    "group_menu.layout": layout("OS_GroupMenu", 860, 610, [
        text("Title", 20, 12, 820, 36, "Skupina", "gui/fonts/metron28", "center"),
        text("InfoText", 20, 52, 820, 26, "", align="center"),
        text("MembersLabel", 20, 84, 400, 24, "Clenove"),
        listbox("MemberList", 20, 110, 400, 260),
        button("BtnKick", 20, 380, 195, 38, "Vyhodit"),
        button("BtnSetAdmin", 225, 380, 195, 38, "Admin / clen"),
        button("BtnPromote", 20, 426, 195, 38, "Predat vlastnictvi"),
        button("BtnDisband", 225, 426, 195, 38, "Rozpustit skupinu"),
        button("BtnLeave", 20, 472, 400, 38, "Opustit skupinu"),
        text("FreeLabel", 440, 84, 400, 24, "Pozvat hrace (online, bez skupiny)"),
        listbox("FreeList", 440, 110, 400, 214),
        button("BtnInvite", 440, 332, 400, 38, "Pozvat"),
        button("BtnBuySlot", 440, 380, 400, 38, "Koupit dalsi misto"),
        text("BalanceText", 440, 426, 400, 38, ""),
        text("CreateLabel", 20, 84, 400, 24, "Zalozit novou skupinu (nazev)"),
        edit("EditName", 20, 112, 400, 34),
        button("BtnCreate", 20, 154, 400, 38, "Zalozit skupinu"),
        text("InviteText", 440, 84, 400, 26, ""),
        text("InviteText2", 440, 110, 400, 26, ""),
        button("BtnAccept", 440, 154, 195, 38, "Prijmout"),
        button("BtnDecline", 645, 154, 195, 38, "Odmitnout"),
        button("BtnClose", 20, 526, 820, 38, "Zavrit"),
        text("Status", 20, 570, 820, 26, "", align="center"),
    ]),
}

# jmenovka nad hlavou spoluhráče (bez okna, pozici nastavuje skript v pixelech)
NAMETAG = """FrameWidgetClass OS_NameTag {
 ignorepointer 1
 position 0 0
 size 300 40
 hexactpos 1
 vexactpos 1
 hexactsize 1
 vexactsize 1
 {
  TextWidgetClass Text {
   ignorepointer 1
   color 0.55 0.85 1 1
   position 0 0
   size 1 1
   hexactpos 1
   vexactpos 1
   hexactsize 0
   vexactsize 0
   text ""
   font "gui/fonts/metron"
   "text halign" center
   "text valign" center
  }
 }
}
"""
LAYOUTS["nametag.layout"] = NAMETAG

# plátno pro obdélník místa spawnu + nápověda (přes celou obrazovku, nepřebírá myš)
PLACER = """FrameWidgetClass OS_Placer {
 ignorepointer 1
 position 0 0
 size 1 1
 hexactpos 1
 vexactpos 1
 hexactsize 0
 vexactsize 0
 {
  CanvasWidgetClass Canvas {
   ignorepointer 1
   position 0 0
   size 1 1
   hexactpos 1
   vexactpos 1
   hexactsize 0
   vexactsize 0
  }
  PanelWidgetClass HintBg {
   ignorepointer 1
   color 0 0 0 0.6
   position 0 40
   size 1100 40
   halign center_ref
   hexactpos 1
   vexactpos 1
   hexactsize 1
   vexactsize 1
   style rover_sim_colorable
  }
  TextWidgetClass Hint {
   ignorepointer 1
   color 0.6 1 0.65 1
   position 0 40
   size 1100 40
   halign center_ref
   hexactpos 1
   vexactpos 1
   hexactsize 1
   vexactsize 1
   text ""
   font "gui/fonts/metron"
   "text halign" center
   "text valign" center
  }
 }
}
"""
LAYOUTS["placer.layout"] = PLACER

# plátno pro kruhy zón na mapě (vkládá se do mapy, velikost = mapa)
LAYOUTS["mapzones.layout"] = """CanvasWidgetClass OS_MapZones {
 ignorepointer 1
 position 0 0
 size 1 1
 hexactpos 1
 vexactpos 1
 hexactsize 0
 vexactsize 0
}
"""

LAYOUTS["admin_tools.layout"] = layout("OS_AdminToolsMenu", 860, 610, [
    text("Title", 20, 12, 820, 36, "Admin nastroje", "gui/fonts/metron28", "center"),
    text("Target", 20, 52, 820, 26, "Cil:", align="center"),
    text("PlayerLabel", 20, 84, 400, 24, "Hrac (cil) a admin"),
    button("BtnStrip", 20, 110, 195, 38, "Vysvleknout"),
    button("BtnInvis", 225, 110, 195, 38, "Neviditelnost"),
    button("BtnEsp", 20, 156, 195, 38, "ESP"),
    button("BtnFreecam", 225, 156, 195, 38, "Freecam"),
    text("ReasonLabel", 20, 202, 400, 24, "Duvod (kick/ban) nebo Steam ID (unban)"),
    edit("EditReason", 20, 228, 400, 34),
    button("BtnKick", 20, 270, 130, 38, "Kick"),
    button("BtnBan", 155, 270, 130, 38, "Ban"),
    button("BtnUnban", 290, 270, 130, 38, "Unban"),
    text("CarLabel", 20, 320, 400, 24, "Vozidlo (do 15 m od cile)"),
    button("BtnCarRepair", 20, 346, 130, 38, "Opravit"),
    button("BtnCarDestroy", 155, 346, 130, 38, "Znicit"),
    button("BtnCarDelete", 290, 346, 130, 38, "Smazat"),
    button("BtnSpawner", 20, 400, 400, 42, "Spawner predmetu a vozidel"),
    text("TimeLabel", 440, 84, 400, 24, "Cas"),
    button("BtnMorning", 440, 110, 94, 38, "6:00"),
    button("BtnNoon", 542, 110, 94, 38, "12:00"),
    button("BtnEvening", 644, 110, 94, 38, "18:00"),
    button("BtnNight", 746, 110, 94, 38, "0:00"),
    edit("EditTime", 440, 156, 195, 34),
    button("BtnSetTime", 645, 156, 195, 38, "Nastavit cas"),
    text("WeatherLabel", 440, 210, 400, 24, "Pocasi"),
    button("BtnClear", 440, 236, 195, 38, "Jasno"),
    button("BtnCloudy", 645, 236, 195, 38, "Zatazeno"),
    button("BtnRain", 440, 282, 195, 38, "Dest"),
    button("BtnStorm", 645, 282, 195, 38, "Bourka"),
    button("BtnFog", 440, 328, 400, 38, "Mlha"),
    text("Hint1", 440, 380, 400, 24, "Mapa (M): dvojklik = teleport cile"),
    text("Hint2", 440, 406, 400, 24, "Freecam: klavesa End, Shift+End = teleport"),
    text("Hint3", 440, 432, 400, 24, "WASD, mys, Q/E dolu/nahoru, Shift rychle"),
    button("BtnClose", 20, 520, 820, 38, "Zavrit"),
    text("Status", 20, 566, 820, 26, "", align="center"),
])

LAYOUTS["admin_spawn.layout"] = layout("OS_AdminSpawnMenu", 760, 640, [
    text("Title", 20, 12, 720, 36, "Spawner", "gui/fonts/metron28", "center"),
    text("Target", 20, 50, 720, 24, "Cil:", align="center"),
    edit("EditSearch", 20, 80, 560, 34),
    button("BtnSearch", 590, 78, 150, 38, "Hledat"),
    listbox("ResultList", 20, 124, 720, 330),
    text("CountLabel", 20, 464, 100, 34, "Pocet"),
    edit("EditCount", 120, 464, 120, 34),
    button("BtnToInv", 20, 508, 230, 38, "Do inventare cile"),
    button("BtnToGround", 260, 508, 230, 38, "Na zem k cili"),
    button("BtnVehicle", 500, 508, 240, 38, "Spawnout vozidlo"),
    button("BtnClose", 20, 554, 720, 38, "Zavrit"),
    text("Status", 20, 600, 720, 26, "", align="center"),
])

LAYOUTS["farm_menu.layout"] = layout("OS_FarmMenu", 860, 700, [
    text("Title", 20, 12, 820, 36, "Farma", "gui/fonts/metron28", "center"),
    text("Info", 20, 52, 820, 26, "", align="center"),
    text("Info2", 20, 78, 820, 26, "", align="center"),
    text("AnimalsLabel", 20, 108, 400, 24, "Zvirata, stromy a zahony (vyber a kup)"),
    listbox("AnimalList", 20, 134, 400, 230),
    button("BtnBuyAnimal", 20, 372, 400, 38, "Koupit vybrane"),
    text("StorageLabel", 440, 108, 400, 24, "Sklad (vyrobene suroviny)"),
    listbox("StorageList", 440, 134, 400, 230),
    button("BtnCollect", 440, 372, 195, 38, "Vybrat vybrane"),
    button("BtnCollectAll", 645, 372, 195, 38, "Vybrat vse"),
    text("FeedText", 20, 418, 820, 24, ""),
    text("FertText", 20, 442, 820, 24, ""),
    edit("EditFeed", 20, 472, 190, 34),
    button("BtnBuyFeed", 220, 472, 200, 34, "Koupit krmivo"),
    button("BtnBuyFert", 430, 472, 200, 34, "Koupit hnojivo"),
    text("BuildingsLabel", 20, 516, 400, 24, "Stavby na farme"),
    listbox("BuildingList", 20, 542, 400, 76),
    button("BtnBuildStructure", 440, 542, 400, 38, "Postavit vybranou stavbu"),
    button("BtnBuild", 440, 542, 400, 38, "Postavit farmu zde"),
    button("BtnRemove", 440, 588, 400, 34, "Zrusit farmu"),
    button("BtnClose", 20, 632, 820, 38, "Zavrit"),
    text("Status", 20, 674, 820, 22, "", align="center"),
])

LAYOUTS["pgarage_menu.layout"] = layout("OS_PersonalGarageMenu", 560, 330, [
    text("Title", 20, 12, 520, 36, "Osobni garaz", "gui/fonts/metron28", "center"),
    text("Info", 20, 58, 520, 26, "", align="center"),
    text("Info2", 20, 86, 520, 26, "", align="center"),
    button("BtnBuild", 20, 124, 520, 40, "Postavit garaz zde"),
    button("BtnSetSpawn", 20, 124, 520, 40, "Nastavit misto pro vozidlo"),
    button("BtnRemove", 20, 172, 520, 40, "Zrusit garaz"),
    button("BtnClose", 20, 240, 520, 40, "Zavrit"),
    text("Status", 20, 290, 520, 26, "", align="center"),
])

TEXTS_FILE = os.path.join(os.path.dirname(__file__), "..", "OS_Core", "Scripts", "5_Mission", "OS_Core", "GUI", "OS_LayoutTexts.c")


def write_layout_texts():
    """texty z layoutů -> OS_LayoutTexts.c (menu je po otevření přeloží přes OS_T)"""
    pattern = re.compile(r'(?:TextWidgetClass|ButtonWidgetClass) (\w+) \{[^{}]*?\btext "([^"]+)"')
    lines = ["// VYGENEROVÁNO nástrojem tools/generate_layouts.py - neupravuj ručně.",
             "// Texty z layoutů; OS_MenuBase je po otevření menu přeloží do jazyka serveru.",
             "",
             "class OS_LayoutTexts",
             "{",
             "\tstatic void Apply(string layoutName, Widget root)",
             "\t{",
             "\t\tif (!root)",
             "\t\t\treturn;"]
    for name, content in LAYOUTS.items():
        found = pattern.findall(content)
        if not found:
            continue
        lines.append(f'\t\tif (layoutName == "{name}")')
        lines.append("\t\t{")
        for widget, txt in found:
            lines.append(f'\t\t\tSet(root, "{widget}", "{txt}");')
        lines.append("\t\t}")
    lines += ["\t}",
              "",
              "\tprotected static void Set(Widget root, string widgetName, string text)",
              "\t{",
              "\t\tWidget w = root.FindAnyWidget(widgetName);",
              "\t\tTextWidget tw = TextWidget.Cast(w);",
              "\t\tif (tw)",
              "\t\t{",
              "\t\t\ttw.SetText(OS_T(text));",
              "\t\t\treturn;",
              "\t\t}",
              "\t\tButtonWidget bw = ButtonWidget.Cast(w);",
              "\t\tif (bw)",
              "\t\t\tbw.SetText(OS_T(text));",
              "\t}",
              "}",
              ""]
    with open(TEXTS_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
    print("OK: OS_LayoutTexts.c")


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, content in LAYOUTS.items():
        with open(os.path.join(OUT, name), "w", newline="\r\n") as f:
            f.write(content)
        print("OK:", name)
    write_layout_texts()
