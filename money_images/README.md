# Vlastní obrázky bankovek

Sem dej obrázky bankovek pojmenované podle hodnoty: `5.png`, `10.jpg`, `20.png`, `50.png`, `100.jpg`, `200.png`, `500.png` …
(podporované hodnoty: 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000).

Pak spusť `python3 tools/import_money_images.py` – přepíše `OS_Core/data/money_<hodnota>_co.png`.
Chybějící hodnoty zůstanou s vygenerovaným potiskem.
