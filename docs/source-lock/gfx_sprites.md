# DM1 V1 Sprite/Creature Graphics

File: /tmp/gfx_sprites.md
Audit: DM1 V1 creature sprite storage, display, multi-frame animation
Sources: ReDMCSB DUNVIEW.C, DEFS.H, MEMORY.C, GRF1.C; Firestaff dm1_v1_creature_render_pc34_compat.c, dm1_v2_creature_animation_pc34.c

---

## 1. Creature Sprite Storage

### 1.1 Native bitmap indices

Creature sprites are compressed bitmaps in GRAPHICS.DAT, starting at
M618_GRAPHIC_FIRST_CREATURE.

PC 3.4 / I34E: M618 = 584
V1.x: M618 = 446
V2.x: M618 = 584

Source: DEFS.H:2321 (I34E), DEFS.H:2392 (V1.x/V2.x)

87 creature bitmap slots (indices 584-670 for I34E). Each slot may contain
multiple frames (front, back, attack, etc.) packed contiguously.

### 1.2 CreatureAspect table (G0219)

ReDMCSB source: DUNVIEW.C:1625, 1656 (I34E block)

G0219_as_Graphic558_CreatureAspects[27] maps creature TYPE to:
  - firstNativeBitmapRelativeIndex: offset from M618
  - firstDerivedBitmapIndex: derived cache entry
  - coordinateSet_transparentColor: packed (coordSet<<4 | transparentColor)
  - replacementColorSetIndices: packed (color10Repl<<4 | color9Repl)
  - graphicInfo: flags from G0243 creature info

Firestaff: dm1_v1_creature_render_pc34_compat.c s_aspects[27]

### 1.3 Multi-frame storage

Each creature type stores multiple frames in GRAPHICS.DAT as contiguous
bitmap indices:

  Creature N frames: M618 + aspect[N].firstNativeBitmapRelativeIndex
                  through M618 + aspect[N+1].firstNativeBitmapRelativeIndex - 1

Example -- Giant Scorpion (aspect 0):
  firstNativeBitmapRelativeIndex = 0
  frames: bitmaps 584, 585, 586, 587 (4 frames)

Example -- Stone Golem (aspect 9):
  firstNativeBitmapRelativeIndex = 25
  frames: bitmaps 609-614

Source: DUNVIEW.C:5223-5315 -- bitmap index computation

---

## 2. Sprite Display

### 2.1 Rendering pipeline

In F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectiles_CPSEF:
  ReDMCSB DUNVIEW.C:5311

  base_idx = M618_GRAPHIC_FIRST_CREATURE
           + creature_aspect->firstNativeBitmapRelativeIndex;

Per-creature decisions:
  - Direction (0-3): M026_CHAMPION_ICON_INDEX(offset, dir) -- DEFS.H:718
  - Back facing: use back bitmap if creature behind party (L0170 flag)
  - Attack: add attack frame offset (MASK0x0020_ATTACK)
  - Flip: MASK0x0004_FLIP_NON_ATTACK + MASK0x0040_FLIP_BITMAP

### 2.2 Sprite scaling (derived bitmaps)

Native graphics drawn at base resolution. Derived bitmap cache stores
scaled versions for perspective:

  D3 scale: 16/32 (closer distance)
  D2 scale: 20/32 (further distance)

Source: DEFS.H:2412-2419 -- derived bitmap layout
Source: DEFS.H:3341 -- M539_DERIVED_BITMAP_FIRST_CREATURE = 495 (V1.x)

### 2.3 Transparent color

Each creature has a transparent color index (stored in coordinateSet |
transparentColor). Color 0 (black) is commonly transparent.

Source: DUNVIEW.C:5311 -- transparent color from CreatureAspect

---

## 3. Multi-Frame Creature Animation

### 3.1 V1 implicit animation (frame-by-frame index)

V1 has no dedicated animation sequencer. Animation occurs by selecting
the correct bitmap index each render tick:

  - Base frame: front-facing idle
  - Back frame: creature behind party
  - Attack frame: during attack action (MASK0x0080_IS_ATTACKING)
  - Additional frame: if CreatureInfo.GraphicInfo & MASK0x0003_ADDITIONAL

Source: DUNVIEW.C:5331 -- L0172_B_UseCreatureAttackBitmap
Source: DUNVIEW.C:5371 -- L0173_B_UseFlippedHorizontallyCreatureFrontImage

### 3.2 V2 explicit animation sequencer

src/dm1v2/dm1_v2_creature_animation_pc34.c:

  M11_V2_CreatureAnim: enum idle, walk, attack, hurt, death, special
  M11_V2_AnimSequence: frame array + loop flag
  M11_V2_AnimFrame: sprite_idx + duration (seconds)
  M11_V2_CreatureAnimState: creature_id, current_anim, frame, timer, playing

  v2_creature_anim_init()
  v2_creature_anim_define(anim, frames, count, loop)
  v2_creature_anim_play(creature_id, anim)
  v2_creature_anim_update(dt) -- advance timer, advance frame
  v2_creature_anim_get_sprite(creature_id) -- returns current sprite index

V2 extension; V1 rendering continues to use CreatureAspect offset approach.

---

## 4. Palette Remapping (Color Replacement)

### 4.1 Color replacement color sets

Creatures use two color replacement slots (colors 9 and 10) for variant
creature types:

  G0222_auc_Graphic558_PaletteChanges_Creature_D2[16] -- D2 display mode
  G0221_auc_Graphic558_PaletteChanges_Creature_D3[16] -- D3 display mode

ReDMCSB source: DUNVIEW.C:1817-1818, 1821-1822 (two variants)

Example: a Giggler renders in blue instead of green via palette remap,
no separate bitmap required.

### 4.2 Replacement application

Source: DUNVIEW.C:2013-2019

  G0222[...]D2[replaced_color] = replacement_color
  G0221[...]D3[replaced_color] = replacement_color

Per-render:
  L0131_puc_PaletteChanges = (display_mode == D2) ? D2_table : D3_table;
  // Applied as per-pixel lookup during blit

Firestaff: dm1_v1_creature_render_pc34_compat.c -- s_paletteD3[16], s_paletteD2[16]

---

## 5. Compatibility Status

| Component                  | ReDMCSB source    | Firestaff status    |
|----------------------------|-------------------|---------------------|
| CreatureAspect table G0219 | DUNVIEW.C:1656    | implemented         |
| Creature frame select      | DUNVIEW.C:5311    | implemented         |
| Attack bitmap flag         | DUNVIEW.C:5331    | implemented         |
| Horizontal flip            | DUNVIEW.C:5371    | implemented         |
| Direction facing           | DEFS.H:718        | documented          |
| Derived bitmap cache       | DEFS.H:3341       | partial             |
| Transparent color          | DUNVIEW.C:5311    | documented          |
| Color replacement D2       | DUNVIEW.C:1818    | implemented         |
| Color replacement D3      | DUNVIEW.C:1817    | implemented         |
| V2 animation sequencer     | dm1_v2_creature_animation_pc34.c | V2 only |
