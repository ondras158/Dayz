#!/usr/bin/env python3
"""Přebarví podklad bankovky na jiný odstín (design zůstane, hologram se nemění).

    python3 tools/recolor_banknote.py --src money_images/raw/10.jpg --value 2000 --ink 20,95,100

Duotón: tmavé části dostanou novou barvu inkoustu, papír a hologram zůstanou.
Výstup: money_images/raw/<value>.png -> pak finish_banknotes.py a import_money_images.py.
"""
import argparse
import os

import numpy as np
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), "..")


def find_hologram(img):
    """Sloupce, kde je hodně barevných pixelů jiného odstínu než hlavní inkoust = hologram."""
    hsv = np.array(img.convert("HSV")).astype(np.float32)
    h, sat = hsv[..., 0], hsv[..., 1]
    colored = sat > 60
    ink_hue = float(np.median(h[colored])) if colored.any() else 0.0
    diff = np.abs(h - ink_hue)
    diff = np.minimum(diff, 255 - diff)
    other = (colored & (diff > 30)).mean(axis=0)
    cols = np.where(other > 0.3)[0]
    w = hsv.shape[1]
    if len(cols) == 0:
        return 0.0, 0.0
    return cols.min() / w + 0.003, cols.max() / w - 0.003   # mírně dovnitř, ať nezůstane rámeček původní barvy


def recolor(src, ink_rgb, holo_from, holo_to, strength=1.0):
    """Duotón: každý pixel se namíchá mezi barvou papíru a novou barvou inkoustu podle tmavosti."""
    img = Image.open(src).convert("RGB")
    if holo_from is None:
        holo_from, holo_to = find_hologram(img)
        print(f"  hologram: {holo_from:.3f}-{holo_to:.3f} sirky")
    rgb = np.array(img).astype(np.float32)
    lum = rgb @ np.array([0.299, 0.587, 0.114], dtype=np.float32)

    paper = np.median(rgb[lum > np.percentile(lum, 70)], axis=0)        # barva papíru
    paper_l = float(paper @ np.array([0.299, 0.587, 0.114]))
    dark_l = float(np.percentile(lum, 1))                               # nejtmavší inkoust

    t = np.clip((paper_l - lum) / max(1.0, paper_l - dark_l), 0, 1) ** 0.9 * strength
    t = t[..., None]
    ink = np.array(ink_rgb, dtype=np.float32)
    out = paper * (1 - t) + ink * t

    # zachovat jemnou texturu papíru (rozdíl jasu proti mediánu) a hologram beze změny
    out += (lum - paper_l)[..., None] * 0.15 * (1 - t)
    # hologram celý z originálu (okraj kolem něj se přebarví spolu s rámečkem)
    w = rgb.shape[1]
    a, b = int(w * holo_from), int(w * holo_to)
    out[:, a:b] = rgb[:, a:b]
    return Image.fromarray(np.clip(out, 0, 255).astype(np.uint8), "RGB")


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", required=True)
    ap.add_argument("--value", type=int, required=True)
    ap.add_argument("--ink", required=True, help="barva inkoustu R,G,B (nejtmavší tón), např. 20,95,100")
    ap.add_argument("--holo", default="auto", help="poloha hologramu: auto, nebo zlomek šířky od,do (např. 0.77,0.84)")
    a = ap.parse_args()

    if a.holo == "auto":
        hf, ht = None, None
    else:
        hf, ht = (float(x) for x in a.holo.split(","))
    out = recolor(a.src, tuple(int(x) for x in a.ink.split(",")), hf, ht)
    path = os.path.join(ROOT, "money_images", "raw", f"{a.value}.png")
    out.save(path)
    print("OK:", os.path.relpath(path, ROOT))
