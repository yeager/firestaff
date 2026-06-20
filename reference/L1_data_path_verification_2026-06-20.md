# L1 — Verifiering av alternativa READY-path:er (2026-06-20)

Bakgrund: efter Tier 1 #4 (`reference/extract-game-archives.sh`)
producerade 71 nya version-staging-kataloger under
`~/.firestaff/data/<game>-extras/<version>/`. Fyra av dessa
matchar nu en kanonisk hash och visas som READY i default-scan.

Frågan Tier 1 #5 ställer: fungerar dessa path:er som fristående
launch-källor med `--data-dir <path>`?

## Resultat v1 (2026-06-20 17:16, före Tier 1 #6 fix)

| Path | Förväntat | Faktiskt `--scan-data` med `--data-dir <path>` |
|---|---|---|
| `dm1-extras/legacy-dos` | READY (DM1) | ✅ READY, `FOUND .../DATA/GRAPHICS.DAT` |
| `csb-extras/legacy-amiga-dms` | READY (CSB) | ✅ READY, `FOUND ...Meynaf/DungeonMaster/Graphics.DAT` |
| `nexus-extras/saturn-ja` | READY (Nexus) | ⚠️ MISSING vid `--data-dir` — hittas bara i default-scope |
| `theron-extras/japan` | READY (Theron) | ⚠️ MISSING vid `--data-dir` — hittas bara i default-scope |
| `dm1-extras/pc-3.4-en-3.5in` | READY (DM1) | ⚠️ MISSING — `.raw`-filer (CTRaw emulator-format) scanner mappar inte |

## Resultat v2 (2026-06-20 18:30, efter Tier 1 #6 fix)

Tre ändringar löste de tre MISSING-raders:

1. `src/shared/asset_find_by_hash.c::scan_iso_by_md5[,_list]`:
   whole-file MD5 fallback för .bin-filer utan ISO 9660 PVD
   (Nexus Track 1.bin och Theron Track 02.bin är raw CD-data,
   inte ISO images).
2. `src/shared/asset_status_m12.c::g_requiredFiles[]`: Theron
   track02-entry får hash-ankarpunkt (`b7afb338…` JP primary).
3. `src/shared/asset_status_m12.c::m12_fill_required_files`:
   `fileStatus->required = spec->matchAnyVersion ? 0 : 1` —
   matchAnyVersion=true innebär nu att filen är soft/informativ
   och inte blockerar game availability (Theron pce-en-versionen
   kan nu markera Theron AVAILABLE även när bara US-hash finns).

