#!/usr/bin/env python3
"""Check a real-media M12 menu runtime probe without shell heredoc temps."""

import json
import sys

if len(sys.argv) < 3:
    raise SystemExit("usage: verify_main_menu_runtime_probe.py <game-id> <probe>")
game_id, probe_path = sys.argv[1:3]
if not game_id.startswith("theron-direct-") and len(sys.argv) != 3:
    raise SystemExit("FAIL: unexpected arguments for non-direct game probe")
with open(probe_path, encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
expected_source_id = "dm2" if game_id == "dm2-amiga" else game_id
if game_id.startswith("theron-direct-"):
    expected_track02_md5_by_route = {
        "theron-direct-us": "f23601102138f87c33025877767ebf76",
        "theron-direct-jp": "b7afb338ad31be1025b53f9aff12d73a",
    }
    if len(sys.argv) != 3 or game_id not in expected_track02_md5_by_route:
        raise SystemExit(
            "FAIL: Theron direct probe requires route theron-direct-us or "
            "theron-direct-jp and exactly one probe path"
        )
    expected_track02_md5 = expected_track02_md5_by_route[game_id]
    if (
        probe.get("launchedEver") != 1
        or probe.get("active") != 1
        or probe.get("sourceId") != "theron"
        or probe.get("bootAssetMd5") != expected_track02_md5
        or startup.get("receiptReady") != 1
        or startup.get("phase") != "theron-startup-0"
        or startup.get("startupActive") != 1
        or startup.get("levelLoaded") != 0
    ):
        raise SystemExit(
            f"FAIL: one {game_id} selection missed the native startup handoff: {probe}"
        )
    print(f"PASS: one no-game M12 {game_id} selection entered native startup")
    raise SystemExit(0)

if (
    probe.get("launchedEver") != 1
    or probe.get("active") != 1
    or probe.get("sourceId") != expected_source_id
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
elif game_id == "dm2-amiga":
    runtime = probe.get("dm2RuntimeFrame", {})
    # DM2_PLATFORM_AMIGA_EN is enum value 6 in include/dm2_v1_boot.h.
    if (
        startup.get("phase") != "dm2-runtime"
        or probe.get("dm2Startup", {}).get("platform") != 6
        or runtime
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
            f"FAIL: DM2 Amiga menu start missed its authentic first runtime: {probe}"
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
