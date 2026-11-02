#!/usr/bin/env python3
"""Compare row-local Ymir NBG0 enable state with indexed register snapshots."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path


class AuditError(ValueError):
    pass


def audit(render_state_path: Path, snapshot_index_path: Path,
          snapshot_dir: Path) -> dict[str, object]:
    rows_by_pass: dict[tuple[int, int], list[tuple[int, int]]] = {}
    with render_state_path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        required = {"frame_index", "vdp2_frame_serial", "row_vcnt", "nbg0_enabled"}
        if reader.fieldnames is None or not required.issubset(reader.fieldnames):
            raise AuditError("render-state CSV lacks required NBG0 fields")
        for row in reader:
            try:
                key = int(row["frame_index"]), int(row["vdp2_frame_serial"])
                y, enabled = int(row["row_vcnt"]), int(row["nbg0_enabled"])
            except (TypeError, ValueError) as error:
                raise AuditError("render-state CSV contains an invalid numeric field") from error
            if y < 0 or enabled not in (0, 1):
                raise AuditError(f"invalid render-state value for pass {key} row {y}")
            rows_by_pass.setdefault(key, []).append((y, enabled))

    snapshots: dict[tuple[int, int], str] = {}
    with snapshot_index_path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        required = {"vdp1_frame", "vdp2_frame_serial", "file_prefix"}
        if reader.fieldnames is None or not required.issubset(reader.fieldnames):
            raise AuditError("snapshot index lacks frame/serial/file-prefix fields")
        for row in reader:
            try:
                key = int(row["vdp1_frame"]), int(row["vdp2_frame_serial"])
                prefix = row["file_prefix"]
            except (TypeError, ValueError) as error:
                raise AuditError("snapshot index contains an invalid numeric field") from error
            if key in snapshots:
                raise AuditError(f"duplicate snapshot-index pass {key}")
            snapshots[key] = prefix

    missing: list[tuple[int, int]] = []
    disagreements: list[dict[str, object]] = []
    for key, samples in sorted(rows_by_pass.items()):
        prefix = snapshots.get(key)
        if prefix is None:
            missing.append(key)
            continue
        register_path = snapshot_dir / f"{prefix}-vdp2-registers.bin"
        data = register_path.read_bytes()
        if len(data) < 0x22:
            raise AuditError(f"truncated VDP2 register snapshot for pass {key}")
        bgon = int.from_bytes(data[0x20:0x22], "big")
        snapshot_enabled = int(bool(bgon & 0x0001))
        samples.sort()
        ys = [sample[0] for sample in samples]
        if len(set(ys)) != len(ys):
            raise AuditError(f"duplicate row samples for pass {key}")
        values = {enabled for _, enabled in samples}
        if len(values) != 1 or next(iter(values)) != snapshot_enabled:
            disagreements.append({
                "frame": key[0],
                "vdp2_serial": key[1],
                "render_state_nbg0_enabled": sorted(values),
                "snapshot_bg_on": f"0x{bgon:04x}",
                "snapshot_nbg0_enabled": snapshot_enabled,
                "row_count": len(samples),
                "row_range": [min(ys), max(ys) + 1],
            })

    if missing:
        raise AuditError(f"render-state passes missing indexed snapshots: {missing[:8]}")
    return {
        "result": "PASS" if not disagreements else "MISMATCH",
        "render_state_passes": len(rows_by_pass),
        "indexed_snapshots": len(snapshots),
        "disagreement_count": len(disagreements),
        "disagreements": disagreements,
        "interpretation": (
            "row-local NBG0 state agrees with frame/serial-indexed register snapshots"
            if not disagreements else
            "frame/serial labels do not time-bind register snapshots to row-local state"
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--render-state", type=Path, required=True)
    parser.add_argument("--snapshot-index", type=Path, required=True)
    parser.add_argument("--snapshot-dir", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = audit(args.render_state, args.snapshot_index, args.snapshot_dir)
    except (OSError, AuditError) as error:
        parser.error(str(error))
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["result"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
