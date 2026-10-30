#!/usr/bin/env python3
"""Exercise follow-trace ordering checks used before authentic capture analysis."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
import sys
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import verify_theron_jp_indirect_jump_follow_trace as verifier


MPRS = " ".join(f"mpr{slot}={'80' if slot == 2 else '00'}" for slot in range(8))
DISPATCH_HEADER = "source=mednafen-pce-fast-indirect-jump-trace-v1 event_sequence_source=TheronTraceCDSharedEventSequence target_pcs=40e1 limit=64"
FOLLOW_HEADER = verifier.HEADER


def dispatch(event: int) -> str:
    return (
        f"indirect_jump event_sequence={event} row=0 instruction_pc=40e1 "
        "instruction_physical_pc=1000e1 base=410d index=24 table_address=4131 "
        "low=19 high=43 low_slot=2 low_mpr=80 low_physical=100131 "
        "high_slot=2 high_mpr=80 high_physical=100132 target=4319 "
        f"{MPRS}"
    )


def follow_row(dispatch_event: int, event: int) -> str:
    return (
        f"indirect_jump_follow dispatch_event={dispatch_event} target_pc=4319 "
        f"event_sequence={event} index=0 logical_pc=4319 physical_pc=100319 "
        "bytes=a51c48 physical_bytes=100319,10031a,10031b valid_mask=7 "
        f"{MPRS}"
    )


class FollowTraceVerifierTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.dispatch_path = self.root / "dispatch.trace"
        self.follow_path = self.root / "follow.trace"
        self.code_follow_path = self.root / "code-follow.trace"
        self.code_follow_path.write_text("unused by the patched independent verifier\n", encoding="ascii")

    def write_traces(self, dispatch_events: list[int], follow_lines: list[str]) -> None:
        self.dispatch_path.write_text(
            "\n".join([DISPATCH_HEADER, *(dispatch(event) for event in dispatch_events)]) + "\n",
            encoding="ascii",
        )
        self.follow_path.write_text("\n".join([FOLLOW_HEADER, *follow_lines]) + "\n", encoding="ascii")

    def verify(self) -> tuple[int, int, int, int, int]:
        with patch.object(verifier, "verify_indirect_jump", return_value=(1, 1, 0)):
            return verifier.verify(self.dispatch_path, self.code_follow_path, self.follow_path)

    def test_accepts_capture_ending_immediately_after_last_dispatch(self) -> None:
        self.write_traces([10, 20], [follow_row(10, 11),
                                     "indirect_jump_follow_complete dispatch_event=10 event_sequence=12 count=1 terminal=next-indirect-dispatch"])
        self.assertEqual(self.verify(), (1, 1, 0, 0, 0))

    def test_accepts_capture_ending_before_first_target_fetch(self) -> None:
        self.write_traces([10], [])
        self.assertEqual(self.verify(), (0, 0, 0, 0, 0))

    def test_accepts_256_row_instruction_cap(self) -> None:
        rows = [follow_row(10, event).replace(" index=0 ", f" index={index} ")
                for index, event in enumerate(range(11, 267))]
        rows.append(
            "indirect_jump_follow_truncated dispatch_event=10 event_sequence=267 "
            "after_rows=256 limit=256 reason=instruction-cap"
        )
        self.write_traces([10], rows)
        self.assertEqual(self.verify(), (256, 1, 0, 1, 0))

    def test_rejects_duplicate_skipped_dispatch_marker(self) -> None:
        self.write_traces(
            [10, 20],
            ["indirect_jump_follow_skipped dispatch_event=10 event_sequence=11 reason=window-limit limit=64",
             "indirect_jump_follow_skipped dispatch_event=10 event_sequence=12 reason=window-limit limit=64"],
        )
        with self.assertRaisesRegex(ValueError, "invalid skipped-window marker"):
            self.verify()

    def test_rejects_missing_earlier_dispatch_when_later_window_is_present(self) -> None:
        self.write_traces([10, 20], [follow_row(20, 21)])
        with self.assertRaisesRegex(ValueError, "missing follow records"):
            self.verify()

    def test_rejects_sidecar_record_that_precedes_its_dispatch(self) -> None:
        self.write_traces([10], [follow_row(10, 9)])
        with self.assertRaisesRegex(ValueError, "does not follow its dispatch"):
            self.verify()


if __name__ == "__main__":
    unittest.main()
