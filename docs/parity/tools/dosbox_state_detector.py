#!/usr/bin/env python3
"""dosbox_state_detector.py — classify DOSBox screenshots into DM1 game states."""
from PIL import Image
import sys
from pathlib import Path

# Density thresholds calibrated from pass94 frame analysis
_VIEWPORT_NONBLACK_THRESH = 0.135   # ~(19+19+19+20)/136 * 224 ≈ 13.5%
_RIGHTCOL_NONBLACK_THRESH = 0.135   # same metric for right-column region
_CHAMP_NONBLACK_THRESH    = 0.50    # champion panel (y=0..64) must be >50% nonblack

def classify(img: Image.Image) -> str:
    """Classify a 320x200 screenshot. Returns one of six state strings.

    State machine:
      title_screen     — v<0.10 AND r<0.10  (mostly black)
      entrance_menu    — v>0.70 AND r>0.70  (viewport AND controls both dense)
      champion_create  — c>0.50 AND v<0.10  (champion panel strong, viewport empty)
      dungeon_gameplay — v>0.70 AND r<0.10  (viewport dense, NO right controls)
      wall_closeup     — v<0.10 AND r>0.70  (no viewport, controls = wall/selector)
      unclassified     — otherwise
    """
    img = img.convert("RGB").resize((320, 200))
    import numpy as np
    arr = np.array(img)

    # Viewport region: x=0..223, y=33..168
    vp = arr[33:169, 0:224]
    vp_nb = np.count_nonzero(np.any(vp != 0, axis=2))
    v_dens = vp_nb / vp.size

    # Right-column controls: x=224..319, y=33..168
    rc = arr[33:169, 224:320]
    rc_nb = np.count_nonzero(np.any(rc != 0, axis=2))
    r_dens = rc_nb / rc.size

    # Champion panel: x=0..320, y=0..64
    cp = arr[0:65, 0:320]
    cp_nb = np.count_nonzero(np.any(cp != 0, axis=2))
    c_dens = cp_nb / cp.size

    if v_dens > 0.70 and r_dens > 0.70:
        return "entrance_menu"
    elif v_dens > 0.70 and r_dens < 0.10:
        return "dungeon_gameplay"
    elif c_dens > _CHAMP_NONBLACK_THRESH and v_dens < 0.10:
        return "champion_create"
    elif v_dens < 0.10 and r_dens > 0.70:
        return "wall_closeup"
    elif v_dens < 0.10 and r_dens < 0.10:
        return "title_screen"
    else:
        return "unclassified"

if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else str(Path("/tmp/dosbox_tmp.png"))
    img = Image.open(path)
    result = classify(img)
    confidence = "HIGH" if result != "unclassified" else "LOW"
    print(f"state={result} confidence={confidence}")
    sys.exit(0)
