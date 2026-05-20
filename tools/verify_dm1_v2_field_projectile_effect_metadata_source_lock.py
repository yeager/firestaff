#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_field_projectile_effect_metadata_source_lock.json"


def redmcsb_source_root() -> Path:
    candidates: list[Path] = []
    if os.environ.get("FIRESTAFF_REDMCSB_SOURCE"):
        candidates.append(Path(os.environ["FIRESTAFF_REDMCSB_SOURCE"]).expanduser())
    candidates.append(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
    for candidate in candidates:
        if (candidate / "PROJEXPL.C").exists() and (candidate / "DUNVIEW.C").exists():
            return candidate
    raise SystemExit("error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE")


SOURCE = redmcsb_source_root()

LINE_ANCHORS = [
    ("DEFS.H", 417, "C14_THING_TYPE_PROJECTILE"),
    ("DEFS.H", 418, "C15_THING_TYPE_EXPLOSION"),
    ("DEFS.H", 421, "C0xFF80_THING_FIRST_EXPLOSION"),
    ("DEFS.H", 422, "C0xFF80_THING_EXPLOSION_FIREBALL"),
    ("DEFS.H", 424, "C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT"),
    ("DEFS.H", 427, "C0xFF86_THING_EXPLOSION_POISON_BOLT"),
    ("DEFS.H", 428, "C0xFF87_THING_EXPLOSION_POISON_CLOUD"),
    ("DEFS.H", 430, "C0xFFB2_THING_EXPLOSION_FLUXCAGE"),
    ("DEFS.H", 946, "C24_EVENT_REMOVE_FLUXCAGE"),
    ("DEFS.H", 947, "C25_EVENT_EXPLOSION"),
    ("PROJEXPL.C", 43, "void F0212_PROJECTILE_Create"),
    ("PROJEXPL.C", 79, "F0163_DUNGEON_LinkThingToList"),
    ("PROJEXPL.C", 82, "C49_EVENT_MOVE_PROJECTILE"),
    ("PROJEXPL.C", 84, "C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS"),
    ("PROJEXPL.C", 95, "void F0213_EXPLOSION_Create"),
    ("PROJEXPL.C", 149, "L0470_ps_Explosion->Type"),
    ("PROJEXPL.C", 150, "L0470_ps_Explosion->Attack"),
    ("PROJEXPL.C", 160, "C25_EVENT_EXPLOSION"),
    ("PROJEXPL.C", 165, "F0238_TIMELINE_AddEvent_GetEventIndex_CPSE"),
    ("PROJEXPL.C", 585, "F0213_EXPLOSION_Create"),
    ("PROJEXPL.C", 817, "C0xFF87_THING_EXPLOSION_POISON_CLOUD"),
    ("PROJEXPL.C", 823, "switch"),
    ("PROJEXPL.C", 859, "C0xFF87_THING_EXPLOSION_POISON_CLOUD"),
    ("PROJEXPL.C", 987, "C050_EXPLOSION_FLUXCAGE"),
    ("PROJEXPL.C", 989, "C24_EVENT_REMOVE_FLUXCAGE"),
    ("PROJEXPL.C", 994, "F0238_TIMELINE_AddEvent_GetEventIndex_CPSE"),
    ("DUNVIEW.C", 4382, "void F0113_DUNGEONVIEW_DrawField"),
    ("DUNVIEW.C", 6816, "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
    ("DUNVIEW.C", 6825, "F0113_DUNGEONVIEW_DrawField"),
    ("DUNVIEW.C", 6831, "F0113_DUNGEONVIEW_DrawField"),
]

FIRESTAFF_REQUIRED = [
    ("include/dm1_v2_field_projectile_effect_metadata_pc34.h", "DM1_V2_FieldProjectileEffectMetadata"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "presentation families"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "PROJEXPL.C:43-92"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "PROJEXPL.C:95-165"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "PROJEXPL.C:817-864"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "PROJEXPL.C:987-994"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "DUNVIEW.C:6816-6831"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "mutatesGameplayState"),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "static const DM1_V2_FieldProjectileEffectMetadata"),
    ("tests/test_dm1_v2_field_projectile_effect_metadata_pc34.c", "check_fluxcage_field_route"),
    ("tests/test_dm1_v2_field_projectile_effect_metadata_pc34.c", "mutatesGameplayState == 0"),
    ("CMakeLists.txt", "dm1_v2_field_projectile_effect_metadata_pc34"),
    ("CMakeLists.txt", "dm1_v2_field_projectile_effect_metadata_source_lock"),
]

FORBIDDEN_FIRESTAFF = [
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "F0212_PROJECTILE_Create("),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "F0213_EXPLOSION_Create("),
    ("src/dm1v2/dm1_v2_field_projectile_effect_metadata_pc34.c", "F0238_TIMELINE_AddEvent_GetEventIndex_CPSE("),
]


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def main() -> int:
    errors: list[str] = []
    anchors: list[dict[str, object]] = []
    source_cache: dict[str, str] = {}

    for filename, line, needle in LINE_ANCHORS:
        text = source_cache.setdefault(filename, read(SOURCE / filename))
        lines = text.splitlines()
        if not (1 <= line <= len(lines)):
            errors.append(f"{filename}:{line}: line out of range")
            actual = ""
        else:
            actual = lines[line - 1].strip()
            if needle not in actual:
                errors.append(f"{filename}:{line}: expected {needle!r}, got {actual!r}")
        anchors.append({"file": filename, "line": line, "needle": needle, "text": actual})

    for rel, needle in FIRESTAFF_REQUIRED:
        text = read(ROOT / rel)
        if needle not in text:
            errors.append(f"missing Firestaff anchor {needle!r} in {rel}")

    for rel, forbidden in FORBIDDEN_FIRESTAFF:
        text = read(ROOT / rel)
        if forbidden in text:
            errors.append(f"metadata module must not call source gameplay route {forbidden!r} in {rel}")

    result = {
        "status": "failed" if errors else "passed",
        "scope": "DM1 V2 field/projectile effect presentation metadata source-lock",
        "redmcsbSourceRoot": str(SOURCE),
        "sourceAnchors": anchors,
        "firestaffBoundary": {
            "presentationOnly": True,
            "gameplayStateOwner": "DM1 V1 source-shaped projectile/explosion/field event routes",
            "v2MetadataMutatesGameplayState": False,
        },
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        for error in errors:
            print("error:", error)
        return 1
    print(f"dm1_v2_field_projectile_effect_metadata_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
