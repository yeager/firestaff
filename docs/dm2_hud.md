# DM2 V1 — HUD (Heads-Up Display)

## Overview

DM2's HUD system represents a major evolution from DM1. The right-side panel system was introduced as the primary HUD interface, replacing DM1's more static layout with a dynamic, state-driven panel switching mechanism. The HUD integrates champion portraits, status bars, and squad hand display through a unified panel rendering system.

## Right Panel System

### Panel Types (enum)
Defined in `skproject/SKWIN/defines.h`:

| Constant | Value | Description |
|---|---|---|
| `RIGHT_PANEL_UNDEFINED` | 0xFFFF | No panel |
| `RIGHT_PANEL_SQUAD_HANDS` | 0 | Default — squad hands/inventory |
| `RIGHT_PANEL_HAND_ACTIONS` | 1 | Hand action menu |
| `RIGHT_PANEL_SPELL` | 2 | Spell panel |
| `RIGHT_PANEL_MAGIC_MAP` | 3 | Magic map overlay |
| `RIGHT_PANEL_MONEY_BOX` | 4 | Money display |
| `RIGHT_PANEL_CONTAINER` | 5 | Container/chest view |
| `RIGHT_PANEL_ATTACK_RESULT` | 6 | Attack result flash |

### Panel State Globals
- `glbRightPanelType` (`_4976_3ddc`) — current active panel type
- `glbPreviousRightPanelType` (`_4976_3f6a`) — previous panel for transition detection
- `glbInventorySubpanel` (`_4976_3d2e`) — sub-inventory tracking (e.g. scabbard/backpack)

## Champion Status Display

### 3-Stat Panel
- `DRAW_PLAYER_3STAT_PANE(player, xx)` — renders health/stamina/mana bars for a champion
- `DRAW_PLAYER_3STAT_HEALTH_BAR(U16 player)` — health bar rendering
- `DRAW_PLAYER_3STAT_TEXT(Champion *ref)` — text overlay on stat bars

### Health Bar Colors
- `glbChampionColor[MAX_CHAMPIONS]` (`_4976_3fec`) — per-champion health bar color
- Health bar uses color coding for status (poisoned, plagues, etc.)

### Stats Displayed
- **Health** — red/gradient bar, numeric display
- **Stamina** — stamina bar
- **Mana** — mana bar
- Bar positions calculated with `glbPanelStatsYDelta = 7` (default spacing)

## Portrait System

- `GDAT_INTERFACE_CLASS_PORTRAIT` (cls1=0x02) — portrait image class in GDAT
- Champion portraits displayed in the HUD area
- Portrait numbering: `glbChampionIndex` selects active champion
- Portraits are drawn as part of the 3-stat panel layout
- `IS_OBJECT_VISIBLE_TEXT()` — checks if portrait text label should display

## HUD Text Rendering

- `DRAW_STRONG_TEXT()` — renders text strings on HUD with color/fill options
- `glbTextEntryEncoded` (`_4976_52de`) — flag for text encoding mode
- Text positioned at `_4976_52d8` (x) and `_4976_52da` (y)
- Text database: `DB_SIZE_TEXT = 0x04`, `DB_CATEGORY_TEXT = 0x02`
- `DIRECT_QUERY_GDAT_TEXT()` — fetch text from GDAT text database
- Fonts stored at `_4976_5c0e` (bitmap font data)

## Stat Bars vs DM1

| Feature | DM1 | DM2 |
|---|---|---|
| Bar type | Single health bar | 3 separate bars (HP/Stamina/Mana) |
| Bar rendering | Simple rect | `DRAW_PLAYER_3STAT_HEALTH_BAR` with color |
| Color coding | Monochrome | Per-champion color (`glbChampionColor`) |
| Text overlay | Damage numbers only | Full text rendering on bars |
| Position | Fixed | Dynamic via `glbPanelStatsYDelta` |

## Right Panel Update Cycle

- `UPDATE_RIGHT_PANEL(xx)` — master panel update function
- Called during main game loop to refresh panel state
- Transitions driven by: `glbRightPanelType != glbPreviousRightPanelType`
- Panel content depends on: champion selection, object interaction, combat state

### Panel Selection Logic
1. Attack result shown after combat
2. Magic map when champion has magic active
3. Container when chest is open
4. Money box when relevant
5. Spell panel when casting
6. Squad hands as default

## Food/Water/Poison Panel

- `GDAT_INTERFACE_CHAR_FOOD_WATER_PANEL` (cls2=0x01) — food/water panel
- `GDAT_INTERFACE_FOOD_TEXT` (cls2=0x06) — food amount text
- `GDAT_INTERFACE_WATER_TEXT` (cls2=0x07) — water amount text
- `GDAT_INTERFACE_POISON_TEXT` (cls2=0x08) — poison indicator text
- `GDAT_INTERFACE_PLAGUED_TEXT` (cls2=0x09) — plague indicator text

## HUD Changes from DM1 Summary

1. **Dynamic right panel** — replaces DM1's static right side
2. **Three-stat bars** — HP + Stamina + Mana (vs HP only in DM1)
3. **Color-coded health bars** — per-champion color setting
4. **Magic map overlay** — new panel type not in DM1
5. **Container view** — chest/container panel new in DM2
6. **Attack result panel** — combat feedback panel new in DM2
7. **Text encoding** — `glbTextEntryEncoded` for UTF/encoded text
8. **Food/water display** — explicit food/water tracking in HUD

## Key Globals

- `glbRightPanelType` — current panel
- `glbPreviousRightPanelType` — previous panel  
- `glbPanelStatsYDelta = 7` — stat text vertical spacing
- `glbChampionColor[MAX_CHAMPIONS]` — health bar colors
- `glbSomeChampionPanelFlag` (`_4976_5352`) — champion panel state flag
- `glbInventorySubpanel` — sub-inventory tracking

## Source Files

- `skproject/SKWIN/SkWinCore.cpp` — panel rendering (`DISPLAY_RIGHT_PANEL_SQUAD_HANDS`, `UPDATE_RIGHT_PANEL`, `DRAW_PLAYER_3STAT_*`)
- `skproject/SKWIN/SkWinCore.h` — panel update method declarations
- `skproject/SKWIN/defines.h` — RIGHT_PANEL_* constants
- `skproject/SKWIN/skval2.h` — panel-related globals
