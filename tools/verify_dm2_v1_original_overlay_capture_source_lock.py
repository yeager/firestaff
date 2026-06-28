#!/usr/bin/env python3
"""Source-lock the DM2 V1 original-overlay capture route before parity claims.

This verifier is intentionally read-only. It ties the DM2 viewport/HUD/mouse
capture path to SKULLWIN/SKWin owned screen/viewport/input paths, then reports
whether the capture harness is semantically usable for future overlay parity.

It is intentionally a scaffold gate, not a parity claim:
- it does not require a paired Firestaff-vs-original pixel diff;
- it does not require a DM2 dungeon_gameplay route to be reproduced on this
  host (none has been observed yet);
- it locks only the structural facts that future paired evidence will need
  (viewport geometry, blit path, mouse/command queue, HUD panel selection,
  right-panel composition, intro/main-menu dispatch).

Source of truth:
- DM2 PC 1.0 EN canonical archive provenance is recorded separately in
  docs/dm2_source_lock.md and tools/verify_dm2_v1_phase0_provenance_gate.py.
- The overlay capture script is scripts/dosbox_dm2_original_overlay_capture.sh.

Honest scope: this script confirms that the SKULLWIN source anchors the
DM2 capture path needs are present on this host. It is the readiness gate
the gap list calls out, not the parity gate.
"""
from __future__ import annotations

import argparse
import json
import os
import re
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

# Search paths for SKULLWIN source. Newest entry wins; missing paths degrade
# gracefully so the verifier can still report partial success.
DEFAULT_SKULLWIN_SEARCH = (
    Path.home() / ".openclaw/data/firestaff-dm2-sources/skproject.git/SKULLWIN",
    Path.home() / ".openclaw/data/firestaff-dm2-sources/skproject/SKULLWIN",
    Path("/Volumes/Extern-disk/openclaw-data/firestaff/skproject-source/SKULLWIN"),
    REPO / "skproject-source/SKULLWIN",
    REPO / "skproject/SKULLWIN",
)

DEFAULT_CAPTURE_SCRIPT = REPO / "scripts/dosbox_dm2_original_overlay_capture.sh"
DEFAULT_VERIFIER_PROBE = REPO / "build/firestaff_dm2_v1_original_overlay_capture_scaffold_probe"
DEFAULT_EVIDENCE_DIR = REPO / "verification-screens/passH2313-dm2-original-overlays"
DEFAULT_PAIR_MANIFEST = DEFAULT_EVIDENCE_DIR / "dm2_original_firestaff_pair_manifest.tsv"

PAIR_MANIFEST_COLUMNS = [
    "pair_id",
    "state",
    "classification",
    "route_label",
    "original_frame_sha256",
    "original_viewport_sha256",
    "original_viewport_width",
    "original_viewport_height",
    "firestaff_frame_sha256",
    "firestaff_viewport_sha256",
    "firestaff_viewport_width",
    "firestaff_viewport_height",
    "diff_sha256",
    "status",
]

SHA256_RE = re.compile(r"^[0-9a-fA-F]{64}$")

