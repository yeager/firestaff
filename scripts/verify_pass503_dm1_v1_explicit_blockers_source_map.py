#!/usr/bin/env python3
"""Pass 503 DM1 V1 explicit blockers source map.

Source/evidence gate for the three Daniel-called blockers: door-with-buttons,
title animation, and end animation. This does not claim pixel/runtime parity; it
keeps local work anchored to audited ReDMCSB line ranges and existing Firestaff
seams before the next landable parity step.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parents[1]
SOURCE = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = REPO / "parity-evidence/verification/pass503_dm1_v1_explicit_blockers_source_map"
MANIFEST = OUT_DIR / "manifest.json"
EVIDENCE = REPO / "parity-evidence/pass503_dm1_v1_explicit_blockers_source_map.md"

CHECKS: list[dict[str, Any]] = [
    {
        "id": "title-animation-pc-f20-cadence",
        "blocker": "title animation",
        "file": "TITLE.C",
        "function": "F0437_STARTEND_DrawTitle",
        "lines": "309-410",
        "ordered": True,
        "needles": [
            "F0490_MEMORY_LoadDecompressAndExpandGraphic(C001_GRAPHIC_TITLE",
            "M518_FillScreenBlack();",
            "F0132_VIDEO_Blit(L1384_puc_Bitmap_Title, G0348_Bitmap_Screen, G0005_ai_Graphic562_Box_Title_Presents",
            "L1381_ui_DestinationHeight = 80;",
            "for (L1380_i_Counter = 0; L1380_i_Counter < 18; L1380_i_Counter++)",
            "F0129_VIDEO_BlitShrinkWithPaletteChanges",
            "for (L1380_i_Counter = L3496_i_ActualBitmapCount; L1380_i_Counter >= 0; L1380_i_Counter--)",
            "M526_WaitVerticalBlank();",
            "F0766_BlitToScreen",
            "F0132_VIDEO_Blit(L1389_puc_Bitmap_Master_StrikesBack, G0348_Bitmap_Screen",
            "M526_WaitVerticalBlank();",
        ],
        "local": [
            ("title_frontend_v1.c", "V1_TitleFrontend_GetSourceAnimationStepCount"),
            ("title_frontend_v1.c", "return 23u;"),
            ("title_frontend_v1.c", "V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT"),
            ("title_frontend_v1.c", "postZoomVblankCount = 2u"),
        ],
    },
    {
        "id": "entrance-door-buttons-source-flow",
        "blocker": "door with buttons",
        "file": "ENTRANCE.C",
        "function": "F0441_STARTEND_ProcessEntrance / F0438_STARTEND_OpenEntranceDoors",
        "lines": "739-944",
        "ordered": True,
        "needles": [
            "G0441_ps_PrimaryMouseInput = G0445_as_Graphic561_PrimaryMouseInput_Entrance;",
            "G0443_ps_PrimaryKeyboardInput = NULL;",
            "G0562_apuc_Bitmap_EntranceDoorAnimationSteps[L1402_ui_AnimationStep]",
            "F0490_MEMORY_LoadDecompressAndExpandGraphic(C003_GRAPHIC_ENTRANCE_RIGHT_DOOR",
            "F0490_MEMORY_LoadDecompressAndExpandGraphic(C002_GRAPHIC_ENTRANCE_LEFT_DOOR",
            "F0439_STARTEND_DrawEntrance();",
            "M526_WaitVerticalBlank();",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "F0060_SOUND_Play_CPSX(G0566_puc_Graphic534_Sound01Switch",
            "F0022_MAIN_Delay(20);",
            "M522_MOUSE_HidePointer();",
            "F0438_STARTEND_OpenEntranceDoors();",
        ],
        "local": [
            ("entrance_frontend_pc34_compat.c", "ENTRANCE_Compat_GetDoorAnimationStepCount"),
            ("entrance_frontend_pc34_compat.c", "return 31u;"),
            ("entrance_frontend_pc34_compat.c", "ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP"),
            ("parity-evidence/pass486_dm1_v1_door_button_occlusion_source_lock.md", "PASS486_DM1_V1_DOOR_BUTTON_OCCLUSION_SOURCE_LOCKED"),
        ],
    },
    {
        "id": "entrance-door-open-31-step-vblank-loop",
        "blocker": "door with buttons",
        "file": "ENTRANCE.C",
        "function": "F0438_STARTEND_OpenEntranceDoors",
        "lines": "142-304",
        "ordered": True,
        "needles": [
            "L1393_ui_AnimationStep = 1;",
            "if ((L1393_ui_AnimationStep % 3) == 1)",
            "F0616_CopyBitmap",
            "F0132_VIDEO_Blit(G0296_puc_Bitmap_Viewport",
            "M705_ZONE_RIGHT(G0007_ai_Graphic562_Box_Entrance_OpeningDoorLeft) -= 4;",
            "M768_BOX_LEFT(G0008_ai_Graphic562_Box_Entrance_OpeningDoorRight) += 4;",
            "M526_WaitVerticalBlank(); /* BUG0_71",
            "F0132_VIDEO_Blit(L1394_ppuc_Bitmap_EntranceDoorAnimationSteps[9], G0348_Bitmap_Screen",
            "} while (++L1393_ui_AnimationStep != 32);",
        ],
        "local": [("entrance_frontend_pc34_compat.c", "source animation steps 1..31")],
    },
    {
        "id": "wall-click-sensor-button-semantics",
        "blocker": "door with buttons",
        "file": "MOVESENS.C",
        "function": "F0275_SENSOR_IsTriggeredByClickOnWall",
        "lines": "1309-1550",
        "ordered": True,
        "needles": [
            "BOOLEAN F0275_SENSOR_IsTriggeredByClickOnWall(",
            "L0761_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing;",
            "case C001_SENSOR_WALL_ORNAMENT_CLICK:",
            "case C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT:",
            "case C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT:",
            "case C013_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE_SENSORS:",
            "case C016_SENSOR_WALL_OBJECT_EXCHANGER:",
            "F0064_SOUND_RequestPlay_CPSD(C01_SOUND_SWITCH",
            "F0272_SENSOR_TriggerEffect",
            "F0271_SENSOR_ProcessRotationEffect();",
        ],
        "local": [("parity-evidence/pass486_dm1_v1_door_button_occlusion_source_lock.md", "door-button occlusion")],
    },
    {
        "id": "end-animation-fuse-sequence-cadence",
        "blocker": "end animation",
        "file": "ENDGAME.C",
        "function": "F0445_STARTEND_FuseSequenceUpdate / F0446_STARTEND_FuseSequence",
        "lines": "742-923",
        "ordered": True,
        "needles": [
            "STATICFUNCTION void F0445_STARTEND_FuseSequenceUpdate(",
            "F0261_TIMELINE_Process_CPSEF();",
            "F0128_DUNGEONVIEW_Draw_CPSF",
            "G0313_ul_GameTime++; /* BUG0_71",
            "void F0446_STARTEND_FuseSequence(",
            "G0302_B_GameWon = C1_TRUE;",
            "F0445_STARTEND_FuseSequenceUpdate();",
            "L1428_ps_Group->Health[0] = 10000;",
            "F0213_EXPLOSION_Create(C0xFF80_THING_EXPLOSION_FIREBALL",
            "L1428_ps_Group->Type = C25_CREATURE_LORD_ORDER;",
            "F0213_EXPLOSION_Create(C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL",
            "L1428_ps_Group->Type = C26_CREATURE_GREY_LORD;",
            "G0077_B_DoNotDrawFluxcagesDuringEndgame = C1_TRUE;",
        ],
        "local": [
            ("test_dm1_v1_endgame_system_pc34_compat.c", "test_fuse_sequence_full"),
            ("test_dm1_v1_endgame_system_pc34_compat.c", "DM1_Endgame_FuseSequence_Step"),
            ("dm1_v1_endgame_system_pc34_compat.h", "DM1_Endgame_FuseSequence_Step"),
        ],
    },
    {
        "id": "vblank-wait-contract-for-frontend-cadence",
        "blocker": "title/end animation timing",
        "file": "VBLANK.C",
        "function": "F0577_VerticalBlank_Handler_CPSDF / F0693_WaitVerticalBlank",
        "lines": "35-64,626-646",
        "needles": [
            "G0317_i_WaitForInputVerticalBlankCount++;",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "void F0693_WaitVerticalBlank(",
            "BNE        L0901B9",
        ],
        "local": [
            ("dm1_v1_vblank_timing.c", "DM1_V1_VBlank"),
            ("title_frontend_v1.c", "GetRuntimeFrameDelayMs"),
            ("entrance_frontend_pc34_compat.c", "vblankLoopCount"),
        ],
    },
]


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(errors="replace")


def slice_lines(text: str, spec: str) -> str:
    parts = []
    lines = text.splitlines()
    for span in spec.split(","):
        start_s, end_s = span.split("-", 1)
        start, end = int(start_s), int(end_s)
        parts.append("\n".join(lines[start - 1 : end]))
    return "\n".join(parts)


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode()).hexdigest()


def check_order(excerpt: str, needles: list[str], ordered: bool) -> tuple[list[str], list[str]]:
    missing = [n for n in needles if n not in excerpt]
    out_of_order: list[str] = []
    if ordered:
        pos = -1
        for n in needles:
            found = excerpt.find(n, pos + 1)
            if found >= 0:
                pos = found
            elif n not in missing:
                out_of_order.append(n)
    return missing, out_of_order


def main() -> None:
    failures: list[str] = []
    audited: list[dict[str, Any]] = []
    for check in CHECKS:
        src = read(SOURCE / check["file"])
        excerpt = slice_lines(src, check["lines"])
        missing, out = check_order(excerpt, check["needles"], bool(check.get("ordered")))
        local_missing: list[str] = []
        for rel, needle in check.get("local", []):
            if needle not in read(REPO / rel):
                local_missing.append(f"{rel}: {needle}")
        status = "passed" if not missing and not out and not local_missing else "failed"
        if status != "passed":
            failures.append("{} missing={!r} out_of_order={!r} local_missing={!r}".format(check["id"], missing, out, local_missing))
        audited.append({
            "id": check["id"],
            "blocker": check["blocker"],
            "status": status,
            "source": {
                "file": check["file"],
                "function": check["function"],
                "lines": check["lines"],
                "sliceSha256": sha256_text(excerpt),
            },
            "localEvidenceCount": len(check.get("local", [])),
        })
    result = {
        "status": "PASS503_DM1_V1_EXPLICIT_BLOCKERS_SOURCE_MAP_VERIFIED" if not failures else "FAILED",
        "sourceRoot": str(SOURCE),
        "repo": str(REPO),
        "checks": audited,
        "blockersCovered": sorted({c["blocker"] for c in CHECKS}),
        "guardrail": "source/evidence lock only; no pixel/runtime parity claim",
        "failures": failures,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    evidence_lines = [
        "# Pass503 — DM1 V1 explicit blockers source map",
        "",
        "Status: `{}`".format(result["status"]),
        "",
        "Covered Daniel blockers: door with buttons, title animation, end animation.",
        "",
        "Audited ReDMCSB anchors:",
    ]
    for c in audited:
        evidence_lines.append("- `{}:{}` — {} — `{}`".format(c["source"]["file"], c["source"]["lines"], c["id"], c["status"]))
    evidence_lines += [
        "",
        "Firestaff local seams checked: title frontend cadence, entrance door-step model, pass486 door-button occlusion evidence, endgame fuse-sequence gate, and VBlank timing seam.",
        "",
        "Guardrail: this is source/evidence only; it does not claim original pixel/runtime parity.",
    ]
    EVIDENCE.write_text("\n".join(evidence_lines) + "\n")
    print(json.dumps(result, indent=2, sort_keys=True))
    if failures:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
