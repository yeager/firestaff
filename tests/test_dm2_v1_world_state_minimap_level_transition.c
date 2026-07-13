/*
 * test_dm2_v1_world_state_minimap_level_transition.c
 *
 * DM2 V1 minimap/exploration persistence regression across one
 * level transition and one save/load boundary.
 *
 * Scope:
 *   - reveal tiles on level A
 *   - transition current_level from A to B via the public setter
 *   - confirm level A reveals are intact (current_level pointer
 *     must not wipe exploration history)
 *   - reveal tiles on level B
 *   - save + reload the world state
 *   - confirm both A and B reveal bitmaps survive the round-trip
 *   - confirm current_level survives the round-trip
 *   - confirm the level-transition setter rejects out-of-range
 *     targets without mutating current_level or any explored bit
 *
 * The explored bitmap is keyed by level index, not by current_level.
 * A regression here would mean level transitions silently wiped
 * prior map exploration — the very thing DM2 players notice when
 * they walk downstairs and the minimap forgets the floor they just
 * cleared.
 *
 * Source: ReDMCSB DEFS.H:560 GLOBAL_DATA.PartyMapIndex
 *         ReDMCSB LOADSAVE.C:1515-1524 GLOBAL_DATA round-trip
 *         ReDMCSB CLIKMENU.C:177-179,265 stairs / map transition
 *         SKULL.ASM T520 party placement tick
 *         docs/dm2_save_format.md — SUPPRESS save format
 */

#include "dm2_v1_world_state.h"
#include "dm2_v1_new_game.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_total = 0;

static int expect_true(int condition, const char *message)
{
    g_total++;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    g_pass++;
    fprintf(stderr, "PASS: %s\n", message);
    return 1;
}

static int append_bytes(uint8_t *out, size_t cap, size_t *pos,
                        const void *src, size_t size)
{
    if (!out || !pos || !src || *pos > cap || size > cap - *pos) return 0;
    memcpy(out + *pos, src, size);
    *pos += size;
    return 1;
}

static int append_i32_le(uint8_t *out, size_t cap, size_t *pos, int value)
{
    uint8_t bytes[4];
    bytes[0] = (uint8_t)(value & 0xFF);
    bytes[1] = (uint8_t)((value >> 8) & 0xFF);
    bytes[2] = (uint8_t)((value >> 16) & 0xFF);
    bytes[3] = (uint8_t)((value >> 24) & 0xFF);
    return append_bytes(out, cap, pos, bytes, sizeof(bytes));
}

static int test_original_envelope_world_restore(void)
{
    uint8_t slot[1024];
    uint8_t encoded_gs[256];
    uint8_t encoded_champion[512];
    uint8_t champion_mask[261];
    DM2_GameStateBlock game_state;
    DM2_ChampionRecord champion;
    DM2_V1_SaveCandidate candidate;
    DM2_WorldState *loaded;
    size_t pos = 42u;
    int gs_size;
    int champion_size;
    int ok;

    memset(slot, 0, sizeof(slot));
    memset(&game_state, 0, sizeof(game_state));
    memset(&champion, 0, sizeof(champion));
    game_state.dwGameTick = 0x1234u;
    game_state.dwRandomSeed = 0x98765432u;
    game_state.wChampionsCount = 1u;
    game_state.wPlayerPosX = 12u;
    game_state.wPlayerPosY = 7u;
    game_state.wPlayerDir = 3u;
    game_state.wPlayerMap = 2u;
    game_state.wChampionLeader = 0u;
    game_state.wTimersCount = 2u;
    game_state.rain_state[0] = 18u;
    memcpy(champion.first_name, "HISS", 4u);
    champion.cur_hp = 51u;
    champion.max_hp = 77u;
    champion.mana = 22u;
    champion.food = 300;
    champion.water = 44;
    dm2_suppress_champion_mask(champion_mask);
    gs_size = dm2_suppress_encode_gamestate(&game_state, encoded_gs,
                                             sizeof(encoded_gs));
    champion_size = dm2_suppress_encode_champion(&champion, champion_mask,
                                                  encoded_champion,
                                                  sizeof(encoded_champion));
    if (gs_size <= 0 || champion_size <= 0 ||
        !append_bytes(slot, sizeof(slot), &pos, "D2RS", 4u) ||
        !append_i32_le(slot, sizeof(slot), &pos, gs_size) ||
        !append_bytes(slot, sizeof(slot), &pos, encoded_gs, (size_t)gs_size) ||
        !append_i32_le(slot, sizeof(slot), &pos, champion_size) ||
        !append_bytes(slot, sizeof(slot), &pos, encoded_champion,
                      (size_t)champion_size)) {
        return 0;
    }
    slot[38] = 0xBEu; slot[39] = 0xEFu;
    slot[40] = 0xDEu; slot[41] = 0xADu;
    if (dm2_v1_session_parse_save_candidate(&candidate, slot + 42u,
                                            pos - 42u) != 0) {
        fprintf(stderr, "original envelope candidate parser rejected fixture\n");
        return 0;
    }
    loaded = dm2_v1_world_state_load_from_mem(slot, pos);
    if (!loaded) return 0;
    ok = loaded->game_tick == 0x1234 &&
         loaded->timer_count == 0 &&
         loaded->current_level == 2 &&
         loaded->party.champion_count == 1 &&
         loaded->party.champions[0].x == 12 &&
         loaded->party.champions[0].y == 7 &&
         loaded->party.champions[0].hp == 51 &&
         loaded->party.champions[0].max_hp == 77 &&
         loaded->party.champions[0].mp == 22 &&
         loaded->weather_by_level[2].intensity == 18 &&
         loaded->raw_save_size == pos - 42u;
    dm2_v1_world_state_free(loaded);
    return ok;
}

