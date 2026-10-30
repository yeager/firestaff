#!/usr/bin/env python3
"""Insert the bounded System Card bootstrap-follow trace into the PCE Fast loop."""

from pathlib import Path
import sys

repo = Path(__file__).resolve().parent
source = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
patch = repo / "mednafen_1.32.1_theron_system_card_bootstrap_follow.patch"
text = source.read_text()
marker = "TheronPCEFastDratorMenuRouteStep(TheronTraceInstructionPC);"
if text.count(marker) != 1:
    raise SystemExit("FAIL: expected one exact Drator route marker")
if "FIRESTAFF_THERON_SYSTEM_CARD_DISPATCH_TRACE" not in text:
    raise SystemExit("FAIL: System Card dispatch hook must be instrumented first")
if "FIRESTAFF_THERON_SYSTEM_CARD_BOOTSTRAP_FOLLOW_TRACE" in text:
    raise SystemExit("FAIL: bootstrap-follow trace is already instrumented")

added = []
in_hunk = False
for line in patch.read_text().splitlines():
    if line.startswith("@@ "):
        in_hunk = True
        continue
    if in_hunk and line.startswith("+") and not line.startswith("+++"):
        added.append(line[1:])
if not added or added[-1].strip() != "}":
    raise SystemExit("FAIL: malformed bootstrap-follow patch additions")
block = "\n".join(added)
line_start = text.rfind("\n", 0, text.index(marker)) + 1
indent = text[line_start:text.index(marker)]
text = text[:line_start] + block + "\n" + indent + text[line_start:]
source.write_text(text)
