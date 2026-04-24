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
GAME_VIEW = ROOT / "m11_game_view.c"
MAP_FILE = ROOT / "sound_event_snd3_map_v1.c"

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

    combat_event_13_count = game.count("m11_audio_emit_source_sound(state, 13, M11_AUDIO_MARKER_COMBAT)")
    if combat_event_13_count == 2:
        pass_line("P55_DIRECT_AUDIO_AUDIT_03 shoot and throw emit source-backed event 13")
    else:
        failures += fail(
            f"P55_DIRECT_AUDIO_AUDIT_03 expected 2 event-13 action emissions, found {combat_event_13_count}"
        )

    # 3. Remaining direct marker calls are allowed only in explicitly documented buckets:
    #    generic non-EMIT tick emissions; CALM/BRANDISH/CONFUSE fallback; spell/invoke
    #    action cues whose exact original request timing/index is not source-backed here.
    calls = direct_marker_calls(game)
    allowed_labels = {
        "generic_non_sound_request_emission": 0,
        "calm_brandish_confuse_fallback": 0,
        "spell_projectile_action_fallback": 0,
        "invoke_action_fallback": 0,
    }
    unexpected: list[tuple[int, str]] = []
    spell_block_start = game.find("case 20:   /* FIREBALL */")
    invoke_block_start = game.find("case 27: { /* INVOKE */")
    throw_block_start = game.find("case 42: { /* THROW */")
    for offset in calls:
        ctx = context_window(game, offset)
        line = line_for_offset(game, offset)
        if "emission->kind == EMIT_SOUND_REQUEST" in ctx and "else" in ctx:
            allowed_labels["generic_non_sound_request_emission"] += 1
        elif "CALM / BRANDISH / CONFUSE are still V1-slice cues only" in ctx:
            allowed_labels["calm_brandish_confuse_fallback"] += 1
        elif spell_block_start <= offset < invoke_block_start:
            allowed_labels["spell_projectile_action_fallback"] += 1
        elif invoke_block_start <= offset < throw_block_start:
            allowed_labels["invoke_action_fallback"] += 1
        else:
            unexpected.append((line, " ".join(ctx.split()[:32])))

    expected_counts = {
        "generic_non_sound_request_emission": 1,
        "calm_brandish_confuse_fallback": 1,
        "spell_projectile_action_fallback": 1,
        "invoke_action_fallback": 1,
    }
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
    for forbidden in [
        "T%u: %s SHOOTS",
        "T%u: %s THROWS",
    ]:
        pos = game.find(forbidden)
        if pos < 0:
            failures += fail(f"P55_DIRECT_AUDIO_AUDIT_05 missing action log {forbidden}")
            continue
        ctx = context_window(game, pos, before=120, after=700)
        if "M11_Audio_EmitMarker" not in ctx and "m11_audio_emit_source_sound" in ctx:
            pass_line(f"P55_DIRECT_AUDIO_AUDIT_05 converted action near {forbidden}")
        else:
            failures += fail(f"P55_DIRECT_AUDIO_AUDIT_05 direct marker remains near {forbidden}")

    if failures:
        return 1
    print("PASS P55_DIRECT_AUDIO_AUDIT_SUMMARY 0 failures")
    return 0


if __name__ == "__main__":
    sys.exit(main())
