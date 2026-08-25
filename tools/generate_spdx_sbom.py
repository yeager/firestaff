#!/usr/bin/env python3
"""Generate the deterministic SPDX 2.3 source SBOM for Firestaff.

The SBOM intentionally describes source and declared software dependencies,
not user-owned game media, BIOS/firmware, saves, captures, or local paths.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "sbom" / "firestaff.spdx.json"


def require_match(path: Path, pattern: str, label: str) -> str:
    match = re.search(pattern, path.read_text(encoding="utf-8"), re.MULTILINE)
    if not match:
        raise ValueError(f"could not determine {label} from {path.relative_to(ROOT)}")
    return match.group(1)


def package(
    spdx_id: str,
    name: str,
    version: str,
    license_id: str,
    download: str,
    *,
    comment: str | None = None,
) -> dict[str, object]:
    result: dict[str, object] = {
        "SPDXID": spdx_id,
        "name": name,
        "versionInfo": version,
        "downloadLocation": download,
        "filesAnalyzed": False,
        "licenseConcluded": license_id,
        "licenseDeclared": license_id,
        "copyrightText": "NOASSERTION",
    }
    if comment:
        result["comment"] = comment
    return result


def document() -> dict[str, object]:
    version = require_match(
        ROOT / "CMakeLists.txt",
        r"^project\(Firestaff VERSION ([0-9]+(?:\.[0-9]+)+) LANGUAGES C\)$",
        "Firestaff version",
    )
    miniz_version = require_match(
        ROOT / "third_party" / "miniz" / "miniz.h",
        r'^#define MZ_VERSION "([^"]+)"$',
        "miniz version",
    )
    sdl_tag = require_match(
        ROOT / ".github" / "workflows" / "verify.yml",
        r"^  SDL3_TAG: (\S+)$",
        "SDL version",
    )
    sdl_version = sdl_tag.removeprefix("release-")
    firestaff = "SPDXRef-Package-Firestaff"
    return {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": f"Firestaff-{version}",
        "documentNamespace": f"https://github.com/yeager/firestaff/spdx/{version}",
        "creationInfo": {
            "creators": ["Tool: tools/generate_spdx_sbom.py"],
            # A fixed timestamp makes the committed source SBOM reproducible.
            "created": "1970-01-01T00:00:00Z",
            "licenseListVersion": "3.25",
        },
        "documentDescribes": [firestaff],
        "comment": (
            "Source SBOM only. It excludes user-owned game data, saves, "
            "BIOS/firmware, capture material, and local filesystem paths."
        ),
        "packages": [
            package(
                firestaff,
                "Firestaff",
                version,
                "MIT",
                "https://github.com/yeager/firestaff",
                comment="Native engine source; original game media is not distributed.",
            ),
            package(
                "SPDXRef-Package-miniz",
                "miniz",
                miniz_version,
                "MIT",
                "https://github.com/richgel999/miniz",
                comment="Vendored, statically linked default ZIP/gzip provider.",
            ),
            package(
                "SPDXRef-Package-SDL",
                "SDL",
                sdl_version,
                "Zlib",
                "https://github.com/libsdl-org/SDL",
                comment="Required host window, input, audio, and rendering dependency (SDL3; SDL2 fallback is supported).",
            ),
            package(
                "SPDXRef-Package-Python",
                "Python",
                "NOASSERTION",
                "PSF-2.0",
                "https://www.python.org/",
                comment="Build-time interpreter for gettext and source-verification tools.",
            ),
            package(
                "SPDXRef-Package-libvorbisfile",
                "libvorbisfile",
                "NOASSERTION",
                "BSD-3-Clause",
                "https://xiph.org/vorbis/",
                comment="Optional in-memory OGG decoder for original Theron Track 01 transcodes.",
            ),
            package(
                "SPDXRef-Package-FFmpeg",
                "FFmpeg",
                "NOASSERTION",
                "LGPL-2.1-or-later",
                "https://ffmpeg.org/",
                comment="Optional in-memory decoder for original Macintosh MooV media.",
            ),
        ],
        "relationships": [
            {"spdxElementId": firestaff, "relationshipType": "CONTAINS", "relatedSpdxElement": "SPDXRef-Package-miniz"},
            {"spdxElementId": firestaff, "relationshipType": "DYNAMIC_LINK", "relatedSpdxElement": "SPDXRef-Package-SDL"},
            {"spdxElementId": "SPDXRef-Package-Python", "relationshipType": "BUILD_DEPENDENCY_OF", "relatedSpdxElement": firestaff},
            {"spdxElementId": "SPDXRef-Package-libvorbisfile", "relationshipType": "OPTIONAL_DEPENDENCY_OF", "relatedSpdxElement": firestaff},
            {"spdxElementId": "SPDXRef-Package-FFmpeg", "relationshipType": "OPTIONAL_DEPENDENCY_OF", "relatedSpdxElement": firestaff},
        ],
    }


def render() -> str:
    return json.dumps(document(), indent=2, sort_keys=True) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true", help="fail if the committed SBOM is stale")
    args = parser.parse_args()
    rendered = render()
    if args.check:
        if not args.output.is_file() or args.output.read_text(encoding="utf-8") != rendered:
            print(f"SPDX SBOM is stale: regenerate with {Path(__file__).relative_to(ROOT)}", file=sys.stderr)
            return 1
        print(f"SPDX SBOM is current: {args.output.relative_to(ROOT)}")
        return 0
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(rendered, encoding="utf-8")
    print(f"wrote {args.output.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
