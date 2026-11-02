# Nexus V1 — HUD (Heads-Up Display) Audit

## Sources
- `src/ui/firestaff_hud.c` (Firestaff HUD implementation)
- `src/nexus/nexus_v1_viewport.c` (viewport rendering)
- `src/nexus/nexus_v1_engine.c` (HUD integration in engine)
- `docs/dm2/dm2_hud.md` (DM2 HUD reference)
- `docs/nexus/nexus_features.md` (feature overview)

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

Nexus production now has authenticated champion-panel geometry from retail
`DM.BIN` (12 stat bars, 4 inventory slots, 8 equipment slots) exposed through
the engine handoff. It still lacks authenticated pixel/CLUT ownership for
stat bars, portraits, spell symbols, inventory art, status effects, VDP2
minimap placement, and Saturn-owned message text binding.

### Authentic frame-300 composition audit

The retained Japanese retail gameplay capture
`nexus_face_vdp2_hud_join_20261030/capture-301` provides one same-frame audit
point (VDP1 frame 300, VDP2 serial 301). All 224 VDP2 row records identify an
enabled NBG0 512x256 8-bit bitmap at byte offset zero. Its rendered 320x224
RGB output matches the corresponding Ymir output at all 71,680 pixels. This
proves output parity for that captured frame, not source ownership or a
general-purpose compositor.

The authenticated source scan decoded 410 retail resources, including
`STABG.BIN` (SHA-256
`7b8e44ffd1249175da1c407993b983a26bc180204e63f9b69274014b336c6913`), and
found no exact NBG0 bitmap source match. The STABG palette also has no exact
match in captured VDP2 CRAM. A raw-byte scan of captured VDP1 VRAM found exact
bytes for only 3 of STABG's 791 8x8 source tiles; one is the all-zero tile.
These are bounded negative byte-join results: they do not rule out decoded,
repacked, or transformed STABG data.

The frame's VDP1 trace contains 68 textured draws, of which 66 have exact
retail DGN image matches and 51 also have palette matches (15 use direct
colour). Two textured commands remain unmatched. This binds many scene draws
to DGN bytes, but does not bind the visible status bars to STABG, HUD widgets,
or live champion state. HUD placement, CLUT ownership, and runtime value
binding therefore remain gated.

The same frame's normal 8bpp sprite at command row 80 is now joined to Japanese
retail `MENU.BPK` entry 144: its decoded 16x41 indexed surface matches all 656
captured VDP1 VRAM bytes at offset `0x643c0`, and each nontransparent palette
index used by the sprite matches the same-frame VDP2 CRAM entry. This accounts
for the row's 431 visible pixels as a retail menu-archive sprite. The DMA
writer's source bytes at transfer time have not yet been captured, and this
single sprite join does not identify the whole-screen HUD or bind it to live
champion state. The capture-only regression verifies the source and palette
join while keeping runtime renderer admission closed.

The other 60 previously capture-classified pixels come from command row 79, an
authenticated untextured VDP1 polyline from the Ymir-captured Japanese retail
execution. Its four vertices form a 16x16 outline at `(272,8)`–`(287,23)`;
PMOD is `0x00e0` and CMDCOLR `0x7083` selects VDP2 CRAM entry `0x83` with
priority index 7. That same-frame CRAM word is `0xe73f`. The integer line
replay records 64 edge writes; four corners are written twice, so the command
owns 60 unique pixels in the composed frame. The captured execution binds the
pixels and palette entry to that VDP1 command, but the outline's HUD meaning
and state owner remain unverified.

## Next Steps
1. Bind the remaining HUD widget ownership and placement from authenticated
   Saturn captures, including status values and their original-data sources.
2. Bind FACE.BIN palette/VDP1 command ownership before portrait blit.
3. Bind FONT256/SLEV text and attribute consumers before message rendering.
4. Bind SMAP00-15 VDP2 placement and explored-state writes.
