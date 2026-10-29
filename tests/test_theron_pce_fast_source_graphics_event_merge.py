#!/usr/bin/env python3
"""Exercise the merger with small test-only trace records to protect real capture joins."""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Optional


repo = Path(__file__).resolve().parents[1]
merger = repo / "scripts/merge_theron_pce_fast_source_graphics_events.py"
root = Path(sys.argv[1])
shutil.rmtree(root, ignore_errors=True)
root.mkdir(parents=True, exist_ok=True)


def run_case(
    name: str,
    cd: str,
    ram: str,
    vram: str,
    expected: int,
    vce: Optional[str] = None,
    expected_sequences: Optional[list[int]] = None,
    expected_events: Optional[list[str]] = None,
    vram_limit: Optional[int] = None,
    vce_limit: Optional[int] = None,
    expected_completeness: str = "not_checked",
    expected_filtered_prefix_streams: Optional[list[str]] = None,
) -> None:
    case = root / name
    case.mkdir()
    trace_data = [cd, ram, vram]
    if vce is not None:
        trace_data.append(vce)
    trace_paths = [case / f"trace-{index}" for index in range(len(trace_data))]
    merged_path = case / "merged"
    for path, contents in zip(trace_paths, trace_data):
        path.write_text(contents, encoding="ascii")
    command = [
        sys.executable,
        str(merger),
        "--cd-data-port", str(trace_paths[0]),
        "--cd-ram-target", str(trace_paths[1]),
        "--vram-commit", str(trace_paths[2]),
    ]
    if vram_limit is not None:
        command.extend(("--vram-commit-limit", str(vram_limit)))
    if vce is not None:
        command.extend(("--vce-write", str(trace_paths[3])))
    if vce_limit is not None:
        command.extend(("--vce-write-limit", str(vce_limit)))
    command.extend(("--output", str(merged_path)))
    result = subprocess.run(
        command,
        check=False,
        capture_output=True,
        text=True,
    )
    if expected == 0:
        if result.returncode != 0:
            raise AssertionError(result.stderr)
        rows = [json.loads(line) for line in merged_path.read_text(encoding="ascii").splitlines()]
        expected_streams = ["cd_data_port_read", "cd_ram_target_write", "vram_commit"]
        if vce is not None:
            expected_streams.append("vce_write")
        if rows[0].get("format") != "firestaff-theron-source-graphics-order-v1" or rows[0].get(
            "ordering"
        ) != "shared_event_sequence" or rows[0].get(
            "interpretation"
        ) != "temporal_order_only_not_causal_dataflow" or rows[0].get(
            "streams"
        ) != expected_streams:
            raise AssertionError(f"wrong merged chronology metadata: {rows[0]!r}")
        if rows[0].get("completeness") != expected_completeness:
            raise AssertionError(f"wrong merged completeness metadata: {rows[0]!r}")
        if rows[0].get("filtered_prefix_streams") != (
            expected_filtered_prefix_streams or []
        ):
            raise AssertionError(f"wrong filtered-prefix metadata: {rows[0]!r}")
        expected_bounded_streams = {}
        if vram_limit is not None:
            count = sum(line.startswith("vram_commit ") for line in trace_data[2].splitlines())
            expected_bounded_streams["vram_commit"] = {
                "limit": vram_limit,
                "records": count,
                "limit_reached": count >= vram_limit,
            }
        if vce_limit is not None:
            assert vce is not None
            count = sum(line.startswith("vce_write ") for line in vce.splitlines())
            expected_bounded_streams["vce_write"] = {
                "limit": vce_limit,
                "records": count,
                "limit_reached": count >= vce_limit,
            }
        if rows[0].get("bounded_streams") != expected_bounded_streams:
            raise AssertionError(f"wrong bounded-stream metadata: {rows[0]!r}")
        if [row["event_sequence"] for row in rows[1:]] != (
            expected_sequences if expected_sequences is not None else [2, 5, 9]
        ):
            raise AssertionError(f"wrong merged chronology: {rows!r}")
        if [row["event"] for row in rows[1:]] != (
            expected_events
            if expected_events is not None
            else ["cd_data_port_read", "cd_ram_target_write", "vram_commit"]
        ):
            raise AssertionError(f"wrong merged event types: {rows!r}")
    else:
        if result.returncode == 0:
            raise AssertionError("invalid chronology was accepted")
        if merged_path.exists():
            raise AssertionError("failed merge left a partial output")


run_case(
    "ordered",
    "source=mednafen-pce-fast-cd-data-port-read\ncd_data_port_read sequence=0 event_sequence=2\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write sequence=0 event_sequence=5\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit sequence=0 event_sequence=9\n",
    0,
)
run_case(
    "ordered-with-vce",
    "source=mednafen-pce-fast-cd-data-port-read\ncd_data_port_read sequence=0 event_sequence=2\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write sequence=0 event_sequence=5\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit sequence=0 event_sequence=9\n",
    0,
    vce="FIRESTAFF_THERON_PCE_FAST_VCE_WRITE_TRACE_V1\nsource=mednafen-1.32.1-pce-fast-vdc-vce-write\nscope=color-table-data-writes-only;address-port-writes-excluded\nvce_write sequence=0 event_sequence=4 reg=4 address=012 input=34 old=000 value=034 logical_pc=1234 physical_pc=000000 mpr_slot=0 mpr_bank=00\n",
    expected_sequences=[2, 4, 5, 9],
    expected_events=["cd_data_port_read", "vce_write", "cd_ram_target_write", "vram_commit"],
    vram_limit=2,
    vce_limit=1,
    expected_completeness="possible_truncation_at_event_limit",
)
run_case(
    "reader-mpr-context-header",
    "source=mednafen-pce-fast-cd-data-port-read-with-reader-mpr-context\ncd_data_port_read sequence=0 event_sequence=2\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write sequence=0 event_sequence=5\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit sequence=0 event_sequence=9\n",
    0,
)
run_case(
    "track02-triggered-vram-prefix",
    "source=mednafen-pce-fast-cd-data-port-read-with-reader-mpr-context\ncd_data_port_read sequence=0 event_sequence=12\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write sequence=0 event_sequence=15\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V4\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded;prefix=omitted-until-first-source-valid-track02-read\nvram_commit sequence=0 event_sequence=19\n",
    0,
    expected_sequences=[12, 15, 19],
    expected_filtered_prefix_streams=["vram_commit"],
)
run_case(
    "below-event-limits",
    "source=mednafen-pce-fast-cd-data-port-read\ncd_data_port_read sequence=0 event_sequence=2\n",
    "source=mednafen-pce-fast-cd-ram-target-write\ncd_ram_target_write sequence=0 event_sequence=5\n",
    "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3\nsource=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window\nscope=cpu-port-vwr-commits-only;dma-writes-excluded\nvram_commit sequence=0 event_sequence=9\n",
    0,
    vram_limit=2,
    expected_completeness="below_configured_event_limits",
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
