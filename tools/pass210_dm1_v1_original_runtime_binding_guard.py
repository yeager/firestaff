#!/usr/bin/env python3
"""Pass210: guarded DM1 PC34 original FIRES runtime-binding scaffold.

This gate intentionally does *not* turn compressed LZEXE loader offsets into
runtime CS:IP breakpoints. It records what is currently known on N2 and emits a
strict future trace contract for binding live debugger hits to ReDMCSB seams:
command_accepted, movement_applied, party_coordinates_committed, viewport_present.
"""
from __future__ import annotations

import hashlib
import json
import struct
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
REDMCSB_ROOT = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206")
IBM_SOURCE = REDMCSB_ROOT / "Toolchains/IBM PC/Source"
ORIGINAL_FIRES = (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/DungeonMasterPC34/FIRES")
OUT_DIR = ROOT / "parity-evidence/verification/pass210_dm1_v1_original_runtime_binding_guard"
REPORT = ROOT / "parity-evidence/pass210_dm1_v1_original_runtime_binding_guard.md"

MZ_FIELDS = ["e_magic", "e_cblp", "e_cp", "e_crlc", "e_cparhdr", "e_minalloc", "e_maxalloc", "e_ss", "e_sp", "e_csum", "e_ip", "e_cs", "e_lfarlc", "e_ovno"]

SOURCE_SEAMS: list[dict[str, Any]] = [
    {
        "id": "command_accepted",
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "lines": "2095-2127,2150-2156,2322-2324",
        "must": [
            "L1160_i_Command = G0432_as_CommandQueue",
            "L1161_i_CommandX",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0377_COMMAND_ProcessType80_ClickInDungeonView",
        ],
        "runtime_hit_requires": ["queue index", "command id", "command x/y", "CS:IP before dispatch"],
    },
    {
        "id": "movement_applied",
        "file": "CLIKMENU.C",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "lines": "256-270,317-329,345-347",
        "must": [
            "AL1118_ui_MovementArrowIndex",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "L1117_B_MovementBlocked",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks",
        ],
        "runtime_hit_requires": ["movement arrow index", "destination x/y", "blocked flag", "CS:IP around move-result call"],
    },
    {
        "id": "party_coordinates_committed",
        "file": "MOVESENS.C",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "lines": "438-444,493-496,573-578",
        "must": [
            "P0557_T_Thing == C0xFFFF_THING_PARTY",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY",
        ],
        "runtime_hit_requires": ["thing == party", "source x/y", "destination x/y", "committed G0306/G0307 values"],
    },
    {
        "id": "viewport_present",
        "file": "GAMELOOP.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF / F0380_COMMAND_ProcessQueue_CPSC cadence",
        "lines": "83-91,164-168,215-219",
        "must": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)",
            "F0361_COMMAND_ProcessKeyPress",
            "F0380_COMMAND_ProcessQueue_CPSC",
        ],
        "runtime_hit_requires": ["post-command party dir/x/y", "draw call observed", "frame/present marker after draw"],
    },
    {
        "id": "viewport_present_blit",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "lines": "849-858",
        "must": ["F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort", "G0296_puc_Bitmap_Viewport"],
        "runtime_hit_requires": ["viewport zone", "viewport bitmap pointer", "video-driver blit call"],
    },
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_mz(path: Path) -> dict[str, Any]:
    data = path.read_bytes()
    vals = dict(zip(MZ_FIELDS, struct.unpack_from("<14H", data, 0)))
    if vals["e_magic"] != 0x5A4D:
        raise ValueError(f"not MZ: {path}")
    image_file_size = (vals["e_cp"] - 1) * 512 + (vals["e_cblp"] or 512)
    header_bytes = vals["e_cparhdr"] * 16
    entry_linear = vals["e_cs"] * 16 + vals["e_ip"]
    return {
        "path": str(path),
        "size": len(data),
        "sha256": sha256(path),
        "header_bytes": header_bytes,
        "load_image_bytes_from_mz_header": max(0, image_file_size - header_bytes),
        "lzexe_signature_at_relocation_table": data[vals["e_lfarlc"]: vals["e_lfarlc"] + 4].decode("ascii", errors="replace"),
        "compressed_loader_entry": {
            "relative_cs_ip": f"{vals['e_cs']:04x}:{vals['e_ip']:04x}",
            "relative_linear_offset_from_load_image_base": f"0x{entry_linear:05x}",
            "safe_interpretation": "LZEXE loader entry only; not a decompressed FIRES runtime text address",
        },
        "stack_at_loader_entry": f"{vals['e_ss']:04x}:{vals['e_sp']:04x}",
    }


def numbered_excerpt(file: str, spec: str) -> str:
    lines = (SOURCE_ROOT / file).read_text(errors="replace").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        start_s, _, end_s = part.strip().partition("-")
        start = int(start_s)
        end = int(end_s or start_s)
        for no in range(start, min(end, len(lines)) + 1):
            out.append(f"{file}:{no}: {lines[no - 1]}")
    return "\n".join(out)


