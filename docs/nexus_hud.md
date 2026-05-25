# Nexus V1 — HUD (Heads-Up Display) Audit

## Sources
- `src/ui/firestaff_hud.c` (Firestaff HUD implementation)
- `src/nexus/nexus_v1_viewport.c` (viewport rendering)
- `src/nexus/nexus_v1_engine.c` (HUD integration in engine)
- `docs/dm2_hud.md` (DM2 HUD reference)
- `docs/nexus_features.md` (feature overview)

## Overview

Nexus V1 HUD is implemented in `src/ui/firestaff_hud.c` as part of Firestaffs
cross-game UI layer. The HUD renders over the 3D viewport and provides
party status, navigation, and message display. The HUD is decoupled from
the Nexus engine — it reads game state from Nexus_V1_Engine and renders to
the same framebuffer as the 3D viewport.

## HUD Implementation: FS_HUD

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

## HUD Update Cycle

```c
void fs_hud_init(FS_HUD *hud)       // zero-init, set msg_duration=3.0f
void fs_hud_tick(FS_HUD *h, float dt)  // advance timer, pop expired
void fs_hud_render(const FS_HUD *h, uint8_t *fb)  // blit to 320x200 fb
void fs_hud_message(FS_HUD *h, const char *text)  // enqueue a message
```

Render target: 320x200 8-bit indexed framebuffer with palette system.

## DM1 vs Nexus HUD Comparison

| Feature | DM1 | Nexus V1 (Firestaff) |
|---------|-----|-----------------------|
| Health display | Text overlay in right panel | champ_hp[] (not rendered yet) |
| Compass | ASCII directional | 24x24 pixel compass rose |
| Movement arrows | Sprite bitmap | Hard-coded filled rects |
| Messages | Bottom text line | Message queue + 3s timer |
| Food/water warnings | Numeric text | 6x6 icon blocks |
| Champion portraits | GDAT portrait sprites | Not rendered in HUD |
| Stamina/mana bars | Text in DM1 | Tracked (not rendered) |
| Panel system | Right-side dynamic panels | Single HUD overlay |

## Whats Implemented vs Whats Missing

Implemented: Compass rose, movement arrows, message queue with timed dismiss,
food/water warning icons.

Not yet implemented: Champion stat bars (HP/Stamina/Mana), portrait display,
spell symbol area, inventory panel, status effect indicators (poison/plague),
3D minimap overlay (SMAP00-15.BIN), text renderer for messages.

## Next Steps
1. Implement champion stat bars rendering in fs_hud_render
2. Implement portrait area (FACE.BIN loading and blit)
3. Implement text renderer using FONT256.S2D glyph data
4. Implement spell symbol area rendering
5. Design minimap overlay using SMAP00-15.BIN data
