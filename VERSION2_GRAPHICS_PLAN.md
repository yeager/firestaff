# ReDMCSB Version 2, Graphics Plan

## Goal

Build a **standalone, legally distributable version** with:
- new high-resolution visuals
- improved audio
- no dependency on the original game's data files
- the same broad game structure and emotional feel, without copying original assets

## Core principle

Version 2 should **not** be:
- AI-upscaled original graphics
- paint-over directly on top of original assets
- a legally grey "enhanced original dump"

Version 2 should be:
- **newly illustrated presentation**
- its own asset pipeline
- a clear separation between gameplay logic and presentation

## Recommended visual direction

### Style
- dark fantasy
- clear silhouettes
- cleaner UI than the original
- painterly 2D feel, not photorealism
- strong contrast between interactive elements and the background

### Resolution
Work in a **high master resolution** from the start.

Recommendation:
- master canvas for menus and scenes: **1920x1080**
- playable UI panels built modularly
- export to smaller targets later as needed

### Rendering style
Two safe directions, pick one early:

1. **Painterly 2D**
   - painted backgrounds
   - characters and objects as clean sprites
   - best route for a good-looking V2 quickly

2. **HD pixel-inspired**
   - not pure pixel art, but influenced by it
   - crisp shapes, limited palette, modern effects
   - harder to make truly elegant

**Recommendation: painterly 2D.**

## Asset categories

### 1. UI and menus
- title screen
- main menu
- submenus
- buttons
- highlight / selection states
- panels, frames, ornaments
- fonts / typography system

### 2. Icons and symbols
- inventory / verb / action symbols
- status icons
- small UI markers
- cursor / highlight indicators

### 3. Scene backgrounds
- rooms
- corridors
- viewpoints
- transition images
- loading / splash / intro backgrounds

### 4. Interactive objects
- doors
- chests
- key objects
- spell / ritual objects
- other clickable or focusable elements

### 5. Character-related assets
- portraits
- character poses
- facial expressions if needed
- dialogue-linked presentation elements

### 6. Effects
- highlight glow
- transitions / fades
- activation flashes
- particles, mist, magical effects

## Technical structure

## Separate logic and presentation

Version 1 teaches us structure.
Version 2 should reuse as much logic as possible, but point at a completely different asset source.

### Recommended model
- **game logic layer**
  - menu structure
  - state transitions
  - interaction rules
- **presentation layer**
  - which images are shown
  - layout
  - effects
  - audio hooks
- **asset manifest layer**
  - maps logical IDs to V2 assets

### Example
Instead of:
- `load graphic index 13`

Use:
- `menu.main.title`
- `menu.main.option.new_game.highlighted`
- `screen.submenu.inventory.background`

That makes V2 clean, swappable, and much easier to maintain.

## File formats

Recommendation:
- **PNG** for 2D assets with alpha
- **WebP lossless** can be considered later for distribution
- **SVG** for some UI elements where it makes sense
- **JSON** or **YAML** for asset manifests

## Directory layout

Suggested structure:

```text
assets-v2/
  ui/
    title/
    menu/
    panels/
    fonts/
  icons/
  backgrounds/
    rooms/
    transitions/
  objects/
  portraits/
  effects/
  audio/
  manifests/
```

## Production pipeline

### Phase 1, art direction
- create a moodboard
- define colour palettes
- choose typography
- make 2-3 styleframes
- lock the exact V2 look before mass production

### Phase 2, design system
- define the UI grid
- define button states
- define highlight / activate visual behaviour
- define standard sizes for panels, icons, and margins

### Phase 3, vertical slice
Build one small playable slice first:
- title screen
- main menu
- one submenu
- one simple scene
- one activation result

If that slice looks good, then scale up.

### Phase 4, asset production
- backgrounds
- UI pack
- interactive objects
- effects
- audio

### Phase 5, integration
- wire assets into the manifest
- wire the manifest into the render layer
- test state transitions, hover, highlight, and activate behaviour

## AI usage, yes but intelligently

AI is good for:
- thumbnails
- moodboards
- idea variation
- composition proposals
- colour exploration

AI is bad as the final V2 pipeline if the goal is quality plus clean legal separation.

**Recommendation:**
- use AI during preproduction
- have humans curate and build the final assets cleanly
- keep prompts and provenance for traceability

## Legal hygiene

To keep V2 clean:
- do not use dumped original assets as final material
- avoid direct paint-over of original images
- avoid reconstructions that are effectively just redrawn originals
- keep inspiration at the systems level, not pixel-for-pixel

## Practical next order

1. Write an **asset manifest schema** for V2
2. List the first V2 asset IDs for:
   - title
   - main menu
   - highlight states
   - activate states
   - first submenu
3. Make a **one-page art bible**
4. Produce a **vertical slice**
5. Connect it to the current menu-state / render / activate seams

## My recommendation

The best path is:
- use Version 1 to establish a **clean gameplay structure**
- build Version 2 as a **newly illustrated presentation layer** on top of that structure
- start with **title + main menu + one submenu**
- leave 3D alone until the 2D version feels polished and coherent

That cuts risk, legal mess, and ugly hacks at the same time.
