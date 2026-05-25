# DM2 V1 Intro Sequence

## Lord Dragoth / Cinematic Intro

DM2 does NOT have a Lord Dragoth cinematic FMV intro in the traditional sense.
Unlike many 1990s games that played full-motion video before the title screen,
DM2 uses a **static image title screen** with optional animated title sequence.

The internal name "Dragoth" refers to the **final boss creature** (AI index 30,
"LORD DRAGOTH") encountered in the endgame, not a pre-game intro movie.

## Animated Title Sequence (ANIM system)

The DM2 title screen supports an **ANIM file player** that can display
animated sequences stored in external ANIM files (e.g., "title.anim").

### ANIM Bootstrap Functions

| Function | File | Arguments | Purpose |
|----------|------|-----------|---------|
| ANIM_BOOTSTRAP_TITLE() | ANIM.EXE | "title +ah +as +ab +pm +sb" | Title screen animation |
| ANIM_BOOTSTRAP_SWOOSH() | ANIM.EXE | "swoosh +pm +sb" | Swoosh transition |
| FIRE_BOOTSTRAP() | FIRE.exe | "+pm +sb" | MIDI music player |

### ANIM Flag Meanings

| Flag | Meaning |
|------|---------|
| +ah | Antialiasing/high-res mode |
| +as | Scanline effect |
| +ab | Border/frame |
| +pm | Permanent mouse cursor |
| +sb | Sound buffer/dma |

### Execution Path

IBMIO_EXEC() at SkWinCore.cpp routes these calls:
1. "FIRE.exe" -> FIRE_BOOTSTRAP() -> FIRE.exe (+pm +sb)
2. "anim swoosh +pm +sb" -> ANIM_BOOTSTRAP_SWOOSH() -> ANIM.EXE
3. "anim title +ah +as +ab +pm +sb" -> ANIM_BOOTSTRAP_TITLE() -> ANIM.EXE

These are launched via IBMIO_MAIN() which wraps DOS int 21h EXEC calls.

### ANIM File Format

ANIM files use a custom 4bpp (4 bits per pixel) format:
- ANIM_DECODE_IMG1() - decodes individual frames
- ANIM_FILL_SEQ_4BPP() - fills sequences with 4bpp data
- ANIM_BLIT_TO_MEMORY_ROW_4TO4BPP() - blits rows to VGA memory
- Supported palette depth: up to 16 colors (4bpp) adjustable via +O flag

### No FMV in DM2

Unlike the DM1 reference game which may have had cinematic elements,
the skproject source shows no evidence of:
- Full-motion video (.AVI, .MPG, .DAT movie files)
- CDXA/Sega CD video playback
- External video codec integration

The "title" shown at startup is a GDAT_CATEGORY_TITLE (0x05) entry,
rendered as a static or optionally-animated image using the ANIM system.

## Lord Dragoth as Boss, Not Intro

The canonical Lord Dragoth encounter is in the final dungeon sequence:
- AI type 30: LORD DRAGOTH (creature with unique multi-spell casting)
- AI type 34: DRAGOTH ATTACK MINION (summoned projectile creature)
- Dragoth casts Reflector spell (0x55) - unique among creatures
- Part of the endgame trigger sequence at DUNGEON_LEVEL_14

## Source References

- ANIM bootstrap: skproject/SKWIN/SkWinCore.cpp:2045
- ANIM exec router: skproject/SKWIN/SkWinCore.cpp:64293
- FIRE bootstrap: skproject/SKWIN/SkWinCore.cpp:64278
- Lord Dragoth creature def: skproject/SKWIN/_4976_03a2.h:38
- GDAT title category: skproject/SKWIN/defines.h:413
