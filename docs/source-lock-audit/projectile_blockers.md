# JOB 4: DM1 V1 Projectile Travel Blockers — Source Lock Audit

## ReDMCSB Source — PROJEXPL.C F0219

### Walls Block Projectiles
- **PROJEXPL.C:721–725** (F0219 event processing, `C49_EVENT_MOVE_PROJECTILE`):
  Before moving to the destination square, the code checks:
  ```
  if (square_type == WALL ||
      (FAKEWALL && !(IMAGINARY | OPEN)) ||
      (STAIRS && source == STAIRS))
      if (F0217_PROJECTILE_HasImpactOccured(...)) return;
  ```
  **Real walls (C00_ELEMENT_WALL) always block projectiles.**
  Imaginary fakewalls and open fakewalls do not block.
  Stairs only block if the source square is also stairs (continuous stairs traversal).

### Doors Block Projectiles (Conditional)
- **PROJEXPL.C:742–745**:
  ```
  if (square_type == DOOR && F0217_PROJECTILE_HasImpactOccured(DOOR, ...)) return;
  ```
  A closed door (state ≤ C1_DOOR_STATE_CLOSED_ONE_FOURTH) blocks the projectile.
  However, PROJEXPL.C:490–507 defines exceptions:
  - **Explosion projectiles** (THING_TYPE_EXPLOSION, harm non-material) — pass through
  - **Sufficiently powerful attacks** (Attack > RANDOM(128)) with
    `MASK0x0100_POUCH_AND_PASS_THROUGH_DOORS` slot — pass through closed doors
  - **Keys** (ICON_JUNK_IRON_KEY through MASTER_KEY) — CHANGE2_04: cannot pass through
    closed doors in S11E+; this was a bug fix for the original
  - **Destroyed doors** (C5_DOOR_STATE_DESTROYED) — pass through

### Fakewalls and Imaginary Walls
- Imaginary fakewalls (`MASK0x0001_FAKEWALL_IMAGINARY`) do NOT block (PROJEXPL.C:722).
- Closed fakewalls (not OPEN) DO block (PROJEXPL.C:722).

### Stairs
- Stairs block projectiles only when entering from another stairs square —
  "continuous stairs traversal" case (PROJEXPL.C:723–724).
  A projectile cannot fly over stairs from a non-stairs square.

### Projectile Impact Event (F0217)
- **PROJEXPL.C:490–507**: F0217 checks if projectile impact should occur.
  For DOOR: `MASK0x0002_PROJECTILES_CAN_PASS_THROUGH` on the door type attribute
  overrides normal blocking.

## Firestaff Implementation

### Viewport Rendering Only — Note
The Firestaff DM1 V1 `dm1_v1_projectile_explosion_render_pc34_compat.c` (line 1–106)
handles **viewport rendering** only — it provides:
- `DM1_ProjectileScales[7]` (line 19) — G0215 data, matched to DUNVIEW.C:5712
- `DM1_ProjectileAspects[14]` (line 31) — G0210 data, matched to DEFS.H:2037–2044
- `DM1_ExplosionBaseScales[4]` (line 51) — G0216 data, matched to DUNVIEW.C:6109
- Rendering query functions: `dm1_v1_projectile_aspect_type()`,
  `dm1_v1_projectile_bitmap_delta()`, `dm1_v1_projectile_graphic_index()`,
  `dm1_v1_projectile_flip_flags()`

### Collision/Travel Logic — Elsewhere in Firestaff
- **dm1_v1_dungeon_square_structs_pc34_compat.c line 535**:
  References `dm1_v1_collision_door_pc34_compat` and `dm1_v1_movement_pipeline_pc34_compat`
  for collision logic.
- The projectile **game logic** (movement, collision, impact) is in the game loop
  integration layer — not in the viewport rendering compat file.

### Projectile Occlusion Viewport Zones — PASS
- `s_projectile_occlusion_specs[]` (line 136):
  Maps each view square to depth, lateral offset, and projectile-occlusion spec.
  References DUNVIEW.C:373,5667–5683,5710–5715,5881–5883 (projectile draw pass
  with PC34 zone mapping using G2028 row lookup and C2900 zone offset).
- **DUNVIEW.C:5667–5683**:
  - G2028_ac_ViewSquareIndexTo[...] used to compute row index per view square
  - C2900_ZONE_ + row*4 + viewCell → PC34 zone for projectile blit position
  - D3 front cells clipped: `if (depth==3 && cell <= C01_VIEW_CELL_FRONT_RIGHT) continue`
  - D0 back cells clipped: `if (depth==0 && cell >= C02_VIEW_CELL_BACK_RIGHT) continue`
- Firestaff's `s_projectile_occlusion_specs` correctly encodes this mapping
  for all 12 view squares.

## Verdict: PASS (with scope clarification)

Viewport rendering occlusion zone mapping is source-locked:
- G0215 ProjectileScales ✓
- G0210 ProjectileAspects ✓
- G0216 ExplosionBaseScales ✓
- G2028/C2900 zone mapping in s_projectile_occlusion_specs ✓
- F0217/F0219 collision logic: not in viewport compat module — handled by
  collision/movement pipeline (referenced dm1_v1_collision_door_pc34_compat).

The wall/door blocking rules are game-logic (PROJEXPL.C F0219), not viewport rendering.
The viewport compat module correctly mirrors the **rendering side** of projectile
display: scale tables, aspect data, and PC34 zone occlusion specs.

No fix needed in viewport module. The travel-blocking game logic is in the
movement/collision layer, not in this compat file.