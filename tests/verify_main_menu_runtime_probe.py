#!/usr/bin/env python3
"""Check a real-media M12 menu runtime probe without shell heredoc temps."""

import json
import sys

game_id, probe_path = sys.argv[1:]
with open(probe_path, encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (
    probe.get("launchedEver") != 1
    or probe.get("active") != 1
    or probe.get("sourceId") != game_id
    or startup.get("receiptReady") != 1
    or startup.get("startupActive") != 0
    or startup.get("levelLoaded") != 1
    or probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}
):
    raise SystemExit(
        f"FAIL: {game_id} menu selection did not reach active runtime: {probe}"
    )

if game_id == "dm1":
    if (
        startup.get("phase") != "dm1-runtime"
        or startup.get("dm1StartupHoCFirstFrameReady") != 1
    ):
        raise SystemExit(f"FAIL: DM1 menu start missed its HoC runtime frame: {probe}")
elif game_id == "csb":
    if (
        startup.get("phase") != "inactive"
        or not probe.get("csbViewportHash")
        or (
            party.get("mapIndex"),
            party.get("mapX"),
            party.get("mapY"),
            party.get("direction"),
            party.get("championCount"),
        )
        != (4, 22, 18, 2, 1)
    ):
        raise SystemExit(
            f"FAIL: CSB menu start missed its original MINI.DAT party: {probe}"
        )
elif game_id == "dm2":
    title = probe.get("dm2FmtownsStartup", {})
    frame = probe.get("dm2RuntimeFrame", {})
    if (
        startup.get("phase") != "dm2-runtime"
        or title.get("titleFinished") != 1
        or title.get("frameIndex") != 225
        or frame
        != {"accepted": 1, "realAssets": 1, "noCoreFallbacks": 1, "fallbackDraws": 0}
        or (
            party.get("mapIndex"),
            party.get("mapX"),
            party.get("mapY"),
            party.get("direction"),
            party.get("championCount"),
        )
        != (0, 1, 8, 0, 1)
    ):
        raise SystemExit(
            f"FAIL: DM2 menu start missed its authentic first party: {probe}"
        )
elif game_id == "theron":
    if (
        startup.get("phase") != "theron-runtime"
        or probe.get("bootAssetMd5") != "f23601102138f87c33025877767ebf76"
        or (
            party.get("mapIndex"),
            party.get("mapX"),
            party.get("mapY"),
            party.get("direction"),
            party.get("championCount"),
        )
        != (0, 1, 0, 0, 2)
    ):
        raise SystemExit(
            f"FAIL: Theron menu start missed its authenticated Track 02 runtime: {probe}"
        )
else:
    raise SystemExit(f"FAIL: unsupported game in real-media menu test: {game_id}")

print(f"PASS: no-game M12 menu {game_id} reached its authentic first runtime")
