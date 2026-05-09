#!/usr/bin/env python3
"""Pass449 DM1 V1 Hall candidate framebuffer evidence gate.

This is a deterministic evidence-path/blocker gate for the Hall of Champions
candidate panel.  It source-locks the panel graphic/palette/transparency,
candidate append/cancel/confirm, and HUD/status suppression/redraw behavior
against ReDMCSB, hash-locks the local DM1 PC34 data files, and records the
exact artifact contract needed before anyone can claim framebuffer/pixel parity.

It intentionally exits PASS with a BLOCKED_* status while original true-stop /
semantically labelled original candidate frames are missing.  Screenshots that
exist today are retained as review clues only, never as pixel parity evidence.
"""
from __future__ import annotations

import hashlib
import json
import os
import struct
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate"
STATUS = "BLOCKED_PASS449_HALL_CANDIDATE_FRAMEBUFFER_ORIGINAL_ARTIFACTS_MISSING"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
CANON_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"

LOCKED_DATA = [
    {
        "label": "dm1_pc34_english_graphics",
        "path": CANON_DM1 / "GRAPHICS.DAT",
        "filename": "GRAPHICS.DAT",
        "variant": "DM PC 3.4 English / I34E",
        "bytes": 363417,
        "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "md5": "fa6b1aa29e191418713bf2cda93d962e",
    },
    {
        "label": "dm1_pc34_english_dungeon",
        "path": CANON_DM1 / "DUNGEON.DAT",
        "filename": "DUNGEON.DAT",
        "variant": "DM PC 3.4 English / I34E",
        "bytes": 33357,
        "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "md5": "766450c940651fc021c92fe5d0d0b3a6",
    },
]

