#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EVIDENCE = ROOT / "parity-evidence/verification/nexus_v2_verification_suite_source_lock.json"

REQUIRED_SOURCE = [
    (SOURCE / "GAMELOOP.C", "F0128_DUNGEONVIEW_Draw_CPSF", (88, 91)),
    (SOURCE / "DUNVIEW.C", "void F0128_DUNGEONVIEW_Draw_CPSF", (8318, 8323)),
    (SOURCE / "DUNVIEW.C", "F0674_F0128_sub(G2109_Ceiling", (8365, 8369)),
    (SOURCE / "DUNVIEW.C", "F0674_F0128_sub(G2108_Floor", (8429, 8433)),
    (SOURCE / "PRIM1.C", "#define M526_WaitVerticalBlank()        Vsync()", (745, 745)),
]

REQUIRED_FIRESTAFF = [
    (ROOT / "include/nexus_v1_rasterizer.h", "#define NEXUS_FB_W 320"),
    (ROOT / "include/nexus_v1_rasterizer.h", "#define NEXUS_FB_H 200"),
    (ROOT / "include/nexus_v2_config.h", "NEXUS_V2_OFF = 0"),
    (ROOT / "include/nexus_v2_config.h", "NEXUS_V2_UPSCALED"),
    (ROOT / "include/nexus_v2_config.h", "NEXUS_V2_ENHANCED"),
    (ROOT / "src/nexus/nexus_v2_render_pipeline.c", "nexus_v2_pipeline_init"),
    (ROOT / "src/nexus/nexus_v2_render_pipeline.c", "nexus_v2_pipeline_render"),
    (ROOT / "src/nexus/nexus_v2_render_pipeline.c", "pipe->output_w = pipe->config.render_width"),
    (ROOT / "src/nexus/nexus_v2_render_pipeline.c", "v1_fb->palette[v1_fb->color_buffer[i]]"),
    (ROOT / "probes/firestaff_nexus_v2_verification_suite_probe.c", "check_v2_off_byte_stability"),
    (ROOT / "probes/firestaff_nexus_v2_verification_suite_probe.c", "check_state_hash_gate"),
    (ROOT / "probes/firestaff_nexus_v2_verification_suite_probe.c", "check_config_mode_to_dimensions"),
    (ROOT / "CMakeLists.txt", "nexus_v2_verification_suite_probe"),
]

errors: list[str] = []
anchors: list[dict[str, str]] = []

for path, needle, line_range in REQUIRED_SOURCE:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    if needle not in text:
        errors.append(f"missing {needle!r} in {path.name}")
    start, end = line_range
    if not (1 <= start <= end <= len(lines)):
        errors.append(f"line range out of range {path.name}:{start}-{end}")
    else:
        anchors.append({
            "file": path.name,
            "lineRange": f"{start}-{end}",
            "needle": needle,
            "text": lines[start - 1].strip(),
        })

for path, needle in REQUIRED_FIRESTAFF:
    text = path.read_text(encoding="utf-8", errors="replace")
    if needle not in text:
        errors.append(f"missing Firestaff Nexus V2 phase7 text {needle!r} in {path.relative_to(ROOT)}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "nexus_v2_verification_suite source-lock",
    "anchors": anchors,
    "errors": errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
if errors:
    for error in errors:
        print("error:", error)
    raise SystemExit(1)
print(f"nexus_v2_verification_suite_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
