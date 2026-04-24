#!/usr/bin/env python3
from __future__ import annotations

import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> None:
    cmd = [
        "python3",
        str(ROOT / "tools/generate_v2_creature_family.py"),
        "--source",
        "assets/cards/creatures/skeleton.png",
        "--slug",
        "skeleton",
        "--display-name",
        "Skeleton",
        "--family-dir",
        "assets-v2/creatures/wave1/skeleton-family",
    ]
    subprocess.run(cmd, cwd=ROOT, check=True)


if __name__ == "__main__":
    main()
