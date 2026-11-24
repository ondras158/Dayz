#!/usr/bin/env python3
"""Realističtější potisk bankovek OS_Core (rytinový styl, poušť) - vše kresleno kódem, bez cizích obrázků.

Použití:
    python3 tools/generate_money_realistic.py                     # euro, nápis OS CORE
    python3 tools/generate_money_realistic.py --currency RUB --title "MUJ SERVER" --size 1024
    python3 tools/generate_money_realistic.py --preview 5,10,20,50,100,200,500

Bankovka je na šířku (poměr 2:1) uprostřed čtvercové textury, okolí má barvu papíru.
Výstup: OS_Core/data/money_<hodnota>_co.png  -> převést na .paa (DayZ Tools -> ImageToPAA).
Pokud je ve hře bankovka bokem nebo malá, použij --layout full (roztáhne bankovku na celou texturu).
"""
import argparse
import colorsys
import math
import os
import random

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.join(os.path.dirname(__file__), "..")
OUT = os.path.join(ROOT, "OS_Core", "data")
SERIF_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf"
SERIF = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf"
SANS_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
SANS = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

# odstín (0-1) pro každou hodnotu - podobně jako u skutečných měn má každá hodnota svou barvu
HUES = {1: 0.0, 2: 0.08, 5: 0.58, 10: 0.02, 20: 0.60, 50: 0.07, 100: 0.33,
        200: 0.12, 500: 0.78, 1000: 0.10, 2000: 0.47, 5000: 0.92}
SAT = {1: 0.0, 5: 0.10}


def hsv(h, s, v, a=255):
    r, g, b = colorsys.hsv_to_rgb(h % 1.0, max(0, min(1, s)), max(0, min(1, v)))
    return (int(r * 255), int(g * 255), int(b * 255), a)


class Palette:
    def __init__(self, value):
        h = HUES.get(value, 0.1)
        s = SAT.get(value, 0.45)
        self.paper = hsv(h, s * 0.18, 0.95)
        self.light = hsv(h, s * 0.45, 0.88)
        self.mid = hsv(h, s * 0.85, 0.62)
        self.ink = hsv(h, min(1, s * 1.1), 0.30)
        self.dark = hsv(h, min(1, s * 1.2), 0.18)
        self.accent = hsv(h + 0.5, s * 0.5, 0.55)


def font(path, size):
    return ImageFont.truetype(path, max(6, int(size)))


def text_center(d, xy, text, fnt, fill, anchor="mm", stroke=0, stroke_fill=None):
    d.text(xy, text, font=fnt, fill=fill, anchor=anchor, stroke_width=stroke, stroke_fill=stroke_fill)


def guilloche_background(d, w, h, pal, rnd, s):
    """jemné vlnovky přes celou bankovku"""
    for k in range(0, h, max(2, int(3 * s))):
        ph = rnd.random() * 6
        pts = [(x, k + 2.2 * s * math.sin(x / (9 * s) + ph) + 1.5 * s * math.sin(x / (23 * s))) for x in range(0, w + 4, 4)]
        d.line(pts, fill=pal.light, width=1)


def rosette(d, cx, cy, r, pal, color=None, loops=36, petals=7, s=1.0):
    """spirografová rozeta (typický ochranný prvek bankovek)"""
    color = color or pal.mid
    for j in range(loops):
        a0 = 2 * math.pi * j / loops
        pts = []
        for t in range(0, 361, 3):
            tt = math.radians(t)
            rr = r * (0.62 + 0.38 * math.cos(petals * tt))
            pts.append((cx + rr * math.cos(tt + a0 * 0.15) * math.cos(a0) - rr * math.sin(tt) * 0.0,
                        cy + rr * math.sin(tt + a0 * 0.15)))
        d.line(pts, fill=color, width=1)


def rosette2(d, cx, cy, R, r, p, pal, color, steps=720, width=1):
    """hypotrochoida - hustá gilošová kresba"""
    pts = []
    for i in range(steps + 1):
        t = 2 * math.pi * i / steps * r / math.gcd(int(R), int(r))
        x = (R - r) * math.cos(t) + p * math.cos((R - r) / r * t)
        y = (R - r) * math.sin(t) - p * math.sin((R - r) / r * t)
        pts.append((cx + x, cy + y))
    d.line(pts, fill=color, width=width)


def microtext_border(d, x0, y0, x1, y1, text, pal, s):
    fnt = font(SANS, 7 * s)
    line = (text + " ") * 80
    d.text((x0 + 4 * s, y0 + 1 * s), line, font=fnt, fill=pal.ink)
    d.text((x0 + 4 * s, y1 - 9 * s), line, font=fnt, fill=pal.ink)


