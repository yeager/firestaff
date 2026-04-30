#!/usr/bin/env python3
"""Verify the narrow CSB utility member/source-lock boundary.

This gate is intentionally limited to ReDMCSB source references and curated
extracted utility-file identities. It does not claim CSB gameplay, rendering, or
save compatibility parity.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from dataclasses import dataclass
from pathlib import Path

DEFAULT_REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_EXTRACTED_DM = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted")
DEFAULT_JSON_OUT = Path("parity-evidence/verification/csb_utility_member_source_lock.json")


@dataclass(frozen=True)
class SourceCheck:
    id: str
    file: str
    line_range: tuple[int, int]
    needles: tuple[str, ...]
    function: str


SOURCE_CHECKS = (
    SourceCheck(
        id="hint-file-index-namespace",
        file="DEFS.H",
        line_range=(4575, 4577),
        function="hint-file index constants",
        needles=(
            "#define C2_HCSB_HTC    2",
            "#define C3_HCSB_DAT    3",
            "#define C4_CSBGAME_DAT 4",
        ),
    ),
    SourceCheck(
        id="su1e-file-name-table",
        file="HINTLOAD.C",
        line_range=(11, 18),
        function="G3637_apc_FileNames MEDIA772_SU1E",
        needles=(
            "char* G3637_apc_FileNames[6]",
            '"0HCSB.HTC"',
            '"0HCSB.DAT"',
            '"1CSBGAME.DAT"',
            '"1CSBGAME.BAK"',
        ),
    ),
    SourceCheck(
        id="au1e-file-name-table",
        file="HINTLOAD.C",
        line_range=(49, 56),
        function="G3637_apc_FileNames MEDIA762_AU1E",
        needles=(
            "char* G3637_apc_FileNames[6]",
            '"0HCSB.HTC"',
            '"0HCSB.DAT"',
            '"1CSBGAME.DAT"',
            '"1CSBGAME.BAK"',
        ),
    ),
    SourceCheck(
        id="hcsb-dat-startup-load",
        file="HINTSCR.C",
        line_range=(91, 100),
        function="hint startup disk/member probe and graphics load",
        needles=(
            "G3637_apc_FileNames[C3_HCSB_DAT]",
            "F1872_LoadGraphics(G3637_apc_FileNames[C3_HCSB_DAT]",
        ),
    ),
    SourceCheck(
        id="csbgame-open-routing",
        file="HINTLOAD.C",
        line_range=(300, 306),
        function="F1916_LoadGame",
        needles=(
            "G3638_i_IORequestIndex_CSBGAME = F1855_GetAvailableIORequestIndex();",
            "G3637_apc_FileNames[C4_CSBGAME_DAT]",
            "C02_OPEN_FILE",
        ),
    ),
)


@dataclass(frozen=True)
class MemberCheck:
    id: str
    relpath: str
    size: int
    sha256: str
    note: str


MEMBER_CHECKS = (
    MemberCheck(
        id="atari-harddisk-hcsb-htc",
        relpath="csb-atari/HardDisk/2009-02-22 PP/HCSB.HTC",
        size=66172,
        sha256="1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38",
        note="Atari CSB utility hint-content member referenced by ReDMCSB as 0HCSB.HTC.",
    ),
    MemberCheck(
        id="atari-harddisk-hcsb-dat",
        relpath="csb-atari/HardDisk/2009-02-22 PP/HCSB.DAT",
        size=30793,
        sha256="5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc",
        note="Atari CSB utility graphics/data member referenced by ReDMCSB as 0HCSB.DAT.",
    ),
    MemberCheck(
        id="amiga-harddisk-hcsb-dat-reference-only",
        relpath=(
            "csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/"
            "FTL_CSB_Utility/HCSB.DAT"
        ),
        size=11360,
        sha256="df22f19ab122dc8b0628db933061262a00f71297d36a356d41385a60f3347b1c",
        note="Amiga utility HCSB.DAT is a different locked lineage; reference only, not interchangeable with Atari lane.",
    ),
    MemberCheck(
        id="amiga-harddisk-hcsbf-htc-reference-only",
        relpath=(
            "csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/"
            "FTL_CSB_Utility/HCSBF.HTC"
        ),
        size=75424,
        sha256="79fd268631d3518462058c9e855b7b8a89c0ef8a0938adf9ed8ea17a55719ea7",
        note="Amiga French hint-content member is locked separately; it must not satisfy the Atari HCSB.HTC gate.",
    ),
)


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def source_excerpt(path: Path, line_range: tuple[int, int]) -> str:
    start, end = line_range
    return "\n".join(path.read_text(errors="replace").splitlines()[start - 1 : end])


def run_checks(redmcsb_source: Path, extracted_dm: Path) -> dict[str, object]:
    failures: list[str] = []
    source_rows: list[dict[str, object]] = []
    for check in SOURCE_CHECKS:
        path = redmcsb_source / check.file
        if not path.exists():
            failures.append(f"missing source file: {path}")
            source_rows.append({"id": check.id, "ok": False, "file": check.file})
            continue
        excerpt = source_excerpt(path, check.line_range)
        missing = [needle for needle in check.needles if needle not in excerpt]
        ok = not missing
        if not ok:
            failures.append(f"source check {check.id} missing needles: {missing!r}")
        source_rows.append(
            {
                "id": check.id,
                "ok": ok,
                "file": check.file,
                "function": check.function,
                "lines": f"{check.line_range[0]}-{check.line_range[1]}",
                "missing": missing,
            }
        )

    member_rows: list[dict[str, object]] = []
    for check in MEMBER_CHECKS:
        path = extracted_dm / check.relpath
        exists = path.exists()
        actual_size = path.stat().st_size if exists else None
        actual_sha = sha256_file(path) if exists else None
        ok = exists and actual_size == check.size and actual_sha == check.sha256
        if not ok:
            failures.append(
                f"member check {check.id} failed: exists={exists} size={actual_size} sha256={actual_sha}"
            )
        member_rows.append(
            {
                "id": check.id,
                "ok": ok,
                "relpath": check.relpath,
                "size": actual_size,
                "expected_size": check.size,
                "sha256": actual_sha,
                "expected_sha256": check.sha256,
                "note": check.note,
            }
        )

    return {
        "schema": "firestaff.csb_utility_member_source_lock.v1",
        "pass": not failures,
        "scope": "CSB utility file-name/source member identity only; no gameplay/render/save parity claim.",
        "redmcsb_source": str(redmcsb_source),
        "extracted_dm_root": str(extracted_dm),
        "source_checks": source_rows,
        "member_checks": member_rows,
        "non_claims": [
            "Does not choose a CSB gameplay/rendering target beyond keeping Atari and Amiga utility members separate.",
            "Does not validate CSBGAME.DAT save contents or compatibility.",
            "Does not use DANNESBURK or any UI/gameplay capture.",
        ],
        "failures": failures,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--extracted-dm", type=Path, default=DEFAULT_EXTRACTED_DM)
    parser.add_argument("--json-out", type=Path, default=DEFAULT_JSON_OUT)
    args = parser.parse_args()

    result = run_checks(args.redmcsb_source, args.extracted_dm)
    args.json_out.parent.mkdir(parents=True, exist_ok=True)
    args.json_out.write_text(json.dumps(result, indent=2) + "\n")
    print(
        f"{'PASS' if result['pass'] else 'FAIL'} csb utility member/source lock: "
        f"{len(result['source_checks'])} source checks, {len(result['member_checks'])} member checks"
    )
    if result["failures"]:
        for failure in result["failures"]:
            print(f"- {failure}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
