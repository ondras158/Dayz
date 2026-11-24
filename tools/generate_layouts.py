#!/usr/bin/env python3
"""Generuje .layout soubory menu módu OS_Core (jednotný vzhled).

Použití:  python3 tools/generate_layouts.py
Výstup:   OS_Core/GUI/layouts/*.layout
Pozice a velikosti jsou v pixelech vůči oknu menu (okno je vycentrované).
"""
import os

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
    "admin_menu.layout": layout("OS_AdminMenu", 760, 600, [
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
        button("BtnUnlockCar", 380, 460, 360, 38, "Odemknout nejblizsi auto"),
        button("BtnClose", 380, 510, 360, 38, "Zavrit"),
        text("Status", 20, 558, 720, 26, "", align="center"),
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
    "group_menu.layout": layout("OS_GroupMenu", 860, 560, [
        text("Title", 20, 12, 820, 36, "Skupina", "gui/fonts/metron28", "center"),
        text("InfoText", 20, 52, 820, 26, "", align="center"),
        text("MembersLabel", 20, 84, 400, 24, "Clenove"),
        listbox("MemberList", 20, 110, 400, 260),
        button("BtnKick", 20, 380, 195, 38, "Vyhodit"),
        button("BtnPromote", 225, 380, 195, 38, "Predat vedeni"),
        button("BtnLeave", 20, 426, 195, 38, "Opustit skupinu"),
        button("BtnDisband", 225, 426, 195, 38, "Rozpustit skupinu"),
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
        button("BtnClose", 20, 480, 820, 38, "Zavrit"),
        text("Status", 20, 524, 820, 26, "", align="center"),
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

if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, content in LAYOUTS.items():
        with open(os.path.join(OUT, name), "w", newline="\r\n") as f:
            f.write(content)
        print("OK:", name)