def desert_vignette(img, box, pal, rnd, s):
    """pouštní krajina kreslená jako rytina (šrafování)"""
    x0, y0, x1, y1 = [int(v) for v in box]
    w, h = x1 - x0, y1 - y0
    layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)

    # obloha: vodorovné šrafy, hustší nahoře
    for y in range(0, int(h * 0.62), max(2, int(2.5 * s))):
        if rnd.random() < 0.15 + 0.7 * (1 - y / (h * 0.62)):
            d.line([(0, y), (w, y)], fill=pal.mid, width=1)

    # slunce s paprsky
    sx, sy, sr = w * 0.68, h * 0.42, h * 0.16
    for a in range(0, 180, 6):
        rad = math.radians(180 + a)
        d.line([(sx + math.cos(rad) * sr * 1.2, sy + math.sin(rad) * sr * 1.2),
                (sx + math.cos(rad) * sr * 2.3, sy + math.sin(rad) * sr * 2.3)], fill=pal.ink, width=1)
    d.ellipse([sx - sr, sy - sr, sx + sr, sy + sr], fill=pal.paper, outline=pal.ink, width=max(1, int(s)))
    for y in range(int(sy - sr), int(sy + sr), max(2, int(3 * s))):
        dy = y - sy
        half = math.sqrt(max(0, sr * sr - dy * dy))
        d.line([(sx - half, y), (sx + half, y)], fill=pal.light, width=1)

    # duny: tři vrstvy, každá vlastní hustota šrafování
    def dune(base, amp, freq, phase, density, color):
        top = [base - amp * (0.6 * math.sin(x / (w * freq) + phase) + 0.4 * math.sin(x / (w * freq * 0.37) + phase * 2)) for x in range(w + 1)]
        for x in range(0, w + 1, max(1, int(density * s))):
            d.line([(x, top[x]), (x + h * 0.08, h)], fill=color, width=1)
        d.line([(x, top[x]) for x in range(0, w + 1, 2)], fill=pal.dark, width=max(1, int(1.5 * s)))

    dune(h * 0.62, h * 0.10, 0.20, 0.5, 5.0, pal.mid)
    dune(h * 0.75, h * 0.09, 0.14, 2.1, 3.5, pal.ink)
    dune(h * 0.90, h * 0.07, 0.11, 4.0, 2.5, pal.dark)

    # strážní věž na duně
    tx, ty = w * 0.25, h * 0.64
    th = h * 0.32
    d.line([(tx - 10 * s, ty), (tx - 4 * s, ty - th)], fill=pal.dark, width=max(1, int(2 * s)))
    d.line([(tx + 10 * s, ty), (tx + 4 * s, ty - th)], fill=pal.dark, width=max(1, int(2 * s)))
    for k in range(4):
        yy = ty - th * (k + 1) / 5
        d.line([(tx - 9 * s + k * 1.3 * s, yy), (tx + 9 * s - k * 1.3 * s, yy + th / 5)], fill=pal.ink, width=1)
    d.rectangle([tx - 9 * s, ty - th - 9 * s, tx + 9 * s, ty - th], fill=pal.dark)
    d.polygon([(tx - 12 * s, ty - th - 9 * s), (tx + 12 * s, ty - th - 9 * s), (tx, ty - th - 18 * s)], fill=pal.dark)

    # oválná maska vinety
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).ellipse([0, 0, w, h], fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(3 * s))
    img.paste(layer, (x0, y0), Image.composite(layer, Image.new("RGBA", (w, h), (0, 0, 0, 0)), mask))
    ImageDraw.Draw(img).ellipse([x0, y0, x1, y1], outline=pal.ink, width=max(1, int(2 * s)))


