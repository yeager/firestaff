#!/usr/bin/env python3
"""Compare the source-bound pass1055 original frame with Firestaff.

The Firestaff input must be produced by
``firestaff_dm1_v1_viewport_wall_capture_probe`` from the authenticated PC 3.4
DATA directory.  This tool refuses a crop unless its sibling manifest binds it
to map 0, party (6,9), direction west.  It also hashes the canonical ZIP
members directly, so a similarly named replacement cannot be promoted.

No image library is required and no game data is extracted by this tool.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zipfile
import zlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ORIGINAL = (
    ROOT
    / "verification-screens/pass1055-dm1-original-closed-door-collision"
    / "viewport_224x136/02_door_before_original_viewport_224x136.ppm"
)
ARCHIVE = Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
EXPECTED = {
    "DATA/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "DATA/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
}
ORIGINAL_SHA256 = "93a07d28805f4a0e554607899406b6d706c88be920415940d2453069e673a5f6"
ORIGINAL_POSE_SCAFFOLD = (
    ROOT
    / "verification-screens/pass1055-dm1-original-closed-door-collision"
    / "pass513_i34e_route_key_transcript_scaffold.json"
)
CAPTURE_NAME = "pass1055_closed_door_6_9_dirW_viewport_224x136.ppm"
# ReDMCSB's D1C native panel is 96x88.  In the PC34 viewport it occupies this
# half-open rectangle; the surrounding full-frame metric remains separate.
D1C = (64, 18, 160, 106)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    parts = data.split(None, 4)
    if len(parts) != 5 or parts[0] != b"P6" or parts[3] != b"255":
        raise ValueError(f"{path}: expected binary P6 PPM")
    width, height = int(parts[1]), int(parts[2])
    pixels = parts[4]
    if len(pixels) != width * height * 3:
        raise ValueError(f"{path}: truncated/extended PPM payload")
    return width, height, pixels


def metric(a: bytes, b: bytes, width: int, box: tuple[int, int, int, int]) -> dict[str, object]:
    x0, y0, x1, y1 = box
    changed = total_delta = max_delta = 0
    for y in range(y0, y1):
        for x in range(x0, x1):
            off = (y * width + x) * 3
            delta = [abs(a[off + c] - b[off + c]) for c in range(3)]
            if any(delta):
                changed += 1
            total_delta += sum(delta)
            max_delta = max(max_delta, *delta)
    pixels = (x1 - x0) * (y1 - y0)
    return {
        "box": [x0, y0, x1, y1],
        "pixels": pixels,
        "changedPixels": changed,
        "changedRatio": changed / pixels,
        "meanAbsoluteChannelError": total_delta / (pixels * 3),
        "maxChannelDelta": max_delta,
        "exact": changed == 0,
    }


def changed_components(
    a: bytes, b: bytes, width: int, box: tuple[int, int, int, int]
) -> list[dict[str, object]]:
    """Return 8-connected delta topology without interpreting its content."""
    x0, y0, x1, y1 = box
    pending = {
        (x, y)
        for y in range(y0, y1)
        for x in range(x0, x1)
        if a[(y * width + x) * 3 : (y * width + x + 1) * 3]
        != b[(y * width + x) * 3 : (y * width + x + 1) * 3]
    }
    components: list[dict[str, object]] = []
    while pending:
        seed = pending.pop()
        todo = [seed]
        component = {seed}
        while todo:
            x, y = todo.pop()
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    neighbour = (x + dx, y + dy)
                    if neighbour in pending:
                        pending.remove(neighbour)
                        component.add(neighbour)
                        todo.append(neighbour)
        components.append(
            {
                "changedPixels": len(component),
                "inclusiveBounds": [
                    min(x for x, _ in component),
                    min(y for _, y in component),
                    max(x for x, _ in component),
                    max(y for _, y in component),
                ],
            }
        )
    return sorted(components, key=lambda row: int(row["changedPixels"]), reverse=True)


def original_pose_proof() -> dict[str, object]:
    scaffold = json.loads(ORIGINAL_POSE_SCAFFOLD.read_text(encoding="utf-8"))
    rows = scaffold.get("rows", [])
    row = rows[0] if len(rows) == 1 else {}
    required = ("f0128MapX", "f0128MapY", "f0128Direction", "f0097Presented")
    missing = [field for field in required if row.get(field) is None]
    proven = (
        scaffold.get("status") != "SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS"
        and row.get("scaffoldOnly") is False
        and not missing
        and (row.get("f0128MapX"), row.get("f0128MapY"), row.get("f0128Direction"))
        == (6, 9, 3)
        and row.get("f0097Presented") is True
    )
    return {
        "proven": proven,
        "scaffoldStatus": scaffold.get("status"),
        "requiredRuntimeFields": list(required),
        "missingRuntimeFields": missing,
        "reason": (
            "Original F0128 map/direction and F0097 presentation are debugger-bound."
            if proven
            else "The original crop is route-labelled only; its F0128 map/direction and F0097 presentation are not debugger-observed."
        ),
    }


def validate_capture_manifest(capture_dir: Path) -> None:
    manifest = json.loads((capture_dir / "dm1_v1_viewport_wall_capture.json").read_text())
    rows = [row for row in manifest.get("captures", []) if row.get("viewportCrop") == CAPTURE_NAME]
    if len(rows) != 1:
        raise ValueError("capture manifest does not contain exactly one pass1055 west crop")
    row = rows[0]
    if row.get("party") != {"mapIndex": 0, "mapX": 6, "mapY": 9, "direction": 3}:
        raise ValueError(f"capture has wrong party tuple: {row.get('party')!r}")


def validate_real_data() -> dict[str, str]:
    found: dict[str, str] = {}
    with zipfile.ZipFile(ARCHIVE) as archive:
        infos = {info.filename.replace("\\", "/"): info for info in archive.infolist()}
        raw_archive = ARCHIVE.read_bytes()
        for canonical, expected in EXPECTED.items():
            info = infos[canonical]
            # This historically-authentic ZIP has forward slashes in its
            # central directory but backslashes in local headers. Python's
            # ZipFile correctly rejects that inconsistency on open(). Read
            # the bounded member payload using the already-validated central
            # directory offsets instead; nothing is written to disk.
            header = raw_archive[info.header_offset : info.header_offset + 30]
            fields = struct.unpack("<IHHHHHIIIHH", header)
            if fields[0] != 0x04034B50:
                raise ValueError(f"{canonical}: invalid local ZIP header")
            start = info.header_offset + 30 + fields[9] + fields[10]
            compressed = raw_archive[start : start + info.compress_size]
            if info.compress_type == zipfile.ZIP_STORED:
                payload = compressed
            elif info.compress_type == zipfile.ZIP_DEFLATED:
                payload = zlib.decompress(compressed, -15)
            else:
                raise ValueError(f"{canonical}: unsupported ZIP method {info.compress_type}")
            if len(payload) != info.file_size:
                raise ValueError(f"{canonical}: decoded ZIP member size mismatch")
            actual = sha256(payload)
            if actual != expected:
                raise ValueError(f"{canonical}: authentic PC34 hash mismatch: {actual}")
            found[canonical] = actual
    return found


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("capture_dir", type=Path)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    capture = args.capture_dir / CAPTURE_NAME
    validate_capture_manifest(args.capture_dir)
    real_hashes = validate_real_data()
    if sha256(ORIGINAL.read_bytes()) != ORIGINAL_SHA256:
        raise ValueError("tracked original pass1055 crop hash mismatch")
    ow, oh, original = read_ppm(ORIGINAL)
    fw, fh, firestaff = read_ppm(capture)
    if (ow, oh) != (224, 136) or (fw, fh) != (224, 136):
        raise ValueError(f"expected paired 224x136 crops, got {(ow, oh)} and {(fw, fh)}")
    pose = original_pose_proof()
    exact = original == firestaff
    result = {
        "schema": "firestaff.dm1_v1.f0111.pass1055_pixel_pair.v2",
        "status": (
            ("EXACT_PIXEL_MATCH" if exact else "AUTHENTIC_PAIR_MISMATCH_MEASURED")
            if pose["proven"]
            else "CANDIDATE_PAIR_ORIGINAL_POSE_UNPROVEN"
        ),
        "party": {"mapIndex": 0, "mapX": 6, "mapY": 9, "direction": 3},
        "originalPoseProof": pose,
        "originalSha256": sha256(ORIGINAL.read_bytes()),
        "firestaffSha256": sha256(capture.read_bytes()),
        "pc34ArchiveMemberSha256": real_hashes,
        "fullViewport": metric(original, firestaff, 224, (0, 0, 224, 136)),
        "d1cNativePanelBounds": metric(original, firestaff, 224, D1C),
        "d1cDeltaComponents8Connected": changed_components(original, firestaff, 224, D1C),
        "honesty": (
            "Metrics compare real PC34 pixels, but they are not a same-pose parity measurement until the original F0128/F0097 runtime tuple is captured."
        ),
    }
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(result["status"])
        print(json.dumps(result["fullViewport"], sort_keys=True))
        print(json.dumps(result["d1cNativePanelBounds"], sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
