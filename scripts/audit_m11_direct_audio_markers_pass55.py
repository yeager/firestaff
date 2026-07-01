#!/usr/bin/env python3
"""Pass 55 bounded audit for remaining direct M11 audio marker calls.

This is intentionally source-shape based: it does not claim original cadence or
runtime overlap.  It locks the current honesty boundary after converting only
source-backed action cues to DM PC v3.4 sound-event indices.
"""

from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
GAME_VIEW = ROOT / "src/engine/m11_game_view.c"
MAP_FILE = ROOT / "src/shared/sound_event_snd3_map_v1.c"

EXPECTED_SOURCE_EVENTS = {
    17: "M619_SOUND_WAR_CRY",
    18: "M620_SOUND_BLOW_HORN",
    13: "M563_SOUND_COMBAT_ATTACK",
}


def fail(msg: str) -> int:
    print(f"FAIL {msg}")
    return 1


def pass_line(msg: str) -> None:
    print(f"PASS {msg}")


def direct_marker_calls(text: str) -> list[int]:
    return [m.start() for m in re.finditer(r"M11_Audio_EmitMarker\s*\(\s*&state->audioState", text)]


def line_for_offset(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def context_window(text: str, offset: int, before: int = 1800, after: int = 260) -> str:
    start = max(0, offset - before)
    end = min(len(text), offset + after)
    return text[start:end]


def main() -> int:
    game = GAME_VIEW.read_text(encoding="utf-8")
    mapping = MAP_FILE.read_text(encoding="utf-8")

    failures = 0

    # 1. The three source-backed action sound events must exist in the Pass 52 map.
    for sound_index, macro in EXPECTED_SOURCE_EVENTS.items():
        needle = f"{{{sound_index},"
        if needle in mapping and macro in mapping:
            pass_line(f"P55_DIRECT_AUDIO_AUDIT_01 map contains sound event {sound_index} {macro}")
        else:
            failures += fail(f"P55_DIRECT_AUDIO_AUDIT_01 missing map event {sound_index} {macro}")

    # 2. The converted action paths must use indexed sound emission, not direct markers.
    expected_literals = {
        "war cry action emits event 17": "m11_audio_emit_source_sound(state, 17, M11_AUDIO_MARKER_CREATURE)",
        "blow horn action emits event 18": "m11_audio_emit_source_sound(state, 18, M11_AUDIO_MARKER_CREATURE)",
    }
    for label, literal in expected_literals.items():
        if literal in game:
            pass_line(f"P55_DIRECT_AUDIO_AUDIT_02 {label}")
        else:
            failures += fail(f"P55_DIRECT_AUDIO_AUDIT_02 missing {label}")

    shoot_pos = game.find("T%u: %s SHOOTS")
    throw_helper_pos = game.find("m11_dm1_f0328_spawn_thrown_thing")
    throw_sound_pos = game.find("m11_audio_emit_source_sound(state, 13, M11_AUDIO_MARKER_COMBAT)",
                                throw_helper_pos)
    if shoot_pos >= 0:
        shoot_ctx = context_window(game, shoot_pos, before=120, after=700)
    else:
        shoot_ctx = ""
    if (
        shoot_pos >= 0
        and throw_helper_pos >= 0
        and throw_sound_pos >= 0
        and "m11_audio_emit_source_sound(state, 13, M11_AUDIO_MARKER_COMBAT)" in shoot_ctx
    ):
        pass_line("P55_DIRECT_AUDIO_AUDIT_03 shoot and F0328 throw emit source-backed event 13")
    else:
        failures += fail(
            "P55_DIRECT_AUDIO_AUDIT_03 missing source-backed event-13 shoot or F0328 throw emission"
        )

    # 3. Remaining direct marker calls are allowed only in explicitly documented
    #    buckets: generic non-EMIT tick emissions. CALM / BRANDISH / CONFUSE and
    #    INVOKE are source-silent in PC34 MENU.C and must not use marker fallback.
    calls = direct_marker_calls(game)
    allowed_labels = {
        "generic_non_sound_request_emission": 0,
    }
    unexpected: list[tuple[int, str]] = []
    spell_block_start = game.find("case 20:   /* FIREBALL */")
    invoke_block_start = game.find("case 27: { /* INVOKE */")
    throw_block_start = game.find("case 42: { /* THROW */")
    spell_block = game[spell_block_start:invoke_block_start]
    for offset in calls:
        ctx = context_window(game, offset)
        line = line_for_offset(game, offset)
        if "emission->kind == EMIT_SOUND_REQUEST" in ctx and "else" in ctx:
            allowed_labels["generic_non_sound_request_emission"] += 1
        else:
            unexpected.append((line, " ".join(ctx.split()[:32])))

    expected_counts = {
        "generic_non_sound_request_emission": 1,
    }

    social_block_start = game.find("case 37: /* CALM */")
    social_block_end = game.find("case 32: { /* SHOOT */")
    social_block = game[social_block_start:social_block_end]
    if social_block_start < 0 or social_block_end < 0:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_04A missing social frighten action block")
    elif (
        "M11_Audio_EmitMarker" not in social_block
        and "m11_audio_emit_source_sound(state, 17, M11_AUDIO_MARKER_CREATURE)" in social_block
        and "m11_audio_emit_source_sound(state, 18, M11_AUDIO_MARKER_CREATURE)" in social_block
        and "ReDMCSB PC34 MENU.C:1347-1362" in social_block
    ):
        pass_line("P55_DIRECT_AUDIO_AUDIT_04A calm/brandish/confuse stay source-silent")
    else:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_04A social frighten block still has marker fallback or lost source-backed horn/cry events")

    if spell_block_start < 0 or invoke_block_start < 0:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_04B missing spell projectile action block")
    elif "M11_Audio_EmitMarker" not in spell_block and "MENU.C:1280-1305" in spell_block and "CHAMPION.C:2073-2106" in spell_block:
        pass_line("P55_DIRECT_AUDIO_AUDIT_04B fireball/dispell/lightning action cast stays source-silent")
    else:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_04B spell projectile action block still has marker fallback or lost source anchors")

    invoke_block = game[invoke_block_start:throw_block_start]
    if invoke_block_start < 0 or throw_block_start < 0:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_04C missing invoke action block")
    elif (
        "M11_Audio_EmitMarker" not in invoke_block
        and "m11_audio_emit_source_sound" not in invoke_block
        and "INVOKE case at lines 1480-1493" in invoke_block
    ):
        pass_line("P55_DIRECT_AUDIO_AUDIT_04C invoke action stays source-silent")
    else:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_04C invoke block has action-time audio fallback or lost source anchors")

    if not unexpected and allowed_labels == expected_counts:
        pass_line(
            "P55_DIRECT_AUDIO_AUDIT_04 remaining direct marker calls are documented TODO buckets "
            f"{allowed_labels}"
        )
    else:
        failures += fail(
            "P55_DIRECT_AUDIO_AUDIT_04 unexpected remaining direct marker calls: "
            f"counts={allowed_labels} unexpected={unexpected}"
        )

    # 4. Guard against accidental regression to direct marker calls in converted actions.
    if shoot_pos < 0:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_05 missing action log T%u: %s SHOOTS")
    elif "M11_Audio_EmitMarker" not in shoot_ctx and "m11_audio_emit_source_sound" in shoot_ctx:
        pass_line("P55_DIRECT_AUDIO_AUDIT_05 converted action near T%u: %s SHOOTS")
    else:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_05 direct marker remains near T%u: %s SHOOTS")

    throw_helper_end = game.find("static int m11_dm1_f0328_throw_attack_probe_legacy",
                                 throw_helper_pos)
    if throw_helper_pos < 0 or throw_helper_end < 0:
        failures += fail("P55_DIRECT_AUDIO_AUDIT_05 missing F0328 throw helper")
    else:
        throw_ctx = game[throw_helper_pos:throw_helper_end]
        if "M11_Audio_EmitMarker" not in throw_ctx and "m11_audio_emit_source_sound" in throw_ctx:
            pass_line("P55_DIRECT_AUDIO_AUDIT_05 converted F0328 throw helper")
        else:
            failures += fail("P55_DIRECT_AUDIO_AUDIT_05 direct marker remains in F0328 throw helper")

    if failures:
        return 1
    print("PASS P55_DIRECT_AUDIO_AUDIT_SUMMARY 0 failures")
    return 0


if __name__ == "__main__":
    sys.exit(main())
