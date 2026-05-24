# DM1 V1 Text/String Table — Source Audit

## ReDMCSB Source
- DEFS.H:2835–2840: Text type constants (C0_INSCRIPTION=0, C1_MESSAGE=1, C2_SCROLL=2)
- DUNGEON.C:2206–2360: F0168_DUNGEON_DecodeText — main text decoder
- DEFS.H:1172–1184: TEXTSTRING struct

## Text Data Encoding (DUNGEON.C:2206–2360)
3 five-bit codes per 16-bit word. File layout:
Header(44) → Maps(mapCount*16) → CumTable(cols*2) → SFT(sftCount*2)
→ TextData(textDataWordCount*2) → ThingData → RawMapData → checksum(2)

Code meanings (DUNGEON.C:2316–2350):
- 0-25: 'A'+code
- 26: ' ' (space)
- 27: '.' (period)
- 28: separator (newline for message/scroll, 0x80 for inscription)
- 29: escape → next code indexes symbol table (G0256_aac_Graphic559_EscapeReplacementCharacters[32][2])
- 30: escape → next code indexes word table (G0255_ac or G0257_ac for inscription)
- 31: end of text

Separator rules: MESSAGE=' ' + prefix newline, INSCRIPTION=0x80 + terminator 0x81, SCROLL='\n'

## TEXTSTRING Struct (DEFS.H:1172–1184, 4 bytes)
THING Next(2), Visible:1(bit0), Unreferenced:2(bits1-2), TextDataWordOffset:13(bits3-15)

## Firestaff Implementation
File: include/memory_dungeon_dat_pc34_compat.h
File: src/memory/memory_dungeon_dat_pc34_compat.c — F0508_DUNGEON_DecodeTextStringThing_Compat(),
F0507_DUNGEON_DecodeTextAtOffset_Compat(), F0506_DUNGEON_DecodeTextTable_Compat(),
F0509_DUNGEON_DecodeScrollText_Compat()

3-code-per-word decoding, visible gating, separator handling: all ALIGNED.
Escape 29/30: placeholders used instead of G0255/G0256/G0257 table lookups.
G0255/G0256/G0257 are in GRAPHICS.DAT, not DUNGEON.DAT.

STATUS: PARTIALLY ALIGNED — Core 3-code decoder correct. Escape expansion is placeholder. No corruption.
