#!/usr/bin/env python3
"""Verify pass363 DM1 V1 F0115 thing-layer source-lock/blocker evidence."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DUNVIEW = SOURCE_ROOT / "DUNVIEW.C"
EVIDENCE = ROOT / "parity-evidence/pass363_dm1_v1_f0115_thing_layer_source_lock.md"
MANIFEST = ROOT / "parity-evidence/verification/pass363_dm1_v1_f0115_thing_layer_source_lock/manifest.json"
VIEW3D_C = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"
VIEW3D_H = ROOT / "dm1_v1_viewport_3d_pc34_compat.h"
M11_VIEW = ROOT / "m11_game_view.c"
EXPECTED_STATUS = "PASS_DM1_V1_F0115_THING_LAYER_SOURCE_LOCK_BLOCKER_NARROWED"


def fail(message: str) -> int:
    print(f"status=FAIL_PASS363_DM1_V1_F0115_THING_LAYER_SOURCE_LOCK reason={message}")
    return 1


def read(path: Path, encoding: str = "utf-8") -> str:
    return path.read_text(encoding=encoding)


def block(path: Path, start: int, end: int) -> str:
    lines = read(path, "latin-1").splitlines()
    if end > len(lines):
        raise AssertionError(f"{path.name} shorter than {end} lines")
    return "\n".join(lines[start - 1:end])


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def require_all(blob: str, needles: list[str], label: str) -> None:
    compact = " ".join(blob.split())
    for needle in needles:
        require(" ".join(needle.split()) in compact, f"{label} missing {needle!r}")


def require_order(blob: str, needles: list[str], label: str) -> None:
    cursor = -1
    for needle in needles:
        pos = blob.find(needle)
        require(pos >= 0, f"{label} missing {needle!r}")
        require(pos > cursor, f"{label} out of order at {needle!r}")
        cursor = pos


def find_function(text: str, name: str) -> str:
    m = re.search(r"\b(?:static\s+)?(?:void|int|size_t|const\s+[^\n]+\*)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    semi = text.find(";", m.end(), brace if brace >= 0 else len(text))
    if semi >= 0:
        next_text = text[semi + 1:]
        return find_function(next_text, name)
    require(brace >= 0, f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[m.start():i + 1]
    raise AssertionError(f"unterminated function {name}")


def main() -> int:
    try:
        manifest = json.loads(read(MANIFEST))
        evidence = read(EVIDENCE)
        require(manifest.get("status") == EXPECTED_STATUS, "manifest status mismatch")
        require(manifest.get("renderer_behavior_changes") is False, "manifest must stay verifier/evidence-only")
        require(manifest.get("pixel_parity_claim") is False, "manifest must not claim pixel parity")
        require("makes no pixel-parity claim" in evidence, "evidence must explicitly avoid pixel-parity claim")
        require("Blocker narrowed" in evidence, "evidence must describe narrowed blocker")

        for anchor in manifest["source_anchors"]:
            marker = "{}:{}".format(anchor["file"], anchor["lines"])
            require(marker in evidence, f"evidence missing anchor {marker}")

        require_all(block(DUNVIEW, 4547, 4582), [
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "draw each object found",
            "Draw one creature at the cell being processed",
            "Draw only projectiles at specified cell",
            "Draw only explosions at specified cell",
        ], "DUNVIEW.C:4547-4582")
        require_all(block(DUNVIEW, 4819, 4860), [
            "/* Draw objects */",
            "C04_THING_TYPE_GROUP",
            "C14_THING_TYPE_PROJECTILE",
            "C15_THING_TYPE_EXPLOSION",
            "M011_CELL(P0141_T_Thing) == L0139_i_Cell",
        ], "DUNVIEW.C:4819-4860")
        require_all(block(DUNVIEW, 5195, 5202), [
            "P0145_i_ViewSquareIndex < M600_VIEW_SQUARE_D3C",
            "L2475_i_ViewDepth > 3",
            "/* Draw creatures */",
            "L0168_B_DrawingLastBackRowCell",
        ], "DUNVIEW.C:5195-5202")
        require_all(block(DUNVIEW, 5679, 5683), [
            "Restart processing list of objects from the beginning",
            "C14_THING_TYPE_PROJECTILE",
            "C2900_ZONE_",
            "AL0126_i_ViewCell",
        ], "DUNVIEW.C:5679-5683")
        require_all(block(DUNVIEW, 5915, 5933), [
            "} while (L0130_ul_RemainingViewCellOrdinalsToProcess);",
            "/* Draw explosions */",
            "P0141_T_Thing = L0146_T_FirstThingToDraw",
            "C15_THING_TYPE_EXPLOSION",
        ], "DUNVIEW.C:5915-5933")

        c_text = read(VIEW3D_C)
        require_order(c_text, [
            "DM1_VIEWPORT_THING_LAYER_OBJECTS",
            "DM1_VIEWPORT_THING_LAYER_CREATURES",
            "DM1_VIEWPORT_THING_LAYER_PROJECTILES",
            "DM1_VIEWPORT_THING_LAYER_EXPLOSIONS",
        ], "s_thing_layers layer order")
        require_all(c_text, [
            "DUNVIEW.C:4567-4571,4853-4860",
            "DUNVIEW.C:4573,5195-5202",
            "DUNVIEW.C:4575-4577,5681-5883",
            "DUNVIEW.C:4579-4581,5915-5933",
        ], "s_thing_layers source citations")
        require_all(read(VIEW3D_H), [
            "object/creature/projectile phases for each",
            "then restarts once after all cells for explosions",
            "DUNVIEW.C:4567-4581, 5915-5933",
        ], "DM1_ViewportThingLayerSpec contract")

        contents = find_function(read(M11_VIEW), "m11_draw_wall_contents")
        require_all(contents, [
            "m11_viewport_cell_is_open(cell)",
            "m11_draw_effect_cue",
            "Layer 3: Projectiles and explosions",
        ], "m11_draw_wall_contents current local effect cue path")

        print(f"status={EXPECTED_STATUS}")
        print("anchors=DUNVIEW.C:4547-4582,DUNVIEW.C:4819-4860,DUNVIEW.C:5195-5202,DUNVIEW.C:5679-5683,DUNVIEW.C:5915-5933")
        print("blocker=M11 effect cue still coalesces projectiles/explosions per open cell; ReDMCSB F0115 explosions run after all packed cells")
        return 0
    except (AssertionError, OSError, json.JSONDecodeError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())
