#!/usr/bin/env python3
"""Verify first-runtime receipts for no-game M12 alternate-platform routes."""

import json
import sys


def fail(message: str, probe: dict) -> None:
    raise SystemExit(f"FAIL: {message}: {probe}")


if len(sys.argv) != 3:
    raise SystemExit(
        "usage: verify_m12_alternate_platform_runtime.py <route> <probe>"
    )

route, probe_path = sys.argv[1:]
with open(probe_path, encoding="utf-8") as source:
    probe = json.load(source)

startup = probe["startup"]
party = probe["party"]
party_state = (
    party["mapIndex"], party["mapX"], party["mapY"],
    party["direction"], party["championCount"],
)
if probe.get("launchedEver") != 1 or probe.get("active") != 1:
    fail(f"{route} did not launch", probe)

if route == "dm1-pc":
    if (probe.get("sourceId") != "dm1" or startup.get("receiptReady") != 1 or
            startup.get("phase") != "dm1-runtime" or
            startup.get("startupActive") != 0 or
            startup.get("levelLoaded") != 1 or party_state != (0, 1, 3, 2, 0)):
        fail("DM1 PC-34 picker route missed its first runtime", probe)
elif route in {
    "dm1-atari-st", "dm1-atari-st-11", "dm1-atari-st-12", "dm1-atari-st-13"
}:
    if (probe.get("sourceId") != "dm1" or startup.get("receiptReady") != 1 or
            startup.get("phase") != "dm1-runtime" or
            startup.get("startupActive") != 0 or
            startup.get("levelLoaded") != 1 or
            startup.get("dm1StartupHandoffExecuted") != 1 or
            startup.get("dm1StartupHoCFirstFrameReady") != 1 or
            party_state != (0, 1, 3, 2, 0)):
        fail("DM1 Atari ST picker route missed its first Hall runtime", probe)
elif route == "csb-atari-st":
    if (probe.get("sourceId") != "csb" or startup.get("receiptReady") != 1 or
            startup.get("phase") != "inactive" or
            startup.get("startupActive") != 0 or
            startup.get("levelLoaded") != 1 or
            probe.get("gameTick", 0) < 1 or
            not probe.get("csbViewportHash") or
            party_state != (0, 9, 1, 2, 0)):
        fail("CSB Atari ST picker route missed its C200-to-runtime step", probe)
elif route == "dm2-pc":
    if (probe.get("sourceId") != "dm2" or startup.get("receiptReady") != 1 or
            startup.get("phase") != "dm2-runtime" or
            startup.get("startupActive") != 0 or
            startup.get("levelLoaded") != 1 or
            probe.get("dm2RuntimeFrame") != {
                "accepted": 1, "realAssets": 1,
                "noCoreFallbacks": 1, "fallbackDraws": 0,
            } or party_state != (0, 1, 8, 0, 1)):
        fail("DM2 PC-DOS picker route missed its first dungeon frame", probe)
else:
    raise SystemExit(f"FAIL: unsupported M12 alternate runtime route: {route}")

print(f"PASS: no-game M12 {route} route reached its authentic first runtime")
