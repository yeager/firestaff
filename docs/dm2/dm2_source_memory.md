# DM2 V1 Memory Layout

## DM1 vs DM2 Memory Models

DM1: Flat 64KB real-mode model
- Single 64KB segment (cs=ds=ss)
- All code+data fit in one segment
- 16-bit offsets only
- Strict size constraints drove efficient design
- ~15KB free RAM after loading dungeon data

DM2: Extended/segmented 16-bit model
- Uses multiple segments (up to 1MB addressable)
- Near/far pointers throughout codebase
- Larger data structures, more breathing room
- No longer constrained to 64KB total

## SKULLWIN Memory Architecture

Original DOS binary (SKULL.ASM):
- Single seg000 segment, ~64KB of addressable memory
- Relocated by DOS loader, runs in real mode
- Uses x86 segment:offset pairs for any data >64KB

SKULLWIN C++ port:
- Uses flat 32-bit C++ memory model via Allegro 4.2
- DM2screen: c_pixel256[ORIG_SWIDTH * ORIG_SHEIGHT] = 320 * 200 = 64,000 pixels
- backbuffer: 224 x 136 pixels (backbuffer_w=0xe0=224, backbuffer_h=0x88=136)
- pictbuff: offscreen picture buffer for UI/compositing
- Two backbuffer indices (backbuff1, backbuff2) for double-buffering

## Key Memory Structures

c_gfx_system (skullwin/c_gfx_main.h):
- dm2screen[320*200]  — main pixel framebuffer
- dm2mscreen[320*200] — mirror screen (HOTTAG/debug)
- bitmapptr           — current drawing surface
- backbuff1, backbuff2 — double-buffer indices
- pictbuff            — offscreen picture buffer
- backbuffer_w=224, backbuffer_h=136

Global state (SkGlobal.h / SkWinCore.h):
- glbChampionTable   — champion/party data (far pointers in original)
- glbChampionSquad    — active squad (pointer alias)
- fileHeader          — loaded game file header
- fset (fileSet)      — master file index
- intrvect[256]       — interrupt vector table
- glbXAmbientSoundActivated — extended mode flag

## Memory Allocation

Original DOS:
- Custom heap manager (c_alloc.c pattern)
- c_dballoc.c for debug allocation tracking
- Memory pools for dungeon tiles, creatures, items

SKULLWIN C++ port:
- Standard C++ new/delete (backed by c_alloc)
- c_dballoc: diagnostic allocator for memory leak detection
- c_dballoc: double-buffered allocation tracking

## Graphics Resolution

- DM1: 320x200 (256-color VGA)
- DM2: 320x200 base, with larger backbuffer for scrolling UI
- c_gfx_system uses ORIG_SWIDTH=320, ORIG_SHEIGHT=200 constants
- Backbuffer double-buffering: blit_toscreen() from backbuffer to screen

## Champion and Dungeon Data

- Dungeon tile data loaded from Graphics .DAT files via c_gdatfile
- Champion state stored in glbChampionTable (4 champions in party)
- Savegame format in c_savegame: per-champion stats, inventory, position
- Dungeon maps stored as tilemap arrays with fog-of-war (c_cloud)
