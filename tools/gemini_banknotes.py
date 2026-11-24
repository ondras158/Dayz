#!/usr/bin/env python3
"""Vygeneruje realistické podklady bankovek přes Gemini API (bez textu) a uloží je do money_images/raw/.

Klíč se čte z proměnné prostředí GEMINI_API_KEY (nikdy ho neukládej do repozitáře):
    export GEMINI_API_KEY=...
    python3 tools/gemini_banknotes.py --values 20
    python3 tools/gemini_banknotes.py --values 5,10,20,50,100,200,500

Pak nápisy (hodnota, název serveru, měna) přidá:  python3 tools/finish_banknotes.py
"""
import argparse
import base64
import json
import os
import sys
import urllib.request

MODEL = "gemini-3.1-flash-image"
ROOT = os.path.join(os.path.dirname(__file__), "..")
RAW = os.path.join(ROOT, "money_images", "raw")

COLORS = {
    1: "neutral grey", 2: "beige", 5: "silver grey with a hint of green",
    10: "red and rose", 20: "blue", 50: "orange and amber", 100: "green",
    200: "yellow and ochre", 500: "purple and violet", 1000: "sandy brown",
    2000: "teal", 5000: "wine red",
}

PROMPT = """Flat, perfectly straight front view of a fictional, realistic paper banknote that fills the ENTIRE image edge to edge, landscape orientation.
Style of modern European currency printing: fine intaglio engraving, guilloche rosettes, microprint line borders, a vertical holographic foil strip near the right edge, an oval watermark window on the right, a dark security thread.
Central motif: a detailed engraved desert landscape - rolling sand dunes, a large rising sun with rays, a lone weathered wooden watchtower on a dune, and a dusty road.
Dominant colour palette: {color} ink on light cream paper, subtle paper texture.
Composition rules: keep a clean, plain, light area along the top centre (for a title), in the top-left corner and in the bottom-right corner (for value numbers), and a plain band along the bottom centre.
ABSOLUTELY NO text, letters, words, numbers or digits anywhere on the banknote. Not a photograph of an object: no hands, no table, no background, no shadow, no perspective."""


def generate(key, value, out_path, model=MODEL):
    body = {
        "contents": [{"parts": [{"text": PROMPT.format(color=COLORS.get(value, "sandy brown"))}]}],
        "generationConfig": {"responseModalities": ["IMAGE"], "imageConfig": {"aspectRatio": "21:9"}},
    }
    req = urllib.request.Request(
        f"https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent",
        data=json.dumps(body).encode(),
        headers={"Content-Type": "application/json", "x-goog-api-key": key},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=300) as resp:
        data = json.load(resp)

    for cand in data.get("candidates", []):
        for part in cand.get("content", {}).get("parts", []):
            inline = part.get("inlineData") or part.get("inline_data")
            if inline and inline.get("data"):
                with open(out_path, "wb") as f:
                    f.write(base64.b64decode(inline["data"]))
                return True
    print("Odpoved bez obrazku:", json.dumps(data)[:800], file=sys.stderr)
    return False


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--values", default="5,10,20,50,100,200,500")
    ap.add_argument("--model", default=MODEL, help="např. gemini-3.1-flash-image, gemini-2.5-flash-image, gemini-3-pro-image")
    a = ap.parse_args()

    key = os.environ.get("GEMINI_API_KEY")
    if not key:
        sys.exit("Nastav promennou GEMINI_API_KEY")

    os.makedirs(RAW, exist_ok=True)
    for v in [int(x) for x in a.values.split(",")]:
        path = os.path.join(RAW, f"{v}.png")
        print(f"Generuji {v}...", flush=True)
        try:
            ok = generate(key, v, path, a.model)
        except urllib.error.HTTPError as e:
            print(f"  CHYBA {e.code}: {e.read()[:500]!r}", file=sys.stderr)
            continue
        print("  OK:", os.path.relpath(path, ROOT) if ok else "  nic")
