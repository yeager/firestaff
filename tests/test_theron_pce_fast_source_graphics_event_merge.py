#!/usr/bin/env python3
"""Exercise the merger with small test-only trace records to protect real capture joins."""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path


repo = Path(__file__).resolve().parents[1]
merger = repo / "scripts/merge_theron_pce_fast_source_graphics_events.py"
root = Path(sys.argv[1])
shutil.rmtree(root, ignore_errors=True)
root.mkdir(parents=True, exist_ok=True)


def run_case(name: str, cd: str, ram: str, vram: str, expected: int) -> None:
    case = root / name
    case.mkdir()
    paths = [case / "cd", case / "ram", case / "vram", case / "merged"]
    for path, contents in zip(paths[:3], (cd, ram, vram)):
        path.write_text(contents, encoding="ascii")
    result = subprocess.run(
        [
            sys.executable,
            str(merger),
            "--cd-data-port", str(paths[0]),
            "--cd-ram-target", str(paths[1]),
            "--vram-commit", str(paths[2]),
            "--output", str(paths[3]),
        ],
        check=False,
        capture_output=True,
        text=True,
    )
    if expected == 0:
        if result.returncode != 0:
            raise AssertionError(result.stderr)
        rows = [json.loads(line) for line in paths[3].read_text(encoding="ascii").splitlines()]
        if rows[0] != {
            "format": "firestaff-theron-source-graphics-order-v1",
            "ordering": "shared_event_sequence",
            "interpretation": "temporal_order_only_not_causal_dataflow",
        }:
            raise AssertionError(f"wrong merged chronology metadata: {rows[0]!r}")
        if [row["event_sequence"] for row in rows[1:]] != [2, 5, 9]:
            raise AssertionError(f"wrong merged chronology: {rows!r}")
        if [row["event"] for row in rows[1:]] != ["cd_data_port_read", "cd_ram_target_write", "vram_commit"]:
            raise AssertionError(f"wrong merged event types: {rows!r}")
    else:
        if result.returncode == 0:
            raise AssertionError("invalid chronology was accepted")
        if paths[3].exists():
            raise AssertionError("failed merge left a partial output")


run_case(
    "ordered",
    "source=mednafen-pce-fast-cd-data-port-read\ncd_data_port_read sequence=0 event_sequence=2\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write sequence=0 event_sequence=5\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit sequence=0 event_sequence=9\n",
    0,
)
run_case(
    "duplicate",
    "source=mednafen-pce-fast-cd-data-port-read\ncd_data_port_read event_sequence=2\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write event_sequence=2\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit event_sequence=9\n",
    1,
)
run_case(
    "unordered",
    "source=mednafen-pce-fast-cd-data-port-read\ncd_data_port_read event_sequence=8\ncd_data_port_read event_sequence=3\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write event_sequence=5\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit event_sequence=9\n",
    1,
)
run_case(
    "wrong-version",
    "source=mednafen-pce-fast-cd-data-port-read\ncd_data_port_read event_sequence=2\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write event_sequence=5\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V2\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit event_sequence=9\n",
    1,
)

protected = root / "protected-output"
protected.write_text("preserve me\n", encoding="ascii")
failed = subprocess.run(
    [
        sys.executable,
        str(merger),
        "--cd-data-port", str(root / "ordered/cd"),
        "--cd-ram-target", str(root / "ordered/ram"),
        "--vram-commit", str(root / "ordered/vram"),
        "--output", str(protected),
    ],
    check=False,
    capture_output=True,
    text=True,
)
if failed.returncode == 0 or protected.read_text(encoding="ascii") != "preserve me\n":
    raise AssertionError("existing output was not preserved on refusal")

shutil.rmtree(root)
