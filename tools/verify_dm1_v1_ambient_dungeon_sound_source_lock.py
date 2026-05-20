#!/usr/bin/env python3
"""DM1 V1 ambient dungeon sound source-lock audit.

This verifier intentionally does not synthesize audio behavior. It locks the
ReDMCSB evidence boundary for the TODO row named "Ambient dungeon sound": the
DM1 V1 PC source exposes event-indexed SFX requests and a per-tick pending flush,
but no always-on/looping dungeon ambience source path.
"""

from __future__ import annotations

import os
import pathlib
import re
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
REDMCSB = pathlib.Path(os.environ.get(
    "REDMCSB_SOURCE_DIR",
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source",
))


def read(path: pathlib.Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def line_of(text: str, needle: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing needle: {needle!r}")
    return text.count("\n", 0, pos) + 1


def require(condition: bool, label: str, failures: list[str]) -> None:
    if condition:
        print(f"PASS {label}")
    else:
        print(f"FAIL {label}")
        failures.append(label)


def all_line_hits(text: str, pattern: str) -> list[int]:
    return [text.count("\n", 0, m.start()) + 1 for m in re.finditer(pattern, text)]


def main() -> int:
    failures: list[str] = []
    if not REDMCSB.is_dir():
        print(f"FAIL REDMCSB_SOURCE_DIR missing: {REDMCSB}")
        return 1

    red = {name: read(REDMCSB / name) for name in [
        "GAMELOOP.C",
        "SOUND.C",
        "DEFS.H",
        "DATA.C",
        "GROUP.C",
        "MOVESENS.C",
        "TIMELINE.C",
    ]}
    fire = {
        "TODO.md": read(ROOT / "TODO.md"),
        "include/audio_sdl_m11.h": read(ROOT / "include/audio_sdl_m11.h"),
        "src/shared/audio_sdl_m11.c": read(ROOT / "src/shared/audio_sdl_m11.c"),
    }

    # Positive source anchors for the actual DM1 V1 PC sound model.
    anchors = [
        ("GAMELOOP.C", "F0065_SOUND_PlayPendingSound_CPSD();", "per-tick pending SFX flush"),
        ("SOUND.C", "void F0064_SOUND_RequestPlay_CPSD(", "event-indexed sound request function"),
        ("SOUND.C", "L0045_s_Event.A.A.Type = C20_EVENT_PLAY_SOUND;", "deferred sound request event"),
        ("SOUND.C", "if (!P0091_ui_Mode) { /* Play the sound immediately */", "immediate sound branch"),
        ("SOUND.C", "Set the requested sound as the pending sound if it is louder", "priority pending branch"),
        ("SOUND.C", "void F0065_SOUND_PlayPendingSound_CPSD(", "pending sound playback function"),
        ("SOUND.C", "BOOLEAN F0505_SOUND_GetVolume(", "directional distance-volume function"),
        ("DEFS.H", "#define C28_SOUND_FIRST_MOVEMENT", "movement sound namespace"),
        ("DEFS.H", "#define C01_MODE_PLAY_IF_PRIORITIZED", "one-sound-per-tick play mode"),
        ("DATA.C", "{ 701, 0, 138,  24, 0, 0, 4, NULL }, /* 7 creature movement sounds */", "I34E SND3 creature movement table"),
        ("MOVESENS.C", "F0064_SOUND_RequestPlay_CPSD(F0514_MOVE_GetSound", "movement-triggered sound request"),
        ("GROUP.C", "F0064_SOUND_RequestPlay_CPSD(F0514_MOVE_GetSound(C13_CREATURE_COUATL)", "creature animation movement sound request"),
        ("TIMELINE.C", "case C20_EVENT_PLAY_SOUND:", "deferred play-sound event dispatch"),
    ]
    for filename, needle, label in anchors:
        try:
            line = line_of(red[filename], needle)
            print(f"PASS SOURCE_ANCHOR {filename}:{line} {label}")
        except AssertionError as exc:
            print(f"FAIL SOURCE_ANCHOR {filename} {label}: {exc}")
            failures.append(f"source anchor {filename} {label}")

    request_call_files: dict[str, list[int]] = {}
    for filename, text in red.items():
        hits = all_line_hits(text, r"F0064_SOUND_RequestPlay_CPSD\s*\(")
        if hits:
            request_call_files[filename] = hits
    require("GAMELOOP.C" not in request_call_files,
            "NO_GAMELOOP_SOUND_REQUEST_LOOP game loop only flushes pending sound, it does not originate ambient SFX",
            failures)

    core_sound_text = "\n".join(red[name] for name in ["GAMELOOP.C", "SOUND.C", "DEFS.H", "DATA.C"])
    forbidden_words = re.findall(r"\bambient\w*|\bambience\w*", core_sound_text, flags=re.IGNORECASE)
    require(not forbidden_words,
            "NO_REDMCSB_AMBIENT_SYMBOL no ambient/ambience symbol in core sound source",
            failures)

    # The only loop-related hits in SOUND.C are decoder/sample-repeat internals,
    # not a dungeon ambience request path. Keep this as a bounded absence check.
    sound_loop_lines = all_line_hits(red["SOUND.C"], r"(?i)loop|repeat")
    require(sound_loop_lines and min(sound_loop_lines) >= 900,
            f"NO_AMBIENT_LOOP_API SOUND.C loop/repeat hits are decoder internals at lines {sound_loop_lines[:8]}",
            failures)

    require("Ambient dungeon sound" in fire["TODO.md"],
            "FIRESTAFF_TODO_ROW_PRESENT Ambient dungeon sound remains tracked in TODO.md",
            failures)
    require("ambient" not in fire["include/audio_sdl_m11.h"].lower()
            and "ambient" not in fire["src/shared/audio_sdl_m11.c"].lower(),
            "FIRESTAFF_NO_AMBIENT_AUDIO_API no ambient-specific M11 audio API exists",
            failures)

    print("# Request call files:")
    for filename in sorted(request_call_files):
        print(f"#   {filename}: {request_call_files[filename]}")

    if failures:
        print(f"FAIL DM1_V1_AMBIENT_DUNGEON_SOUND_SOURCE_LOCK {len(failures)} failures")
        return 1
    print("PASS DM1_V1_AMBIENT_DUNGEON_SOUND_SOURCE_LOCK no source-backed ambient dungeon loop found")
    return 0


if __name__ == "__main__":
    sys.exit(main())