# Each anchor pins a specific DM2 capture-pipeline fact against SKULLWIN source.
SOURCE_CHECKS = [
    {
        "id": "viewport-screen-size",
        "file": "dm2global.h",
        "start": 1,
        "end": 60,
        "needles": ["#define ORIG_SWIDTH  (320)", "#define ORIG_SHEIGHT (200)"],
        "claim": (
            "DM2 PC 1.0 EN original VGA screen buffer is 320x200 (dm2global.h ORIG_SWIDTH/ORIG_SHEIGHT). "
            "Capture normalization must downsample any DOSBox 640x400 2x capture to 320x200 before cropping."
        ),
    },
    {
        "id": "backbuffer-geometry",
        "file": "c_gfx_main.cpp",
        "start": 1,
        "end": 60,
        "needles": ["backbuffer_w = 0xe0;", "backbuffer_h = 0x88;"],
        "claim": (
            "DM2 dungeon backbuffer is 224x136 (0xe0 x 0x88). Cropping the 320x200 frame at x=0..223, "
            "y=33..169 matches the SKULLWIN backbuffer rectangle."
        ),
    },
    {
        "id": "backbuffer-init-header",
        "file": "c_gfx_main.cpp",
        "start": 30,
        "end": 60,
        "needles": ["DM2_INIT_BACKBUFF(void)", "bmpheader->width = gfxsys.backbuffer_w;", "bmpheader->height = gfxsys.backbuffer_h;"],
        "claim": (
            "DM2_INIT_BACKBUFF writes the 224x136 backbuffer dimensions into the bitmap header before "
            "any viewport blit; this is the source-side fact the scaffold crop normalizes to."
        ),
    },
    {
        "id": "viewport-blit-region-queries",
        "file": "c_gui_vp.cpp",
        "start": 870,
        "end": 970,
        "needles": ["DM2_QUERY_BLIT_RECT", "DM2_blit_specialeffects"],
        "claim": (
            "DM2 viewport rendering composes ceiling/wall/floor/sprite regions via DM2_QUERY_BLIT_RECT "
            "and DM2_blit_specialeffects (RG1R/RG2R/RG3R region queries). The capture pipeline must "
            "preserve these named regions so future overlay diffs can target them individually."
        ),
    },
    {
        "id": "viewport-buffer-bind",
        "file": "c_gfx_main.h",
        "start": 1,
        "end": 60,
        "needles": ["c_pixel256 dm2screen[ORIG_SWIDTH * ORIG_SHEIGHT];", "c_pixel256 dm2mscreen[ORIG_SWIDTH * ORIG_SHEIGHT];"],
        "claim": (
            "The DM2 320x200 surface buffer (dm2screen) and mouse overlay surface (dm2mscreen) are "
            "ORIG_SWIDTH * ORIG_SHEIGHT arrays; capture normalization must match these dimensions exactly."
        ),
    },
    {
        "id": "mouse-input-queue-length",
        "file": "c_tmouse.h",
        "start": 1,
        "end": 40,
        "needles": ["#define MOUSE_QUEUE_LENGTH (10)", "c_evententry queue[MOUSE_QUEUE_LENGTH];"],
        "claim": (
            "DM2's c_mousequeue is bounded at 10 entries; the capture script must pace injected clicks "
            "so the queue does not overflow when the operator runs a long route."
        ),
    },
    {
        "id": "command-queue-length",
        "file": "c_tmouse.h",
        "start": 115,
        "end": 150,
        "needles": ["#define COMMAND_QUEUE_LENGTH  (10)", "c_servercommand queue[COMMAND_QUEUE_LENGTH];"],
        "claim": (
            "DM2's c_commandqueue is bounded at 10 entries; capture routes must wait between "
            "command-bearing keystrokes to keep the queue under capacity."
        ),
    },
    {
        "id": "mouse-event-fields",
        "file": "types.h",
        "start": 130,
        "end": 160,
        "needles": ["class c_evententry", "i16 b;", "i16 x;", "i16 y;"],
        "claim": (
            "DM2 mouse events carry button (b), x, y ints. Capture-side click coordinates in the original "
            "320x200 frame map to (x, y) of c_evententry without any aspect-fit remap on the engine side."
        ),
    },
    {
        "id": "right-panel-squad-hands",
        "file": "c_gui_draw.cpp",
        "start": 4170,
        "end": 4200,
        "needles": ["DM2_DISPLAY_RIGHT_PANEL_SQUAD_HANDS(void)"],
        "claim": (
            "The DM2 default right-panel (squad hands) is drawn via DM2_DISPLAY_RIGHT_PANEL_SQUAD_HANDS; "
            "the capture pipeline labels this state explicitly so future overlay diffs can isolate "
            "HUD changes from viewport changes."
        ),
    },
    {
        "id": "input-loop-dispatch",
        "file": "c_input.cpp",
        "start": 790,
        "end": 820,
        "needles": ["void DM2_IBMIO_USER_INPUT_CHECK(void)"],
        "claim": (
            "DM2's input loop is dispatched through DM2_IBMIO_USER_INPUT_CHECK; capture routes "
            "must keep DOSBox focus on the SKULL.EXE window so input events route to c_Tmouse."
        ),
    },
]

