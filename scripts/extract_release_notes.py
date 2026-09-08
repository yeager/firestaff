#!/usr/bin/env python3
"""Write one version's release-notes section for a distributable package."""

import argparse
from pathlib import Path

from verify_release_notes import fail, selected_section


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--notes", required=True, type=Path)
    parser.add_argument("--version", required=True)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    try:
        notes = args.notes.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {args.notes}: {exc}")

    section = "\n".join(selected_section(notes, args.version)).rstrip() + "\n"
    try:
        args.output.write_text(section, encoding="utf-8")
    except OSError as exc:
        fail(f"cannot write {args.output}: {exc}")


if __name__ == "__main__":
    main()