int main(void)
{
    DM2_WorldState state;
    DM2_WorldState *loaded = NULL;
    uint8_t *serialized = NULL;
    size_t serialized_size = 0;
    int sentinel_kept = 0;

    memset(&state, 0, sizeof(state));
    state.current_level = 0;

    /* ── 1. Reveal a few cells on level 0 (starting level) ── */
    dm2_v1_world_state_set_explored(&state, 0, 2, 3, 1);
    dm2_v1_world_state_set_explored(&state, 0, 7, 8, 1);
    dm2_v1_world_state_set_explored(&state, 0, 31, 0, 1);

    expect_true(dm2_v1_world_state_get_explored(&state, 0, 2, 3) == 1,
                "level 0 reveal is readable before transition");
    expect_true(dm2_v1_world_state_get_explored(&state, 0, 31, 0) == 1,
                "level 0 edge reveal is readable before transition");

    /* ── 2. Transition current_level 0 -> 1 (simulated stairs down) ── */
    dm2_v1_world_state_set_current_level(&state, 1);
    expect_true(state.current_level == 1,
                "set_current_level updates the pointer to 1");
    /* The pointer move MUST NOT wipe prior exploration. */
    expect_true(dm2_v1_world_state_get_explored(&state, 0, 2, 3) == 1,
                "level 0 reveal survives level transition 0->1");
    expect_true(dm2_v1_world_state_get_explored(&state, 0, 7, 8) == 1,
                "level 0 cell (7,8) reveal survives level transition 0->1");
    expect_true(dm2_v1_world_state_get_explored(&state, 0, 31, 0) == 1,
                "level 0 edge reveal survives level transition 0->1");
    expect_true(dm2_v1_world_state_get_explored(&state, 1, 0, 0) == 0,
                "fresh level 1 starts unexplored after transition");

    /* ── 3. Reveal tiles on level 1 ── */
    dm2_v1_world_state_set_explored(&state, 1, 5, 5, 1);
    dm2_v1_world_state_set_explored(&state, 1, 6, 5, 1);
    expect_true(dm2_v1_world_state_get_explored(&state, 1, 5, 5) == 1,
                "level 1 reveal recorded after transition");

    /* ── 4. Transition current_level 1 -> 5 (multi-floor jump) ── */
    dm2_v1_world_state_set_current_level(&state, 5);
    expect_true(state.current_level == 5,
                "set_current_level accepts a multi-floor jump to 5");
    expect_true(dm2_v1_world_state_get_explored(&state, 0, 2, 3) == 1,
                "level 0 reveal survives multi-floor transition 1->5");
    expect_true(dm2_v1_world_state_get_explored(&state, 1, 5, 5) == 1,
                "level 1 reveal survives multi-floor transition 1->5");
    expect_true(dm2_v1_world_state_get_explored(&state, 5, 0, 0) == 0,
                "level 5 still unexplored after jumping in");

    /* ── 5. Reveal on level 5 then bounce back to level 0 ── */
    dm2_v1_world_state_set_explored(&state, 5, 10, 10, 1);
    dm2_v1_world_state_set_current_level(&state, 0);
    expect_true(state.current_level == 0,
                "set_current_level can return to a previously visited floor");
    expect_true(dm2_v1_world_state_get_explored(&state, 5, 10, 10) == 1,
                "level 5 reveal survives return trip to level 0");
    expect_true(dm2_v1_world_state_get_explored(&state, 0, 2, 3) == 1,
                "level 0 reveal still readable after returning");

    /* ── 6. Out-of-range setter calls are rejected ── */
    sentinel_kept = state.current_level;
    dm2_v1_world_state_set_current_level(&state, -1);
    expect_true(state.current_level == sentinel_kept,
                "negative target level is rejected (no mutation)");
    dm2_v1_world_state_set_current_level(&state,
                                          DM2_WORLD_STATE_MAX_LEVELS);
    expect_true(state.current_level == sentinel_kept,
                "target level at MAX_LEVELS is rejected (no mutation)");
    dm2_v1_world_state_set_current_level(&state,
                                          DM2_WORLD_STATE_MAX_LEVELS + 999);
    expect_true(state.current_level == sentinel_kept,
                "wildly out-of-range target is rejected (no mutation)");
    /* Out-of-range setter must not touch exploration either. */
    expect_true(dm2_v1_world_state_get_explored(&state, 0, 2, 3) == 1,
                "level 0 reveal untouched by rejected setter");
    expect_true(dm2_v1_world_state_get_explored(&state, 5, 10, 10) == 1,
                "level 5 reveal untouched by rejected setter");
    expect_true(test_original_envelope_world_restore(),
                "original SUPPRESS envelope restores world fields transactionally");

    /* ── 7. Save/load round-trip across the transition boundary ── */
    serialized = dm2_v1_world_state_serialize(&state, &serialized_size);
    if (!expect_true(serialized != NULL,
                     "serialize returns a buffer with transition history"))
    {
        fprintf(stderr,
                "DM2 V1 world-state minimap level-transition: %d/%d passed\n",
                g_pass, g_total);
        return 1;
    }
    expect_true(serialized_size > 64,
                "serialized buffer carries exploration extension");

    loaded = dm2_v1_world_state_load_from_mem(serialized, serialized_size);
    if (!expect_true(loaded != NULL,
                     "loaded world-state is non-NULL after round-trip"))
    {
        free(serialized);
        fprintf(stderr,
                "DM2 V1 world-state minimap level-transition: %d/%d passed\n",
                g_pass, g_total);
        return 1;
    }

    /* current_level pointer survives the round-trip. */
    expect_true(loaded->current_level == 0,
                "current_level pointer survives save/load");

    /* Per-level reveal bitmaps survive the round-trip. */
    expect_true(dm2_v1_world_state_get_explored(loaded, 0, 2, 3) == 1,
                "level 0 reveal survives save/load after transitions");
    expect_true(dm2_v1_world_state_get_explored(loaded, 0, 7, 8) == 1,
                "level 0 cell (7,8) reveal survives save/load");
    expect_true(dm2_v1_world_state_get_explored(loaded, 0, 31, 0) == 1,
                "level 0 edge reveal survives save/load");
    expect_true(dm2_v1_world_state_get_explored(loaded, 1, 5, 5) == 1,
                "level 1 reveal survives save/load after transitions");
    expect_true(dm2_v1_world_state_get_explored(loaded, 1, 6, 5) == 1,
                "level 1 second reveal survives save/load");
    expect_true(dm2_v1_world_state_get_explored(loaded, 5, 10, 10) == 1,
                "level 5 reveal survives save/load");
    /* A level we never touched stays unexplored after the round-trip. */
    expect_true(dm2_v1_world_state_get_explored(loaded, 2, 0, 0) == 0,
                "level 2 stays unexplored after save/load round-trip");
    /* Same cell coordinate on a different level is independently tracked. */
    expect_true(dm2_v1_world_state_get_explored(loaded, 4, 2, 3) == 0,
                "level 4 cell (2,3) is unexplored even though level 0 was");

    dm2_v1_world_state_free(loaded);
    free(serialized);

    fprintf(stderr,
            "DM2 V1 world-state minimap level-transition: %d/%d passed\n",
            g_pass, g_total);
    return (g_pass == g_total) ? 0 : 1;
}
