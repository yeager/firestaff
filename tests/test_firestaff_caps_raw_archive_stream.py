#!/usr/bin/env python3
"""Feed original CTRaw archive members to the bounded reader via memory pipes."""

import shutil
import subprocess
import sys
from pathlib import Path


MEMBERS = (
    "ChaosStrikesBack_Disk1of2 GameDisk_English.raw",
    "ChaosStrikesBack_Disk1of2_GameDisk_EnglishFrenchGerman.raw",
    "ChaosStrikesBack_Disk2of2 UtilityDisk_Supplied with English.raw",
    "ChaosStrikesBack_Disk2of2 UtilityDisk_Supplied with EnglishFrenchGerman.raw",
)


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: test_firestaff_caps_raw_archive_stream.py READER ARCHIVE",
              file=sys.stderr)
        return 2
    reader = Path(sys.argv[1])
    archive = Path(sys.argv[2])
    seven_zip = shutil.which("7zz") or shutil.which("7z")
    if not seven_zip or not archive.is_file():
        print("SKIP: 7zz/7z or original CTRaw archive is unavailable")
        return 77

    for member in MEMBERS:
        extracted = subprocess.run(
            [seven_zip, "x", "-so", str(archive), member],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if extracted.returncode != 0:
            print(f"FAIL: 7z stream failed for {member}", file=sys.stderr)
            return 1
        parsed = subprocess.run(
            [str(reader)],
            input=extracted.stdout,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if parsed.returncode != 0:
            sys.stderr.buffer.write(parsed.stderr)
            print(f"FAIL: reader rejected {member}", file=sys.stderr)
            return 1
        print(f"{member}: {parsed.stdout.decode('ascii').strip()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
