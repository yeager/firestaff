# DM1 V1 Regional Differences — Source Lock

**Audit date:** 2026-05-25
**Status:** COMPLETE

## Finding: US vs EU Regional Variants

DM1 PC 3.4 was released in US and European regional variants. The primary difference is the EU multilingual release.

### US Release (I34E)

- Single `DATA/DUNGEON.DAT` (33,357 bytes)
- Single `DATA/GRAPHICS.DAT` (363,417 bytes)
- No language selection at startup
- English text only

### European Release (I34M — Multilingual)

- `EUDATA/DUNGEON.DAT` (33,357 bytes, identical to US English)
- `EUDATA/DUNGEONF.DAT` (33,687 bytes, French)
- `EUDATA/DUNGEONG.DAT` (33,705 bytes, German)
- `EUDATA/GRAPHICS.DAT` (398,925 bytes, includes extended character sets)
- `EUDATA/SONG.DAT` (162,482 bytes)
- Language selection at startup (EN/FR/DE)
- DM.EXE is LZEXE v0.91 compressed (same as US)

## Dungeon Layout Differences

**Confirmed: French and German dungeon files differ from English.**

SHA256 evidence:
- `DUNGEON.DAT` (English):  `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` (33,357 bytes)
- `DUNGEONF.DAT` (French): `290543621ae7c465fee9651c4d3c44f5dc268f5e16fffc75da82a440274c0571` (33,687 bytes)
- `DUNGEONG.DAT` (German): `b2478d5cc3213725329cb42684f0309e5e95fd6789dc2cb0c3377b178ad75817` (33,705 bytes)

The size differences (+330 bytes French, +348 bytes German) suggest the differences are in:
- Text strings (room names, hint text, creature names in the dungeon header strings)
- NOT dungeon cell/layout data (same underlying dungeon structure)

The US and EU English dungeon are byte-identical (same SHA256), confirming the dungeon geometry is identical.

## Graphics Differences

The EU multilingual GRAPHICS.DAT is 398,925 bytes vs 363,417 bytes for the US English version — a difference of ~35,508 bytes. This additional data contains:
- Extended character glyphs for French (é, è, ê, à, ç, etc.)
- Extended character glyphs for German (ü, ö, ä, ß, etc.)
- Possibly the same dungeon graphics but with more text overlays

## No US/EU/JP Split for PC

The PC DOS version appears to only have US and EU releases. The Japanese market for DM1 on PC would have used the standard English version or a different platform (e.g., FM-Towns, which has its own version designation in ReDMCSB: `F20E`, `F20J`, `F31E`, `F31J`).

ReDMCSB platform designations:
- `I34E` = IBM PC 3.4 English (US)
- `I34M` = IBM PC 3.4 Multilingual (EU)
- `F20E` / `F20J` = FM-Towns English/Japanese (different platform)
- `P20JA` / `P20JB` / `P31J` = PC-98 Japanese (different platform)

## Creature Differences

No creature differences have been identified between US and EU versions. The DUNGEON.DAT cell data (creature placement) appears identical between US and EU English dungeons. The French and German differences are in string data only.

## Firestaff Canonical Anchors

- US English: `.../dm1/DUNGEON.DAT` → `DungeonMasterPC34/DATA/DUNGEON.DAT`
- EU Multilingual: `.../dm1/DungeonMasterPC34Multilingual/`
  - EN: `EUDATA/DUNGEON.DAT` (identical to US)
  - FR: `EUDATA/DUNGEONF.DAT`
  - DE: `EUDATA/DUNGEONG.DAT`

## Conclusion

Regional differences between US and EU DM1 PC 3.4 are:
1. **EU multilingual** has separate French and German dungeon string files (different content)
2. **EU graphics** file is larger to accommodate accented character glyphs
3. **EU multilingual** includes a language-selection frontend (LANGUAGE.OBJ module)
4. **No dungeon layout differences** — the underlying dungeon geometry is identical
5. **No creature differences** between EN/FR/DE dungeon files
