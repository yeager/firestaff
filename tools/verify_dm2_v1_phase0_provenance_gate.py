#!/usr/bin/env python3
"""DM2 V1 Phase 0 source/provenance gate.

This gate locks the exact Skullkeep original assets and the external DM2 source
references used for future parser/runtime work. It deliberately does not claim
DM2 runtime parity.
"""
from __future__ import annotations

from dataclasses import dataclass
import argparse
import hashlib
import json
import subprocess
import sys
import zipfile
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DM2_CANONICAL = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm2"
REDMCSB_SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DEFAULT_SKPROJECT_MIRROR = Path.home() / ".openclaw/data/firestaff-dm2-sources/skproject.git"
DEFAULT_SPHENX_CACHE = Path.home() / ".openclaw/data/firestaff-dm2-sources/sphenx-skwin"
DEFAULT_EVIDENCE = ROOT / "parity-evidence/verification/dm2_v1_phase0_provenance_gate/manifest.json"

SKPROJECT_REMOTE = "https://github.com/gbsphenx/skproject"
SKPROJECT_HEAD = "a962896e42aaf54c76157a7b062fb5b0526929e6"
SKPROJECT_TREE = "a095e458cfaaa0490b9c4d4d2adf88108a8ad92f"
SKPROJECT_TREE_FILE_COUNT = 1868
SKPROJECT_TREE_NAME_SHA256 = "3aa08529543f998f97936e0c23861793f2caf3257853d3b1f58f0e3dec011417"

SPHENX_PAGE_URL = "https://dmbuilder.sphenxmusics.fr/skwin.php"
SPHENX_PACKAGE_URL = "https://dmbuilder.sphenxmusics.fr/skwin/SkWinCurrent.zip"


@dataclass(frozen=True)
class MemberSpec:
    role: str
    path: str
    bytes: int
    crc32: str
    sha256: str


@dataclass(frozen=True)
class ZipSpec:
    label: str
    filename: str
    bytes: int
    sha256: str
    member_count: int
    members: tuple[MemberSpec, ...]


DM2_ARCHIVES: tuple[ZipSpec, ...] = (
    ZipSpec(
        label="DM2 DOS EN release",
        filename="Dungeon-Master-II-Skullkeep_DOS_EN.zip",
        bytes=13203537,
        sha256="d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929",
        member_count=34,
        members=(
            MemberSpec("dungeon", "data/dungeon.dat", 39437, "2530a08a", "cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef"),
            MemberSpec("graphics", "data/graphics.dat", 8639757, "c6d130f6", "c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346"),
            MemberSpec("songlist", "data/songlist.dat", 63, "fa6cdaca", "401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72"),
            MemberSpec("runtime", "skull.exe", 522637, "c1526f95", "0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35"),
        ),
    ),
    ZipSpec(
        label="DM2 DOS CD/layout candidate",
        filename="Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip",
        bytes=46596215,
        sha256="a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228",
        member_count=36,
        members=(
            MemberSpec("dungeon", "dumast2/DATA/DUNGEON.DAT", 39437, "2530a08a", "cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef"),
            MemberSpec("graphics", "dumast2/DATA/GRAPHICS.DAT", 8639757, "c6d130f6", "c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346"),
            MemberSpec("songlist", "dumast2/DATA/SONGLIST.DAT", 63, "fa6cdaca", "401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72"),
            MemberSpec("runtime", "dumast2/SKULL.EXE", 522637, "c1526f95", "0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35"),
        ),
    ),
)

SKULL_ASM = MemberSpec("source_disassembly", "SKULL.ASM", 7841116, "", "a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099")

SPHENX_PACKAGE = ZipSpec(
    label="Sphenx SKWin SPX package",
    filename="SkWinCurrent.zip",
    bytes=71221297,
    sha256="ed6f1f8a38c43fbff36421090c4bab4e5f939707db12f55cfcfac04688df4645",
    member_count=212,
    members=(
        MemberSpec("skwin_pc9821_runtime", "skwinspx/skwin9821.exe", 761856, "dd80d0de", "95ab275d465c8dee600f24a78df056618b25c06f44105c1d05f200780640787c"),
        MemberSpec("skwin_dos_runtime", "skwinspx/SKULLV4.EXE", 33585316, "26ecb296", "54d3d5a7a58413dcd1d329312fce1280908b738d5b5fbd98073d7c95d0576a36"),
        MemberSpec("skwin_dm2_dungeon", "skwinspx/data/DUNGEON.DAT", 37957, "a6cbf283", "530b8e28c70f231ae3dc6eee12f75181954635db33b3f2a9872a8d396fcf3982"),
        MemberSpec("skwin_dm2_graphics", "skwinspx/data/GRAPHICS.DAT", 8717101, "501e75a3", "7b8bbf6a843b01449c1cca20438c9e5c5a8c5ba8fddad761d3e2b64c5daf1a9b"),
        MemberSpec("skwin_dm2_songlist", "skwinspx/data/SONGLIST.DAT", 63, "eb361d01", "0561ef73aa0f0abe2086b238fcb72d2c231ae1e2c1035a6582a35a72c03233aa"),
        MemberSpec("skwin_log_reference", "skwinspx/LOGDM2.TXT", 57344, "af2c2c69", "2c33f09a515ce9a36c323aeb71f22b4f6fd87791020051aeacbeaad287c89edc"),
    ),
)


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def git_output(mirror: Path, *args: str) -> str:
    return subprocess.check_output(["git", "-C", str(mirror), *args], text=True).strip()


