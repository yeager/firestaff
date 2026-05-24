# DM1 V1 — Localization / i18n

## Source-locked
ReDMCSB: LANGUAGE.C, APPBLANG.C, DEFS.H (language constants), CEDTINC4.C
Firestaff: src/dm1/dm1_v1_palette_font_pc34_compat.c (font loading), po/ directory

## Language Selection

G2000_Language (LANGUAGE.C): holds current language index.
C0_ENGLISH is the base value (defined in DEFS.H language constants).

On startup, the game probes for localized data or uses default English.

## String Loading (LANGUAGE.C:19–47)

F0757_LoadTexts():
1. Allocate permanent memory for text block
2. Load graphic C700_GRAPHIC_TEXTS (decompressed) into allocated buffer
3. Scan byte-by-byte: each null byte marks end of a string; count total strings
4. Allocate array of char* (one pointer per string)
5. Walk text buffer again, filling pointer array with string start addresses

C700_GRAPHIC_TEXTS is a packed string table in GRAPHICS.DAT, compressed.
F0758_TranslateLanguage() does the lookup:
  if (index < 0 || index >= string_count) return "" else return pointer_array[index]

## Multi-Language Support by Platform

Atari ST: English, German, French, Spanish, Italian, Swedish — strings in GRAPHICS.DAT.
Amiga: Same multi-language approach via graphic 700.
PC-98 / FM-Towns / X68k: Japanese character support (JAPANESE.C exists in source).
Apple IIGS: Separate string handling.

CEDTINC4.C contains language-specific logic for the editor (CEDT).
G2000_Language controls which string table is active.

## Firestaff Implementation

po/ — gettext-style .po files for translations (SWEDISH etc.)
Current lang: Swedish target (naturlig-svenska skill for natural Swedish output).

Firestaff engine uses its own i18n for UI text outside the original game data.
Original game strings are preserved as-loaded from GRAPHICS.DAT.

## Text Rendering
Font loaded via F0490_MEMORY_LoadDecompressAndExpandGraphic(C700_GRAPHIC_TEXTS) path.
Font bitmap embedded in GRAPHICS.DAT; rendered with blitter/font routines.
