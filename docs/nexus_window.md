# Nexus V1 — Window Management Audit

## Sources
- `src/nexus/nexus_v1_engine.c` (engine init, framebuffer setup)
- `src/nexus/nexus_v1_viewport.c` (viewport rendering to framebuffer)
- `src/engine/firestaff_sdl_bridge.c` (SDL window/renderer management)
- `src/engine/config_m12.c` (V2.1/V2.2 display configuration)
- `docs/nexus_features.md` (Saturn VDP1/VDP2 hardware)
- `docs/nexus_graphics.md` (graphics pipeline overview)

## Overview

Nexus V1 renders to a 320x200 8-bit indexed framebuffer managed by
Nexus_Framebuffer in nexus_v1_viewport.c. Window management (SDL window,
fullscreen/windowed toggle, resolution scaling) is handled by the Firestaff
engine layer (firestaff_sdl_bridge.c), not by the Nexus engine itself.

On Sega Saturn, window management is handled by VDP2 (video display
processor 2), managing background layers, scroll planes, and screen
compositing. On PC/Linux, this is emulated via SDL2.

## Framebuffer Architecture

Nexus_Framebuffer (in nexus_v1_viewport.c):
```c
typedef struct {
    uint8_t color_buffer[NEXUS_FB_W * NEXUS_FB_H];  // 320x200 indexed
    uint32_t palette[256];                           // RGBA palette
    int width, height;
} Nexus_Framebuffer;
```

Internal resolution: 320x200 (fixed). Color depth: 8-bit indexed (palette mode).
Framebuffer cleared by nexus_fb_clear() each frame.

Viewport render pipeline:
- nexus_v1_viewport_render(vp, engine) -> nexus_fb_clear()
- nexus_camera_init() -> for d in 0..NEXUS_VIEW_DISTANCE: draw floor/wall
- nexus_viewport_to_rgba() -> convert to RGBA for display

Output is 320x200 RGBA (64000 bytes) blitted to the SDL window surface.

## SDL Window Management (Firestaff Engine)

SDL bridge (firestaff_sdl_bridge.c) creates and manages the window:
- Resolution: 320x200 internal, scaled via integer or bilinear scaling
- Window mode: windowed or fullscreen (g_settings.display_mode)
- Aspect ratio: controlled by g_settings.display_aspect (4:3 or stretch)
- Scale mode: integer scaling (pixel-perfect) or linear interpolation

Config options from config_m12.c:

| Setting | Values | Description |
|---------|--------|-------------|
| display_mode | fullscreen / windowed | Screen mode |
| display_aspect | 4:3 / stretch / 16:9 | Aspect ratio |
| scale_mode | integer / linear | Scaling algorithm |
| integer_scaling | bool | Pixel-perfect scaling |
| scaling_filter | sharp / blurry | Filter mode |
| vsync | bool | Vertical sync |

## DM1 vs Nexus Window Comparison

| Feature | DM1 | Nexus V1 (Firestaff) |
|---------|-----|-----------------------|
| Internal resolution | 320x200 | 320x200 (same) |
| Display resolution | CGA 320x200 | Scalable (SDL2) |
| Fullscreen support | DOS mode 13h | SDL2 fullscreen |
| Aspect ratio | 4:3 (CRT) | Configurable |
| Window management | DOS interrupt (int 10h) | SDL_CreateWindow |
| Scaling | None | Integer or linear |
| VSync | None | Configurable |

## Saturn VDP2 Window Management (Native)

On Saturn, VDP2 handles:
- NBG0, NBG1, NBG2, NBG3: Normal background layers (tilemap-based)
- RBG0: Rotation background layer (starfield effects)
- Resolution: 320x224 or 352x224 (NTSC) per layer

VDP2 composites VDP1 3D output + background layers. Window regions
(horizontal/vertical clips) support up to 8 regions.

Nexus V1 uses VDP1 for the 3D dungeon viewport and VDP2 for HUD overlay,
minimap, and background scenes.

## Whats Implemented vs Whats Missing

Implemented: 320x200 framebuffer, SDL2 window creation, resolution scaling,
aspect ratio configuration, VSync toggle, RGBA conversion.

Not yet implemented: VDP2 background layer emulation (NBG0-3, RBG0),
minimap overlay compositing (SMAP00-15.BIN), HUD VDP2 sprite rendering,
multi-window region support, FMV video overlay (DMV0-2.AVI) compositing.

## Next Steps
1. Implement VDP2 layer emulation (minimum NBG0 for HUD background)
2. Implement minimap overlay as VDP2 NBG1 layer
3. Add FMV overlay compositor (above 3D viewport, below HUD)
4. Implement Saturn window region system for HUD placement
5. Add display resolution override (640x400, 960x600, 1280x800)
