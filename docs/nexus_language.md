# Nexus V1 — Language Variants

## Sources
- `docs/nexus_text.md`
- `docs/nexus_champions.md`
- `docs/nexus_features.md`
- `docs/nexus_overview.md`
- `docs/NEXUS_PLAN.md`
- `src/nexus/nexus_v1_saturn_font.c`, `nexus_v1_text.c`
- `docs/NEXUS_FILE_CLASSIFICATION.md`
- Extracted Saturn files: `DMN_ABS.TXT`, `DMN_BIB.TXT`, `DMN_CPY.TXT`, `FONT256.S2D`

---

## 1. Only Japanese — No English Release

Dungeon Master Nexus was **Japanese language only**. There was no English localization — it was never released outside Japan.

### Evidence
- Disc product: T-9111G V1.003 — the G suffix indicates Japan (NTSCJ territory)
- All UI text, menus, champion names, inscriptions are in Japanese
- `DMN_BIB.TXT` (in-game text file) contains only Japanese Shift-JIS characters
- `FONT256.S2D` is a 256-character font including Japanese katakana
- `docs/nexus_overview.md`: "Exclusive to Sega Saturn, Japanese only"

### In-Game Text Samples
`DMN_ABS.TXT` content (Shift-JIS, translated):
> "The Dungeon Master will show you the way. The Ruin of this fortress has stood for centuries. The Dungeon Master and his minions wait within its depths..."

`DMN_BIB.TXT` copyright notice:
> "Copyright 1998 Victor Interactive Software Inc. / FTL Games"

---

## 2. Font and Text Rendering

### FONT256.S2D — Sega Saturn SCR Font Format
A 256-character bitmap font with Japanese support:
- **Magic:** "SEGA SATURN SCR" (15 bytes)
- **Glyph sizes:** 8×8, 12×12, or 16×16 px auto-detected
- **Charset:** ASCII 0x20–0x7E + half-width katakana (0xA1–0xDF)
- **Japanese mapping:** Half-width katakana → UTF-8 U+FF61–U+FF9F

### Text Conversion: Shift-JIS to UTF-8
From `nexus_v1_text.c`:
- ASCII pass-through (0x20–0x7E → 1 byte → 1 byte)
- Half-width katakana (0xA1–0xDF → 3-byte UTF-8)
- No full-width kanji in the font (katakana only for UI/names)

### No Kanji in Font
The font is 256 chars total — insufficient for full kanji (thousands needed).
Nexus uses **katakana transliteration** for Japanese text. Champion names are
rendered in katakana (e.g., "シラ" = Syra, "レイラ" = Leyla, "ナビ" = Nabi).

---

## 3. Champion Roster: Japanese Names

From `src/nexus/nexus_v1_champions.c`, the 24-champion roster uses Japanese names exclusively for the in-game display:

| # | ASCII Name | Japanese | Class |
|---|-----------|---------|-------|
| 1 | Syra | シラ | Fighter |
| 2 | Leyla | レイラ | Wizard |
| 3 | Nabi | ナビ | Ninja |
| 4 | Gando | ガンド | Priest |
| 5 | Torham | トルハム | Fighter |
| 6 | Elija | エリジャ | Wizard |
| 7 | Wu Tse | ウー・ツエ | Ninja |
| 8 | Stamm | スタム | Fighter |

The ASCII names exist in the data as a parallel field (for debug/encoding compatibility), but the game displays only the Japanese katakana versions.

---

## 4. No Multilingual Release

Unlike DM1 CSB (which had separate Japanese GRAPHICS.DAT files for the JP localization) or DM2 (which at least had English-only binaries that could run on any locale), Nexus had:

| Aspect | DM1 CSB | DM2 | Nexus |
|--------|---------|-----|-------|
| English version | Yes | Yes | **No** |
| Japanese version | Yes (separate DAT files) | No | **Yes (only)** |
| Language variants | EN + JP | EN only | **JP only** |
| Multilingual binary | No (separate files) | N/A | **N/A** |

---

## 5. Language Conclusion

**One language: Japanese. One territory: Japan. One release: Saturn.**
- No English text, no English menus, no English UI
- Font supports ASCII + half-width katakana (no kanji)
- All champion names in katakana
- All in-game text in Japanese
- The Firestaff reimplementation uses the same Japanese data but renders via Unicode/SDL (UTF-8 output)
