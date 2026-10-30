"""Synthetic parser fixtures for the real-media CD-to-RAM source join."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from verify_theron_cd_ram_source_join import (
    READ_HEADER,
    WRITE_HEADER,
    join_rows,
    verify,
)


class CDToRAMSourceJoinTests(unittest.TestCase):
    def setUp(self) -> None:
        self.reads = [
            {
                "event_sequence": 40,
                "source_valid": 1,
                "address": 0x1808,
                "reader_pc": 0xEA99,
                "reader_physical_pc": 0x0A99,
                "source_lba": 3849,
                "source_user_offset": 0x4D2,
                "source_raw_offset": 0x4E2,
                "track02_start_lba": 3590,
                "track02_end_lba": 6961,
                "track02_index1_file_offset": 526848,
                "track02_sector_bytes": 2352,
                "track02_raw_file_offset": 526848 + (3849 - 3590) * 2352 + 16 + 0x4D2,
                "value": 0xAD,
            }
        ]
        self.writes = [
            {
                "event_sequence": "41",
                "sequence": "0",
                "physical_address": "0d04d2",
                "value": "ad",
                "writer_pc": "ea9c",
                "writer_physical_pc": "000a9c",
                "mpr2": "68",
            }
        ]

    def join(self, movement_rows=None):
        return join_rows(
            self.reads, self.writes, 0x0D04D2, 0x0D04E6, 0x68, 0xEA9C, movement_rows
        )

    def test_joins_authentic_shape_of_immediate_equal_read_and_write(self) -> None:
        joined = self.join()
        self.assertEqual(len(joined), 1)
        self.assertEqual(joined[0][0]["source_lba"], 3849)

    def test_joins_when_verified_movement_hook_observes_same_store(self) -> None:
        self.writes[0]["event_sequence"] = "42"
        movement = [{
            "event_sequence": "41",
            "physical_address": "0d04d2",
            "value": "ad",
            "pc": "ea9c",
        }]

        joined = self.join(movement)

        self.assertEqual(len(joined), 1)
        self.assertEqual(joined[0][0]["event_sequence"], 40)

    def test_rejects_intervening_movement_row_for_different_store(self) -> None:
        self.writes[0]["event_sequence"] = "42"
        movement = [{
            "event_sequence": "41",
            "physical_address": "0d04d3",
            "value": "ad",
            "pc": "ea9c",
        }]
        with self.assertRaisesRegex(ValueError, "verified movement row"):
            self.join(movement)

    def test_rejects_same_movement_store_from_different_writer_pc(self) -> None:
        self.writes[0]["event_sequence"] = "42"
        movement = [{
            "event_sequence": "41",
            "physical_address": "0d04d2",
            "value": "ad",
            "pc": "ea9b",
        }]
        with self.assertRaisesRegex(ValueError, "verified movement row"):
            self.join(movement)

    def test_rejects_same_movement_store_with_different_value(self) -> None:
        self.writes[0]["event_sequence"] = "42"
        movement = [{
            "event_sequence": "41",
            "physical_address": "0d04d2",
            "value": "00",
            "pc": "ea9c",
        }]
        with self.assertRaisesRegex(ValueError, "verified movement row"):
            self.join(movement)

    def test_verify_reads_and_forwards_paired_movement_context_trace(self) -> None:
        scratch_root = Path(__file__).resolve().parents[1] / ".codex-scratch"
        scratch_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(
            prefix="theron-cd-ram-join-", dir=scratch_root
        ) as scratch_dir:
            capture = Path(scratch_dir)
            read_path = capture / "capture.cd-data-port-read"
            write_path = capture / "capture.cd-ram-target-write"
            movement_path = capture / "capture.pce-fast-movement-context"
            track02_path = capture / "unused-track02.bin"
            read_path.write_text(READ_HEADER + "\n", encoding="ascii")
            write_path.write_text(
                WRITE_HEADER
                + "\ncd_ram_target_write sequence=0 event_sequence=42 "
                + "physical_address=0d04d2 value=ad writer_pc=ea9c "
                + "writer_physical_pc=000a9c mpr2=68\n",
                encoding="ascii",
            )
            movement_path.write_text(
                "source=mednafen-pce-fast-theron-movement-context-v3 "
                "event_sequence_source=TheronTraceCDSharedEventSequence\n"
                "main_ram_write event_sequence=41 physical_address=0d04d2 "
                "value=ad pc=ea9c\n",
                encoding="ascii",
            )
            with mock.patch(
                "verify_theron_cd_ram_source_join.verify_data_port_trace",
                return_value=(b"unused", self.reads),
            ) as verify_data_port_trace:
                joined = verify(
                    write_path,
                    read_path,
                    track02_path,
                    0x0D04D2,
                    0x0D04E6,
                    0x68,
                    0xEA9C,
                    movement_path,
                )

        self.assertEqual(len(joined), 1)
        self.assertEqual(joined[0][0]["event_sequence"], 40)
        verify_data_port_trace.assert_called_once_with(
            read_path, track02_path, allow_other_sources=True
        )

    def test_rejects_value_mismatch(self) -> None:
        self.writes[0]["value"] = "00"
        with self.assertRaisesRegex(ValueError, "values differ"):
            self.join()

    def test_rejects_non_adjacent_event(self) -> None:
        self.writes[0]["event_sequence"] = "42"
        with self.assertRaisesRegex(ValueError, "immediately preceding"):
            self.join()

    def test_rejects_unbound_read(self) -> None:
        self.reads[0]["source_valid"] = 0
        with self.assertRaisesRegex(ValueError, "authenticated System Card"):
            self.join()

    def test_rejects_source_outside_authentic_track02(self) -> None:
        self.reads[0]["source_lba"] = 7000
        with self.assertRaisesRegex(ValueError, "authenticated System Card"):
            self.join()

    def test_rejects_wrong_writer_mapping(self) -> None:
        self.writes[0]["writer_physical_pc"] = "010a9c"
        with self.assertRaisesRegex(ValueError, "expected System Card store"):
            self.join()

    def test_ignores_write_outside_selected_target_range(self) -> None:
        self.writes[0]["physical_address"] = "0d0500"
        with self.assertRaisesRegex(ValueError, "no target writes"):
            self.join()

    def test_ignores_other_mpr_windows_before_joining_selected_window(self) -> None:
        other_window_write = self.writes[0].copy()
        other_window_write.update(event_sequence="41", sequence="0", mpr2="80")
        self.reads.append({**self.reads[0], "event_sequence": 42})
        self.writes[0].update(event_sequence="43", sequence="1")
        self.writes.insert(0, other_window_write)

        joined = self.join()

        self.assertEqual(len(joined), 1)
        self.assertEqual(joined[0][1]["mpr2"], "68")


if __name__ == "__main__":
    unittest.main()
