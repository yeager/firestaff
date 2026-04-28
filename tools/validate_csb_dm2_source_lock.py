#!/usr/bin/env python3
"""Validate CSB/DM2 source-lock inputs from local original archives.

The gate is intentionally archive/member-hash evidence only. It does not claim
that CSB or DM2 runtime/rendering parity exists in Firestaff.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
import zipfile
from dataclasses import dataclass
from pathlib import Path


DEFAULT_ROOT = Path(os.environ.get("FIRESTAFF_ORIGINAL_DM_ROOT", Path.home() / ".openclaw/data/firestaff-original-games/DM"))


@dataclass(frozen=True)
class MemberSpec:
    role: str
    path: str
    sha256: str


@dataclass(frozen=True)
class ArchiveSpec:
    game: str
    archive: str
    archive_sha256: str
    locked_status: str
    members: tuple[MemberSpec, ...]


ARCHIVES: tuple[ArchiveSpec, ...] = (
    ArchiveSpec(
        game="CSB Amiga hard-disk candidate",
        archive="Game,Chaos_Strikes_Back,Amiga,Software.7z",
        archive_sha256="77c3b9ceb3b6d7a9cf96b7cb4801e2b7e51e6de11c5982c82342da268dfddc58",
        locked_status="archive_and_candidate_members_locked_not_yet_curated",
        members=(
            MemberSpec("dungeon", "HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Dungeon.DAT", "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
            MemberSpec("graphics", "HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Graphics.DAT", "3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942"),
        ),
    ),
    ArchiveSpec(
        game="CSB Atari ST hard-disk candidate",
        archive="Game,Chaos_Strikes_Back,Atari_ST,Software.7z",
        archive_sha256="ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5",
        locked_status="archive_and_candidate_members_locked_not_yet_curated",
        members=(
            MemberSpec("dungeon", "HardDisk/2009-02-22 PP/DUNGEON.DAT", "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
            MemberSpec("graphics", "HardDisk/2009-02-22 PP/GRAPHICS.DAT", "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af"),
        ),
    ),
    ArchiveSpec(
        game="DM2 DOS EN release",
        archive="Dungeon-Master-II-Skullkeep_DOS_EN.zip",
        archive_sha256="d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929",
        locked_status="archive_and_members_locked",
        members=(
            MemberSpec("dungeon", "data/dungeon.dat", "cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef"),
            MemberSpec("graphics", "data/graphics.dat", "c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346"),
            MemberSpec("songlist", "data/songlist.dat", "401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72"),
            MemberSpec("runtime", "skull.exe", "0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35"),
        ),
    ),
    ArchiveSpec(
        game="DM2 DOS repack",
        archive="Dungeon Master 2.zip",
        archive_sha256="56716eea8ed4e64a5a4ce66d2f3d0c4ab19b462129b7ef7b0219f4166d4c6812",
        locked_status="archive_and_core_data_locked_runtime_differs_from_en_release",
        members=(
            MemberSpec("dungeon", "GAME/DATA/DUNGEON.DAT", "cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef"),
            MemberSpec("graphics", "GAME/DATA/GRAPHICS.DAT", "c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346"),
            MemberSpec("songlist", "GAME/DATA/SONGLIST.DAT", "401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72"),
            MemberSpec("runtime", "GAME/SKULL.EXE", "0efc121cab852aadb01302e517296c1b63e301abb17c31390b27565607a1151a"),
        ),
    ),
    ArchiveSpec(
        game="DM2 DOS CD/layout candidate",
        archive="Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip",
        archive_sha256="a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228",
        locked_status="archive_and_core_data_locked",
        members=(
            MemberSpec("dungeon", "dumast2/DATA/DUNGEON.DAT", "cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef"),
            MemberSpec("graphics", "dumast2/DATA/GRAPHICS.DAT", "c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346"),
            MemberSpec("songlist", "dumast2/DATA/SONGLIST.DAT", "401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72"),
            MemberSpec("runtime", "dumast2/SKULL.EXE", "0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35"),
        ),
    ),
    ArchiveSpec(
        game="DM2 DOS source disassembly",
        archive="Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z",
        archive_sha256="beb703174fe2e263d47e80f56d90b61fad30d2ce04a39e896e5205d6d698265a",
        locked_status="archive_and_source_member_locked_curated_extract_present",
        members=(
            MemberSpec("source_disassembly", "SKULL.ASM", "a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099"),
        ),
    ),
)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read_member(archive_path: Path, member: str) -> bytes:
    if archive_path.suffix.lower() == ".zip":
        with zipfile.ZipFile(archive_path) as zf:
            return zf.read(member)
    try:
        proc = subprocess.run(
            ["7z", "x", "-so", str(archive_path), member],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except FileNotFoundError as exc:
        raise RuntimeError("7z is required to verify .7z member hashes") from exc
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(f"7z failed for {archive_path.name}::{member}: {exc.stderr.decode(errors='replace')}") from exc
    return proc.stdout


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=DEFAULT_ROOT, help="directory containing original CSB/DM2 archives")
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--markdown-out", type=Path)
    args = parser.parse_args()

    rows = []
    failures: list[str] = []
    for spec in ARCHIVES:
        archive_path = args.root / spec.archive
        archive_exists = archive_path.exists()
        archive_actual = sha256_file(archive_path) if archive_exists else None
        archive_ok = archive_actual == spec.archive_sha256
        if not archive_ok:
            failures.append(f"archive mismatch/missing: {spec.archive}")
        member_rows = []
        if archive_ok:
            for member in spec.members:
                try:
                    actual = sha256_bytes(read_member(archive_path, member.path))
                    ok = actual == member.sha256
                except Exception as exc:  # noqa: BLE001 - report exact verifier failure
                    actual = None
                    ok = False
                    failures.append(f"member read failed: {spec.archive}::{member.path}: {exc}")
                if not ok:
                    failures.append(f"member mismatch: {spec.archive}::{member.path}")
                member_rows.append({"role": member.role, "path": member.path, "sha256": actual, "expected_sha256": member.sha256, "ok": ok})
        rows.append({
            "game": spec.game,
            "archive": spec.archive,
            "archive_sha256": archive_actual,
            "expected_archive_sha256": spec.archive_sha256,
            "archive_ok": archive_ok,
            "locked_status": spec.locked_status,
            "members": member_rows,
        })

    result = {
        "schema": "firestaff.csb_dm2_source_lock.v1",
        "root_source": "FIRESTAFF_ORIGINAL_DM_ROOT/local original archive directory; path intentionally not recorded",
        "pass": not failures,
        "archives_checked": len(rows),
        "members_checked": sum(len(row["members"]) for row in rows),
        "source_locks": rows,
        "missing_gates": [
            "No curated CSB extracted source-manifest is present in repo; CSB remains archive/member-candidate locked only.",
            "No CSB equivalent of tools/greatstone_dm1_source_lock_check.py exists yet.",
            "No DM2 runtime/source-manifest gate maps SKULL.ASM symbols to Firestaff code paths yet.",
            "No CSB/DM2 DOSBox/original capture or rendering parity probe is enabled.",
        ],
        "parity_claimed": False,
        "failures": failures,
    }

    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(result, indent=2) + "\n")

    if args.markdown_out:
        args.markdown_out.parent.mkdir(parents=True, exist_ok=True)
        lines = [
            "# CSB/DM2 source-lock evidence",
            "",
            "This is a source-lock/provenance gate for local CSB and DM2 original inputs. It records archive and selected member identities only; it does **not** claim CSB/DM2 runtime or visual parity.",
            "",
            f"Result: {'PASS' if result['pass'] else 'FAIL'} — {result['archives_checked']} archives, {result['members_checked']} selected members checked.",
            "",
            "## Locked inputs",
            "",
            "| game/source | archive | status | selected members |",
            "| --- | --- | --- | ---: |",
        ]
        for row in rows:
            lines.append(f"| {row['game']} | `{row['archive']}` | `{row['locked_status']}` | {len(row['members'])} |")
        lines += ["", "## Missing gates", ""]
        lines += [f"- {item}" for item in result["missing_gates"]]
        lines += ["", "## Notes", "", "- DM2 DOS EN and CD/layout candidate share core `DUNGEON.DAT`, `GRAPHICS.DAT`, `SONGLIST.DAT`, and `SKULL.EXE` hashes.", "- The `Dungeon Master 2.zip` repack shares core data with DM2 DOS EN but has a different `SKULL.EXE`; keep it as a locked alternate, not the canonical runtime source until curated.", "- CSB Amiga and Atari candidates share the same selected dungeon hash but have different graphics hashes; a curator still needs to choose the CSB target/version before runtime work."]
        args.markdown_out.write_text("\n".join(lines) + "\n")

    print(f"{'PASS' if result['pass'] else 'FAIL'} csb/dm2 source lock: {result['archives_checked']} archives, {result['members_checked']} members")
    if failures:
        print("failures:", file=sys.stderr)
        for failure in failures:
            print(f"- {failure}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
