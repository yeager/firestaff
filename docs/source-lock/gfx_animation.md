# DM1 V1 Animation Frame Sequencing

File: /tmp/gfx_animation.md
Audit: DM1 V1 animation: doors, creatures, frame timing, sequence tables
Sources: ReDMCSB DUNVIEW.C, MEMORY.C, EXPAND.C; Firestaff dm1_v2_creature_animation_pc34.c

---

## 1. Animated Graphic Types

DM1 V1 has two classes of animated graphics:

### 1.1 Door animations
Doors animate open/close when a party moves through them.
Two drawing passes: behind door (pass 1), door frame, in front (pass 2).
ReDMCSB DUNVIEW.C:4795, 4823

### 1.2 Creature animations
Each creature type has multiple frames (idle, move, attack) stored as
separate bitmap indices in GRAPHICS.DAT.
ReDMCSB DUNVIEW.C:5223-5315

### 1.3 Projectile/explosion animations
Animated by cycling through frame indices with scale-based derived bitmaps.
ReDMCSB DUNVIEW.C:5691

---

## 2. Creature Animation

### 2.1 CreatureAspect table (G0219)

ReDMCSB source: DUNVIEW.C:1625, 1656 (I34E block)

G0219_as_Graphic558_CreatureAspects[27] fields:
  - firstNativeBitmapRelativeIndex: offset from M618 (584)
  - firstDerivedBitmapIndex: derived cache start
  - coordinateSet_transparentColor: packed
  - replacementColorSetIndices: packed
  - graphicInfo: flags

### 2.2 Animation frame selection

In F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectiles_CPSEF:
  ReDMCSB DUNVIEW.C:5311

  AL0127_i_NativeBitmapIndex = M618_GRAPHIC_FIRST_CREATURE
    + creature_aspect->firstNativeBitmapRelativeIndex;

Attack frames: offset by creature-specific attack frame count.
ReDMCSB DUNVIEW.C:5331

Flip: MASK0x0004_FLIP_NON_ATTACK + MASK0x0040_FLIP_BITMAP

### 2.3 Direction-based facing (4 directions)

M026_CHAMPION_ICON_INDEX(baseIndex, direction) -- DEFS.H:718
  (value + 4 - direction) & 0x0003

### 2.4 Attack vs idle state

ReDMCSB DUNVIEW.C:5331:
  L0172_B_UseCreatureAttackBitmap =
    !L0170_B_UseCreatureBackBitmap
    && M007_GET(creatureAspect, MASK0x0080_IS_ATTACKING)
    && M007_GET(creatureInfo->GraphicInfo, MASK0x0020_ATTACK);

### 2.5 V2 creature animation (Firestaff)

src/dm1v2/dm1_v2_creature_animation_pc34.c -- V2 animation sequencer:

  6 animation slots: idle, walk, attack, hurt, death, special
  M11_V2_AnimSequence: frame_count (max 16) + loop flag
  M11_V2_AnimFrame: sprite_idx + duration (seconds)
  v2_creature_anim_update(dt): advance frame by elapsed time

V2 extension only; V1 uses bitmap index offset from CreatureAspect.

---

## 3. Door Animation

### 3.1 Door drawing (two-pass)

ReDMCSB DUNVIEW.C:4795:
  L0175_i_DoorFrontViewDrawingPass = (cellOrdinals & 0x0001) + 1;
  // Pass 1: draw objects behind door
  // Pass 2: draw objects in front of door

### 3.2 Door panel animation

Door panels animate from closed to open as party moves through.
Different door bitmap indices are selected based on party position
in the doorway. No frame sequence -- position-dependent selection.

---

## 4. Projectile / Explosion Animation

### 4.1 Projectile scaling

ReDMCSB DEFS.H:3354:
  M537_DERIVED_BITMAP_FIRST_PROJECTILE = 282 (V1.x)
  156 bitmaps: 6 scales per projectile type

### 4.2 Explosion scaling

ReDMCSB DEFS.H:3355:
  M538_DERIVED_BITMAP_FIRST_EXPLOSION = 438 (V1.x)
  3 x 14 bitmaps: Fire, Spell, Poison at scales 4,6,8,...,30
  1 x 15 bitmaps: Smoke at same scales + 32

### 4.3 Animation timing

No central animation clock in V1. Each subsystem uses game tick
G0003_i_Game_Tick_20Hz. Frame hold is implicit -- the bitmap index
is recomputed each render tick from creature aspect state.

---

## 5. Compatibility Status

| Component              | ReDMCSB source    | Firestaff status |
|------------------------|-------------------|------------------|
| CreatureAspect table    | DUNVIEW.C:1625    | implemented      |
| Creature frame select   | DUNVIEW.C:5311    | implemented      |
| Attack bitmap flag      | DUNVIEW.C:5331    | implemented      |
| Direction facing        | DEFS.H:718        | documented       |
| Door two-pass draw      | DUNVIEW.C:4795    | documented       |
| Door panel animation    | DUNVIEW.C:4823    | documented       |
| Projectile scaling      | DEFS.H:3354       | documented       |
| Explosion scaling       | DEFS.H:3355       | documented       |
| V2 anim sequencer       | dm1_v2_creature_animation_pc34.c | V2 only |
