# DM2 V1 — Text Rendering System

## Overview

DM2's text rendering is a multi-layer system: bitmapped fonts stored in GDAT, encoded text retrieval, and various drawing functions for displaying text in different contexts (HUD, dialogs, scroll text, combat feedback). Text is rendered on top of the graphical layer using custom drawing functions.

## Font System

### Font Data Storage
- `Bit8u *_4976_5c0e` — main font bitmap pointer (loaded at runtime from GDAT)
- `GDAT_CATEGORY_JAPANESE_FONT` (0x1C) — separate kanji/extended font table for Japanese text
- Fonts loaded via `QUERY_GDAT_ENTRY_DATA_BUFF(GDAT_CATEGORY_JAPANESE_FONT, cls2, dtImage, ...)` iteration

### Font Categories
- `GDAT_CATEGORY_JAPANESE_FONT` (0x1C) — kanji table, iterated at startup to load all glyphs
- `dtImage` — font image data type in GDAT
- `dtWordValue` — character code/value lookup in GDAT
- Text database categories: `DB_SIZE_TEXT = 0x04`, `DB_CATEGORY_TEXT = 0x02`

## Text Drawing Functions

### Core Text Functions

#### `DRAW_STRONG_TEXT()`
```
void SkWinCore::DRAW_STRONG_TEXT(
    Bit8u *buff,    // buffer/screen target
    Bit16u ww,      // width
    Bit16u cx,      // center x
    Bit16u xx,      // x position
    Bit16u yy,      // y position
    Bit16u clr1,    // foreground color
    Bit16u fill,    // background/mask
    Bit8u *str      // string to draw
)
```
- Primary text drawing function for HUD and menus
- Draws text with foreground color and background fill/mask
- Used for stat text, inventory labels, damage numbers

#### `DRAW_VP_RC_STR()`
- Viewport-relative string drawing
- Takes `rectno` (rectangle number from layout data) + text from GDAT
- Used for pause screen, dialog labels, champion status

#### `DRAW_LOCAL_TEXT()`
```
void SkWinCore::DRAW_LOCAL_TEXT(
    Bit16u rectno,   // rect number for positioning
    Bit16u clr1,     // foreground color
    Bit16u clr2,     // shadow/secondary color
    Bit8u *str       // string
)
```
- Rect-anchored text rendering for inventory screen
- Uses rect number to look up position from layout system

#### `DRAW_TEXT_TO_BACKBUFF()`
```
void SkWinCore::DRAW_TEXT_TO_BACKBUFF(i16 xx, i16 yy, U8 *str)
```
- Direct-to-backbuffer text rendering for general use
- Text centered at `xx - (width >> 1)`

#### `DRAW_SCROLL_TEXT()`
- Renders scrolling text / message scrolls
- Uses `_3929_04e2_DRAW_TEXT_STRINGS()` for multi-line text

### Text String Drawing
`_3929_04e2_DRAW_TEXT_STRINGS()` — core string layout engine:
- Takes string + rect bounds, returns drawn size
- Used for scroll text, multi-line displays

## Text Retrieval (GDAT)

### Primary Text Query
`QUERY_GDAT_TEXT(category, cls2, cls4, buffer)`:
- `category` — GDAT category (e.g., `GDAT_CATEGORY_INTERFACE_CHARSHEET`)
- `cls2` — subclass 2
- `cls4` — data index (e.g., 0x18 = name, 0x06 = food text, etc.)
- Returns pointer to encoded text in buffer

### Key Text Interfaces (from defines.h)
- `GDAT_INTERFACE_CHAR_FOOD_WATER_PANEL` (cls2=0x01) — food/water panel
- `GDAT_INTERFACE_FOOD_TEXT` (cls2=0x06) — food amount
- `GDAT_INTERFACE_WATER_TEXT` (cls2=0x07) — water amount  
- `GDAT_INTERFACE_POISON_TEXT` (cls2=0x08) — poison text
- `GDAT_INTERFACE_PLAGUED_TEXT` (cls2=0x09) — plague text

### Champion Text
- `GDAT_CATEGORY_CHAMPIONS` (cls2=heroType, cls4=0x18) — champion names
- `GDAT_CATEGORY_CREATURES` (cls2, cls4+0x10) — creature names

### Direct Query
`DIRECT_QUERY_GDAT_TEXT(cls1, cls2, cls4, buff)` — inline text fetch, no cache

## Text Positioning

### Global Text Position Variables
- `_4976_52d8` — text draw x position
- `_4976_52da` — text draw y position
- `glbTextEntryEncoded` (`_4976_52de`) — text encoding mode flag

### Text Coordinate Systems
- `_4976_0124` / `_4976_0126` — text dimensions (cy/cx)
- `_4976_013a` — text cy (Japanese text)
- `glbScreenWidth` / `glbScreenHeight` — screen dimensions for centering

## Text Rendering Pipeline

1. `QUERY_GDAT_TEXT()` — fetch encoded text from GDAT
2. `_3929_04e2_DRAW_TEXT_STRINGS()` — calculate layout, determine positions
3. `DRAW_STRONG_TEXT()` — render with color, background mask
4. Optional: `DRAW_VP_RC_STR()` for viewport-anchored rendering

## Text vs DM1 Differences

| Feature | DM1 | DM2 |
|---|---|---|
| Font storage | ROM font bitmap | GDAT-loaded font bitmaps |
| Font type | Fixed single font | Multiple fonts (Japanese separate) |
| Text rendering | Direct pixel write | Layered buffer + mask system |
| Text encoding | ASCII | GDAT-encoded with `glbTextEntryEncoded` |
| Text retrieval | Direct memory | `QUERY_GDAT_TEXT()` API |
| Multi-language | None | Japanese font system (0x1C) |
| Layout system | Fixed positions | Rect-number based layout |

## Text-Related Globals

- `_4976_5c0e` — font bitmap data pointer
- `glbTextEntryEncoded` — encoding mode flag
- `_4976_52d8/_52da` — text position
- `_4976_0124/_0126` — text dimensions
- `dunTextData*` (`_4976_4d12`) — dungeon text data pointer

## Source Files

- `skproject/SKWIN/SkWinCore.cpp` — all text drawing functions
- `skproject/SKWIN/SkWinCore.h` — text method declarations
- `skproject/SKWIN/defines.h` — GDAT text category constants
- `skproject/SKWIN/SkGlobal.h` / `SkGlobal.cpp` — global font/text state