# These strings must be present in the capture script for the route to be
# reproducible. They mirror tools/verify_original_overlay_capture_source_lock.py
# ROUTE_TOOL_CHECKS.
ROUTE_TOOL_CHECKS = [
    ("scripts/dosbox_dm2_original_overlay_capture.sh", [
        "DM2_ORIGINAL_ROUTE_EVENTS",
        "DM2_ORIGINAL_EXPECTED_SHOTS",
        "shot:<label>",
        "rclick:<x>,<y>",
        "--normalize-only",
        "224x136",
        "backbuffer_w",
        "DM2_BLIT_SPECIALEFFECTS_HONEST_BOUNDARY",
    ]),
]


def display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def find_skullwin_root() -> Path | None:
    for candidate in DEFAULT_SKULLWIN_SEARCH:
        if candidate.exists() and candidate.is_dir():
            return candidate
    return None


def read_source(root: Path, rel: str) -> list[str]:
    path = root / rel
    if not path.exists():
        raise AssertionError(f"missing source file: {path}")
    return path.read_text(errors="replace").splitlines()


def check_sources(root: Path | None) -> tuple[bool, str]:
    if root is None:
        print("section=redmcsb_source_lock")
        print("skullwinSource=missing")
        for check in SOURCE_CHECKS:
            print(f"sourceRange={check['file']} id={check['id']} status=skullwin_missing")
            print(f"sourceClaim={check['claim']}")
        print("redmcsbSourceLockOk=0")
        print("honesty=SKULLWIN source mirror not found on this host; "
              "downloader + extract gate required before any overlay claim.")
        return False, "missing"
    print("section=skullwin_source_lock")
    print(f"skullwinSource={root}")
    ok = True
    for check in SOURCE_CHECKS:
        try:
            lines = read_source(root, check["file"])
            excerpt = "\n".join(lines[check["start"] - 1 : check["end"]])
            missing = [needle for needle in check["needles"] if needle not in excerpt]
        except AssertionError as exc:
            missing = [str(exc)]
            excerpt = ""
        status = "ok" if not missing else "missing:" + ";".join(missing)
        print(f"sourceRange={check['file']}:{check['start']}-{check['end']} id={check['id']} status={status}")
        print(f"sourceClaim={check['claim']}")
        ok = ok and not missing
    print(f"skullwinSourceLockOk={1 if ok else 0}")
    return ok, "ok"


def check_route_tools() -> bool:
    ok = True
    print("section=route_tool_lock")
    for rel, needles in ROUTE_TOOL_CHECKS:
        path = REPO / rel
        if not path.exists():
            print(f"routeTool={rel} status=missing")
            ok = False
            continue
        text = path.read_text(errors="replace")
        missing = [needle for needle in needles if needle not in text]
        status = "ok" if not missing else "missing:" + ",".join(missing)
        print(f"routeTool={rel} status={status}")
        ok = ok and not missing
    # Probe must build before the CTest gate can claim readiness.
    probe = DEFAULT_VERIFIER_PROBE
    if probe.exists():
        print(f"routeTool={display(probe)} status=present")
    else:
        print(f"routeTool={display(probe)} status=not_built (cmake --build build --target firestaff_dm2_v1_original_overlay_capture_scaffold_probe)")
        # Treat the probe as a soft requirement: scaffold is still OK if the
        # verifier script + capture script + source locks are in place, but the
        # CTest gate does require the probe binary.
        print("routeToolProbeBuilt=0")
    print(f"routeToolLockOk={1 if ok else 0}")
    return ok


