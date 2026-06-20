#!/usr/bin/env bash
# dmweb.free.fr /community/documentation/ - hämtar alla sidor med curl.
# Respektfullt mot free.fr: User-Agent identifiering + 1.2s fördröjning mellan requests.
# Kör från denna katalog: ./crawl.sh

set -euo pipefail

UA="FirestaffDocArchiver/1.0 (+https://github.com/yeager/firestaff; offline docs mirror)"
ROOT="http://dmweb.free.fr"
DOC_PREFIX="/community/documentation/"
SITEMAP="${ROOT}/sitemap.xml"
DELAY=1.2

cd "$(dirname "$0")"
mkdir -p html

echo "=== [$(date -u +%FT%TZ)] Crawl start ===" | tee -a SCRAPE_LOG.md

# 1) Hämta sitemap om vi inte redan har den
if [ ! -s html/_sitemap.xml ]; then
  echo "Hämtar sitemap..." | tee -a SCRAPE_LOG.md
  curl -sL --compressed -A "$UA" "$SITEMAP" -o html/_sitemap.xml
fi

# 2) Extrahera URL-listan
python3 - <<PY
import re, json
s = open('html/_sitemap.xml', encoding='utf-8', errors='replace').read()
urls = re.findall(r'<loc>([^<]+)</loc>', s)
doc = sorted(u for u in urls if '${DOC_PREFIX}' in u)
with open('_urls.txt', 'w') as f:
    for u in doc:
        f.write(u + '\n')
print(f'Hittade {len(doc)} URL:er under ${DOC_PREFIX}')
PY

# 3) Hämta varje sida
i=0
total=$(wc -l < _urls.txt | tr -d ' ')
while IFS= read -r url; do
  i=$((i+1))
  # bygg lokal filsökväg: html/community/documentation/<slug>.html
  path=${url#${ROOT}}
  # Ta bort avslutande slash för att undvika problem
  path=${path%/}
  local="html${path}.html"
  mkdir -p "$(dirname "$local")"

  printf "[%2d/%2d] %s\n" "$i" "$total" "$url" | tee -a SCRAPE_LOG.md

  http_code=$(curl -sL --compressed -A "$UA" \
    -w "%{http_code}" -o "$local" \
    "$url" || echo "000")

  size=$(wc -c < "$local" 2>/dev/null | tr -d ' ' || echo 0)
  echo "  -> $http_code, ${size} bytes, $local" >> SCRAPE_LOG.md

  sleep "$DELAY"
done < _urls.txt

# 4) Generera index
python3 build_index.py

echo "=== [$(date -u +%FT%TZ)] Crawl klar ===" | tee -a SCRAPE_LOG.md
echo "Resultat:"
echo "  Sidor hämtade: $(find html/community -type f -name '*.html' | wc -l | tr -d ' ')"
echo "  Total storlek: $(du -sh html/community | awk '{print $1}')"
echo "  INDEX:    INDEX.md"
echo "  JSON:     index.json"
echo "  Logg:     SCRAPE_LOG.md"
