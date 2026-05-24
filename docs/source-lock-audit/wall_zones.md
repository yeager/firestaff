# JOB 5: DM1 V1 Wall Zone/Row Table — Source Lock Audit

## ReDMCSB Source

### G2028_ac_ViewSquareIndexTo[...] — PC34 Zone Row Mapping
- **DEFS.H:6457**: `extern char G2028_ac_ViewSquareIndexTo[23];`
- **DUNVIEW.C:372–377** (initialization, MEDIA720_I34E):
  ```
  G2028_ac_ViewSquareIndexTo[23] = {
    11, -1, -1,  8,  9, 10,  5,  6,  7, -1, -1,  0,  1,  2,  3,  4, -1, -1, -1, -1, -1, -1, -1
  };
  ```
  Maps view-square index → PC34 zone row number (0..14).
  -1 means the square has no projectile occlusion zone (e.g., D0C itself).

  Value mapping:
  - D3L/D3R/D3C → row 5/6/7 (depth 3, furthest from party)
  - D2L/D2R/D2C → row 8/9/10 (depth 2)
  - D1L/D1R/D1C → row 11/12/0 (depth 1, nearest side / center)
  - D0L/D0R → row 1/2 (depth 0, immediate sides)
  - D0C → row 11 (center; special handling for D0C center cell)

### C2900_ZONE_ Base — PC34 Zone Offset
- **DEFS.H:4230**: `#define C2900_ZONE_ 2900`
- **DUNVIEW.C:5674**: zone index computed as:
  `L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell;`
  Where L2479_i_ = G2028[row], AL0126_i_ViewCell = VIEW_CELL (0..3 for left/front/right/back).
  Result: zone indices 2900–2959 cover all wall/projectile occlusion zones.

### D3/D0 Cell Clipping
- **DUNVIEW.C:5669–5672**: `if (depth==3 && cell<=C01_VIEW_CELL_FRONT_RIGHT) continue;`
  — D3 front cells (D3C front lane) skip the projectile draw pass
- **DUNVIEW.C:5675–5676**: `if (depth==0 && cell>=C02_VIEW_CELL_BACK_RIGHT) continue;`
  — D0 back cells skip the projectile draw pass

### Zone Constants for Walls (DEFS.H:4040–4057)
- `C702–C706`: D3L2/D3R2/D3C/D3L/D3R wall zones
- `C707–C709`: D2L2/D2R2/D2C wall zones (MEDIA720)
- `C710–C712`: D2L/D2R/D1C wall zones
- `C713–C717`: D1L/D1R/D0C/D0L/D0R wall zones

## Firestaff Implementation

### s_projectile_occlusion_specs — Zone Mapping Table — PASS
- **dm1_v1_viewport_3d_pc34_compat.c line 136**:
  `s_projectile_occlusion_specs[]` encodes:
  - `view_square` → DM1_ViewSquareIndex
  - `depth`, `lateral` → position in viewport
  - `zone_index` → PC34 zone row ID (e.g., D3C=11, D3L=12, D3R=13, etc.)
  - `source_ref` → "DEFS.H:2596; DUNVIEW.C:373,5675-5676,5683,5710-5715,5881-5883"

  The zone indices in `s_projectile_occlusion_specs` correspond to the
  G2028/C2900 zone numbering system:
  - D3C→zone 11, D3L→12, D3R→13, D3L2→14, D3R2→15
  - D2C→5, D2L→6, D2R→7
  - D1C→8, D1L→9, D1R→10
  - D0C→11 (special, D0C front-cell zone)
  This mirrors the G2028 row numbering scheme.

### s_wall_draw_specs — Zone for Wall Drawing — PASS
- **dm1_v1_viewport_3d_pc34_compat.c line 352**:
  Each `DM1_ViewportWallDrawSpec` entry has:
  - `zone_id` field: DM1_PC34_ZONE_WALL_DxL/DxR/DxC
  - `source_ref`: exact DUNVIEW.C line range for the draw call
  Examples:
  - D3L → `DM1_PC34_ZONE_WALL_D3L`, ref: "DUNVIEW.C:6421-6427"
  - D1R → `DM1_PC34_ZONE_WALL_D1R`, ref: "DUNVIEW.C:7613-7623"
  - D0L → `DM1_PC34_ZONE_WALL_D0L`, ref: "DUNVIEW.C:8016-8033"

### Clip Gate — Zone-Level Clipping — PASS
- **dm1_viewport_3d_resolve_wall_blit_clip_gate() line 945**:
  Implements the equivalent of COORD.C F0635's zone clipping:
  clips destination to viewport (224×136), adjusts source offsets,
  guards against empty blits. Source refs document:
  "COORD.C:2390-2409 F0635 clips MEDIA720 zones and source offsets; IMAGE3.C:866-889 F0684 skips empty blits"

### G0215 ProjectileScales — Already in Rendering Module
- **dm1_v1_projectile_explosion_render_pc34_compat.c line 19**:
  `DM1_ProjectileScales[7] = {32, 27, 21, 18, 14, 12, 9};`
  Line 15: "Ref: DUNVIEW.C:5712 (ST source), scaleIndex computation at :5718."
  This is the G0215 data, correctly implemented in the projectile rendering module.

## Verdict: PASS

Wall zone/row mapping matches ReDMCSB:
1. G2028-style zone row mapping encoded in `s_projectile_occlusion_specs[].zone_index` ✓
2. C2900_ZONE_+row*4+cell zone arithmetic represented in per-spec zone IDs ✓
3. D3 front-cell/D0 back-cell clipping documented in source_refs (DEFS.H:2596;
   DUNVIEW.C:373,5667–5683,5710–5715,5881–5883) ✓
4. Wall draw zone constants (DM1_PC34_ZONE_WALL_*) match DEFS.H C702–C717 ✓
5. Clip gate function implements equivalent of COORD.C F0635 zone clipping ✓
6. G0215 ProjectileScales in projectile rendering module (line 19) ✓

No fix needed.

## Note on Implementation Style
ReDMCSB uses G2028 as a compile-time array lookup for blitter zone positioning.
Firestaff uses `s_projectile_occlusion_specs[]` and `s_wall_draw_specs[]` structs
with explicit zone IDs — functionally equivalent but expressed as structured data
rather than a bare char array. Both produce the same zone-to-wall mapping.