def check_attempt(attempt_dir: Path) -> bool:
    classifier = attempt_dir / "dm2_raw_frame_health.json"
    labels = attempt_dir / "dm2_original_overlay_shot_labels.tsv"
    crop_manifest = attempt_dir / "dm2_viewport_224x136_manifest.tsv"
    print("section=attempt_semantic_gate")
    print(f"attemptDir={display(attempt_dir)}")
    if not labels.exists():
        print(f"labelManifest={display(labels)} status=missing")
    else:
        print(f"labelManifest={display(labels)} status=present")
    if not crop_manifest.exists():
        print(f"cropManifest={display(crop_manifest)} status=missing")
    else:
        print(f"cropManifest={display(crop_manifest)} status=present")
    if not classifier.exists():
        print(f"classifierJson={display(classifier)} status=missing")
        print("semanticReadyForOverlay=0")
        return False
    data = json.loads(classifier.read_text())
    passed = bool(data.get("pass"))
    print(f"classifierJson={display(classifier)} status=present pass={1 if passed else 0}")
    print(f"captureCount={data.get('capture_count')}")
    for problem in data.get("problems", []):
        print(f"attemptProblem={problem}")
    print(f"semanticReadyForOverlay={1 if passed else 0}")
    return passed


def _valid_sha256(value: str) -> bool:
    return bool(SHA256_RE.fullmatch(value or ""))


