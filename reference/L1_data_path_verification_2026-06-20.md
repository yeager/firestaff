# L1 — Verifiering av alternativa READY-path:er (2026-06-20)

Bakgrund: efter Tier 1 #4 (`reference/extract-game-archives.sh`)
producerade 71 nya version-staging-kataloger under
`~/.firestaff/data/<game>-extras/<version>/`. Fyra av dessa
matchar nu en kanonisk hash och visas som READY i default-scan.

Frågan Tier 1 #5 ställer: fungerar dessa path:er som fristående
launch-källor med `--data-dir <path>`?

## Resultat

| Path | Förväntat | Faktiskt scan-resultat |
|---|---|---|
| `dm1-extras/legacy-dos` | READY (DM1) | ✅ READY, `FOUND .../DATA/GRAPHICS.DAT` |
| `csb-extras/legacy-amiga-dms` | READY (CSB) | ✅ READY, `FOUND ...Meynaf/DungeonMaster/Graphics.DAT` |
| `nexus-extras/saturn-ja` | READY (Nexus) | ❌ MISSING — filnamnsmatchning |
| `theron-extras/japan` | READY (Theron) | ❌ MISSING — filnamnsmatchning |
| `dm1-extras/pc-3.4-en-3.5in` | READY (DM1) | ❌ MISSING — fel katalogstruktur |

## Vad detta betyder

### Fungerar (DM1 + CSB legacy path:er)
DM1 PC 3.4 (legacy-dos) och CSB Amiga 3.3 Meynaf FR är genuint
"READY" — `--data-dir` mot katalogen hittar matchande hashar i
`GRAPHICS.DAT`/`Dungeon.DAT` på disk. Dessa kan användas som
real-asset launch-källor.

### Inte fristående (Nexus + Theron container-binärer)
Nexus Saturn JA (`Track 1.bin`) och Theron JP/US (`Track 02.bin`)
har rätt MD5 (Nexus Track 1: `d83623212c7fd61623377cc9074bf3ea`,
Theron JP Track 02: `b7afb338ad31be1025b53f9aff12d73a`) men
`--data-dir` ensamt mot katalogen hittar dem inte. Orsak:

- Scanner letar efter specifika filnamn i `g_nexusArchiveNames` /
  `g_theronTrack02Names` (`DM.BIN`, `SEGADATA.BIN`,
  `Theron's Quest (Japan) (Track 02).bin`, etc.).
- Våra extraktioner behåller source-filenamnet
  (`Dungeon Master Nexus (Japan) (Track 1).bin`,
  `Dungeon Master - Theron's Quest (Japan) (Track 02).bin`).
- I **default-läget** (utan `--data-dir`) hittar scannern dem ändå
  genom att rekursivt söka igenom ALLA kataloger — men då visar
  `Data dir:` root-katalogen, inte den specifika path:en.

### Workaround (tills fix)

```bash
# Nexus: peka --data-dir mot parent-dir
./build/firestaff --data-dir ~/.firestaff/data/nexus-extras/saturn-ja/..  --scan-data
# Eller: skapa symlink med rätt namn
ln -sf "...Track 1).bin" /some/path/DM.BIN
./build/firestaff --data-dir /some/path --scan-data
```

## Gap-status efter L1

| Path | Status före L1 | Status efter L1 |
|---|---|---|
| DM1 PC 3.4 (legacy-dos) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| CSB Amiga 3.3 (Meynaf FR) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| Nexus Saturn JA (Track 1) | EXTRACTED + VERIFIED | EXTRACTED + VERIFIED (default-scope) — behöver path-rename för `--data-dir` |
| Theron JP Track 02 | EXTRACTED + VERIFIED | EXTRACTED + VERIFIED (default-scope) — behöver path-rename för `--data-dir` |
| DM1 PC 3.4 English 3.5" | EXTRACTED (extras) | EXTRACTED (extras, .raw-format som scanner ej mappar) |

## Öppna följdfrågor

1. **`extract-game-archives.sh` L3**: skapa canonical-namnade
   symlinks/aliases för Track 02 .bin → `track02.bin` etc. så
   `--data-dir` fungerar direkt utan workaround.
2. **Tier 1 #5 strict definition**: launch-test ska också kräva
   `m11_phase_a --game X --data-dir <path>` PASS, inte bara
   `--scan-data` READY. Nexus + Theron path:er behöver detta
   också efter alias-steget.