def check_zip(root: Path, spec: ZipSpec, errors: list[str]) -> dict[str, Any]:
    path = root / spec.filename
    record: dict[str, Any] = {
        "label": spec.label,
        "filename": spec.filename,
        "expected": {"bytes": spec.bytes, "sha256": spec.sha256, "memberCount": spec.member_count},
        "members": [],
    }
    if not path.is_file():
        errors.append(f"missing archive {path}")
        record["exists"] = False
        return record
    actual_size = path.stat().st_size
    actual_sha = sha256_file(path)
    record["exists"] = True
    record["actual"] = {"bytes": actual_size, "sha256": actual_sha}
    if actual_size != spec.bytes or actual_sha != spec.sha256:
        errors.append(f"archive mismatch {spec.filename}: bytes={actual_size} sha256={actual_sha}")
    with zipfile.ZipFile(path) as zf:
        infos = zf.infolist()
        record["actual"]["memberCount"] = len(infos)
        if len(infos) != spec.member_count:
            errors.append(f"member count mismatch {spec.filename}: {len(infos)} != {spec.member_count}")
        for member in spec.members:
            try:
                info = zf.getinfo(member.path)
                data = zf.read(member.path)
            except KeyError:
                errors.append(f"missing member {spec.filename}::{member.path}")
                record["members"].append({"role": member.role, "path": member.path, "exists": False})
                continue
            actual_member = {"bytes": len(data), "crc32": f"{info.CRC:08x}", "sha256": sha256_bytes(data)}
            expected_member = {"bytes": member.bytes, "crc32": member.crc32, "sha256": member.sha256}
            if actual_member != expected_member:
                errors.append(f"member mismatch {spec.filename}::{member.path}: {actual_member}")
            record["members"].append({"role": member.role, "path": member.path, "expected": expected_member, "actual": actual_member, "ok": actual_member == expected_member})
    return record


def check_skull_asm(dm2_canonical: Path, errors: list[str]) -> dict[str, Any]:
    path = dm2_canonical / SKULL_ASM.path
    record: dict[str, Any] = {"label": "DM2 DOS source disassembly", "filename": SKULL_ASM.path, "expected": {"bytes": SKULL_ASM.bytes, "sha256": SKULL_ASM.sha256}}
    if not path.is_file():
        errors.append(f"missing SKULL.ASM {path}")
        record["exists"] = False
        return record
    actual = {"bytes": path.stat().st_size, "sha256": sha256_file(path)}
    record["exists"] = True
    record["actual"] = actual
    if actual != record["expected"]:
        errors.append(f"SKULL.ASM mismatch: {actual}")
    return record


def check_skproject(mirror: Path, errors: list[str]) -> dict[str, Any]:
    record: dict[str, Any] = {
        "remote": SKPROJECT_REMOTE,
        "mirror": str(mirror),
        "expected": {"masterHead": SKPROJECT_HEAD, "tree": SKPROJECT_TREE, "treeFileCount": SKPROJECT_TREE_FILE_COUNT, "treeNameSha256": SKPROJECT_TREE_NAME_SHA256},
    }
    if not mirror.is_dir():
        errors.append(f"missing skproject mirror {mirror}")
        record["exists"] = False
        return record
    try:
        head = git_output(mirror, "rev-parse", "refs/heads/master")
        tree = git_output(mirror, "rev-parse", f"{SKPROJECT_HEAD}^{{tree}}")
        commit_type = git_output(mirror, "cat-file", "-t", SKPROJECT_HEAD)
        names = git_output(mirror, "ls-tree", "-r", "--name-only", SKPROJECT_HEAD).splitlines()
        tree_name_sha = hashlib.sha256(("\n".join(names) + "\n").encode("utf-8")).hexdigest()
    except subprocess.CalledProcessError as exc:
        errors.append(f"skproject git check failed: {exc}")
        record["exists"] = False
        return record
    actual = {"masterHead": head, "tree": tree, "commitType": commit_type, "treeFileCount": len(names), "treeNameSha256": tree_name_sha}
    record["exists"] = True
    record["actual"] = actual
    if head != SKPROJECT_HEAD:
        errors.append(f"skproject master HEAD mismatch: {head}")
    if tree != SKPROJECT_TREE or commit_type != "commit" or len(names) != SKPROJECT_TREE_FILE_COUNT or tree_name_sha != SKPROJECT_TREE_NAME_SHA256:
        errors.append(f"skproject tree identity mismatch: {actual}")
    return record


