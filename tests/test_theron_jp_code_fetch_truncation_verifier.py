#!/usr/bin/env python3
"""Ensure JP code-fetch verifiers reject explicit bounded-trace loss."""

from pathlib import Path
import subprocess
import sys
import tempfile

repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / "scripts"))

import verify_theron_jp_cd_ram_writer_pc_trace as source_bound
import verify_theron_jp_fetch_target_writes as target_fetch


def expect_incomplete(label: str, callback) -> None:
    try:
        callback()
    except ValueError as error:
        if "incomplete code-fetch trace" not in str(error):
            raise AssertionError(f"{label} rejected for the wrong reason: {error}") from error
    else:
        raise AssertionError(f"{label} accepted an explicitly truncated trace")


def main() -> None:
    repo_build = repo / "build"
    repo_build.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="theron-code-fetch-truncation-", dir=repo_build) as tmp:
        root = Path(tmp)
        writer_path = root / "capture.cd-ram-writer-pc"
        writer_path.write_text(
            "source=mednafen-pce-fast-cd-ram-writer-pc-write-v1 "
            "event_sequence_source=TheronTraceCDSharedEventSequence\n"
            "cd_ram_writer_pc_truncated sequence=131072 limit=131072 reason=sample-cap\n",
            encoding="ascii",
        )
        try:
            source_bound.verify_writer_rows(writer_path)
        except ValueError as error:
            if "writer-PC trace reached its sample cap" not in str(error):
                raise AssertionError(f"writer-PC cap rejected for the wrong reason: {error}") from error
        else:
            raise AssertionError("writer-PC verifier accepted a trace that reached its sample cap")

        target_path = root / "capture.cd-ram-target-write"
        target_path.write_text("source=mednafen-pce-fast-cd-ram-target-write\n", encoding="ascii")
        fetch_path = root / "capture.system-card-dispatch"
        cases = (
            ("4000-5fff", "source=mednafen-pce-fast-system-card-controller-dispatch-v2"),
            ("6000-7fff", "source=mednafen-pce-fast-system-card-controller-dispatch-v2"),
            ("40d9", "source=mednafen-pce-fast-system-card-controller-dispatch-v3 code_fetch_scope=targeted"),
            ("6138-6144", "source=mednafen-pce-fast-system-card-controller-dispatch-v3 code_fetch_scope=targeted"),
            (
                "40d9,42ab,42b2,44c1,44d2",
                "source=mednafen-pce-fast-system-card-controller-dispatch-v4 code_fetch_scope=targeted-consumer-boundaries",
            ),
            (
                "6138-6148",
                "source=mednafen-pce-fast-system-card-controller-dispatch-v4 code_fetch_scope=targeted-consumer-boundaries",
            ),
        )
        for window, header in cases:
            marker = f"code_fetch_truncated event_sequence=1 window={window} cap=262144\n"
            source_path = root / f"source-{window}.system-card-dispatch"
            source_path.write_text(header + "\n" + marker, encoding="ascii")
            expect_incomplete(
                f"source-bound verifier for {window}",
                lambda: source_bound.verify_code_fetch_rows(source_path),
            )

            fetch_path.write_text(header + "\n" + marker, encoding="ascii")
            expect_incomplete(
                f"target-fetch verifier for {window}",
                lambda: target_fetch.reconcile(target_path, fetch_path),
            )

        valid_target = root / "valid-v4.cd-ram-target-write"
        target_addresses = (*range(0x1000D2, 0x1000E4), 0x10468D, 0x10468E)
        target_values = [((index + 1) & 0xFF) for index in range(len(target_addresses))]
        target_values[target_addresses.index(0x1000D9)] = 0xA9
        target_values[target_addresses.index(0x1000DA)] = 0x4C
        target_values[target_addresses.index(0x1000DB)] = 0x8D
        target_rows = ["source=mednafen-pce-fast-cd-ram-target-write"]
        for sequence, (address, value) in enumerate(zip(target_addresses, target_values, strict=True)):
            mprs = [0] * 7 + [0x80]
            target_rows.append(
                f"cd_ram_target_write sequence={sequence} event_sequence={sequence} "
                f"physical_address={address:06x} writer_pc=ea9c writer_physical_pc=100a9c "
                + " ".join(f"mpr{index}={mpr:02x}" for index, mpr in enumerate(mprs))
                + f" value={value:02x}"
            )
        valid_target.write_text("\n".join(target_rows) + "\n", encoding="ascii")

        valid_v4 = root / "valid-v4.system-card-dispatch"
        valid_v4.write_text(
            "source=mednafen-pce-fast-system-card-controller-dispatch-v4 code_fetch_scope=targeted-consumer-boundaries\n"
            "code_fetch event_sequence=50 logical_pc=40d9 physical_pc=1000d9 "
            "mpr0=00 mpr1=00 mpr2=80 mpr3=00 mpr4=00 mpr5=00 mpr6=00 mpr7=00 "
            "bytes=a94c8d physical_bytes=1000d9,1000da,1000db valid_mask=7\n"
            "code_fetch_extended event_sequence=51 logical_pc=6148 physical_pc=102148 "
            "mpr0=00 mpr1=00 mpr2=00 mpr3=81 mpr4=00 mpr5=00 mpr6=00 mpr7=00 "
            "bytes=4c0040 physical_bytes=102148,102149,10214a valid_mask=7\n",
            encoding="ascii",
        )
        rows = source_bound.verify_code_fetch_rows(valid_v4)
        if len(rows) != 2 or rows[0]["logical_pc"] != "40d9" or rows[1]["logical_pc"] != "6148":
            raise AssertionError("source-bound verifier did not accept complete targeted v4 fetch rows")
        counts = target_fetch.reconcile(valid_target, valid_v4)
        if counts[3] != 3 or counts[4] != 0 or counts[5] != 0:
            raise AssertionError("target-fetch verifier did not reconcile complete targeted v4 rows")
        invalid_v4 = root / "invalid-v4.system-card-dispatch"
        invalid_v4.write_text(
            valid_v4.read_text(encoding="ascii").replace("logical_pc=6148", "logical_pc=6149"),
            encoding="ascii",
        )
        for label, callback in (
            ("source-bound verifier", lambda: source_bound.verify_code_fetch_rows(invalid_v4)),
            ("target-fetch verifier", lambda: target_fetch.reconcile(valid_target, invalid_v4)),
        ):
            try:
                callback()
            except ValueError as error:
                if "outside v4 target set" not in str(error):
                    raise AssertionError(f"{label} rejected an unlisted PC for the wrong reason: {error}") from error
            else:
                raise AssertionError(f"{label} accepted a v4 code-fetch PC outside its target set")
        result = subprocess.run(
            [
                sys.executable,
                str(repo / "scripts/verify_theron_jp_fetch_target_writes.py"),
                str(valid_target),
                str(valid_v4),
            ],
            check=False,
            capture_output=True,
            text=True,
        )
        if result.returncode != 0 or "address_10468d_fetch_bytes=0" not in result.stdout:
            raise AssertionError("target-fetch CLI did not report unobserved candidate addresses as zero")

    print("PASS: both JP code-fetch verifiers reject v2/v3/v4 truncation and reconcile complete v4 rows")


if __name__ == "__main__":
    main()
