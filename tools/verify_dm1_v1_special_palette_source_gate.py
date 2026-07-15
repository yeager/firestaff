#!/usr/bin/env python3
"""Keep ReDMCSB-owned TITLE/SWSH/ENTRANCE palette output out of V2 filters."""

from pathlib import Path
import sys


source = Path("src/engine/render_sdl_m11.c").read_text(encoding="utf-8")
start = source.find("int M11_Render_PresentIndexedWithSpecialPalette(")
end = source.find("\nint M11_Render_PresentEpxIndexedWithSpecialPalette(", start)
if start < 0 or end < 0:
    print("missing M11 special-palette presenter", file=sys.stderr)
    sys.exit(1)

body = source[start:end]
if "m11_framebuffer_to_rgba_special" not in body:
    print("special-palette presenter no longer expands source RGB", file=sys.stderr)
    sys.exit(1)
for forbidden in ("m11_apply_v2_special_palette_correction", "m11_apply_v2_filters_rgba_post"):
    if forbidden in body:
        print(f"source-owned special palette passes through {forbidden}", file=sys.stderr)
        sys.exit(1)

print("ok: DM1 TITLE/SWSH/ENTRANCE source palettes bypass V2 color filters")
