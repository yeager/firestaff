#!/usr/bin/env python3
"""Stream unchanged original CSB CTRaw captures into the production OFS reader."""
import os
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    if len(sys.argv) not in (2, 3):
        print("usage: test_firestaff_amiga_ctraw_real_media.py <probe> [original.7z]", file=sys.stderr)
        return 1
    probe = Path(sys.argv[1]).resolve()
    archive = Path(sys.argv[2] if len(sys.argv) == 3 else os.environ.get(
        "FIRESTAFF_CSB_AMIGA35_CTRAW_ARCHIVE",
        str(Path.home() / ".firestaff/data/csb/Chaos Strikes Back for Amiga 3.5 CTRaw.7z"))).resolve()
    sevenzip = shutil.which("7zz")
    if not probe.is_file() or not archive.is_file() or not sevenzip:
        print("SKIP: original CSB CTRaw archive, native probe, or development 7zz is unavailable")
        return 77
    editions = (
        ("en", "ChaosStrikesBack_Disk1of2 GameDisk_English.raw"),
        ("multi", "ChaosStrikesBack_Disk1of2_GameDisk_EnglishFrenchGerman.raw"),
    )
    for edition, member in editions:
        # A pipe preserves the exact original member bytes without writing a
        # raw image or a reconstructed game-data file anywhere on disk.
        source = subprocess.Popen(
            [sevenzip, "x", "-so", "-bd", str(archive), member],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            checked = subprocess.run([str(probe), edition, str(archive)],
                                     stdin=source.stdout, check=False)
            source.stdout.close()
            _, diagnostics = source.communicate()
        except BaseException:
            source.kill()
            source.communicate()
            raise
        if source.returncode != 0 or checked.returncode != 0:
            if diagnostics:
                sys.stderr.write(diagnostics.decode("utf-8", errors="replace"))
            print(f"FAIL: original CTRaw member {member}", file=sys.stderr)
            return 1
    print("PASS: original English and multilingual CSB CTRaw captures decoded in memory")
    return 0


if __name__ == "__main__":
    sys.exit(main())
