#!/usr/bin/env python3
"""Guard CUE-backed static VDP owner analysis against loose-file regressions."""

from pathlib import Path


source = (Path(__file__).resolve().parents[1] /
          "scripts/analyze_nexus_tm_bin_vdp_owner.py").read_text()
required = (
    'source.add_argument("--cue"',
    "iso_members_in_memory(cue_track1(args.cue), {name})[name]",
    'LOAD_BASE = {"DM.BIN": 0x06010040, "TM.BIN": 0x06010000}',
    "runtime_instruction=",
    "runtime_literal=",
    "semantic_admission=blocked",
)
missing = [item for item in required if item not in source]
if missing:
    raise SystemExit("missing CUE VDP owner invariant: " + ", ".join(missing))
print("ok: Nexus CUE VDP owner analyser retains source identity and load base")