def check_pair_manifest(manifest: Path) -> tuple[bool, bool]:
    """Validate the optional original-vs-Firestaff pair manifest.

    Missing is an honest OPEN state, not a failure. Once a manifest is tracked,
    malformed rows fail the readiness gate so placeholder hashes cannot promote
    the DM2 original-overlay row. A pair-ready row must be a same-state
    dungeon_gameplay pair with original and Firestaff 224x136 viewport hashes.
    """
    print("section=pair_manifest_gate")
    print(f"pairManifest={display(manifest)}")
    if not manifest.exists():
        print("pairManifestStatus=missing_open")
        print("pairReadyForOverlay=0")
        print("pairManifestFormatOk=1")
        print("pairManifestBoundary=OPEN_NO_PAIRED_HASHES")
        return True, False

    lines = [line.rstrip("\n") for line in manifest.read_text(encoding="utf-8").splitlines()
             if line.strip() and not line.startswith("#")]
    if not lines:
        print("pairManifestStatus=empty")
        print("pairReadyForOverlay=0")
        print("pairManifestFormatOk=0")
        return False, False

    header = lines[0].split("\t")
    if header != PAIR_MANIFEST_COLUMNS:
        print("pairManifestStatus=bad_header")
        print(f"pairManifestExpectedHeader={'|'.join(PAIR_MANIFEST_COLUMNS)}")
        print(f"pairManifestActualHeader={'|'.join(header)}")
        print("pairReadyForOverlay=0")
        print("pairManifestFormatOk=0")
        return False, False

    format_ok = True
    pair_ready = False
    row_count = 0
    ready_count = 0
    for row_index, line in enumerate(lines[1:], start=2):
        cells = line.split("\t")
        if len(cells) != len(PAIR_MANIFEST_COLUMNS):
            print(f"pairManifestProblem=row{row_index}:cell_count={len(cells)} expected={len(PAIR_MANIFEST_COLUMNS)}")
            format_ok = False
            continue
        row = dict(zip(PAIR_MANIFEST_COLUMNS, cells))
        row_count += 1
        row_ok = True
        for field in ("original_frame_sha256", "original_viewport_sha256",
                      "firestaff_frame_sha256", "firestaff_viewport_sha256"):
            if not _valid_sha256(row[field]):
                print(f"pairManifestProblem=row{row_index}:{field}:not_sha256")
                row_ok = False
        if row["diff_sha256"] and not _valid_sha256(row["diff_sha256"]):
            print(f"pairManifestProblem=row{row_index}:diff_sha256:not_sha256_or_blank")
            row_ok = False
        try:
            original_w = int(row["original_viewport_width"])
            original_h = int(row["original_viewport_height"])
            firestaff_w = int(row["firestaff_viewport_width"])
            firestaff_h = int(row["firestaff_viewport_height"])
        except ValueError:
            print(f"pairManifestProblem=row{row_index}:viewport_dimensions:not_integer")
            row_ok = False
            original_w = original_h = firestaff_w = firestaff_h = -1
        if (original_w, original_h) != (224, 136):
            print(f"pairManifestProblem=row{row_index}:original_viewport_geometry={original_w}x{original_h} expected=224x136")
            row_ok = False
        if (firestaff_w, firestaff_h) != (224, 136):
            print(f"pairManifestProblem=row{row_index}:firestaff_viewport_geometry={firestaff_w}x{firestaff_h} expected=224x136")
            row_ok = False
        if row["classification"] not in {"SAME_STATE_PAIR", "MEASUREMENT_ONLY", "BLOCKED"}:
            print(f"pairManifestProblem=row{row_index}:classification={row['classification']}:unknown")
            row_ok = False
        if row["status"] not in {"PAIR_READY", "MEASUREMENT_ONLY", "OPEN"}:
            print(f"pairManifestProblem=row{row_index}:status={row['status']}:unknown")
            row_ok = False

        if row_ok:
            same_state = (
                row["state"] == "dungeon_gameplay" and
                row["classification"] == "SAME_STATE_PAIR" and
                row["status"] == "PAIR_READY" and
                row["original_viewport_sha256"].lower() != row["firestaff_viewport_sha256"].lower()
            )
            # Equal hashes are allowed only if a future exact-match row still
            # carries a diff receipt. This keeps placeholder duplicated hashes
            # from silently promoting the row.
            exact_match_with_diff = (
                row["state"] == "dungeon_gameplay" and
                row["classification"] == "SAME_STATE_PAIR" and
                row["status"] == "PAIR_READY" and
                row["original_viewport_sha256"].lower() == row["firestaff_viewport_sha256"].lower() and
                _valid_sha256(row["diff_sha256"])
            )
            if same_state or exact_match_with_diff:
                ready_count += 1
                pair_ready = True
        else:
            format_ok = False

    print(f"pairManifestStatus=present rows={row_count} readyRows={ready_count}")
    print(f"pairReadyForOverlay={1 if pair_ready else 0}")
    print(f"pairManifestFormatOk={1 if format_ok else 0}")
    if not pair_ready:
        print("pairManifestBoundary=OPEN_UNTIL_DUNGEON_GAMEPLAY_ORIGINAL_AND_FIRESTAFF_HASHES_EXIST")
    return format_ok, pair_ready


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--skullwin-source", type=Path, default=None,
                        help="SKULLWIN source root (defaults to search list).")
    parser.add_argument("--attempt-dir", type=Path, default=DEFAULT_EVIDENCE_DIR,
                        help="Capture attempt directory to evaluate.")
    parser.add_argument("--pair-manifest", type=Path, default=DEFAULT_PAIR_MANIFEST,
                        help="Optional original-vs-Firestaff pair manifest TSV.")
    parser.add_argument("--allow-missing-skullwin", action="store_true",
                        help="Return success even if SKULLWIN sources are not on this host. "
                             "Useful for CI smoke tests; will still fail on attempt semantic gate.")
    args = parser.parse_args()

    print("probe=dm2_v1_original_overlay_capture_source_lock")
    if args.skullwin_source is not None:
        source_root = args.skullwin_source if args.skullwin_source.exists() else None
    else:
        source_root = find_skullwin_root()
    source_ok, source_state = check_sources(source_root)
    tool_ok = check_route_tools()
    attempt_ok = check_attempt(args.attempt_dir)
    pair_manifest_ok, pair_ready = check_pair_manifest(args.pair_manifest)

    print("honesty=source/tool lock only; semanticReadyForOverlay and pairReadyForOverlay must both be 1 before any original-vs-Firestaff DM2 pixel parity claims")
    if args.allow_missing_skullwin and source_state == "missing":
        # Allow the structural pieces (capture script + verifier probe +
        # scaffold source locks via grep) to succeed even on a host without
        # SKULLWIN; never allow parity claims without the attempt semantic gate.
        source_ok = True

    overall = source_ok and tool_ok and pair_manifest_ok
    print(f"overallReadyForOverlayScaffold={1 if overall else 0}")
    print(f"overallReadyForPairPromotion={1 if (overall and attempt_ok and pair_ready) else 0}")
    return 0 if overall else 1


if __name__ == "__main__":
    raise SystemExit(main())
