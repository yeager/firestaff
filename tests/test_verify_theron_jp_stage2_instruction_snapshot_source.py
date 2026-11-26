"""Synthetic sidecar fixtures exercise the authentic-media snapshot verifier."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / "scripts"))

import verify_theron_jp_stage2_instruction_snapshot_source as verifier


class Stage2FetchSourceTests(unittest.TestCase):
    def setUp(self) -> None:
        self.fetch = {
            "event_sequence": "30",
            "logical_pc": "4002",
            "physical_pc": "100002",
            "mpr2": "80",
            "bytes": "73002001",
            "physical_bytes": "100002,100003,100004,100005",
            "valid_mask": "f",
        }
        self.writes = [
            {
                "event_sequence": str(20 + offset),
                "physical_address": f"{0x100002 + offset:06x}",
                "value": value,
            }
            for offset, value in enumerate(("73", "00", "20", "01"))
        ]
        self.joined = self.writes.copy()

    def test_accepts_latest_pre_snapshot_writes_when_each_is_source_bound(self) -> None:
        result = verifier.verify_fetch_binding(
            self.fetch, self.writes, self.joined, 0x4002, 0x100002
        )
        self.assertEqual(result["fetch_4002_event"], 30)
        self.assertEqual(
            [result[f"fetch_4002_byte_{index}_write_event"] for index in range(4)],
            [20, 21, 22, 23],
        )

    def test_accepts_second_snapshot_site(self) -> None:
        fetch = dict(
            self.fetch,
            event_sequence="40",
            logical_pc="4009",
            physical_pc="100009",
            bytes="73002000",
            physical_bytes="100009,10000a,10000b,10000c",
        )
        writes = [
            {
                "event_sequence": str(30 + offset),
                "physical_address": f"{0x100009 + offset:06x}",
                "value": value,
            }
            for offset, value in enumerate(("73", "00", "20", "00"))
        ]
        result = verifier.verify_fetch_binding(fetch, writes, writes, 0x4009, 0x100009)
        self.assertEqual(result["fetch_4009_event"], 40)

    def test_requires_contiguous_source_span_before_first_fetch(self) -> None:
        writes = []
        joined = []
        for offset in range(11):
            address = 0x100002 + offset
            event = 10 + offset
            value = offset
            write = {
                "event_sequence": str(event),
                "physical_address": f"{address:06x}",
                "value": f"{value:02x}",
            }
            read = {
                "event_sequence": event - 1,
                "track02_raw_file_offset": 0x2973A2 + offset,
                "value": value,
            }
            writes.append(write)
            joined.append((read, write))
        verifier.verify_source_span_before_fetch(writes, joined, 40)

    def test_rejects_source_span_with_gap(self) -> None:
        with self.assertRaisesRegex(ValueError, "contiguous authentic source span"):
            verifier.verify_source_span_before_fetch(
                [
                    {
                        "event_sequence": str(10 + offset),
                        "physical_address": f"{0x100002 + offset:06x}",
                        "value": "00",
                    }
                    for offset in range(11)
                ],
                [
                    (
                        {"event_sequence": 9 + offset, "track02_raw_file_offset": 0x2973A3 + offset, "value": 0},
                        {
                            "event_sequence": str(10 + offset),
                            "physical_address": f"{0x100002 + offset:06x}",
                            "value": "00",
                        },
                    )
                    for offset in range(11)
                ],
                40,
            )

    def test_rejects_intervening_event_between_source_read_and_ram_write(self) -> None:
        writes = []
        joined = []
        for offset in range(11):
            address = 0x100002 + offset
            event = 10 + offset * 2
            write = {
                "event_sequence": str(event),
                "physical_address": f"{address:06x}",
                "value": "00",
            }
            read = {
                "event_sequence": event - 2,
                "track02_raw_file_offset": 0x2973A2 + offset,
                "value": 0,
            }
            writes.append(write)
            joined.append((read, write))
        with self.assertRaisesRegex(ValueError, "not immediately followed"):
            verifier.verify_source_span_before_fetch(writes, joined, 40)

    def test_requires_cd_sidecars_to_identify_shared_sequence_source(self) -> None:
        with tempfile.TemporaryDirectory() as scratch:
            write = Path(scratch) / "capture.cd-ram-target-write"
            read = Path(scratch) / "capture.cd-data-port-read"
            write.write_text(verifier.cd_ram_join.WRITE_HEADER + "\n", encoding="ascii")
            read.write_text(verifier.cd_read_verifier.TRACE_HEADER_WITH_READER_MPR_CONTEXT + "\n", encoding="ascii")
            with self.assertRaisesRegex(ValueError, "shared event-sequence source marker"):
                verifier.require_shared_sequence_headers(write, read)

    def test_rejects_later_mismatching_write(self) -> None:
        later = self.writes + [
            {"event_sequence": "29", "physical_address": "100004", "value": "ff"}
        ]
        joined = self.joined + [later[-1]]
        with self.assertRaisesRegex(ValueError, "differs from snapshot"):
            verifier.verify_fetch_binding(self.fetch, later, joined)

    def test_rejects_latest_write_without_authentic_source_join(self) -> None:
        later = self.writes + [
            {"event_sequence": "29", "physical_address": "100004", "value": "20"}
        ]
        with self.assertRaisesRegex(ValueError, "not source-validated"):
            verifier.verify_fetch_binding(self.fetch, later, self.joined)

    def test_rejects_missing_prefetch_byte_write(self) -> None:
        with self.assertRaisesRegex(ValueError, "no recorded pre-snapshot write"):
            verifier.verify_fetch_binding(self.fetch, self.writes[:3], self.joined[:3])

    def test_rejects_incomplete_or_wrongly_mapped_fetch(self) -> None:
        wrong = dict(self.fetch, physical_pc="100003")
        with self.assertRaisesRegex(ValueError, "complete mapped"):
            verifier.verify_fetch_binding(wrong, self.writes, self.joined)

    def test_paired_verifier_rejects_cross_sidecar_event_collision(self) -> None:
        scratch_root = repo / ".codex-scratch"
        scratch_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="theron-stage2-fetch-", dir=scratch_root) as scratch:
            root = Path(scratch)
            follow_path = root / "capture.system-card-bootstrap-follow"
            write_path = root / "capture.cd-ram-target-write"
            read_path = root / "capture.cd-data-port-read"
            track02_path = root / "unused-track02.bin"
            follow_path.write_text(
                verifier.follow_verifier.HEADER
                + "\ntarget_follow index=0 event_sequence=30 logical_pc=4002 "
                + "physical_pc=100002 bytes=73002001 physical_bytes=100002,100003,100004,100005 "
                + "valid_mask=f mpr2=80\n"
                + "target_follow index=1 event_sequence=31 logical_pc=4009 "
                + "physical_pc=100009 bytes=73002000 physical_bytes=100009,10000a,10000b,10000c "
                + "valid_mask=f mpr2=80\n",
                encoding="ascii",
            )
            write_path.write_text(
                verifier.cd_ram_join.WRITE_HEADER_SHARED_SEQUENCE
                + "\ncd_ram_target_write sequence=0 event_sequence=20 physical_address=100002 value=73\n",
                encoding="ascii",
            )
            read_path.write_text(
                verifier.cd_read_verifier.TRACE_HEADER_WITH_READER_MPR_SHARED_SEQUENCE + "\n",
                encoding="ascii",
            )
            source_joined = [
                (None, row) for row in self.writes
            ]
            with (
                mock.patch.object(verifier.follow_verifier, "verify"),
                mock.patch.object(verifier.cd_read_verifier, "verify", return_value=(b"x", [{"event_sequence": 30}])),
                mock.patch.object(verifier.cd_ram_join, "verify", return_value=source_joined),
            ):
                with self.assertRaisesRegex(ValueError, "collides across paired"):
                    verifier.verify(follow_path, write_path, read_path, track02_path)


if __name__ == "__main__":
    unittest.main()
