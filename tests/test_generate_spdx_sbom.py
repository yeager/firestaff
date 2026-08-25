#!/usr/bin/env python3
"""Regression checks for the committed reproducible SPDX source SBOM."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SBOM = ROOT / "sbom" / "firestaff.spdx.json"


def main() -> int:
    result = subprocess.run(
        [sys.executable, "tools/generate_spdx_sbom.py", "--check"],
        cwd=ROOT,
        check=False,
    )
    if result.returncode:
        return result.returncode
    payload = json.loads(SBOM.read_text(encoding="utf-8"))
    assert payload["spdxVersion"] == "SPDX-2.3"
    assert payload["dataLicense"] == "CC0-1.0"
    assert payload["documentDescribes"] == ["SPDXRef-Package-Firestaff"]
    packages = {package["name"]: package for package in payload["packages"]}
    assert packages["Firestaff"]["licenseDeclared"] == "MIT"
    assert packages["miniz"]["licenseDeclared"] == "MIT"
    assert packages["SDL"]["licenseDeclared"] == "Zlib"
    serialized = SBOM.read_text(encoding="utf-8").lower()
    # The explanatory comment may name excluded categories; it must never
    # expose a local media location or enumerate original media files.
    for forbidden in ("/home/", "~/.firestaff", "dungeon.dat", "dm.bin", "track02.bin"):
        assert forbidden not in serialized, forbidden
    print("SPDX SBOM regression: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
