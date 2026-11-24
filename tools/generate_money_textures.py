#!/usr/bin/env python3
"""Vygeneruje textury bankovek BadlandsCore (PNG 512x512).

Výstup: BadlandsCore/data/money_<hodnota>_co.png
PNG převeď v DayZ Tools na .paa (ImageToPAA nebo TexView2: File -> Save As .paa),
config.cpp odkazuje na BadlandsCore\\data\\money_<hodnota>_co.paa.
"""
import math
import os
import random

from PIL import Image, ImageDraw, ImageFont

OUT = os.path.join(os.path.dirname(__file__), "..", "BadlandsCore", "data")
FONT_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf"
FONT_SANS = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

# hodnota: (hlavní barva, tmavá barva, světlá barva)
NOTES = {
    10:   ((176, 140, 92),  (92, 64, 34),  (232, 214, 180)),   # písková
    50:   ((120, 150, 96),  (52, 76, 40),  (210, 226, 190)),   # olivová
    100:  ((96, 136, 168),  (34, 62, 92),  (200, 220, 236)),   # modrá
    500:  ((170, 96, 82),   (96, 40, 32),  (236, 204, 196)),   # cihlová
    1000: ((138, 108, 160), (66, 44, 86),  (222, 208, 232)),   # fialová
}

SIZE = 512


def font(path, size):
    return ImageFont.truetype(path, size)


def centered(draw, xy, text, fnt, fill):
    l, t, r, b = draw.textbbox((0, 0), text, font=fnt)
    draw.text((xy[0] - (r - l) / 2 - l, xy[1] - (b - t) / 2 - t), text, font=fnt, fill=fill)


def make(value, colors):
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
    centered(d, (SIZE // 2, 70), "BADLANDS", font(FONT_BOLD, 46), dark)
    centered(d, (SIZE // 2, 112), "BANKOVKA", font(FONT_SANS, 22), dark)
    centered(d, (SIZE // 2, 420), f"{value} KORUN", font(FONT_BOLD, 40), dark)

    corner = font(FONT_BOLD, 44 if value < 1000 else 34)
    for x, y in ((84, 165), (SIZE - 84, 165), (84, 340), (SIZE - 84, 340)):
        centered(d, (x, y), str(value), corner, dark)

    serial = f"BC {rnd.randint(100000, 999999)}"
    centered(d, (SIZE // 2, 462), serial, font(FONT_SANS, 18), main)
    return img


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for value, colors in NOTES.items():
        path = os.path.join(OUT, f"money_{value}_co.png")
        make(value, colors).save(path)
        print("OK:", path)
