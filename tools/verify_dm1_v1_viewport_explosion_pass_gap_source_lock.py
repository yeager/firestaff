#!/usr/bin/env python3
"""Source-lock the DM1 V1 viewport explosion-pass timing gap.

This verifier intentionally documents a narrowed blocker: current M11 rendering
keeps projectiles-before-explosions inside one per-cell effect cue, while
ReDMCSB F0115 draws explosions only after all packed cells have completed their
object/creature/projectile passes.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DUNVIEW = SOURCE_ROOT / "DUNVIEW.C"
VIEW = ROOT / "m11_game_view.c"
EVIDENCE = ROOT / "parity-evidence/dm1_v1_viewport_explosion_pass_gap_source_lock_20260508.md"
MANIFEST = ROOT / "parity-evidence/verification/dm1_v1_viewport_explosion_pass_gap_source_lock/manifest.json"
EXPECTED_STATUS = "PASS_DM1_V1_VIEWPORT_EXPLOSION_PASS_GAP_SOURCE_LOCK_BLOCKER_NARROWED"


def fail(message: str) -> int:
    print(f"status=FAIL_DM1_V1_VIEWPORT_EXPLOSION_PASS_GAP_SOURCE_LOCK reason={message}")
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


def find_function(text: str, name: str) -> str:
    m = re.search(r"\b(?:static\s+)?(?:void|int|size_t|const\s+[^\n]+\*)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    semi = text.find(";", m.end(), brace if brace >= 0 else len(text))
    if semi >= 0:
        return find_function(text[semi + 1:], name)
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
        view_text = read(VIEW)

        require(manifest.get("status") == EXPECTED_STATUS, "manifest status mismatch")
        require(manifest.get("renderer_behavior_changes") is False, "manifest must be evidence-only")
        require(manifest.get("pixel_parity_claim") is False, "manifest must not claim pixel parity")
        require("Minimal fix proposal" in evidence, "evidence must include minimal fix proposal")
        require("Blocker narrowed" in evidence, "evidence must state narrowed blocker")
        require("not a renderer rewrite" in evidence, "evidence must keep scope non-rewrite")

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
        require_all(block(DUNVIEW, 5679, 5683), [
            "Restart processing list of objects from the beginning",
            "C14_THING_TYPE_PROJECTILE",
            "AL0126_i_ViewCell",
        ], "DUNVIEW.C:5679-5683")
        require_all(block(DUNVIEW, 5915, 5933), [
            "} while (L0130_ul_RemainingViewCellOrdinalsToProcess);",
            "/* Draw explosions */",
            "P0141_T_Thing = L0146_T_FirstThingToDraw",
            "C15_THING_TYPE_EXPLOSION",
        ], "DUNVIEW.C:5915-5933")

        effect = find_function(view_text, "m11_draw_effect_cue")
        projectile_pos = effect.find("summary.projectiles > 0")
        explosion_pos = effect.find("summary.explosions > 0")
        require(projectile_pos >= 0, "m11_draw_effect_cue missing projectile branch")
        require(explosion_pos >= 0, "m11_draw_effect_cue missing explosion branch")
        require(projectile_pos < explosion_pos, "m11_draw_effect_cue no longer coalesces projectile-before-explosion locally")
        require("m11_draw_projectile_sprite" in effect, "m11_draw_effect_cue missing projectile sprite draw")
        require("m11_draw_explosion_sprite" in effect, "m11_draw_effect_cue missing explosion sprite draw")

        wall_contents = find_function(view_text, "m11_draw_wall_contents")
        require_all(wall_contents, [
            "m11_viewport_cell_is_open(cell)",
            "Layer 3: Projectiles and explosions",
            "m11_draw_effect_cue",
        ], "m11_draw_wall_contents current per-open-cell effect path")

        require("split projectile draw from explosion draw" in manifest["minimal_fix"], "manifest minimal fix must be the draw split")
        print(f"status={EXPECTED_STATUS}")
        print("anchors=DUNVIEW.C:4547-4582,DUNVIEW.C:5679-5683,DUNVIEW.C:5915-5933")
        print("gap=M11 m11_draw_effect_cue coalesces projectiles/explosions per open cell; ReDMCSB F0115 defers explosions until after all packed cells")
        print("minimal_fix=split projectile cue from deferred after-all-cells explosion pass")
        return 0
    except (AssertionError, OSError, json.JSONDecodeError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())