SOURCE_LOCKS = [
    {
        "id": "panel_graphic_index_and_dark_green_transparency",
        "file": "PANEL.C",
        "lines": "1619-1636",
        "needles": [
            "F0346_INVENTORY_DrawPanel_ResurrectReincarnate",
            "G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE",
            "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE",
            "C06_COLOR_DARK_GREEN",
            "F0658_BlitBitmapIndexToZoneIndexWithTransparency(C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE, C101_ZONE_PANEL, C06_COLOR_DARK_GREEN)",
        ],
        "claim": "Candidate panel uses graphic 40 and transparent palette index 6; PC34 route blits it into C101_ZONE_PANEL.",
    },
    {
        "id": "panel_graphic_constant",
        "file": "DEFS.H",
        "lines": "2194-2201",
        "needles": ["#define C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE       40"],
        "claim": "Resurrect/reincarnate panel graphic identity is the numbered C040 asset, not filename-only evidence.",
    },
    {
        "id": "transparent_color_constant",
        "file": "DEFS.H",
        "lines": "2078-2086",
        "needles": ["#define C06_COLOR_DARK_GREEN       6"],
        "claim": "Transparent color is palette index 6 (dark green).",
    },
    {
        "id": "pc34_panel_zone",
        "file": "DEFS.H",
        "lines": "3774-3777",
        "needles": ["#define C101_ZONE_PANEL"],
        "claim": "The PC34 panel target is zone C101.",
    },
    {
        "id": "old_pc_panel_box_coordinates",
        "file": "DATA.C",
        "lines": "314-319",
        "needles": ["G0032_ai_Graphic562_Box_Panel[4] = { 80, 223, 52, 124 }"],
        "claim": "The old PC panel bitmap box is viewport-relative x=80..223 y=52..124; when lifted to full screen this is x=80..223 y=85..157 because viewport y origin is 33.",
    },
    {
        "id": "candidate_forces_resurrect_panel",
        "file": "PANEL.C",
        "lines": "1654-1656",
        "needles": ["if (G0299_ui_CandidateChampionOrdinal)", "F0346_INVENTORY_DrawPanel_ResurrectReincarnate", "return"],
        "claim": "Any candidate inventory redraw must show the Hall decision panel.",
    },
    {
        "id": "candidate_hides_save_rest_close",
        "file": "PANEL.C",
        "lines": "2376-2385",
        "needles": ["F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY, G0296_puc_Bitmap_Viewport)", "if (G0299_ui_CandidateChampionOrdinal)", "C562_ZONE_SAVE_GAME_ICON", "C564_ZONE_REST_ICON", "C566_ZONE_CLOSE_INVENTORY_ICON"],
        "claim": "Candidate mode redraws inventory/panel but suppresses save/rest/close affordances.",
    },
    {
        "id": "candidate_append_slot_and_leader_hud",
        "file": "REVIVE.C",
        "lines": "272-294",
        "needles": ["G0299_ui_CandidateChampionOrdinal = L0799_ui_PreviousPartyChampionCount + 1", "if (++G0305_ui_PartyChampionCount == 1)", "F0368_COMMAND_SetLeader(C00_CHAMPION_FIRST)", "F0388_MENUS_ClearActingChampion", "F0386_MENUS_DrawActionIcon"],
        "claim": "Portrait click appends a temporary candidate champion and updates leader/action UI according to party count.",
    },
    {
        "id": "candidate_cancel_hud_clear_and_redraw",
        "file": "REVIVE.C",
        "lines": "744-783",
        "needles": ["P0598_i_Command == C162_COMMAND_CLICK_IN_PANEL_CANCEL", "F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)", "G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)", "G0305_ui_PartyChampionCount--", "F0733_FillZoneByIndex", "F0457_START_DrawEnabledMenus_CPSF"],
        "claim": "Cancel closes inventory, clears candidate ordinal, removes the candidate status/HUD slot, and redraws menus.",
    },
    {
        "id": "candidate_confirm_sensor_disable",
        "file": "REVIVE.C",
        "lines": "785-807",
        "needles": ["G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)", "F0164_DUNGEON_UnlinkThingFromList", "M044_SET_TYPE_DISABLED", "P0598_i_Command == C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "F0281_CHAMPION_Rename"],
        "claim": "Resurrect/reincarnate confirmation clears candidate mode, unlinks possessions, disables the mirror sensor, and branches to reincarnation rename.",
    },
    {
        "id": "panel_click_dispatch_requires_empty_hand",
        "file": "COMMAND.C",
        "lines": "1985-1991",
        "needles": ["case M568_PANEL_RESURRECT_REINCARNATE", "if (!G0415_ui_LeaderEmptyHanded)", "G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"],
        "claim": "Panel choice commands dispatch only from the decision panel and only while the leader hand is empty.",
    },
    {
        "id": "panel_choice_command_constants",
        "file": "COMMAND.C",
        "lines": "228-240",
        "needles": ["G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel", "C160_COMMAND_CLICK_IN_PANEL_RESURRECT", "C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "C162_COMMAND_CLICK_IN_PANEL_CANCEL"],
        "claim": "The Hall decision set is exactly resurrect/reincarnate/cancel.",
    },
    {
        "id": "candidate_blocks_status_inventory_close",
        "file": "COMMAND.C",
        "lines": "2159-2184",
        "needles": ["C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX", "!G0299_ui_CandidateChampionOrdinal", "C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0", "C04_CHAMPION_CLOSE_INVENTORY", "!G0299_ui_CandidateChampionOrdinal"],
        "claim": "Status box/inventory toggles and inventory close are blocked while the candidate panel is active.",
    },
    {
        "id": "candidate_blocks_rest_wake_save",
        "file": "COMMAND.C",
        "lines": "2336-2370",
        "needles": ["if (!G0299_ui_CandidateChampionOrdinal)", "G0300_B_PartyIsResting = C1_TRUE", "C146_COMMAND_WAKE_UP", "G0305_ui_PartyChampionCount > 0", "!G0299_ui_CandidateChampionOrdinal"],
        "claim": "Rest/wake/save menu paths cannot escape the Hall candidate modal.",
    },
    {
        "id": "candidate_slot_draw_suppression",
        "file": "CHAMDRAW.C",
        "lines": "536-545",
        "needles": ["If drawing a slot for a champion other than the champion whose inventory is open", "G0299_ui_CandidateChampionOrdinal == M000_INDEX_TO_ORDINAL(P0613_ui_ChampionIndex)", "return"],
        "claim": "Slot drawing is candidate-aware and prevents unrelated champion slots from overwriting candidate UI.",
    },
    {
        "id": "candidate_changed_leader_hand_suppression",
        "file": "CHAMDRAW.C",
        "lines": "1210-1212",
        "needles": ["L0883_ui_InventoryChampionOrdinal = G0423_i_InventoryChampionOrdinal", "if (G0299_ui_CandidateChampionOrdinal && !L0883_ui_InventoryChampionOrdinal)", "return"],
        "claim": "Changed leader-hand object drawing is suppressed if candidate mode has no inventory champion.",
    },
    {
        "id": "asset_preload_on_hall_map",
        "file": "DUNVIEW.C",
        "lines": "2463-2467",
        "needles": ["These graphics are loaded only if there are no creature types allowed on the map", "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE", "C026_GRAPHIC_CHAMPION_PORTRAITS", "C027_GRAPHIC_PANEL_RENAME_CHAMPION"],
        "claim": "Hall/prison map loads panel and portrait graphics together when map creature types allow it.",
    },
    {
        "id": "blit_function_preserves_transparent_color",
        "file": "BASE.C",
        "lines": "1341-1369",
        "needles": ["void F0658_BlitBitmapIndexToZoneIndexWithTransparency", "P2257_i_TransparentColor", "F0132_VIDEO_Blit", "P2257_i_TransparentColor"],
        "claim": "Zone blit passes the requested transparent palette index through to the video blitter.",
    },
    {
        "id": "graphics_expand_native_bitmap_path",
        "file": "MEMORY.C",
        "lines": "2474-2525",
        "needles": ["void F0488_MEMORY_ExpandGraphicToBitmap", "F0466_EXPAND_GraphicToBitmap", "unsigned char* F0489_MEMORY_GetNativeBitmapOrGraphic", "G0636_ppuc_Graphics"],
        "claim": "Panel graphic bytes enter the bitmap path through the original memory/graphic expansion routines.",
    },
]

REQUIRED_SCENES = [
    "candidate_select",
    "panel_visible",
    "cancel",
    "resurrect_confirm",
    "reincarnate_confirm",
    "hud_status_after",
]

FRAMEBUFFER_INPUT_DIR = VERIFY_DIR / "framebuffer_inputs"
FRAMEBUFFER_MANIFEST = FRAMEBUFFER_INPUT_DIR / "hall_candidate_framebuffer_manifest.json"
FRAMEBUFFER_SCHEMA_PATH = VERIFY_DIR / "hall_candidate_framebuffer_manifest_schema.json"
COMPARATOR_RESULT = VERIFY_DIR / "hall_candidate_framebuffer_compare.json"

REGIONS = {
    "fullframe": {"xywh": [0, 0, 320, 200], "requiredFor": REQUIRED_SCENES},
    "panel_crop": {"xywh": [80, 85, 144, 73], "requiredFor": ["panel_visible", "cancel", "resurrect_confirm", "reincarnate_confirm"]},
    "hud_status_crop": {"xywh": [0, 0, 320, 33], "requiredFor": ["candidate_select", "cancel", "resurrect_confirm", "reincarnate_confirm", "hud_status_after"]},
}

SCENE_CONTRACT = {
    "candidate_select": "after source-bound C127 portrait click/F0280 append, before any C160/C161/C162 panel command",
    "panel_visible": "first stable frame with G0299 candidate mode forcing C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE into C101",
    "cancel": "stable frame after C162 cancel closes inventory, clears G0299, decrements party count, and redraws menus",
    "resurrect_confirm": "stable frame after C160 clears candidate mode, keeps appended champion, unlinks possessions, and disables mirror sensor",
    "reincarnate_confirm": "stable frame after C161 confirm plus reincarnation rename/stat/vital reset branch enters its original-compatible stop",
    "hud_status_after": "top HUD/status stable frame after each terminal Hall action, labelled with terminalAction=cancel|resurrect_confirm|reincarnate_confirm",
}

KNOWN_REVIEW_ARTIFACTS = [
    "parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/summary.json",
    "parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0011-after_portrait_click.png",
    "parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect/image0012-after_c160_resurrect.png",
    "parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/summary.json",
    "parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0011-after_portrait_click.png",
    "parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate/image0012-after_c161_reincarnate.png",
    "parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json",
]


SOURCE_FUNCTIONS = {
    "panel_graphic_index_and_dark_green_transparency": "F0346_INVENTORY_DrawPanel_ResurrectReincarnate",
    "panel_graphic_constant": "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE",
    "transparent_color_constant": "C06_COLOR_DARK_GREEN",
    "pc34_panel_zone": "C101_ZONE_PANEL",
    "old_pc_panel_box_coordinates": "G0032_ai_Graphic562_Box_Panel",
    "candidate_forces_resurrect_panel": "F0347_INVENTORY_DrawPanel",
    "candidate_hides_save_rest_close": "F0355_INVENTORY_Toggle_CPSE",
    "candidate_append_slot_and_leader_hud": "F0280_CHAMPION_AddCandidateChampionToParty",
    "candidate_cancel_hud_clear_and_redraw": "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel",
    "candidate_confirm_sensor_disable": "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel",
    "panel_click_dispatch_requires_empty_hand": "F0445_COMMAND_ProcessCommands160To162_ClickInPanel",
    "panel_choice_command_constants": "G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel",
    "candidate_blocks_status_inventory_close": "F0446_COMMAND_ProcessCommands12To27_ClickInChampionStatusBox / F0448_COMMAND_ProcessCommands7To11_ToggleInventory",
    "candidate_blocks_rest_wake_save": "F0449_COMMAND_ProcessCommands100To149_ClickInMenu",
    "candidate_slot_draw_suppression": "F0323_CHAMPION_DrawSlot",
    "candidate_changed_leader_hand_suppression": "F0333_CHAMPION_DrawChangedObjectIcons",
    "asset_preload_on_hall_map": "F0136_DUNGEONVIEW_LoadViewportGraphics",
    "blit_function_preserves_transparent_color": "F0658_BlitBitmapIndexToZoneIndexWithTransparency",
    "graphics_expand_native_bitmap_path": "F0488_MEMORY_ExpandGraphicToBitmap / F0489_MEMORY_GetNativeBitmapOrGraphic",
}

PRIOR_BLOCKERS = [
    {
        "label": "original_true_stop",
        "path": "parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json",
        "expected_status": "BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED",
        "why": "Original FIRES must be stopped/logged at F0128 -> F0097/VIDRV before screenshots are promotable.",
    },
    {
        "label": "paired_diff_artifacts",
        "path": "parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json",
        "expected_status": "BLOCKED_PASS377_PAIRED_DIFF_REVIEW_METADATA_READY_SEMANTIC_ORIGINAL_BLOCKED",
        "why": "Paired Firestaff/original review metadata exists for movement, but original semantic promotion is still blocked.",
    },
]


def norm(text: str) -> str:
    return " ".join(text.split())


def sha(path: Path, algo: str = "sha256") -> str:
    h = hashlib.new(algo)
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def line_block(path: Path, spec: str) -> str:
    data = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start_s, end_s = part.split("-", 1)
            start, end = int(start_s), int(end_s)
        else:
            start = end = int(part)
        chunks.append("\n".join(data[start - 1:end]))
    return "\n".join(chunks)


def png_dims(path: Path) -> list[int] | None:
    try:
        header = path.read_bytes()[:24]
    except FileNotFoundError:
        return None
    if len(header) >= 24 and header[:8] == b"\x89PNG\r\n\x1a\n" and header[12:16] == b"IHDR":
        return list(struct.unpack(">II", header[16:24]))
    return None


def rel(path: Path) -> str:
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def expected_frame_path(side: str, scene: str, artifact: str) -> Path:
    return FRAMEBUFFER_INPUT_DIR / side / scene / f"{artifact}.png"


def build_framebuffer_manifest_schema() -> dict[str, Any]:
    provenance_schema = {
        "required": ["variant", "file", "sha256", "bytes"],
        "properties": {
            "variant": "exact DM data variant, e.g. DM PC 3.4 English / I34E",
            "file": "canonical file path or artifact path; filename alone is insufficient",
            "sha256": "64 hex chars; must match the locked original data row",
            "bytes": "integer byte size; must match the locked original data row",
        },
    }
    frame_schema = {
        "required": ["path", "sha256", "bytes", "width", "height", "captureProvenance"],
        "properties": {
            "path": "repo-relative PNG path",
            "sha256": "64 hex chars of PNG artifact",
            "bytes": "integer byte size",
            "width": "320 for fullframe, crop width for crops",
            "height": "200 for fullframe, crop height for crops",
            "captureProvenance": {
                "required": ["stop", "route", "partyTuple", "commandTranscriptSha256"],
                "properties": {
                    "stop": "source-bound stop label/function, e.g. F0128->F0097/VIDRV for original",
                    "route": "semantic command route, not screenshot filename",
                    "partyTuple": "map/x/y/dir/championCount/candidateOrdinal/terminalAction as applicable",
                    "commandTranscriptSha256": "hash of exact command/input transcript",
                },
            },
        },
    }
    return {
        "schema": "pass449_hall_candidate_framebuffer_manifest.schema.v1",
        "manifestPath": rel(FRAMEBUFFER_MANIFEST),
        "requiredOriginalDataProvenance": {
            "GRAPHICS.DAT": provenance_schema,
            "DUNGEON.DAT": provenance_schema,
        },
        "requiredScenes": {scene: SCENE_CONTRACT[scene] for scene in REQUIRED_SCENES},
        "requiredRegions": REGIONS,
        "sceneEntrySchema": {
            "required": ["scene", "semanticStop", "firestaff", "original"],
            "properties": {
                "scene": REQUIRED_SCENES,
                "semanticStop": "one of the source-bound descriptions in requiredScenes",
                "firestaff": {"fullframe": frame_schema, "panel_crop": frame_schema, "hud_status_crop": frame_schema},
                "original": {"fullframe": frame_schema, "panel_crop": frame_schema, "hud_status_crop": frame_schema},
            },
        },
        "comparisonOutputSchema": {
            "perScenePerRegion": ["region", "xywh", "firestaffSha256", "originalSha256", "differingPixels", "totalPixels", "maxChannelDelta", "meanAbsDeltaRgb"],
            "passRule": "all required artifacts exist, hashes/dimensions/provenance validate, and region comparisons are recorded; zero deltas may be claimed only from this JSON plus matching provenance",
        },
    }


def expected_framebuffer_artifacts() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for scene in REQUIRED_SCENES:
        for side in ("firestaff", "original"):
            for artifact, spec in REGIONS.items():
                if scene not in spec["requiredFor"]:
                    continue
                path = expected_frame_path(side, scene, artifact)
                dims = png_dims(path) if path.exists() else None
                row: dict[str, Any] = {
                    "scene": scene,
                    "semanticStop": SCENE_CONTRACT[scene],
                    "side": side,
                    "artifact": artifact,
                    "path": rel(path),
                    "requiredSha256Field": f"scenes.{scene}.{side}.{artifact}.sha256",
                    "expectedXywh": spec["xywh"],
                    "expectedDimensions": spec["xywh"][2:4],
                    "exists": path.exists(),
                }
                if path.exists():
                    row.update({"bytes": path.stat().st_size, "sha256": sha(path), "pngDims": dims})
                    if dims != spec["xywh"][2:4]:
                        row["dimensionError"] = f"{dims} != {spec['xywh'][2:4]}"
                else:
                    row["missingBlocker"] = f"missing {side} {scene} {artifact}; expected {rel(path)} and manifest hash field {row['requiredSha256Field']}"
                rows.append(row)
    return rows


def validate_manifest_hash_entry(manifest: dict[str, Any], scene: str, side: str, artifact: str, path: Path) -> list[str]:
    errors: list[str] = []
    try:
        entry = manifest["scenes"][scene][side][artifact]
    except Exception:
        return [f"manifest missing scenes.{scene}.{side}.{artifact}"]
    required_path = rel(path)
    if entry.get("path") != required_path:
        errors.append(f"manifest path mismatch scenes.{scene}.{side}.{artifact}.path: {entry.get('path')} != {required_path}")
    if path.exists():
        actual_sha = sha(path)
        actual_bytes = path.stat().st_size
        dims = png_dims(path)
        if entry.get("sha256") != actual_sha:
            errors.append(f"manifest sha mismatch scenes.{scene}.{side}.{artifact}: {entry.get('sha256')} != {actual_sha}")
        if entry.get("bytes") != actual_bytes:
            errors.append(f"manifest bytes mismatch scenes.{scene}.{side}.{artifact}: {entry.get('bytes')} != {actual_bytes}")
        if entry.get("width") != dims[0] or entry.get("height") != dims[1]:
            errors.append(f"manifest dims mismatch scenes.{scene}.{side}.{artifact}: {entry.get('width')}x{entry.get('height')} != {dims}")
    for key in ("path", "sha256", "bytes", "width", "height", "captureProvenance"):
        if key not in entry:
            errors.append(f"manifest missing scenes.{scene}.{side}.{artifact}.{key}")
    return errors


def compare_png_pair(firestaff: Path, original: Path, expected_dims: list[int]) -> dict[str, Any]:
    from PIL import Image

    fs = Image.open(firestaff).convert("RGB")
    og = Image.open(original).convert("RGB")
    expected = tuple(expected_dims)
    if fs.size != expected or og.size != expected:
        raise ValueError(f"dimension mismatch for {rel(firestaff)} / {rel(original)}: {fs.size} vs {og.size}, expected {expected}")
    fsp = fs.load()
    ogp = og.load()
    width, height = expected
    differing = 0
    max_delta = 0
    total_abs = 0
    for y in range(height):
        for x in range(width):
            fr, fg, fb = fsp[x, y]
            or_, og_, ob = ogp[x, y]
            dr, dg, db = abs(fr - or_), abs(fg - og_), abs(fb - ob)
            max_delta = max(max_delta, dr, dg, db)
            total_abs += dr + dg + db
            if dr or dg or db:
                differing += 1
    total_pixels = width * height
    return {
        "firestaff": rel(firestaff),
        "firestaffSha256": sha(firestaff),
        "original": rel(original),
        "originalSha256": sha(original),
        "dimensions": [width, height],
        "differingPixels": differing,
        "totalPixels": total_pixels,
        "deltaPercent": round(100.0 * differing / total_pixels, 6),
        "maxChannelDelta": max_delta,
        "meanAbsDeltaRgb": round(total_abs / (total_pixels * 3), 6),
    }


def run_framebuffer_comparisons() -> tuple[list[dict[str, Any]], list[str]]:
    rows: list[dict[str, Any]] = []
    errors: list[str] = []
    for scene in REQUIRED_SCENES:
        for artifact, spec in REGIONS.items():
            if scene not in spec["requiredFor"]:
                continue
            try:
                row = compare_png_pair(
                    expected_frame_path("firestaff", scene, artifact),
                    expected_frame_path("original", scene, artifact),
                    spec["xywh"][2:4],
                )
                row.update({"scene": scene, "region": artifact, "xywh": spec["xywh"]})
                rows.append(row)
            except Exception as exc:
                errors.append(f"compare failed {scene}.{artifact}: {exc}")
    return rows, errors


def audit_framebuffer_manifest(data_rows: list[dict[str, Any]]) -> dict[str, Any]:
    schema = build_framebuffer_manifest_schema()
    FRAMEBUFFER_SCHEMA_PATH.parent.mkdir(parents=True, exist_ok=True)
    FRAMEBUFFER_SCHEMA_PATH.write_text(json.dumps(schema, indent=2) + "\n", encoding="utf-8")
    artifacts = expected_framebuffer_artifacts()
    missing = [row["missingBlocker"] for row in artifacts if row.get("missingBlocker")]
    errors: list[str] = []
    manifest_data: dict[str, Any] | None = None
    if not FRAMEBUFFER_MANIFEST.exists():
        missing.insert(0, f"missing framebuffer manifest: {rel(FRAMEBUFFER_MANIFEST)}")
    else:
        try:
            manifest_data = json.loads(FRAMEBUFFER_MANIFEST.read_text(encoding="utf-8"))
        except Exception as exc:
            errors.append(f"framebuffer manifest JSON parse failed: {exc}")
    if manifest_data:
        prov = manifest_data.get("originalDataProvenance", {})
        for data in data_rows:
            file_name = data["filename"]
            entry = prov.get(file_name)
            if not entry:
                errors.append(f"manifest missing originalDataProvenance.{file_name}")
                continue
            for key in ("variant", "sha256", "bytes"):
                if entry.get(key) != data.get(key):
                    errors.append(f"manifest originalDataProvenance.{file_name}.{key} mismatch: {entry.get(key)} != {data.get(key)}")
            if not entry.get("file") or Path(str(entry.get("file"))).name != file_name:
                errors.append(f"manifest originalDataProvenance.{file_name}.file must name exact source file/path, not be omitted")
        for scene in REQUIRED_SCENES:
            for side in ("firestaff", "original"):
                for artifact, spec in REGIONS.items():
                    if scene in spec["requiredFor"]:
                        errors += validate_manifest_hash_entry(manifest_data, scene, side, artifact, expected_frame_path(side, scene, artifact))
    comparisons: list[dict[str, Any]] = []
    compare_errors: list[str] = []
    if not missing and not errors:
        comparisons, compare_errors = run_framebuffer_comparisons()
        errors += compare_errors
    status = "COMPARE_COMPLETE" if not missing and not errors else "BLOCKED_FRAMEBUFFER_ARTIFACTS_OR_MANIFEST_MISSING"
    result = {
        "schema": "pass449_hall_candidate_framebuffer_comparator.v1",
        "status": status,
        "manifestPath": rel(FRAMEBUFFER_MANIFEST),
        "schemaPath": rel(FRAMEBUFFER_SCHEMA_PATH),
        "scenes": REQUIRED_SCENES,
        "regions": REGIONS,
        "expectedArtifacts": artifacts,
        "missing": missing,
        "errors": errors,
        "compareExecuted": bool(comparisons),
        "comparisons": comparisons,
        "passRule": "COMPARE_COMPLETE requires manifest provenance+hash validation and all Firestaff/original scene-region PNG comparisons recorded. Pixel parity for a region is differingPixels == 0 with matching original data provenance.",
    }
    COMPARATOR_RESULT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    return result


def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"cmd": cmd, "returncode": proc.returncode, "outputTail": proc.stdout[-4000:]}


def audit_sources() -> tuple[list[dict[str, Any]], list[str]]:
    rows: list[dict[str, Any]] = []
    errors: list[str] = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock["file"]
        text = line_block(path, lock["lines"]) if path.exists() else ""
        compact = norm(text)
        missing = [needle for needle in lock["needles"] if norm(needle) not in compact]
        ok = path.exists() and not missing
        if not ok:
            errors.append(f"source lock {lock['id']} failed: {missing or ['missing file']}")
        rows.append({
            **lock,
            "function": SOURCE_FUNCTIONS.get(lock["id"], ""),
            "path": str(path),
            "ok": ok,
            "missing": missing,
        })
    return rows, errors


def audit_data() -> tuple[list[dict[str, Any]], list[str]]:
    rows: list[dict[str, Any]] = []
    errors: list[str] = []
    for item in LOCKED_DATA:
        path: Path = item["path"]
        row = {k: (str(v) if isinstance(v, Path) else v) for k, v in item.items()}
        if not path.is_file():
            row.update({"exists": False, "ok": False})
            errors.append(f"missing locked data {item['label']}: {path}")
        else:
            actual_sha = sha(path, "sha256")
            actual_md5 = sha(path, "md5")
            actual_bytes = path.stat().st_size
            ok = actual_sha == item["sha256"] and actual_md5 == item["md5"] and actual_bytes == item["bytes"]
            row.update({"exists": True, "actualSha256": actual_sha, "actualMd5": actual_md5, "actualBytes": actual_bytes, "ok": ok})
            if not ok:
                errors.append(f"hash mismatch {item['label']}: sha256={actual_sha} md5={actual_md5} bytes={actual_bytes}")
        rows.append(row)
    return rows, errors


def audit_review_artifacts() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for rel in KNOWN_REVIEW_ARTIFACTS:
        path = ROOT / rel
        row: dict[str, Any] = {"path": rel, "exists": path.exists()}
        if path.exists():
            row["bytes"] = path.stat().st_size
            row["sha256"] = sha(path)
            if path.suffix.lower() == ".png":
                row["pngDims"] = png_dims(path)
            if path.name == "summary.json":
                try:
                    data = json.loads(path.read_text(encoding="utf-8"))
                    row["classification"] = data.get("classification")
                    row["reason"] = data.get("reason")
                except Exception as exc:  # pragma: no cover - defensive evidence inventory
                    row["jsonError"] = str(exc)
        return_kind = "review_only_not_promotable"
        row["parityUse"] = return_kind
        rows.append(row)
    return rows


def audit_priors() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for prior in PRIOR_BLOCKERS:
        path = ROOT / prior["path"]
        data: dict[str, Any] = {}
        if path.exists():
            try:
                data = json.loads(path.read_text(encoding="utf-8"))
            except Exception:
                data = {}
        rows.append({
            **prior,
            "exists": path.exists(),
            "status": data.get("status"),
            "ok": path.exists() and data.get("status") == prior["expected_status"],
            "activeBlocker": data.get("activeBlocker") or data.get("blocker") or data.get("blockers"),
        })
    return rows


def build_capture_contract() -> dict[str, Any]:
    return {
        "manifestPath": rel(FRAMEBUFFER_MANIFEST),
        "schemaPath": rel(FRAMEBUFFER_SCHEMA_PATH),
        "scenes": REQUIRED_SCENES,
        "sceneStops": SCENE_CONTRACT,
        "regions": REGIONS,
        "requiredPerScene": [
            "Firestaff fullframe 320x200 framebuffer with command script/hash/party tuple",
            "Firestaff panel crop x=80 y=85 w=144 h=73 for old-PC C101 panel bitmap content when the scene requires panel_crop",
            "Firestaff top HUD/status crop x=0 y=0 w=320 h=33 when the scene requires hud_status_crop",
            "Original PC34 fullframe 320x200 captured only after a source-bound true-stop at F0128 -> F0097/VIDRV, with the same command route and party tuple",
            "Original panel/HUD crops derived from that labelled fullframe, with SHA256 and dimensions recorded in the manifest",
            "Comparator output recording exact pixel counts for panel, status/HUD, and fullframe; zero/non-zero is not interpreted without source-bound matching state",
        ],
        "mustNotUseAsParity": [
            "filename-only DUNGEON.DAT/GRAPHICS.DAT identity",
            "DOSBox screenshots without original true-stop transcript",
            "pass173 static no-party frames after portrait/panel clicks",
            "Firestaff-only screenshots without original paired frames",
        ],
        "promotionRule": "Only promote after every required scene has hash-locked Firestaff and original frames/crops, original true-stop transcript, source-bound command route, and panel/HUD comparator JSON.",
    }


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        f"# {PASS}",
        "",
        f"- status: `{manifest['status']}`",
        f"- redmcsb: `{manifest['redmcsbRoot']}`",
        "- parity claim: **not made**; this is a source-locked evidence path and blocker gate.",
        "",
        "## Locked original data",
    ]
    for row in manifest["dataHashLock"]:
        lines.append(f"- `{row['label']}` `{row['variant']}` `{row['filename']}` sha256 `{row.get('actualSha256', row['sha256'])}` bytes `{row.get('actualBytes', row['bytes'])}` ok={row['ok']}")
    lines += ["", "## ReDMCSB source locks"]
    for row in manifest["sourceLocks"]:
        lines.append(f"- `{row['file']}:{row['function']}:{row['lines']}` — {row['claim']} ok={row['ok']}")
    lines += ["", "## Required deterministic capture scenes"]
    for scene in manifest["captureContract"]["scenes"]:
        lines.append(f"- `{scene}`")
    lines += ["", "## Exact framebuffer comparator manifest"]
    lines.append(f"- manifest: `{manifest['captureContract']['manifestPath']}`")
    lines.append(f"- schema: `{manifest['captureContract']['schemaPath']}`")
    lines.append(f"- comparator result: `{rel(COMPARATOR_RESULT)}` status=`{manifest['framebufferComparator']['status']}`")
    lines.append("- required original data provenance: `GRAPHICS.DAT` and `DUNGEON.DAT` must include exact variant, file/path, bytes, and SHA256; filename-only identity is rejected.")
    for row in manifest["framebufferComparator"]["expectedArtifacts"]:
        lines.append(f"- `{row['scene']}` `{row['side']}` `{row['artifact']}` path=`{row['path']}` hashField=`{row['requiredSha256Field']}` exists={row['exists']}")
    lines += ["", "## Current artifacts"]
    for row in manifest["reviewArtifacts"]:
        suffix = f" dims={row.get('pngDims')}" if row.get("pngDims") else ""
        cls = f" classification=`{row.get('classification')}`" if row.get("classification") else ""
        lines.append(f"- `{row['path']}` exists={row['exists']}{suffix}{cls} use=`{row['parityUse']}`")
    lines += [
        "",
        "## Remaining blocker",
        "Original PC34 Hall candidate select/panel/cancel/resurrect/reincarnate/HUD frames are not semantically promotable yet. The pass173 images are review clues only: they remain static/no-party after the gate, so they cannot prove panel pixel parity.",
        "",
        "## Non-claims",
        "No original-vs-Firestaff pixel parity, no candidate panel framebuffer parity, and no HUD/status pixel parity is claimed by this pass.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    source_rows, source_errors = audit_sources()
    data_rows, data_errors = audit_data()
    priors = audit_priors()
    review = audit_review_artifacts()
    framebuffer = audit_framebuffer_manifest(data_rows)
    existing_gate = run(["python3", "tools/verify_dm1_v1_hall_of_champions_full_source_lock.py"])
    panel_gate = run(["./run_firestaff_resurrect_reincarnate_cancel_routes_probe.sh"])
    errors = source_errors + data_errors
    if existing_gate["returncode"] != 0:
        errors.append("existing Hall source lock gate failed")
    if panel_gate["returncode"] != 0:
        errors.append("resurrect/reincarnate/cancel route probe failed")
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "FAIL_PASS449_SOURCE_OR_DATA_LOCK" if errors else STATUS,
        "repo": str(ROOT),
        "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(),
        "redmcsbRoot": str(REDMCSB),
        "dataHashLock": data_rows,
        "sourceLocks": source_rows,
        "priorBlockers": priors,
        "reviewArtifacts": review,
        "captureContract": build_capture_contract(),
        "framebufferComparator": framebuffer,
        "probes": {
            "hallFullSourceLock": existing_gate,
            "resurrectReincarnateCancelRoutes": panel_gate,
        },
        "activeBlocker": "missing source-bound/promotable original PC34 Hall candidate fullframes/crops/manifest for candidate_select, panel_visible, cancel, resurrect_confirm, reincarnate_confirm, and hud_status_after",
        "notClaimed": ["pixel parity", "candidate panel framebuffer parity", "HUD/status framebuffer parity"],
        "errors": errors,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    write_report(manifest)
    if errors:
        print("FAIL pass449 source/data locks")
        for error in errors:
            print(f"- {error}")
        return 1
    print(f"PASS {PASS}: {STATUS}")
    print(f"wrote {MANIFEST}")
    print(f"wrote {REPORT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
