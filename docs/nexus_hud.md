# Nexus V1 — HUD (Heads-Up Display) Audit

## Sources
- `src/ui/firestaff_hud.c` (Firestaff HUD implementation)
- `src/nexus/nexus_v1_viewport.c` (viewport rendering)
- `src/nexus/nexus_v1_engine.c` (HUD integration in engine)
- `docs/dm2_hud.md` (DM2 HUD reference)
- `docs/nexus_features.md` (feature overview)

## Overview

The cross-game `src/ui/firestaff_hud.c` module is not wired to the Nexus V1
engine and must not be presented as a retail Nexus HUD. Its procedural arrows,
compass, warning blocks, and message layout remain an isolated diagnostic
surface. Nexus production currently retains source-owned STABG pixels and
FONT256 data, but VDP2 placement, text/attribute ownership, and runtime HUD
composition remain gated pending Saturn evidence.

## Isolated host diagnostic: FS_HUD

Defined in `src/ui/firestaff_hud.h`. Main struct:

```c
typedef struct {
    int party_dir;          // 0-3 N/E/S/W
    int party_x, party_y;   // dungeon position
    int food_warning;
    int water_warning;
    int msg_count;
    float msg_timer;
    char messages[HUD_MSG_QUEUE][HUD_MSG_MAX_LEN];
    int champ_hp[HUD_MAX_CHAMPS];
    int champ_max_hp[HUD_MAX_CHAMPS];
    int champ_stamina[HUD_MAX_CHAMPS];
    int champ_mana[HUD_MAX_CHAMPS];
} FS_HUD;
```

Message queue: HUD_MSG_QUEUE = 8, HUD_MSG_MAX_LEN = 128. Message duration: 3.0 seconds.

The following elements belong only to the isolated host diagnostic and are not
claims about Saturn coordinates, palette ownership, or retail widget meaning.

## HUD Elements

### 1. Compass (Top-Left, 24x24 px)
- Black background filled rectangle
- 4 direction dots (N/E/S/W) at radius 8 from center
- Active direction: N=light gray, E=yellow, S=brown, W=cyan
- Current direction shown as 3x3 bright block

### 2. Movement Arrows (Bottom of Viewport)
- Left arrow: x=40, y=138, w=12, h=10, color 7 (dark gray)
- Forward arrow: x=100, y=138, w=40, h=10, color 7
- Right arrow: x=190, y=138, w=12, h=10, color 7
- Turn left: x=60, y=150, w=20, h=8, color 7
- Turn right: x=160, y=150, w=20, h=8, color 7
- Back: x=100, y=150, w=40, h=8, color 7

### 3. Message Line (Very Bottom, y=190)
- Full-width black bar (320x10 px)
- Small white dots at (4,194) and (5,194) as message indicator
- Messages queue with 3-second display duration

### 4. Warning Icons (Top-Right)
- Food warning: x=295, y=2, 6x6 px, color 6 (brown)
- Water warning: x=305, y=2, 6x6 px, color 13 (light cyan/blue)

## HUD Update Cycle (diagnostic only)

```c
void fs_hud_init(FS_HUD *hud)       // zero-init, set msg_duration=3.0f
void fs_hud_tick(FS_HUD *h, float dt)  // advance timer, pop expired
void fs_hud_render(const FS_HUD *h, uint8_t *fb)  // blit to 320x200 fb
void fs_hud_message(FS_HUD *h, const char *text)  // enqueue a message
```

Render target: 320x200 8-bit indexed framebuffer with palette system. No Nexus
launcher or engine production path calls these functions.

## DM1 vs Nexus HUD Comparison

| Feature | DM1 | Nexus V1 (Firestaff) |
|---------|-----|-----------------------|
| Health display | Text overlay in right panel | champ_hp[] (not rendered yet) |
| Compass | ASCII directional | Diagnostic-only procedural rose |
| Movement arrows | Sprite bitmap | Diagnostic-only hard-coded rectangles |
| Messages | Bottom text line | Diagnostic queue; no Nexus event owner |
| Food/water warnings | Numeric text | Diagnostic-only icon blocks |
| Champion portraits | GDAT portrait sprites | FACE.BIN route blocked pending capture |
| Stamina/mana bars | Text in DM1 | Tracked (not rendered) |
| Panel system | Right-side dynamic panels | Single HUD overlay |

## Whats Implemented vs Whats Missing

Implemented only in the isolated diagnostic: compass rose, movement arrows,
message queue with timed dismiss, and food/water warning icons.

Nexus production still lacks authenticated champion stat bars, portrait display,
spell symbol area, inventory panel, status effect indicators, VDP2 minimap
placement, and Saturn-owned message text binding.

## Next Steps
1. Bind HUD widget ownership and placement from an authenticated Saturn capture.
2. Bind FACE.BIN palette/VDP1 command ownership before portrait blit.
3. Bind FONT256/SLEV text and attribute consumers before message rendering.
4. Bind SMAP00-15 VDP2 placement and explored-state writes.
