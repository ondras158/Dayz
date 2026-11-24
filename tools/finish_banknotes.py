#!/usr/bin/env python3
"""Na podklady bankovek z money_images/raw/<hodnota>.png dopíše přesné nápisy a uloží money_images/<hodnota>.png.

    python3 tools/finish_banknotes.py --currency EUR --title "OS CORE"
    python3 tools/import_money_images.py          # převede na textury módu

AI generátory text často zkomolí, proto se podklad generuje bez textu a nápisy se přidají tady.
"""
import argparse
import os

from PIL import Image, ImageDraw, ImageFont, ImageOps, ImageStat

ROOT = os.path.join(os.path.dirname(__file__), "..")
RAW = os.path.join(ROOT, "money_images", "raw")
OUT = os.path.join(ROOT, "money_images")
SERIF_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf"
SANS_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"


def ink_color(img):
    """tmavá barva podkladu (průměr nejtmavších pixelů) - nápisy pak ladí s bankovkou"""
    small = img.resize((64, 32)).convert("RGB")
    raw = small.tobytes()
    px = sorted([tuple(raw[i:i + 3]) for i in range(0, len(raw), 3)], key=lambda p: sum(p))[:200]
    r = sum(p[0] for p in px) // len(px)
    g = sum(p[1] for p in px) // len(px)
    b = sum(p[2] for p in px) // len(px)
    return (int(r * 0.8), int(g * 0.8), int(b * 0.8))


def paper_color(img):
    st = ImageStat.Stat(img.resize((64, 32)).convert("RGB"))
    return tuple(min(255, int(m * 1.15)) for m in st.mean)


def finish(path, value, title, currency):
    img = Image.open(path).convert("RGB")
    W = 2000
    img = img.resize((W, W // 2), Image.LANCZOS)   # poměr 2:1, bez ořezu (zachová rámeček i hologram)
    H = W // 2
    d = ImageDraw.Draw(img)
    ink, paper = ink_color(img), paper_color(img)
    stroke = dict(stroke_width=6, stroke_fill=paper)

    d.text((W * 0.5, H * 0.10), title, font=ImageFont.truetype(SERIF_BOLD, 72), fill=ink, anchor="mm", **stroke)
    size = 130 if value < 100 else (108 if value < 1000 else 86)
    big = ImageFont.truetype(SERIF_BOLD, size)
    d.text((W * 0.115, H * 0.16), str(value), font=big, fill=ink, anchor="mm", **stroke)
    d.text((W * 0.905, H * 0.84), str(value), font=big, fill=ink, anchor="mm", **stroke)
    d.text((W * 0.5, H * 0.865), f"{value} {currency}", font=ImageFont.truetype(SERIF_BOLD, 50), fill=ink, anchor="mm", **stroke)
    d.text((W * 0.115, H * 0.27), currency, font=ImageFont.truetype(SANS_BOLD, 40), fill=ink, anchor="mm", **stroke)
    return img


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--currency", default="EUR")
    ap.add_argument("--title", default="OS CORE")
    a = ap.parse_args()

    done = 0
    for name in sorted(os.listdir(RAW)) if os.path.isdir(RAW) else []:
        stem, ext = os.path.splitext(name)
        if not stem.isdigit() or ext.lower() not in (".png", ".jpg", ".jpeg", ".webp"):
            continue
        out = os.path.join(OUT, f"{stem}.png")
        finish(os.path.join(RAW, name), int(stem), a.title.upper(), a.currency.upper()).save(out)
        done += 1
        print("OK:", os.path.relpath(out, ROOT))
    if not done:
        print("Ve slozce money_images/raw/ nejsou zadne podklady (20.png, 50.png ...)")
