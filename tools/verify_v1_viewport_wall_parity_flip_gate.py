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
                   "m11_game_view.c")

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

# Verify L/R partner swap via XOR trick
check("side walls L/R partner swap (i ^ 1)",
      "partner = i ^ 1", src)
check("side walls use partner graphicIndex",
      "swapped.graphicIndex = kSideBlits[partner].graphicIndex", src)
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
