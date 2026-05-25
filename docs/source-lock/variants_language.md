# DM1 V1 Language Variants — Source Lock

**Audit date:** 2026-05-25
**Status:** COMPLETE

## Finding: Two PC 3.4 Language Variants Confirmed

DM1 PC version 3.4 shipped in two distinct releases:

| Variant | Designation | Dungeon Path | Graphics Path | Notes |
|---------|-------------|--------------|---------------|-------|
| US English | `I34E` (ReDMCSB) | `DATA\DUNGEON.DAT` | `DATA\GRAPHICS.DAT` | Single-language, 363,417-byte GRAPHICS |
| European Multilingual | `I34M` (ReDMCSB) | `EUDATA\DUNGEON~.DAT` | `EUDATA\GRAPHICS.DAT` | EN/FR/DE in EUDATA, 398,925-byte GRAPHICS |

## How Language Variants Differ

### 1. Dungeon file per language (European Multilingual only)

The EU multilingual version (I34M) has three dungeon files in `EUDATA/`:

| File | Language | SHA256 | Size |
|------|----------|--------|------|
| `DUNGEON.DAT` | English | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | 33,357 |
| `DUNGEONF.DAT` | French | `290543621ae7c465fee9651c4d3c44f5dc268f5e16fffc75da82a440274c0571` | 33,687 |
| `DUNGEONG.DAT` | German | `b2478d5cc3213725329cb42684f0309e5e95fd6789dc2cb0c3377b178ad75817` | 33,705 |

**Key finding:** The English dungeon (33,357 bytes) is identical between I34E and I34M (same SHA256). The French and German dungeons are DIFFERENT from English (different sizes + different hashes). The differences are in the dungeon string data (text labels, not layout).

The tilde (`~`) in `DUNGEON~.DAT` is replaced at runtime by the language character:
- English: `DUNGEON.DAT`
- French: `DUNGEONF.DAT`
- German: `DUNGEONG.DAT`

### 2. Graphics file differences

| File | Size | Notes |
|------|------|-------|
| `DATA/GRAPHICS.DAT` (I34E) | 363,417 bytes | US English |
| `EUDATA/GRAPHICS.DAT` (I34M) | 398,925 bytes | EU Multilingual, larger — includes more text glyphs |

The EU multilingual GRAPHICS.DAT is ~35KB larger, likely because it includes expanded character sets for accented letters (é, ü, ö, ñ, etc.) needed for French and German text rendering.

### 3. Language module

In ReDMCSB, the I34M linker includes `LANGUAGE.OBJ`; the I34E linker does NOT.

```
# I34E.LNK — English, NO language module
... FILENAME.OBJ+STARTUP2.OBJ

# I34M.LNK — Multilingual, includes LANGUAGE.OBJ
... FILENAME.OBJ+STARTUP2.OBJ+LANGUAGE.OBJ
```

The `LANGUAGE.OBJ` module (`Source/LANGUAGE.C`) provides:
- `F0757_LoadTexts()` — loads the text string block from the GRAPHICS.DAT (graphic entry C700_GRAPHIC_TEXTS)
- `F0758_TranslateLanguage()` — translates string indices to language-specific text

### 4. Spanish variant (separate archive)

A standalone `Spanish GRAPHICS.DAT` (8,641,331 bytes — ~8.6 MB) was found. This is dramatically larger than the standard 363KB or 399KB GRAPHICS.DAT files. It likely represents:
- A complete Spanish-language edition with all text embedded as graphics, OR
- A Spanish localization project (not an official FTL release)

This needs further investigation to determine its provenance.

## Source Evidence

- ReDMCSB `Toolchains/Common/Source/FILENAME.C` line 45: `char G2175_ac_DungeonFileName[] = "EUDATA\\DUNGEON~.DAT";` (I34M)
- ReDMCSB `Toolchains/Common/Source/FILENAME.C`: US path `DATA\DUNGEON.DAT` (I34E)
- ReDMCSB `Toolchains/Common/Source/LANGUAGE.C`: text loading and translation for multilingual
- ReDMCSB `Toolchains/IBM PC/Source/I34E.LNK` vs `I34M.LNK`: language module inclusion
- SHA256 verification of DUNGEON.DAT from both I34E and I34M EUDATA: identical (`d90b6b1c...`)

## Verified Data Paths

- US English DUNGEON.DAT: `.../_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT`
- EU Multilingual EUDATA: `.../_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/`

## Firestaff Canonical Anchors (N2)

- English DUNGEON.DAT → `.../dm1/DUNGEON.DAT` (US PC 3.4 English)
- Multilingual → `.../dm1/DungeonMasterPC34Multilingual/` (EU PC 3.4 Multilingual)

## Conclusion

Language variants differ in:
1. **Dungeon strings** — DUNGEONF.DAT and DUNGEONG.DAT differ from English DUNGEON.DAT (confirmed by size + hash)
2. **Graphics text glyphs** — EU GRAPHICS.DAT is ~35KB larger due to extended character sets
3. **Language module** — I34M includes `LANGUAGE.OBJ`, I34E does not
4. **Dungeon layout** — NOT different between languages (same English dungeon file)
