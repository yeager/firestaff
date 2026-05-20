# DM1 V1 creature runtime audio trigger coverage - 2026-05-20

## Source evidence

Primary source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

- `GROUP.C:1668-1745` enters the creature attack path, initializes `L1876_B_UseSpellSoundAsFallback`, updates `G0361_l_LastCreatureAttackTime`, resolves the creature type from `G0243_as_Graphic559_CreatureInfo`, and marks projectile-style attacks as spell-sound fallback candidates.
- `GROUP.C:1788-1818` completes the attack path. For I34E (`MEDIA485...`) it takes `CreatureInfo.AttackSoundOrdinal`, indexes `G2003_aauc_CreatureSounds[ordinal - 1][C0_ATTACK_SOUND]`, rejects `CM1_SOUND_NONE`, and calls `F0064_SOUND_RequestPlay_CPSD(soundIndex, mapX, mapY, C01_MODE_PLAY_IF_PRIORITIZED)`. If no attack sound exists and the projectile fallback flag is set, it requests `M542_SOUND_SPELL` instead.
- `GROUP.C:260-281` has the Couatl special movement/flip path and, for I34E, requests `F0514_MOVE_GetSound(C13_CREATURE_COUATL)` through `F0064_SOUND_RequestPlay_CPSD`.
- `MOVESENS.C:830-854` routes successful group movement; I34E unconditionally calls `F0064_SOUND_RequestPlay_CPSD(F0514_MOVE_GetSound(group.Type), destinationX, destinationY, C01_MODE_PLAY_IF_PRIORITIZED)`.
- `MOVESENS.C:984-995` defines `F0514_MOVE_GetSound`: if `G0300_B_PartyIsResting`, return `-1`; otherwise map the creature attack-sound ordinal through `G2003_aauc_CreatureSounds[ordinal - 1][C1_MOVEMENT_SOUND]`, or return `-1` when the creature has no ordinal.
- `DUNGEON.C:667-733` source-locks the I34E `G0243_as_Graphic559_CreatureInfo` table, including each creature's attack sound ordinal. `DUNGEON.C:735-753` source-locks `G2003_aauc_CreatureSounds[18][2]` attack/movement sound indices.
- `DEFS.H:100-133` defines the I34E sound namespace: `CM1_SOUND_NONE=-1`, attack sounds `19..27`, movement sounds `28..34`, `C19_SOUND_FIRST_ATTACK`, and `C28_SOUND_FIRST_MOVEMENT`.
- `SOUND.C:1475-1640` defines `F0064_SOUND_RequestPlay_CPSD`: it ignores `CM1_SOUND_NONE`, rejects prioritized sounds on a non-party map, validates sound metadata, computes volume/range, and records/plays the original sound index.

## Firestaff implementation

- Existing source-index helpers in `src/dm1/dm1_v1_creature_sound_pc34_compat.c` already lock the ReDMCSB ordinal table and `G2003_aauc_CreatureSounds` mapping. This pass did not change that table.
- `src/engine/m11_game_view.c:959-990` now adds M11 runtime helpers that request creature attack/movement source sound indices through `DM1_CreatureSound_AttackIndexForType` and `DM1_CreatureSound_MovementIndexForType`, then emit via `M11_Audio_EmitSourceSoundIndex` so the original source index is preserved even in the fallback backend.
- `src/engine/m11_game_view.c:4121-4252` now passes group map coordinates into autonomous creature attacks and emits the source-indexed attack sound after the M11 creature attack damage path.
- `src/engine/m11_game_view.c:4297-4322` now emits the source-indexed movement sound immediately after a successful M11 creature movement step. The helper preserves the ReDMCSB party-resting movement sound gate and does not add party footstep or ambient behavior.

## Runtime probe

`probes/m11/firestaff_m11_creature_runtime_audio_probe.c` builds a synthetic two-map M11 dungeon so map 1 is an active creature map, plants one group and one champion, and advances the real `M11_GameView_AdvanceIdleTick` path.

The probe verifies:

- Giant scorpion attack on the party emits source sound index `20` (`C20_SOUND_ATTACK_GIANT_SCORPION_SCORPION`) with `M11_AUDIO_MARKER_CREATURE`.
- Red dragon movement emits source sound index `33` (`C33_SOUND_MOVE_RED_DRAGON`) with `M11_AUDIO_MARKER_CREATURE`.
- The same red dragon movement while the party is resting still moves the creature but emits no movement sound (`lastSoundIndex == -1`, marker `NONE`), matching `MOVESENS.C:988-990`.

## Verification

Commands run from `/home/trv2/work/firestaff-worktrees/audio-creature-runtime-triggers-20260520`:

```sh
cmake -S "$wt" -B "$build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build" --target firestaff_m11_creature_runtime_audio_probe test_dm1_v1_creature_sound_pc34_compat firestaff_m11_audio_probe -j2
"$build"/firestaff_m11_creature_runtime_audio_probe
"$build"/test_dm1_v1_creature_sound_pc34_compat
"$build"/firestaff_m11_audio_probe
ctest --test-dir "$build" -R "firestaff_m11_creature_runtime_audio_probe|dm1_v1_creature_sound_source_lock|m11_audio" --output-on-failure
git diff --check
git diff -- . ":(exclude)build-audio-creature-runtime-triggers" | rg -n -i "(AKIA[0-9A-Z]{16}|-----BEGIN (RSA|DSA|EC|OPENSSH|PRIVATE) KEY-----|xox[baprs]-[0-9A-Za-z-]+|gh[pousr]_[A-Za-z0-9_]{20,}|sk-[A-Za-z0-9]{20,}|api[_-]?key\s*=|secret\s*=|token\s*=|password\s*=)"
```

Results:

- `firestaff_m11_creature_runtime_audio_probe`: `11/11` invariants passed.
- `test_dm1_v1_creature_sound_pc34_compat`: `33 passed, 0 failed`.
- `firestaff_m11_audio_probe`: `9/9` invariants passed.
- CTest filter: `3/3` tests passed (`m11_audio`, `firestaff_m11_creature_runtime_audio_probe`, `dm1_v1_creature_sound_source_lock`).
- `git diff --check`: clean.
- Secret scan: no matches.
