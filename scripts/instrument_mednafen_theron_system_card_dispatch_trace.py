#!/usr/bin/env python3
"""Insert the System Card dispatch trace from its reviewable unified patch."""

from pathlib import Path
import sys

repo = Path(__file__).resolve().parent
source = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
patch = repo / "mednafen_1.32.1_theron_system_card_controller_dispatch_trace.patch"
text = source.read_text()
marker = "TheronPCEFastDratorMenuRouteStep(TheronTraceInstructionPC);"
if text.count(marker) != 1:
    raise SystemExit("FAIL: expected one exact Drator route marker")
if "source=mednafen-pce-fast-system-card-controller-dispatch-v2" in text or \
   "source=mednafen-pce-fast-system-card-controller-dispatch-v3" in text:
    raise SystemExit("FAIL: System Card dispatch trace already instrumented")

added = []
in_hunk = False
for line in patch.read_text().splitlines():
    if line.startswith("@@ "):
        in_hunk = True
        continue
    if not in_hunk:
        continue
    if line.startswith("+") and not line.startswith("+++"):
        added.append(line[1:])
if not added or added[-1].strip() != "}":
    raise SystemExit("FAIL: malformed trace patch additions")
# The route invocation already exists in the instrumented source.
if "TheronPCEFastDratorMenuRouteStep(TheronTraceInstructionPC);" in added[-1]:
    raise SystemExit("FAIL: unexpected route invocation in final patch line")
block = "\n".join(added)
line_start = text.rfind("\n", 0, text.index(marker)) + 1
indent = text[line_start:text.index(marker)]
text = text[:line_start] + block + "\n" + indent + text[line_start:]
source.write_text(text)
