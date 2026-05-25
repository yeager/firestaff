# DM2 V1 Title Screen

## Title Screen Architecture

DM2 title screen is part of GDAT_CATEGORY_TITLE (0x05):
- Entry index 4: Main menu background image
- Entry index 1: Credit/tombstone screen image
- Entry index 0: Unknown (reserved)

The menu background is loaded via:
- QUERY_GDAT_ENTRY_DATA_PTR() for raw data format (dt07, index 4)
- QUERY_GDAT_IMAGE_ENTRY_BUFF() for compressed RLE format

## Title vs DM1

DM1 title screen was likely a simpler static image without the ANIM system
overhead. DM2 adds:

1. **ANIM file support**: Optional title.anim can animate over static bg
2. **GDAT three-phase loading**: Title assets loaded in INIT phase
3. **glbSpecialScreen state machine**: Controls title vs credits vs game

## ANIM Title Animation

When ANIM_BOOTSTRAP_TITLE() is called:
- Loads "title.anim" file via ANIM_FILE_OPEN()
- Decodes 4bpp frames using ANIM_DECODE_IMG1()
- Blits to VGA memory via ANIM_BLIT_TO_MEMORY_ROW_4TO4BPP()
- Renders with antialiasing (+ah), scanlines (+as), border (+ab)

Command: "anim title +ah +as +ab +pm +sb"

Flags:
- +ah: Antialiasing
- +as: Scanline effect  
- +ab: Add border
- +pm: Permanent mouse cursor
- +sb: Sound buffer enable

## DM2 Title Screen States

| State | glbSpecialScreen | Meaning |
|-------|-----------------|---------|
| Title | 0x63 (99) | At main menu, message loop active |
| Credits | 0xDA (218) | Viewing credits |
| In Game | 0 | Active gameplay |
| New Game | 1 | Starting new game (party creation) |

## No FMV Title

DM2 does NOT use full-motion video for the title. The "title" is:
- Either a static GDAT image
- Or an ANIM sequence (custom 4bpp animation format)
- No external video codec (.AVI, .MPG, etc.)

## Comparison with DM1 Title

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Title format | Static image | Static or ANIM animation |
| FMV intro | Possibly | No FMV |
| Audio | DOS beep/PC speaker | MIDI via FIRE.exe |
| Credits | Yes | Yes (0xDA event) |
| Menu integration | Simple | State machine |

## Source References

- Title loading: skproject/SKWIN/SkWinCore.cpp:55187-55196
- ANIM title: skproject/SKWIN/SkWinCore.cpp:2045
- ANIM exec: skproject/SKWIN/SkWinCore.cpp:64293
- Defines: skproject/SKWIN/defines.h:413
