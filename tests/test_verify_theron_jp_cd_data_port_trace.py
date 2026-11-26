#!/usr/bin/env python3
"""Exercise JP CD data-port source checks with format-only sidecar fixtures."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / "scripts"))

import verify_theron_jp_cd_data_port_trace as verifier


def trace_row(
    sequence: int,
    user_offset: int,
    value: int,
    layout: verifier.Track02Layout,
) -> str:
    sector_index = 3674 - layout.start_lba
    file_offset = (
        layout.index1_file_offset
        + sector_index * layout.sector_bytes
        + layout.file_user_data_offset
        + user_offset
    )
    mprs = [0xFF, 0xF8, 0x68, 0x6B, 0x78, 0x70, 0x69, 0x00]
    return (
        f"cd_data_port_read sequence={sequence} cpu_pc=ea99 address=1808 "
        f"value={value:02x} reader_pc=ea99 reader_physical_pc=000a99 "
        f"reader_mpr_slot=7 "
        + " ".join(f"reader_mpr{i}={mpr:02x}" for i, mpr in enumerate(mprs))
        + f" source_valid=1 source_lba=3674 source_user_offset={user_offset} "
        f"source_raw_offset={layout.source_user_data_offset + user_offset} "
        f"track02_start_lba={layout.start_lba} track02_end_lba={layout.end_lba} "
        f"track02_index1_file_offset={layout.index1_file_offset} "
        f"track02_sector_bytes={layout.sector_bytes} "
        f"track02_raw_file_offset={file_offset} event_sequence={sequence + 1}"
    )


class JPDataPortTraceTests(unittest.TestCase):
    def verify_fixture(
        self,
        layout: verifier.Track02Layout,
        offsets: tuple[int, ...],
    ) -> tuple[Path, Path, tempfile.TemporaryDirectory]:
        temp = tempfile.TemporaryDirectory()
        root = Path(temp.name)
        track = bytearray(layout.size)
        values = {offset: (0xA0 + index) & 0xFF for index, offset in enumerate(offsets)}
        sector_file_offset = (
            layout.index1_file_offset
            + (3674 - layout.start_lba) * layout.sector_bytes
            + layout.file_user_data_offset
        )
        for offset, value in values.items():
            track[sector_file_offset + offset] = value
        track_path = root / "TQJP02.media"
        track_path.write_bytes(track)
        lines = [verifier.TRACE_HEADER_WITH_READER_MPR_SHARED_SEQUENCE]
        lines.extend(
            trace_row(sequence, offset, values[offset], layout)
            for sequence, offset in enumerate(offsets)
        )
        trace_path = root / "capture.cd-data-port-read"
        trace_path.write_text("\n".join(lines) + "\n", encoding="ascii")
        return trace_path, track_path, temp

    def test_verifies_akutuba_span_against_authenticated_iso_layout(self) -> None:
        offsets = (285, 286, 287)
        trace, track, temp = self.verify_fixture(verifier.JP_TRACK02_ISO, offsets)
        try:
            with mock.patch.object(
                verifier, "identify_track02", return_value=verifier.JP_TRACK02_ISO
            ):
                observed, rows = verifier.verify(
                    trace,
                    track,
                    required_lba=3674,
                    required_user_range=(285, 288),
                )
            self.assertEqual(observed, bytes((0xA0, 0xA1, 0xA2)))
            self.assertEqual([row["source_lba"] for row in rows], [3674] * 3)
        finally:
            temp.cleanup()

    def test_verifies_raw_bin_file_offsets_with_sector_headers(self) -> None:
        offsets = (285, 286)
        trace, track, temp = self.verify_fixture(verifier.JP_TRACK02_BIN, offsets)
        try:
            with mock.patch.object(
                verifier, "identify_track02", return_value=verifier.JP_TRACK02_BIN
            ):
                observed, _ = verifier.verify(
                    trace,
                    track,
                    required_lba=3674,
                    required_user_range=(285, 287),
                )
            self.assertEqual(observed, bytes((0xA0, 0xA1)))
        finally:
            temp.cleanup()

    def test_rejects_incomplete_akutuba_source_range(self) -> None:
        trace, track, temp = self.verify_fixture(verifier.JP_TRACK02_ISO, (285, 287))
        try:
            with mock.patch.object(
                verifier, "identify_track02", return_value=verifier.JP_TRACK02_ISO
            ):
                with self.assertRaisesRegex(ValueError, "missing 1 required user-data byte"):
                    verifier.verify(
                        trace,
                        track,
                        required_lba=3674,
                        required_user_range=(285, 288),
                    )
        finally:
            temp.cleanup()

    def test_rejects_wrong_exact_lba(self) -> None:
        trace, track, temp = self.verify_fixture(verifier.JP_TRACK02_ISO, (285,))
        try:
            with mock.patch.object(
                verifier, "identify_track02", return_value=verifier.JP_TRACK02_ISO
            ):
                with self.assertRaisesRegex(ValueError, "required source LBA 3675 was not observed"):
                    verifier.verify(trace, track, required_lba=3675)
        finally:
            temp.cleanup()

    def test_rejects_unauthenticated_track02_bytes(self) -> None:
        with self.assertRaisesRegex(ValueError, "not an authenticated JP Rev. 1"):
            verifier.identify_track02(b"unverified media")


if __name__ == "__main__":
    unittest.main()
