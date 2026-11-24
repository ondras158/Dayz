#!/usr/bin/env python3
"""Převede vlastní obrázky bankovek na textury pro mód OS_Core.

1. Dej obrázky do složky money_images/ a pojmenuj je podle hodnoty: 5.png, 10.jpg, 20.png, 500.jpg ...
   (formáty: png, jpg, jpeg, webp, bmp; libovolná velikost a poměr stran)
2. Spusť:  python3 tools/import_money_images.py
3. Vzniknou OS_Core/data/money_<hodnota>_co.png -> převeď je v DayZ Tools na .paa (ImageToPAA) a přebal mód.

Volby:
  --mode fit      celý obrázek se vejde, okraje se doplní barvou z okraje bankovky (výchozí)
  --mode fill     obrázek vyplní celou texturu, přečnívající části se oříznou
  --mode stretch  obrázek se roztáhne (zdeformuje) na celou texturu
  --rotate 90     otočí obrázek (pokud se ve hře zobrazí bokem)
  --size 1024     velikost textury (512 nebo 1024; větší = ostřejší, ale víc paměti)
  --mark TEXT     přes bankovku vepíše text (např. SPECIMEN)
"""
import argparse
import os
import sys

from PIL import Image, ImageDraw, ImageFont, ImageOps, ImageStat

ROOT = os.path.join(os.path.dirname(__file__), "..")
SRC = os.path.join(ROOT, "money_images")
OUT = os.path.join(ROOT, "OS_Core", "data")
SUPPORTED = [1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000]
EXTS = (".png", ".jpg", ".jpeg", ".webp", ".bmp")
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"


def edge_color(img):
    """průměrná barva okraje - na doplnění prázdných míst"""
    w, h = img.size
    strip = 4
    parts = [img.crop((0, 0, w, strip)), img.crop((0, h - strip, w, h)),
             img.crop((0, 0, strip, h)), img.crop((w - strip, 0, w, h))]
    means = [ImageStat.Stat(p).mean for p in parts]
    return tuple(int(sum(m[i] for m in means) / len(means)) for i in range(3))


def convert(path, size, mode, rotate, mark):
    img = Image.open(path)
    img = ImageOps.exif_transpose(img).convert("RGB")
    if rotate:
        img = img.rotate(-rotate, expand=True)

    if mode == "stretch":
        out = img.resize((size, size), Image.LANCZOS)
    elif mode == "fill":
        out = ImageOps.fit(img, (size, size), Image.LANCZOS)
    else:
        out = Image.new("RGB", (size, size), edge_color(img))
        fitted = ImageOps.contain(img, (size, size), Image.LANCZOS)
        out.paste(fitted, ((size - fitted.width) // 2, (size - fitted.height) // 2))

    if mark:
        layer = Image.new("RGBA", out.size, (0, 0, 0, 0))
        d = ImageDraw.Draw(layer)
        try:
            fnt = ImageFont.truetype(FONT, size // 8)
        except OSError:
            fnt = ImageFont.load_default()
        l, t, r, b = d.textbbox((0, 0), mark, font=fnt)
        d.text(((size - (r - l)) / 2 - l, (size - (b - t)) / 2 - t), mark, font=fnt, fill=(200, 30, 30, 150))
        layer = layer.rotate(30, resample=Image.BICUBIC)
        out = Image.alpha_composite(out.convert("RGBA"), layer).convert("RGB")
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", default=SRC, help="složka s obrázky (výchozí money_images/)")
    ap.add_argument("--mode", choices=["fit", "fill", "stretch"], default="fit")
    ap.add_argument("--rotate", type=int, choices=[0, 90, 180, 270], default=0)
    ap.add_argument("--size", type=int, choices=[512, 1024], default=512)
    ap.add_argument("--mark", default="", help="text přes bankovku, např. SPECIMEN")
    a = ap.parse_args()

    found = 0
    for name in sorted(os.listdir(a.src)):
        stem, ext = os.path.splitext(name)
        if ext.lower() not in EXTS:
            continue
        if not stem.isdigit() or int(stem) not in SUPPORTED:
            print(f"Preskakuji {name}: nazev musi byt hodnota bankovky ({', '.join(map(str, SUPPORTED))})")
            continue
        out = convert(os.path.join(a.src, name), a.size, a.mode, a.rotate, a.mark)
        target = os.path.join(OUT, f"money_{int(stem)}_co.png")
        out.save(target)
        found += 1
        print(f"OK: {name} -> {os.path.relpath(target, ROOT)}")

    if not found:
        sys.exit(f"Ve slozce {a.src} nejsou zadne obrazky typu 20.png, 50.jpg ...")
    print("Hotovo. Ted preved PNG na .paa (DayZ Tools -> ImageToPAA) a prebal mod.")


if __name__ == "__main__":
    main()