def audit_seams() -> list[dict[str, Any]]:
    audited = []
    for seam in SOURCE_SEAMS:
        excerpt = numbered_excerpt(seam["file"], seam["lines"])
        missing = [needle for needle in seam["must"] if needle not in excerpt]
        audited.append({**seam, "source_path": str(SOURCE_ROOT / seam["file"]), "ok": not missing, "missing": missing, "excerpt": excerpt})
    return audited


def scan_runtime_artifacts() -> dict[str, Any]:
    original_roots = [
        (Path.home() / ".openclaw/data/firestaff-original-games/DM"),
        REDMCSB_ROOT / "Reference",
    ]
    map_roots = original_roots + [REDMCSB_ROOT]
    maps = sorted({str(p) for base in map_roots if base.exists() for p in base.rglob("*.MAP")})
    unpack_tools = sorted({str(p) for p in REDMCSB_ROOT.rglob("*") if p.is_file() and ("unlz" in p.name.lower() or p.name.upper() in {"LZEXE.EXE", "TLINK.EXE"})})

    fires_like: list[dict[str, Any]] = []
    seen: set[str] = set()
    for base in original_roots:
        if not base.exists():
            continue
        for p in base.rglob("*"):
            if not p.is_file() or not p.name.upper().startswith("FIRES"):
                continue
            digest = sha256(p)
            key = f"{digest}:{p.stat().st_size}"
            if key in seen:
                continue
            seen.add(key)
            try:
                mz = parse_mz(p)
                loader_entry = dict(mz["compressed_loader_entry"])
                if mz["lzexe_signature_at_relocation_table"] != "LZ91":
                    loader_entry["safe_interpretation"] = "MZ entry only; not bound to a DM1 V1 PC34 runtime seam"
                fires_like.append({
                    "path": str(p),
                    "size": mz["size"],
                    "sha256": mz["sha256"],
                    "mz": True,
                    "header_bytes": mz["header_bytes"],
                    "loader_entry": loader_entry,
                    "stack_at_loader_entry": mz["stack_at_loader_entry"],
                    "lzexe_signature_at_relocation_table": mz["lzexe_signature_at_relocation_table"],
                    "runtime_claim": "none; inventory only",
                })
            except Exception as exc:
                fires_like.append({
                    "path": str(p),
                    "size": p.stat().st_size,
                    "sha256": digest,
                    "mz": False,
                    "error": str(exc),
                    "runtime_claim": "none; inventory only",
                })

    candidate_roots = [ROOT / "parity-evidence", ROOT / "verification-screens", ROOT / "tmp", Path("/tmp")]
    dump_suffixes = {".bin", ".dump", ".mem", ".img"}
    dump_hits: list[str] = []
    for base in candidate_roots:
        if not base.exists():
            continue
        for p in base.rglob("*"):
            if p.is_file() and p.suffix.lower() in dump_suffixes and "fires" in p.name.lower():
                dump_hits.append(str(p))
    return {
        "redmcsb_map_artifacts": maps,
        "candidate_fires_runtime_dumps": sorted(dump_hits),
        "original_fires_like_inventory": fires_like,
        "unpack_or_link_tools_found": unpack_tools,
        "has_runtime_address_inputs": bool(maps or dump_hits),
        "inventory_conclusion": "only packed/original FIRES-like binaries and tooling are present; no TLINK .MAP or verified decompressed runtime dump was found",
    }


