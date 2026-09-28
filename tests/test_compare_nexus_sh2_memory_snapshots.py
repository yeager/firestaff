"""Parser tests for authentic, frame-aligned Nexus WorkRAMH captures."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import tempfile


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "compare_nexus_sh2_memory_snapshots",
    ROOT / "scripts" / "compare_nexus_sh2_memory_snapshots.py",
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def write_snapshot(path: Path, rows: list[tuple[int, bytes]]) -> None:
    with path.open("wb") as stream:
        stream.write(MODULE.SNAPSHOT_HEADER)
        for frame, payload in rows:
            assert len(payload) == MODULE.WORKRAMH_SIZE
            stream.write(f"frame={frame} base=0x06000000 size=1048576\n".encode())
            stream.write(payload)
            stream.write(b"\n")


def test_matched_authentic_capture_format_and_spans(tmp_path: Path) -> None:
    control = tmp_path / "control.raw"
    observed = tmp_path / "input.raw"
    baseline = bytes(MODULE.WORKRAMH_SIZE)
    changed = bytearray(baseline)
    changed[8:10] = b"\x12\x34"
    changed[12] = 0x56
    write_snapshot(control, [(10500, baseline)])
    write_snapshot(observed, [(10500, bytes(changed))])

    control_row = next(MODULE.iter_snapshots(control))
    input_row = next(MODULE.iter_snapshots(observed))

    assert control_row[:2] == input_row[:2] == (10500, 0x06000000)
    assert MODULE.differing_spans(control_row[2], input_row[2], control_row[1]) == [
        (0x06000008, 0x0600000A),
        (0x0600000C, 0x0600000D),
    ]


def test_input_trace_deduplicates_same_frame_and_rejects_conflicts(tmp_path: Path) -> None:
    trace = tmp_path / "input.trace"
    trace.write_text(
        "FIRESTAFF_NEXUS_INPUT_TRACE_V1\n"
        "frame=10500 mask=0x0010\nframe=10500 mask=0x0010\n",
        encoding="ascii",
    )
    assert MODULE.read_input_masks(trace) == {10500: 0x10}

    trace.write_text(
        "FIRESTAFF_NEXUS_INPUT_TRACE_V1\n"
        "frame=10500 mask=0x0010\nframe=10500 mask=0x0020\n",
        encoding="ascii",
    )
    try:
        MODULE.read_input_masks(trace)
    except ValueError as error:
        assert "conflicting input masks" in str(error)
    else:
        raise AssertionError("conflicting same-frame input masks were accepted")


def test_snapshot_rejects_truncated_payload(tmp_path: Path) -> None:
    snapshot = tmp_path / "truncated.raw"
    snapshot.write_bytes(
        MODULE.SNAPSHOT_HEADER
        + b"frame=10500 base=0x06000000 size=1048576\n"
        + b"\0" * 32
    )
    try:
        next(MODULE.iter_snapshots(snapshot))
    except ValueError as error:
        assert "truncated WorkRAMH payload" in str(error)
    else:
        raise AssertionError("truncated WorkRAMH snapshot was accepted")


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="nexus-memory-snapshot-test-") as directory:
        fixture_dir = Path(directory)
        test_matched_authentic_capture_format_and_spans(fixture_dir)
        test_input_trace_deduplicates_same_frame_and_rejects_conflicts(fixture_dir)
        test_snapshot_rejects_truncated_payload(fixture_dir)
    print("Nexus SH-2 memory snapshot comparator parser tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