def check_sphenx(cache: Path, errors: list[str]) -> dict[str, Any]:
    page = cache / "skwin.php"
    record: dict[str, Any] = {
        "page": {"url": SPHENX_PAGE_URL, "filename": "skwin.php", "expected": {"bytes": 4351, "sha256": "ef5ed8402262d5eb95b4fe0f6e3ef6e1074a69cdaa9c2f864d3309f545d63091"}},
        "packageUrl": SPHENX_PACKAGE_URL,
    }
    if not page.is_file():
        errors.append(f"missing Sphenx SKWin page cache {page}")
        record["page"]["exists"] = False
    else:
        actual = {"bytes": page.stat().st_size, "sha256": sha256_file(page)}
        text = page.read_text(encoding="utf-8", errors="replace")
        anchors = ["SKWin is a project initiated by Kentaro", "skwin/SkWinCurrent.zip", "https://github.com/gbsphenx/skproject"]
        missing = [anchor for anchor in anchors if anchor not in text]
        record["page"]["exists"] = True
        record["page"]["actual"] = actual
        record["page"]["anchors"] = anchors
        if actual != record["page"]["expected"] or missing:
            errors.append(f"Sphenx page mismatch: actual={actual} missing={missing}")
    record["package"] = check_zip(cache, SPHENX_PACKAGE, errors)
    return record


def check_redmcsb_boundary(errors: list[str]) -> dict[str, Any]:
    checks = {
        "FILENAME.C": ["DATA\\\\DUNGEON.DAT", "DATA\\\\GRAPHICS.DAT"],
        "LOADSAVE.C": ["F0770_FILE_Open(G1059_pc_DungeonFileName)", "F0770_FILE_Open(\"\\\\DUNGEON.DAT\")"],
        "MEMORY.C": ["F0477_MEMORY_OpenGraphicsDat_CPSDF", "F0770_FILE_Open(G2130_GraphicsDatFileName)"],
    }
    result: dict[str, Any] = {"policy": "DM1/CSB source discipline comparison only; ReDMCSB is not a DM2 source.", "files": []}
    for rel, needles in checks.items():
        path = REDMCSB_SOURCE / rel
        file_rec = {"file": rel, "path": str(path), "needles": []}
        if not path.is_file():
            errors.append(f"missing ReDMCSB boundary file {path}")
            file_rec["exists"] = False
            result["files"].append(file_rec)
            continue
        text = path.read_text(encoding="latin-1", errors="replace")
        file_rec["exists"] = True
        for needle in needles:
            line = text.count("\n", 0, text.find(needle)) + 1 if needle in text else None
            file_rec["needles"].append({"needle": needle, "line": line})
            if line is None:
                errors.append(f"missing ReDMCSB boundary needle {rel}: {needle}")
        result["files"].append(file_rec)
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dm2-canonical", type=Path, default=DEFAULT_DM2_CANONICAL)
    parser.add_argument("--skproject-mirror", type=Path, default=DEFAULT_SKPROJECT_MIRROR)
    parser.add_argument("--sphenx-cache", type=Path, default=DEFAULT_SPHENX_CACHE)
    parser.add_argument("--evidence", type=Path, default=DEFAULT_EVIDENCE)
    args = parser.parse_args()

    using_default_roots = not any(
        arg.startswith(("--dm2-canonical", "--skproject-mirror", "--sphenx-cache"))
        for arg in sys.argv[1:]
    )
    if using_default_roots and not (
        args.dm2_canonical.is_dir()
        and args.skproject_mirror.is_dir()
        and args.sphenx_cache.is_dir()
    ):
        print("SKIP default DM2 provenance roots are not available on this host")
        return 0

    errors: list[str] = []
    result = {
        "schema": "firestaff.dm2_v1_phase0_provenance_gate.v1",
        "status": "PASS",
        "contract": "DM2 V1 parser/runtime work is blocked until these exact original assets and DM2 source references are present and hash-locked. This gate claims provenance only, not runtime parity.",
        "dm2CanonicalAssets": {
            "root": str(args.dm2_canonical),
            "archives": [check_zip(args.dm2_canonical, spec, errors) for spec in DM2_ARCHIVES],
            "sourceDisassembly": check_skull_asm(args.dm2_canonical, errors),
        },
        "skproject": check_skproject(args.skproject_mirror, errors),
        "sphenxSkwin": check_sphenx(args.sphenx_cache, errors),
        "redmcsbBoundaryDiscipline": check_redmcsb_boundary(errors),
        "parityClaimed": False,
        "runtimeOrParserImplemented": False,
        "nextAllowedWork": [
            "DM2-specific parity matrix and acceptance labels",
            "SKWin/skproject-backed data-format source map",
            "runtime probes only after a source-evidence manifest cites this gate",
        ],
        "errors": errors,
    }
    result["status"] = "PASS" if not errors else "FAIL"
    args.evidence.parent.mkdir(parents=True, exist_ok=True)
    args.evidence.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        print("FAIL dm2_v1_phase0_provenance_gate")
        for error in errors:
            print(f"  {error}")
        return 1
    print(f"PASS dm2_v1_phase0_provenance_gate: evidence={args.evidence.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
