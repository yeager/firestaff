#!/usr/bin/env python3
"""Ensure JP code-fetch verifiers reject explicit bounded-trace loss."""

from pathlib import Path
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
        target_path = root / "capture.cd-ram-target-write"
        target_path.write_text("source=mednafen-pce-fast-cd-ram-target-write\n", encoding="ascii")
        fetch_path = root / "capture.system-card-dispatch"
        for window in ("4000-5fff", "6000-7fff"):
            marker = f"code_fetch_truncated event_sequence=1 window={window} cap=262144\n"
            source_path = root / f"source-{window}.system-card-dispatch"
            source_path.write_text(
                "source=mednafen-pce-fast-system-card-controller-dispatch-v2\n" + marker,
                encoding="ascii",
            )
            expect_incomplete(
                f"source-bound verifier for {window}",
                lambda: source_bound.verify_code_fetch_rows(source_path),
            )

            fetch_path.write_text(target_fetch.FETCH_HEADER + "\n" + marker, encoding="ascii")
            expect_incomplete(
                f"target-fetch verifier for {window}",
                lambda: target_fetch.reconcile(target_path, fetch_path),
            )

    print("PASS: both JP code-fetch verifiers reject explicit trace truncation")


if __name__ == "__main__":
    main()