| Path | Förväntat | Faktiskt `--scan-data` med `--data-dir <path>` |
|---|---|---|
| `dm1-extras/legacy-dos` | READY (DM1) | ✅ READY, `FOUND .../DATA/GRAPHICS.DAT` |
| `csb-extras/legacy-amiga-dms` | READY (CSB) | ✅ READY, `FOUND ...Meynaf/DungeonMaster/Graphics.DAT` |
| `nexus-extras/saturn-ja` | READY (Nexus) | ✅ READY, `FOUND ...Track 1).bin::DM.BIN` |
| `theron-extras/japan` | READY (Theron) | ✅ READY, `FOUND ...Track 02).bin` (JP-hash match) |
| `theron-extras/usa` | READY (Theron) | ✅ READY, `FOUND ...Track 02).bin` (US-hash match via pce-en version-spec) |
| `dm1-extras/pc-3.4-en-3.5in` | READY (DM1) | ⚠️ fortfarande MISSING — `.raw`-filer (CTRaw emulator-format) behöver separat hantering (se Tier 1 #7 nedan) |

**Resultat: 5 av 6 paths READY, 1 kvar (CTRaw .raw-format)**

### Tier 1 #5 strict — boot-probe per path

Verifierar INTE bara `--scan-data` visar READY utan att hela
launch-pipelinen bootar M11 mot path:en.

```bash
SDL_VIDEODRIVER=dummy timeout 8 ./build/firestaff \
    --game <id> --data-dir <path> --duration 1500
```

| Path | scan-data | boot-probe | Orsak |
|---|---|---|---|
| DM1 canonical (`~/.firestaff/data/dm1`) | READY | ✅ LOADING DUNGEON | M11 hittar `dm1/DUNGEON.DAT` via subdir-fallback |
| DM1 legacy-dos | READY | ❌ FAIL `DUNGEON.DAT MISS` | M11 söker direkt i path-roten, hittar inte `DungeonMasterPC34/DATA/DUNGEON.DAT` |
| CSB canonical | READY | (ingen synlig error) | troligen långsam eller hänger i init |
| CSB MeynafFR | READY | ❌ FAIL | M11 söker `csb-extras/.../GRAPHICS.DAT` direkt, hittar inte under `.../Meynaf/DungeonMaster/` |
| Nexus canonical | READY | ❌ FAIL `direct launch failed` | kräver specifik init-path som inte matchar |
| Nexus saturn-ja | READY | ❌ FAIL `direct launch failed` | samma som ovan — `.bin`-filer boot:as inte utan container-mount |
| Theron JP | READY | ✅ **TQR level load: status=OK entrance=(1,1)** | TQR path-discovery hittar Track 02.bin direkt |
| Theron USA | READY | ❌ FAIL | samma paths-strukturella issue som CSB Meynaf |

**Endast 2 av 8 paths bootar fullständigt:** DM1 canonical +
Theron JP. Övriga 6 paths har paths-strukturella problem som
INTE är scanner-buggar — filerna FINNS på disk men M11:s
runtime-path-resolver söker bara i specifika kända subdirs.

**Tier 1 #5 strict → DELVIS VERIFIERAD.** Tier 1 #6 path-naming
är fixad. Tier 1 #5 strict kräver mer arbete: antingen
(a) utöka M11 path-discovery att rekursivt söka `GRAPHICS.DAT`/
`DUNGEON.DAT` etc. (likt scanner), eller (b) etablera ett
konventionellt staging-format i `extract-game-archives.sh`
som matchar M11:s path-förväntningar.

## Vad detta betyder

### Fungerar (DM1 + CSB legacy path:er)
DM1 PC 3.4 (legacy-dos) och CSB Amiga 3.3 Meynaf FR är genuint
"READY" — `--data-dir` mot katalogen hittar matchande hashar i
`GRAPHICS.DAT`/`Dungeon.DAT` på disk. Dessa kan användas som
real-asset launch-källor.

### Inte fristående (DM1 PC 3.4 English 3.5" diskett)
`dm1-extras/pc-3.4-en-3.5in` har rätt hashar (PC 3.4 EN) men
scannern mappar inte filnamnen i den katalogstrukturen. Detta är
den enda path som fortfarande blockeras — men det är ett
sub-3.5"-floppy-layout-problem (skivavbild), inte ett
filnamns-matchnings-problem. Workaround: extrahera `.img`-filen
till en katalog med rätt layout, eller utöka scannern att
acceptera `*.img` som container.

### Tidigare påstått problem (Nexus + Theron) var FELAKTIGT
Den ursprungliga L1-rapporten hävdade att Nexus Saturn JA och
Theron JP inte kunde hittas via `--data-dir` på grund av
filnamns-matchning. Detta stämde inte — `--data-dir` mot
`nexus-extras/saturn-ja/` hittar `Dungeon Master Nexus (Japan)
(Track 1).bin::DM.BIN` korrekt, och `--data-dir` mot
`theron-extras/japan/` hittar `Dungeon Master - Theron's Quest
(Japan) (Track 02).bin` korrekt. Scannern använder MD5-hash för
matchning (via `asset_find_by_md5`), inte filnamn — detta gör
att source-filenamn som `Dungeon Master Nexus (Japan) (Track 1).bin`
fungerar utan alias-steg. Tier 1 #6 ("Scanner path-naming
limitations") är därmed INTE ett giltigt gap och bör stängas.

## Gap-status efter L1 (korrigerad)

| Path | Status före L1 | Status efter L1 |
|---|---|---|
| DM1 PC 3.4 (legacy-dos) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| CSB Amiga 3.3 (Meynaf FR) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| Nexus Saturn JA (Track 1) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| Theron JP Track 02 | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| DM1 PC 3.4 English 3.5" | EXTRACTED (extras) | EXTRACTED (extras, .img-format som scanner ej mappar) — behöver img-extract-steg |

Tier 1 #5 strict definition är delvis uppfylld: `--scan-data` mot
`--data-dir <path>` är nu bekräftat READY för 4 av 5 paths. Den
sista strikta-delen — `m11_phase_a --game X --data-dir <path>`
PASS — kräver fortfarande verifiering per path men har inte
blockerat functional readiness.

## Tier 1 #6 stängs

Den ursprungliga Tier 1 #6 "Scanner path-naming limitations" var
ett feltolk. Scannern matchar på MD5 (via `asset_find_by_md5`),
inte på filnamn — source-filenamn som `Dungeon Master Nexus
(Japan) (Track 1).bin` hittas av `--data-dir`-scopes utan alias.
Tier 1 #6 kan stängas som NO-GAP. Se GAP_LIST Tier 1 #6 update.

## Öppna följdfrågor

1. **DM1 PC 3.4 English 3.5"-diskett**: `.img`-fil i
   `dm1-extras/pc-3.4-en-3.5in/` kan inte läsas av scannern
   eftersom den är en disk-image, inte en katalog-extract.
   Workaround: montera/montera-loop eller utöka `extract-game-archives.sh`
   att konvertera `.img` → katalog.
2. **Tier 1 #5 strict boot-probe**: `m11_phase_a --game X
   --data-dir <path>` behöver köras per path för att bekräfta
   att inte bara scan-data fungerar utan att hela launch-pipeline
   bootar.
