#!/usr/bin/env python3
"""Exercise capture binding and fail-closed byte joins with bounded fixtures."""

from __future__ import annotations

import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from verify_theron_runtime_page_write_join import verify  # noqa: E402


CAPTURE_ID = "0123456789abcdef0123456789abcdef"
OTHER_CAPTURE_ID = "fedcba9876543210fedcba9876543210"
RUNTIME_HEADER = (
    "source=mednafen-pce-fast-theron-c852-runtime-window-v3 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    f"capture_id={CAPTURE_ID}"
)
WRITER_HEADER = (
    "source=mednafen-pce-fast-main-ram-target-write-v2 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    f"capture_id={CAPTURE_ID} range=0d0000-0d1fff sample_limit=1048576"
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class RuntimePageWriteJoinTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.runtime = self.root / "runtime.trace"
        self.writer = self.root / "writers.trace"
        self.receipt = self.root / "capture.transition"
        self._write_runtime()
        self._write_writers()
        self._write_receipt()

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _write_runtime(self) -> None:
        rows = [
            (0, 10, 0xC85E, 0x20, 0x39, 0x50, 0x68),
            (1, 20, 0x5039, 0x44, 0x05, 0x86, 0x68),
        ]
        rows.extend((step, 30 + step, 0x5000, 0xEA, 0x01, 0x02, 0x68)
                    for step in range(2, 129))
        lines = [RUNTIME_HEADER]
        for step, event, pc, opcode, operand1, operand2, mapped_bank in rows:
            mprs = [0] * 8
            mprs[2] = mapped_bank
            mprs[pc >> 13] = mapped_bank
            physical = (mapped_bank << 13) | (pc & 0x1FFF)
            fields = (
                f"window=0 step={step} event_sequence={event} logical_pc={pc:04x} "
                f"physical_pc={physical:06x} opcode={opcode:02x} "
                f"operand1={operand1:02x} operand2={operand2:02x} "
                "a=00 x=00 y=00 p=00 s=00 "
                + " ".join(f"mpr{i}={value:02x}" for i, value in enumerate(mprs))
            )
            lines.append("theron_c852_instruction " + fields)
        lines.append("theron_c852_window_end window=0 rows=129 status=complete")
        self.runtime.write_text("\n".join(lines) + "\n", encoding="ascii")

    def _writer_row(self, sequence: int, event: int, logical: int,
                    value: int, mapped_bank: int) -> str:
        physical = (mapped_bank << 13) | (logical & 0x1FFF)
        mprs = [0] * 8
        mprs[2] = mapped_bank
        mprs[logical >> 13] = mapped_bank
        return (
            f"main_ram_target_write sequence={sequence} event_sequence={event} "
            f"logical_address={logical:04x} physical_address={physical:06x} "
            f"value={value:02x} writer_pc=5000 writer_physical_pc=0d1000 "
            "writer_code_bytes=0000000000000000 "
            + " ".join(f"mpr{i}={item:02x}" for i, item in enumerate(mprs))
        )

    def _write_writers(self, *, omit: set[int] | None = None,
                       extra: list[str] | None = None,
                       header: str = WRITER_HEADER) -> None:
        expected = [
            (0xC85E, 0x20, 0x68), (0xC85F, 0x39, 0x68), (0xC860, 0x50, 0x68),
            (0x5039, 0x44, 0x68), (0x503A, 0x05, 0x68), (0x503B, 0x86, 0x68),
            (0x5000, 0xEA, 0x68), (0x5001, 0x01, 0x68), (0x5002, 0x02, 0x68),
        ]
        omitted = omit or set()
        rows = [self._writer_row(i, i + 1, logical, value, bank)
                for i, (logical, value, bank) in enumerate(expected)
                if i not in omitted]
        if extra:
            rows.extend(extra)
        self.writer.write_text("\n".join([header, *rows]) + "\n", encoding="ascii")

    def _write_receipt(self, *, capture_id: str = CAPTURE_ID,
                       target_range: str = "0d0000-0d1fff",
                       writer_rows: int | None = None,
                       writer_truncated: str = "0") -> None:
        if writer_rows is None:
            writer_rows = sum(
                line.startswith("main_ram_target_write ")
                for line in self.writer.read_text(encoding="ascii").splitlines()
            )
        fields = {
            "source": "authentic-mednafen-transition-receipt",
            "capture_id": capture_id,
            "c852_runtime_window_rows": "129",
            "c852_runtime_window_complete": "1",
            "c852_runtime_window_status": "complete",
            "c852_runtime_window_sha256": sha256(self.runtime),
            "pce_fast_main_ram_target_write_range": target_range,
            "pce_fast_main_ram_target_write_sample_limit": "1048576",
            "pce_fast_main_ram_target_write_rows": str(writer_rows),
            "pce_fast_main_ram_target_write_truncated": writer_truncated,
            "pce_fast_main_ram_target_write_sha256": sha256(self.writer),
        }
        self.receipt.write_text(
            "".join(f"{key}={value}\n" for key, value in fields.items()),
            encoding="ascii",
        )

    def test_matching_capture_and_latest_prior_writes_pass(self) -> None:
        stats, _ = verify(self.runtime, self.writer, self.receipt)
        self.assertGreater(stats["matched_latest_prior_write"], 0)
        self.assertEqual(stats["mismatched_latest_prior_write"], 0)
        self.assertEqual(stats["fetch_without_prior_write"], 0)

    def test_cross_capture_writer_header_is_rejected(self) -> None:
        self._write_writers(header=WRITER_HEADER.replace(CAPTURE_ID, OTHER_CAPTURE_ID))
        self._write_receipt()
        with self.assertRaisesRegex(ValueError, "different capture IDs"):
            verify(self.runtime, self.writer, self.receipt)

    def test_cross_capture_receipt_is_rejected(self) -> None:
        self._write_receipt(capture_id=OTHER_CAPTURE_ID)
        with self.assertRaisesRegex(ValueError, "different capture IDs"):
            verify(self.runtime, self.writer, self.receipt)

    def test_partial_target_range_is_rejected(self) -> None:
        partial_header = WRITER_HEADER.replace("0d0000-0d1fff", "0d0000-0d13ff")
        self._write_writers(header=partial_header)
        self._write_receipt(target_range="0d0000-0d13ff")
        with self.assertRaisesRegex(ValueError, "complete MPR2"):
            verify(self.runtime, self.writer, self.receipt)

    def test_receipt_hash_binds_both_sidecars(self) -> None:
        self.runtime.write_text(self.runtime.read_text() + "tampered\n")
        with self.assertRaisesRegex(ValueError, "hash does not match"):
            verify(self.runtime, self.writer, self.receipt)

    def test_writer_truncation_marker_is_rejected(self) -> None:
        self.writer.write_text(
            self.writer.read_text() + "main_ram_target_write_truncated=sample_limit limit=1048576\n"
        )
        self._write_receipt(writer_truncated="1")
        with self.assertRaisesRegex(ValueError, "uncapped writer trace"):
            verify(self.runtime, self.writer, self.receipt)

    def test_writer_event_sequence_must_increase(self) -> None:
        lines = self.writer.read_text().splitlines()
        parts = lines[2].split("event_sequence=")
        lines[2] = parts[0] + "event_sequence=1 " + parts[1].split(" ", 1)[1]
        self.writer.write_text("\n".join(lines) + "\n")
        self._write_receipt()
        with self.assertRaisesRegex(ValueError, "non-increasing writer event sequence"):
            verify(self.runtime, self.writer, self.receipt)

    def test_missing_prior_write_is_reported(self) -> None:
        self._write_writers(omit={6, 7, 8})
        self._write_receipt()
        stats, _ = verify(self.runtime, self.writer, self.receipt)
        self.assertGreater(stats["fetch_without_prior_write"], 0)

    def test_later_mismatching_overwrite_is_reported(self) -> None:
        overwrite = self._writer_row(9, 10, 0x5000, 0xFF, 0x68)
        self._write_writers(extra=[overwrite])
        self._write_receipt()
        stats, _ = verify(self.runtime, self.writer, self.receipt)
        self.assertGreater(stats["mismatched_latest_prior_write"], 0)


if __name__ == "__main__":
    unittest.main()