def make_note(value, title, currency, label, W):
    s = W / 1000.0
    H = W // 2
    rnd = random.Random(value * 7919)
    pal = Palette(value)

    img = Image.new("RGBA", (W, H), pal.paper)
    d = ImageDraw.Draw(img)

    guilloche_background(d, W, H, pal, rnd, s)

    # velká rozeta vlevo a vpravo
    rosette2(d, W * 0.16, H * 0.55, 96 * s, 21 * s, 60 * s, pal, pal.mid)
    rosette2(d, W * 0.16, H * 0.55, 70 * s, 13 * s, 40 * s, pal, pal.ink)
    rosette2(d, W * 0.86, H * 0.32, 60 * s, 11 * s, 38 * s, pal, pal.mid)

    # okénko vodoznaku (světlý ovál se slabou siluetou věže)
    wx, wy, ww, wh = W * 0.86, H * 0.66, 70 * s, 95 * s
    wm = Image.new("RGBA", img.size, (0, 0, 0, 0))
    wd = ImageDraw.Draw(wm)
    wd.ellipse([wx - ww / 2, wy - wh / 2, wx + ww / 2, wy + wh / 2], fill=pal.paper)
    wd.polygon([(wx - 8 * s, wy + 30 * s), (wx + 8 * s, wy + 30 * s), (wx + 3 * s, wy - 20 * s), (wx - 3 * s, wy - 20 * s)], fill=pal.light)
    wd.polygon([(wx - 12 * s, wy - 20 * s), (wx + 12 * s, wy - 20 * s), (wx, wy - 34 * s)], fill=pal.light)
    img = Image.alpha_composite(img, wm.filter(ImageFilter.GaussianBlur(2 * s)))
    d = ImageDraw.Draw(img)

    # ochranný proužek (svislý)
    px = W * 0.70
    for i in range(0, H, max(3, int(6 * s))):
        d.rectangle([px, i, px + 5 * s, i + 3 * s], fill=pal.dark)

    # holografická ploška (duhový přechod)
    hx0, hx1 = W * 0.73, W * 0.78
    for x in range(int(hx0), int(hx1)):
        t = (x - hx0) / (hx1 - hx0)
        d.line([(x, 0), (x, H)], fill=hsv(0.55 + t * 0.5, 0.25, 0.92, 200))
    rosette2(d, (hx0 + hx1) / 2, H * 0.5, 20 * s, 5 * s, 12 * s, pal, (255, 255, 255, 180))

    # vineta s pouští
    desert_vignette(img, (W * 0.30, H * 0.22, W * 0.68, H * 0.86), pal, rnd, s)
    d = ImageDraw.Draw(img)

    # rámečky + mikrotext
    d.rectangle([6 * s, 6 * s, W - 6 * s, H - 6 * s], outline=pal.dark, width=max(1, int(4 * s)))
    d.rectangle([16 * s, 16 * s, W - 16 * s, H - 16 * s], outline=pal.ink, width=max(1, int(1.5 * s)))
    microtext_border(d, 16 * s, 17 * s, W - 16 * s, H - 17 * s, f"{title} {value} {currency}", pal, s)

    # nápisy
    text_center(d, (W * 0.49, H * 0.13), title, font(SERIF_BOLD, 48 * s), pal.dark)
    text_center(d, (W * 0.49, H * 0.205), label, font(SANS, 15 * s), pal.ink)
    text_center(d, (W * 0.16, H * 0.55), str(value), font(SERIF_BOLD, (78 if value < 1000 else 58) * s), pal.dark,
                stroke=max(2, int(4 * s)), stroke_fill=pal.paper)
    text_center(d, (W * 0.16, H * 0.82), currency, font(SANS_BOLD, 26 * s), pal.ink)
    text_center(d, (W * 0.93, H * 0.12), str(value), font(SERIF_BOLD, 34 * s), pal.dark)
    text_center(d, (W * 0.49, H * 0.93), f"{value} {currency}", font(SERIF_BOLD, 24 * s), pal.dark)

    # sériové číslo (dvakrát, jako na skutečných bankovkách)
    serial = f"O{chr(65 + value % 26)}{rnd.randint(1000000, 9999999)}"
    d.text((W * 0.04, H * 0.06), serial, font=font(SANS_BOLD, 16 * s), fill=hsv(0.0, 0.8, 0.55))
    d.text((W * 0.86, H * 0.92), serial, font=font(SANS_BOLD, 16 * s), fill=pal.dark, anchor="mm")

    # podpis
    sig = [(W * 0.81 + i * 3 * s, H * 0.83 + 6 * s * math.sin(i * 0.9) - i * 0.3 * s) for i in range(30)]
    d.line(sig, fill=pal.dark, width=max(1, int(1.5 * s)))

    # papírová textura (šum)
    noise = Image.effect_noise((W, H), 18).convert("L")
    noise = Image.merge("RGBA", (noise, noise, noise, Image.new("L", (W, H), 18)))
    img = Image.alpha_composite(img, noise)
    return img.convert("RGB")


def to_texture(note, size, layout, paper):
    if layout == "full":
        return note.resize((size, size), Image.LANCZOS)
    tex = Image.new("RGB", (size, size), paper[:3])
    w = size
    h = size // 2
    tex.paste(note.resize((w, h), Image.LANCZOS), (0, (size - h) // 2))
    return tex


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--currency", default="EUR")
    ap.add_argument("--title", default="OS CORE")
    ap.add_argument("--label", default="BANKOVKA")
    ap.add_argument("--size", type=int, choices=[512, 1024, 2048], default=1024)
    ap.add_argument("--layout", choices=["center", "full"], default="center")
    ap.add_argument("--preview", help="hodnoty do náhledu, např. 5,10,20,50,100,200,500")
    a = ap.parse_args()

    os.makedirs(OUT, exist_ok=True)
    notes = {}
    for value in HUES:
        note = make_note(value, a.title.upper(), a.currency.upper(), a.label.upper(), 2000)
        notes[value] = note
        tex = to_texture(note, a.size, a.layout, Palette(value).paper)
        path = os.path.join(OUT, f"money_{value}_co.png")
        tex.save(path)
        print("OK:", os.path.relpath(path, ROOT))

    if a.preview:
        values = [int(v) for v in a.preview.split(",")]
        tw, th = 560, 280
        cols = 2
        rows = (len(values) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * (tw + 12) + 12, rows * (th + 12) + 12), (28, 28, 28))
        for i, v in enumerate(values):
            sheet.paste(notes[v].resize((tw, th), Image.LANCZOS), (12 + (i % cols) * (tw + 12), 12 + (i // cols) * (th + 12)))
        sheet.save(os.path.join(OUT, "preview_bankovky.png"))
        print("Nahled: OS_Core/data/preview_bankovky.png")
