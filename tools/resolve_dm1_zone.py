#!/usr/bin/env python3
"""Resolve DM1 layout-696 zones using the ReDMCSB COORD.C F0635_ algorithm.

This is a small parity aid, not a renderer. It consumes
zones_h_reconstruction.json and prints viewport-relative destination zones
and source clipping offsets for native bitmaps.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
records = {int(k): v for k, v in json.load(open(ROOT / "zones_h_reconstruction.json"))["records"].items()}


def right(z): return z[0] + z[2] - 1

def bottom(z): return z[1] + z[3] - 1

def minv(a, b): return a if a < b else b


def resolve(zone_index: int, bmp_w: int, bmp_h: int, shift_x: int = 0, shift_y: int = 0):
    """Port of COORD.C F0635_ for common non-negative bitmap zones.

    Returns dict with dst x/y/w/h and source clip x/y.
    """
    rec = records.get(zone_index)
    if rec is None:
        return None

    l2300_shift = 0  # MASK0x8000 not handled here; pass shift_x/y instead.
    l2307 = [0, 0, 20000, 20000]
    rtype = rec["type"]
    if rtype <= 8:
        l2302 = rec["d1"]
        l2303 = rec["d2"]
    elif rtype == 9:
        return None
    else:
        rtype -= 10
        l2302 = 0
        l2303 = 0

    if l2300_shift:
        l2302 += shift_x
        l2303 += shift_y
        shift_x = shift_y = 0

    l2305 = False
    cur_index = zone_index
    cur = rec
    while cur["parent"]:
        if 10 <= cur["type"] <= 18:
            parent1 = records.get(cur["parent"])
            if parent1 is None:
                break
            l2300 = parent1["d1"]
            l2301 = parent1["d2"]
            parent1_type = parent1["type"]
            parent2 = records.get(parent1["parent"])
            if parent2 is None:
                break
            p2w = parent2["d1"]
            p2h = parent2["d2"]

            if parent1_type == 0:
                l2301 -= (p2h + 1) >> 1
                l2300 -= (p2w + 1) >> 1
            elif parent1_type == 5:
                l2300 -= (p2w + 1) >> 1
            elif parent1_type == 3:
                l2301 -= p2h - 1
                l2300 -= p2w - 1
            elif parent1_type == 2:
                l2300 -= p2w - 1
            elif parent1_type == 6:
                l2300 -= p2w - 1
                l2301 -= (p2h + 1) >> 1
            elif parent1_type == 8:
                l2301 -= (p2h + 1) >> 1
            elif parent1_type == 7:
                l2300 -= (p2w + 1) >> 1
                l2301 -= p2h - 1
            elif parent1_type == 4:
                l2301 -= p2h - 1
            elif parent1_type == 1:
                pass
            else:
                return None

            l2307[0] += l2300
            if l2307[0] < l2300: l2307[0] = l2300
            if right(l2307) >= p2w + l2300:
                l2307[2] = p2w - l2307[0] + l2300
            l2307[1] += l2301
            if l2307[1] < l2301: l2307[1] = l2301
            if bottom(l2307) >= p2h + l2301:
                l2307[3] = p2h - l2307[1] + l2301

            if cur["type"] == 10:
                l2301 += (p2h + 1) >> 1
                l2300 += (p2w + 1) >> 1
            elif cur["type"] == 15:
                l2300 += (p2w + 1) >> 1
            elif cur["type"] == 13:
                l2301 += p2h - 1
                l2300 += p2w - 1
            elif cur["type"] == 12:
                l2300 += p2w - 1
            elif cur["type"] == 16:
                l2300 += p2w - 1
                l2301 += (p2h + 1) >> 1
            elif cur["type"] == 18:
                l2301 += (p2h + 1) >> 1
            elif cur["type"] == 17:
                l2300 += (p2w + 1) >> 1
                l2301 += p2h - 1
            elif cur["type"] == 14:
                l2301 += p2h - 1
            elif cur["type"] == 11:
                pass
            else:
                return None
            l2302 += l2300 + cur["d1"]
            l2303 += l2301 + cur["d2"]
            cur = parent2
            cur_index = parent1["parent"]
        else:
            parent = records.get(cur["parent"])
            if parent is None:
                break
            l2300 = parent["d1"]
            l2301 = parent["d2"]
            if parent["type"] == 1:
                l2302 += l2300
                l2303 += l2301
                l2307[0] += l2300
                l2307[1] += l2301
            elif parent["type"] == 9:
                if cur["type"] == 0:
                    l2300 = cur["d1"] - ((l2300 + 1) >> 1)
                    l2301 = cur["d2"] - ((l2301 + 1) >> 1)
                elif cur["type"] == 1:
                    l2300 = cur["d1"]
                    l2301 = cur["d2"]
                elif cur["type"] == 2:
                    l2300 = cur["d1"] - (l2300 - 1)
                    l2301 = cur["d2"]
                elif cur["type"] == 3:
                    l2300 = cur["d1"] - (l2300 - 1)
                    l2301 = cur["d2"] - (l2301 - 1)
                elif cur["type"] == 4:
                    l2300 = cur["d1"]
                    l2301 = cur["d2"] - (l2301 - 1)
                elif cur["type"] == 5:
                    l2300 = cur["d1"] - ((l2300 + 1) >> 1)
                    l2301 = cur["d2"]
                elif cur["type"] == 6:
                    l2300 = cur["d1"] - (l2300 - 1)
                    l2301 = cur["d2"] - ((l2301 + 1) >> 1)
                elif cur["type"] == 7:
                    l2300 = cur["d1"] - ((l2300 + 1) >> 1)
                    l2301 = cur["d2"] - (l2301 - 1)
                elif cur["type"] == 8:
                    l2300 = cur["d1"]
                    l2301 = cur["d2"] - ((l2301 + 1) >> 1)
                if l2305:
                    l2305 = False
                    l2302 += l2300
                    l2303 += l2301
                    l2307[0] += l2300
                    l2307[1] += l2301
                if l2307[0] < l2300: l2307[0] = l2300
                if right(l2307) >= parent["d1"] + l2300:
                    l2307[2] = parent["d1"] - l2307[0] + l2300
                if l2307[1] < l2301: l2307[1] = l2301
                if bottom(l2307) >= parent["d2"] + l2301:
                    l2307[3] = parent["d2"] - l2307[1] + l2301
            elif parent["type"] <= 8:
                l2305 = True
            cur = parent

    l2300 = shift_x or bmp_w
    l2301 = shift_y or bmp_h
    if rtype == 0:
        dstx = l2302 - ((l2300 + 1) >> 1); dsty = l2303 - ((l2301 + 1) >> 1)
    elif rtype == 1:
        dstx = l2302; dsty = l2303
    elif rtype == 2:
        dstx = l2302 - (l2300 - 1); dsty = l2303
    elif rtype == 3:
        dstx = l2302 - (l2300 - 1); dsty = l2303 - (l2301 - 1)
    elif rtype == 4:
        dstx = l2302; dsty = l2303 - (l2301 - 1)
    elif rtype == 5:
        dstx = l2302 - ((l2300 + 1) >> 1); dsty = l2303
    elif rtype == 6:
        dstx = l2302 - (l2300 - 1); dsty = l2303 - ((l2301 + 1) >> 1)
    elif rtype == 7:
        dstx = l2302 - ((l2300 + 1) >> 1); dsty = l2303 - (l2301 - 1)
    elif rtype == 8:
        dstx = l2302; dsty = l2303 - ((l2301 + 1) >> 1)
    else:
        return None

    sx = l2307[0] - dstx
    sy = l2307[1] - dsty
    if sx <= 0:
        srcx = 0
        dstw = minv(l2300, l2307[2] + sx)
    else:
        srcx = sx
        dstx = l2307[0]
        dstw = minv(l2300 - sx, l2307[2])
    if sy <= 0:
        srcy = 0
        dsth = minv(l2301, l2307[3] + sy)
    else:
        srcy = sy
        dsty = l2307[1]
        dsth = minv(l2301 - sy, l2307[3])
    if dstw <= 0 or dsth <= 0:
        return None
    return {"zone": zone_index, "x": dstx, "y": dsty, "w": dstw, "h": dsth, "srcX": srcx, "srcY": srcy}


WALLS = [
    (104, 702, "D3L2"), (103, 703, "D3R2"), (107, 704, "D3C"),
    (106, 705, "D3L"), (105, 706, "D3R"),
    (99, 707, "D2L2"), (98, 708, "D2R2"), (102, 709, "D2C"),
    (101, 710, "D2L"), (100, 711, "D2R"),
    (97, 712, "D1C"), (96, 713, "D1L"), (95, 714, "D1R"),
    (94, 716, "D0L"), (93, 717, "D0R"),
]

if __name__ == "__main__":
    import json as _json
    manifest = _json.load(open(ROOT / "extracted-graphics-v1/manifest.json"))["entries"]
    by_index = {e["index"]: e for e in manifest}
    print("graphic,zone,name,bmpW,bmpH,dstX,dstY,dstW,dstH,srcX,srcY")
    for gfx, zone, name in WALLS:
        e = by_index[gfx]
        z = resolve(zone, e["width"], e["height"])
        if z is None:
            print(f"{gfx},{zone},{name},{e['width']},{e['height']},NULL")
        else:
            print(f"{gfx},{zone},{name},{e['width']},{e['height']},{z['x']},{z['y']},{z['w']},{z['h']},{z['srcX']},{z['srcY']}")
