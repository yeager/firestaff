#!/usr/bin/env python3
"""Source-lock gate: DM1 V1 viewport wall bitmap parity flip.

Verifies that the Firestaff viewport wall renderer implements the
ReDMCSB DUNVIEW.C G0076_B_UseFlippedWallAndFootprintsBitmaps wall
texture flip pattern:

  (mapX + mapY + direction) & 1  =>  swap L/R wall graphics and flip
  horizontally, matching F0128_DUNGEONVIEW_Draw_CPSF behaviour.

Evidence chain:
  ReDMCSB DUNVIEW.C line ~8359:
    G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001
  DUNVIEW.C F0116 (MEDIA709/720):
    if (G0076_B_UseFlippedWallAndFootprintsBitmaps)
        F0105_DrawFlippedHorizontally(G2107_WallSet[C12_WALL_D3R], C705_ZONE_WALL_D3L)
    else
        F0104_Draw(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L)
  DUNVIEW.C F0117 (MEDIA709/720):
    if (G0076_B_UseFlippedWallAndFootprintsBitmaps)
        F0105_DrawFlippedHorizontally(G2107_WallSet[C13_WALL_D3L], C706_ZONE_WALL_D3R)
    else
        F0104_Draw(G2107_WallSet[C12_WALL_D3R], C706_ZONE_WALL_D3R)
"""
import os, sys, re

SRC = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   "src/engine/m11_game_view.c")

def read(path):
    with open(path) as f:
        return f.read()

def check(label, needle, haystack):
    if needle not in haystack:
        print(f"FAIL: {label}")
        print(f"  Expected substring: {needle!r}")
        sys.exit(1)

src = read(SRC)

# 1. Parity flag helper exists and computes (mapX + mapY + direction) & 1
check("parity helper exists",
      "m11_dm1_use_flipped_walls", src)
check("parity uses mapX+mapY+direction",
      "state->world.party.mapX", src)
check("parity uses direction",
      "state->world.party.direction", src)

# 2. Side walls call the flip helper and swap L/R partner
check("side walls call m11_dm1_use_flipped_walls",
      "flipWalls = m11_dm1_use_flipped_walls(state)", src)

# 2026-07-20 round 15 re-anchor (same-drift-family as the pass510 round-14
# re-anchor): the kSideBlits `partner = i ^ 1` swap was replaced by the
# per-spec receipt decision in the dm1_viewport_3d contract module
# (parity_wall vs native_wall plus parity_flips_horizontally).  The
# original's global G3048->G2107/G3071 wallset swap exists only because
# every square routine consumes one global table; Firestaff's receipt
# decision selects the identical bitmap/flip.
CONTRACT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                        "src/dm1/dm1_v1_viewport_3d_pc34_compat.c")
contract = read(CONTRACT)
check("side wall L/R swap via per-spec parity receipt",
      "return spec->parity_wall;", contract)
check("side wall native wall via per-spec receipt",
      "return spec->native_wall;", contract)
check("side wall horizontal flip via per-spec receipt",
      "spec->parity_flips_horizontally;", contract)
check("contract parity predicate keeps the party tuple",
      "(party_map_x + party_map_y + party_direction) & 1", contract)
check("side walls build host receipt",
      "dm1_viewport_3d_build_side_wall_host_receipt_pc34(", src)
check("side walls draw host receipt",
      "(void)m11_draw_dm1_side_wall_host_receipt(state,", src)
check("side walls call flipped blit",
      "m11_draw_dm1_wall_blit_flipped", src)

# 3. Front walls also flip when parity is set
# Find the front walls function
front_match = re.search(r'static void m11_draw_dm1_front_walls\(.*?\n\}',
                        src, re.DOTALL)
assert front_match, "front walls function not found"
front_src = front_match.group(0)
check("front walls call m11_dm1_use_flipped_walls",
      "m11_dm1_use_flipped_walls(state)", front_src)
check("front walls flip when parity set",
      "m11_draw_dm1_wall_blit_flipped", front_src)
check("front walls cite G0076",
      "G0076", front_src)

# 4. Flip helper does horizontal pixel reversal
flip_match = re.search(r'static int m11_draw_dm1_wall_blit_flipped\(.*?\n\}',
                        src, re.DOTALL)
assert flip_match, "wall blit flipped helper not found"
flip_src = flip_match.group(0)
check("flip helper reverses x",
      "blit->width - 1 - x", flip_src)

print("PASS: viewport wall parity flip gate (4/4 checks)")
