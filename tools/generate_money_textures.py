#!/usr/bin/env python3
"""Vygeneruje textury bankovek OS_Core (PNG 512x512) pro zvolenou měnu.

Použití:
    python3 tools/generate_money_textures.py                       # euro (výchozí)
    python3 tools/generate_money_textures.py --currency RUB --word RUBLU
    python3 tools/generate_money_textures.py --currency USD --title "MUJ SERVER"

Výstup: OS_Core/data/money_<hodnota>_co.png pro všechny hodnoty 1 až 5000
(server si v Bank.json vybere, které používá).
PNG převeď v DayZ Tools na .paa (ImageToPAA nebo TexView2: File -> Save As .paa).
"""
import argparse
import math
import os
import random

from PIL import Image, ImageDraw, ImageFont

OUT = os.path.join(os.path.dirname(__file__), "..", "OS_Core", "data")
FONT_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf"
FONT_SANS = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

# barvy podle hodnoty: (hlavní, tmavá, světlá)
NOTES = {
    1:    ((150, 150, 150), (60, 60, 60),   (226, 226, 226)),   # šedá
    2:    ((168, 150, 110), (80, 66, 40),   (230, 222, 200)),   # béžová
    5:    ((150, 150, 150), (70, 70, 80),   (220, 222, 228)),   # stříbrná
    10:   ((190, 110, 100), (100, 40, 36),  (240, 210, 204)),   # červená
    20:   ((96, 136, 180),  (34, 62, 100),  (204, 222, 240)),   # modrá
    50:   ((210, 140, 80),  (110, 60, 24),  (246, 222, 196)),   # oranžová
    100:  ((110, 160, 100), (40, 84, 36),   (208, 234, 200)),   # zelená
    200:  ((196, 170, 90),  (100, 80, 24),  (244, 232, 196)),   # žlutá
    500:  ((150, 110, 170), (70, 44, 90),   (226, 210, 236)),   # fialová
    1000: ((176, 140, 92),  (92, 64, 34),   (232, 214, 180)),   # písková
    2000: ((90, 150, 150),  (30, 76, 76),   (200, 232, 232)),   # tyrkysová
    5000: ((160, 90, 120),  (86, 36, 60),   (236, 204, 220)),   # vínová
}

SIZE = 512


def font(path, size):
    return ImageFont.truetype(path, size)


def centered(draw, xy, text, fnt, fill):
    l, t, r, b = draw.textbbox((0, 0), text, font=fnt)
    draw.text((xy[0] - (r - l) / 2 - l, xy[1] - (b - t) / 2 - t), text, font=fnt, fill=fill)


def make(value, colors, title, label, currency):
    main, dark, light = colors
    rnd = random.Random(value)
    img = Image.new("RGB", (SIZE, SIZE), light)
    d = ImageDraw.Draw(img)

    # jemný gilošový vzor
    for i in range(0, SIZE, 6):
        phase = i / 18.0
        pts = [(x, i + 6 * math.sin(x / 22.0 + phase)) for x in range(0, SIZE + 8, 8)]
        d.line(pts, fill=main, width=1)

    # rámečky
    d.rectangle([10, 10, SIZE - 11, SIZE - 11], outline=dark, width=10)
    d.rectangle([30, 30, SIZE - 31, SIZE - 31], outline=main, width=4)

    # středový znak: slunce nad dunami
    cx, cy, r = SIZE // 2, SIZE // 2 + 10, 110
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=light, outline=dark, width=6)
    for a in range(0, 360, 15):
        rad = math.radians(a)
        d.line([(cx + math.cos(rad) * 40, cy - 20 + math.sin(rad) * 40),
                (cx + math.cos(rad) * 62, cy - 20 + math.sin(rad) * 62)], fill=main, width=4)
    d.ellipse([cx - 34, cy - 54, cx + 34, cy + 14], fill=main, outline=dark, width=3)
    d.polygon([(cx - r + 14, cy + 50), (cx - 30, cy + 10), (cx + 20, cy + 46), (cx + 60, cy + 20),
               (cx + r - 14, cy + 50), (cx + r - 30, cy + 80), (cx - r + 30, cy + 80)], fill=dark)

    # texty
    centered(d, (SIZE // 2, 70), title, font(FONT_BOLD, 46 if len(title) <= 10 else 34), dark)
    centered(d, (SIZE // 2, 112), label, font(FONT_SANS, 22), dark)
    centered(d, (SIZE // 2, 420), f"{value} {currency}", font(FONT_BOLD, 40), dark)

    corner = font(FONT_BOLD, 44 if value < 1000 else 34)
    for x, y in ((84, 165), (SIZE - 84, 165), (84, 340), (SIZE - 84, 340)):
        centered(d, (x, y), str(value), corner, dark)

    serial = f"OS {rnd.randint(100000, 999999)}"
    centered(d, (SIZE // 2, 462), serial, font(FONT_SANS, 18), main)
    return img


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--currency", default="EUR", help="text měny na bankovce (EUR, RUB, USD, KC...)")
    ap.add_argument("--title", default="OS CORE", help="nápis nahoře (název serveru)")
    ap.add_argument("--label", default="BANKOVKA", help="menší nápis pod názvem")
    ap.add_argument("--preview", help="uložit i náhled vybraných hodnot, např. 5,10,20,50,100,200,500")
    a = ap.parse_args()

    os.makedirs(OUT, exist_ok=True)
    images = {}
    for value, colors in NOTES.items():
        path = os.path.join(OUT, f"money_{value}_co.png")
        images[value] = make(value, colors, a.title.upper(), a.label.upper(), a.currency.upper())
        images[value].save(path)
        print("OK:", path)

    if a.preview:
        values = [int(v) for v in a.preview.split(",")]
        thumb = 200
        sheet = Image.new("RGB", (len(values) * (thumb + 8), thumb), (30, 30, 30))
        for i, v in enumerate(values):
            sheet.paste(images[v].resize((thumb, thumb)), (i * (thumb + 8), 0))
        sheet.save(os.path.join(OUT, "preview_bankovky.png"))
        print("Nahled:", os.path.join(OUT, "preview_bankovky.png"))