def binding_contract() -> dict[str, Any]:
    return {
        "schema": "dm1_v1_original_runtime_trace_binding.v1",
        "required_loader_facts": ["psp_segment", "program_load_segment = psp_segment + 0x10", "post_lzexe_transfer_cs_ip_or_decompressed_entry", "decompressed_image_sha256_or_map_sha256"],
        "required_symbol_inputs": ["FIRES.MAP public symbols or verified decompressed image offset map", "symbol segment:offset for every seam", "load/relocation rule used"],
        "required_hits": [
            {"seam": seam["id"], "source": f"{seam['file']}:{seam['lines']}", "function": seam["function"], "required_observations": seam["runtime_hit_requires"]}
            for seam in SOURCE_SEAMS[:4]
        ],
        "hard_reject_if": [
            "only compressed LZEXE loader CS:IP is supplied",
            "no PSP/load segment is recorded",
            "no decompressed transfer point or map symbol address is recorded",
            "a breakpoint hit is missing its observed CS:IP and register/memory context",
            "static compressed-file offsets are promoted as runtime text addresses",
        ],
    }


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    mz = parse_mz(ORIGINAL_FIRES)
    seams = audit_seams()
    artifacts = scan_runtime_artifacts()
    contract = binding_contract()
    classification = "blocked/runtime-base-and-symbol-map-unavailable"
    if any(not seam["ok"] for seam in seams):
        classification = "blocked/source-seam-audit-failed"

    blocker = (
        "No decompressed stock FIRES runtime image base, post-LZEXE transfer CS:IP, "
        "or TLINK FIRES.MAP is present on N2; compressed loader CS:IP remains loader-only evidence."
    )
    manifest = {
        "schema": "pass210_dm1_v1_original_runtime_binding_guard.v1",
        "classification": classification,
        "exact_remaining_blocker": blocker,
        "n2_only": True,
        "stock_fires": mz,
        "source_root": str(SOURCE_ROOT),
        "ibm_source_root": str(IBM_SOURCE),
        "runtime_artifact_scan": artifacts,
        "source_seams": seams,
        "binding_contract": contract,
        "non_claims": [
            "does not claim a loaded/decompressed FIRES runtime image base",
            "does not claim any source symbol is bound to stock runtime CS:IP",
            "does not claim debugger breakpoints were hit",
            "does not promote compressed LZEXE loader offsets as runtime text addresses",
        ],
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "trace_binding_contract.json").write_text(json.dumps(contract, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "source_seams.json").write_text(json.dumps(seams, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "runtime_trace_template.json").write_text(json.dumps({
        "schema": contract["schema"],
        "psp_segment_hex": None,
        "program_load_segment_hex": None,
        "post_lzexe_transfer_cs_ip": None,
        "decompressed_image_sha256": None,
        "map_sha256": None,
        "symbol_bindings": [],
        "hits": [],
    }, indent=2, sort_keys=True) + "\n")

    runbook = [
        "# Pass210 guarded runtime-binding runbook",
        "",
        "Use this only after a live DOS debugger or map build provides real runtime inputs.",
        "",
        "## Required order",
        "1. Record PSP segment and compute program load segment as `PSP+0x10`.",
        "2. Break at the stock FIRES LZEXE loader entry only to find the decompressor handoff; do not treat that loader entry as game code.",
        "3. Record the post-LZEXE transfer CS:IP or produce a verified decompressed FIRES dump/map.",
        "4. Bind ReDMCSB map/source symbols to runtime segment:offsets using the recorded load/relocation rule.",
        "5. Only then set/check the four seam hits in `trace_binding_contract.json`.",
        "",
        "## Loader-only fact currently known",
        f"- Stock compressed loader entry: `{mz['compressed_loader_entry']['relative_cs_ip']}` relative to DOS load-image base; this is **not** a breakpoint for `command_accepted` etc.",
        "",
        "## Guardrail",
        "If the trace has only compressed-file offsets or the compressed loader CS:IP, classify it as `blocked/runtime-base-and-symbol-map-unavailable`.",
    ]
    (OUT_DIR / "guarded_runtime_binding_runbook.md").write_text("\n".join(runbook) + "\n")

    lines = [
        "# Pass210 — DM1 V1 original runtime binding guard",
        "",
        f"Classification: `{classification}`",
        f"Exact remaining blocker: {blocker}",
        "",
        "## What was investigated",
        f"- Stock FIRES: `{ORIGINAL_FIRES}` size `{mz['size']}` sha256 `{mz['sha256']}`.",
        f"- LZEXE signature at relocation table: `{mz['lzexe_signature_at_relocation_table']}`.",
        f"- Compressed loader entry: `{mz['compressed_loader_entry']['relative_cs_ip']}`; interpretation: `{mz['compressed_loader_entry']['safe_interpretation']}`.",
        f"- ReDMCSB `*.MAP` artifacts found: `{len(artifacts['redmcsb_map_artifacts'])}`.",
        f"- Candidate FIRES runtime dumps found: `{len(artifacts['candidate_fires_runtime_dumps'])}`.",
        f"- FIRES-like original binaries inventoried: `{len(artifacts['original_fires_like_inventory'])}`.",
        f"- Unpack/link tools found: `{len(artifacts['unpack_or_link_tools_found'])}`.",
        f"- Inventory conclusion: `{artifacts['inventory_conclusion']}`.",
        "",
        "## Source seams re-audited",
    ]
    for seam in seams:
        status = "PASS" if seam["ok"] else "FAIL"
        lines.append(f"- {status} `{seam['id']}` — `{seam['file']}:{seam['lines']}`; function(s): `{seam['function']}`")
    lines += [
        "",
        "## Binding rule",
        "A debugger hit can be promoted only when the trace includes PSP/load segment, post-LZEXE transfer or map/decompressed-image evidence, symbol segment:offset, and observed hit CS:IP/context for each seam. Static compressed offsets and the MZ loader entry are explicitly rejected.",
        "",
        "## Artifacts",
        f"- Manifest: `{OUT_DIR / 'manifest.json'}`",
        f"- Trace contract: `{OUT_DIR / 'trace_binding_contract.json'}`",
        f"- Runtime trace template: `{OUT_DIR / 'runtime_trace_template.json'}`",
        f"- Guarded runbook: `{OUT_DIR / 'guarded_runtime_binding_runbook.md'}`",
        "",
        "## Non-claims",
    ]
    lines += [f"- {x}" for x in manifest["non_claims"]]
    REPORT.write_text("\n".join(lines) + "\n")

    print(json.dumps({"classification": classification, "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json")}, indent=2, sort_keys=True))
    return 0 if classification == "blocked/runtime-base-and-symbol-map-unavailable" else 1


if __name__ == "__main__":
    raise SystemExit(main())
