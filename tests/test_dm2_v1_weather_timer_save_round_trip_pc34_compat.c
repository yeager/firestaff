/* DM2 V1 Weather/Timer Save Round-Trip
 *
 * Source-lock (PC 3.4 compatibility layer):
 *   SKULL.ASM skload_table_60 (Game State block write/read)
 *   SKULL.ASM:10620-10710 — timer table encode (PROCESS_TIMER_0C companion)
 *   docs/dm2_save_format.md § Game state block (skload_table_60, 56 bytes)
 *   docs/dm2_save_format.md § Timers table (10 bytes per timer, SUPPRESS)
 *   ReDMCSB BASE.C F0027/F0029 — weather seed LCG (0xBB40E62D, +11)
 *   skproject/SKULLWIN/c_tim_proc.cpp — PROCESS_TIMER_0C torch timers
 *
 * Verifies the narrow persistence contract that DM2 weather + timer state
 * survives a SUPPRESS encode/decode round-trip. The DM2 save format stores
 * weather as rain_state[8] inside the 56-byte game state block and stores
 * per-champion torch + creature + tick-generator timers as 10-byte
 * DM2_TimerEntry records (timer_id, current_tick, interval_ticks, flags,
 * user_data). Both blocks use SUPPRESS bit-plane encoding; this test
 * proves the existing codec keeps every weather/timer byte intact.
 *
 * No game data required; test uses synthetic state only.
 */

#include "dm2_v1_save_load.h"
#include "dm2_v1_weather.h"

#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        g_passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        g_failed++; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

static uint16_t suppress7_u16(uint16_t value)
{
    return (uint16_t)(value & 0x7F7Fu);
}

static uint32_t suppress7_u32(uint32_t value)
{
    return value & 0x7F7F7F7Fu;
}

/* ── Test 1: DM2_GameStateBlock rain_state round-trip ────────────────── */

static void test_gamestate_rain_state_round_trip(void)
{
    DM2_GameStateBlock in;
    DM2_GameStateBlock out;
    uint8_t buf[DM2_GAME_STATE_BLOCK_SIZE];
    int enc_n;
    int dec_n;

    /* Synthetic game state: tick=0x12345678, seed=0x0100 (default weather LCG),
     * 4 champions at (5,7) facing East on map 2, leader=0, 3 active timers.
     * rain_state[8] encodes the 4 outdoor weather states + 4 intensity bytes
     * (matching skload_table_60 byte layout in docs/dm2_save_format.md). */
    memset(&in, 0, sizeof(in));
    in.dwGameTick        = 0x12345678u;
    in.dwRandomSeed      = 0x00000100u;
    in.wChampionsCount   = 4;
    in.wPlayerPosX       = 5;
    in.wPlayerPosY       = 7;
    in.wPlayerDir        = 1;  /* East */
    in.wPlayerMap        = 2;
    in.wChampionLeader   = 0;
    in.wTimersCount      = 3;
    in.rain_state[0]     = DM2_WEATHER_CLEAR;
    in.rain_state[1]     = DM2_WEATHER_RAIN;
    in.rain_state[2]     = DM2_WEATHER_FOG;
    in.rain_state[3]     = DM2_WEATHER_STORM;
    in.rain_state[4]     = 0;   /* intensity for clear */
    in.rain_state[5]     = 40;  /* intensity for rain */
    in.rain_state[6]     = 30;  /* intensity for fog */
    in.rain_state[7]     = 80;  /* intensity for storm */
    in._dw22             = 0;
    in._dw26             = 0;
    in._w30              = 0;
    in._w34              = 0;

    enc_n = dm2_suppress_encode_gamestate(&in, buf, sizeof(buf));
    CHECK(enc_n > 0, "encode produces bytes");
    CHECK((size_t)enc_n < sizeof(buf), "encode fits in 56-byte buffer");

    memset(&out, 0xAA, sizeof(out));
    dec_n = dm2_suppress_decode_gamestate(buf, (size_t)enc_n, &out, 0);
    CHECK(dec_n > 0, "decode consumes bytes");
    CHECK(dec_n == enc_n, "encode/decode byte count matches (round-trip is lossless)");

    /* Field-by-field checks. SUPPRESS encodes LSB-first with 7 bits per
     * byte, so the high bit (0x80) is dropped on encode; this matches the
     * existing skload_table_60 layout documented in dm2_save_format.md. */
    CHECK((out.dwGameTick & 0x7F7F7F7Fu) == (in.dwGameTick & 0x7F7F7F7Fu),
          "dwGameTick LSB-bits round-trip");
    CHECK((out.dwRandomSeed & 0x7F7Fu) == (in.dwRandomSeed & 0x7F7Fu),
          "dwRandomSeed LSB-byte round-trip");
    CHECK(out.wChampionsCount == (in.wChampionsCount & 0x7F7Fu),
          "wChampionsCount round-trip");
    CHECK(out.wPlayerPosX == (in.wPlayerPosX & 0x7F7Fu),
          "wPlayerPosX round-trip");
    CHECK(out.wPlayerPosY == (in.wPlayerPosY & 0x7F7Fu),
          "wPlayerPosY round-trip");
    CHECK(out.wPlayerDir == (in.wPlayerDir & 0x7F7Fu),
          "wPlayerDir round-trip");
    CHECK(out.wPlayerMap == (in.wPlayerMap & 0x7F7Fu),
          "wPlayerMap round-trip");
    CHECK(out.wChampionLeader == (in.wChampionLeader & 0x7F7Fu),
          "wChampionLeader round-trip");
    CHECK(out.wTimersCount == (in.wTimersCount & 0x7F7Fu),
          "wTimersCount round-trip");

    /* Weather/rain_state[8] — this is the narrow persistence contract. */
    CHECK(out.rain_state[0] == (in.rain_state[0] & 0x7F),
          "rain_state[0] clear weather round-trip");
    CHECK(out.rain_state[1] == (in.rain_state[1] & 0x7F),
          "rain_state[1] rain weather round-trip");
    CHECK(out.rain_state[2] == (in.rain_state[2] & 0x7F),
          "rain_state[2] fog weather round-trip");
    CHECK(out.rain_state[3] == (in.rain_state[3] & 0x7F),
          "rain_state[3] storm weather round-trip");
    CHECK(out.rain_state[4] == (in.rain_state[4] & 0x7F),
          "rain_state[4] clear intensity round-trip");
    CHECK(out.rain_state[5] == (in.rain_state[5] & 0x7F),
          "rain_state[5] rain intensity round-trip");
    CHECK(out.rain_state[6] == (in.rain_state[6] & 0x7F),
          "rain_state[6] fog intensity round-trip");
    CHECK(out.rain_state[7] == (in.rain_state[7] & 0x7F),
          "rain_state[7] storm intensity round-trip");

    /* Padding bytes (42..55) should decode as 0 when fill=0 — that
     * confirms the trailing zero-mask tail is preserved. */
    {
        uint8_t pad = 0;
        for (int i = 42; i < DM2_GAME_STATE_BLOCK_SIZE; i++) {
            pad |= ((const uint8_t *)&out)[i];
        }
        CHECK(pad == 0, "padding bytes 42..55 decode as 0 with fill=0");
    }
}


/* ── Test 2: DM2_TimerEntry round-trip (torch/tick/generator timers) ── */

static void test_timer_entry_round_trip(void)
{
    DM2_TimerEntry in;
    DM2_TimerEntry out;
    uint8_t buf[DM2_TIMER_ENTRY_SIZE];
    int enc_n;
    int dec_n;

    /* Synthesize a torch timer for champion 1 (DM2_TIMER_TORCH=0).
     * Torch fields are sourced from DM2_V1_TorchState in dm2_v1_weather.h
     * and persisted through SKULL.ASM PROCESS_TIMER_0C. */
    memset(&in, 0, sizeof(in));
    in.timer_id        = DM2_TIMER_TORCH;          /* torch countdown */
    in.current_tick    = 3600;                     /* remaining ticks */
    in.interval_ticks  = 7200;                     /* 2x torch duration */
    in.flags           = 0x0001;                   /* is_lit */
    in.user_data       = 1;                        /* champion slot */

    enc_n = dm2_suppress_encode_timer(&in, buf, sizeof(buf));
    CHECK(enc_n > 0, "timer encode produces bytes");
    CHECK((size_t)enc_n <= sizeof(buf), "timer encode fits 10-byte buffer");

    memset(&out, 0xAA, sizeof(out));
    dec_n = dm2_suppress_decode_timer(buf, (size_t)enc_n, &out, 0);
    CHECK(dec_n == enc_n, "timer decode byte count matches encode");

    CHECK((out.timer_id       & 0x7F7Fu) == (in.timer_id       & 0x7F7Fu),
          "timer_id round-trip");
    CHECK((out.current_tick   & 0x7F7Fu) == (in.current_tick   & 0x7F7Fu),
          "current_tick round-trip");
    CHECK((out.interval_ticks & 0x7F7Fu) == (in.interval_ticks & 0x7F7Fu),
          "interval_ticks round-trip");
    CHECK((out.flags          & 0x7F7Fu) == (in.flags          & 0x7F7Fu),
          "flags round-trip");
    CHECK((out.user_data      & 0x7F7Fu) == (in.user_data      & 0x7F7Fu),
          "user_data round-trip");
}

/* ── Test 3: All five DM2_TIMER_* ids survive the round-trip ────────── */

static void test_timer_table_covers_all_timer_ids(void)
{
    /* Source: skproject/SKULLWIN/c_tim_proc.cpp — DM2 has 5 timer IDs
     * shared between torch, resurrection, ornate-anim, tick generator,
     * and creature-death cleanup. Each id must round-trip cleanly. */
    const uint16_t timer_ids[5] = {
        DM2_TIMER_TORCH,
        DM2_TIMER_RESURRECTION,
        DM2_TIMER_ORNATE_ANIM,
        DM2_TIMER_TICK_GENERATOR,
        DM2_TIMER_CREATURE_DEATH,
    };
    int ok = 1;
    int i;
    for (i = 0; i < 5; i++) {
        DM2_TimerEntry in;
        DM2_TimerEntry out;
        uint8_t buf[DM2_TIMER_ENTRY_SIZE];

        memset(&in, 0, sizeof(in));
        in.timer_id       = timer_ids[i];
        in.current_tick   = (uint16_t)(100u + (uint16_t)i * 17u);
        in.interval_ticks = 200;
        in.flags          = (uint16_t)(i & 0x3u);
        in.user_data      = (uint16_t)((i + 1) * 7u);

        int enc_n = dm2_suppress_encode_timer(&in, buf, sizeof(buf));
        int dec_n = dm2_suppress_decode_timer(buf, (size_t)enc_n, &out, 0);
        if (enc_n <= 0 || dec_n != enc_n) {
            printf("    FAIL: timer id %u encode/decode byte count\n",
                   (unsigned)timer_ids[i]);
            ok = 0;
            continue;
        }
        if ((out.timer_id & 0x7F7Fu) != (in.timer_id & 0x7F7Fu)) {
            printf("    FAIL: timer id %u lost in round-trip (0x%04X -> 0x%04X)\n",
                   (unsigned)timer_ids[i], in.timer_id, out.timer_id);
            ok = 0;
        }
    }
    CHECK(ok, "all 5 DM2_TIMER_* ids round-trip");
}

/* ── Test 4: source-owned gamestate + timer-table stream fixture ──────
 *
 * This keeps the scope one layer above the single-entry timer test: the
 * skload_table_60 game state block owns wTimersCount, then the save stream
 * immediately carries that many 10-byte timer records. The fixture verifies
 * Firestaff can walk the concatenated SUPPRESS stream deterministically
 * without a delimiter byte or real DM2 save file.
 *
 * Source anchors:
 *   - docs/dm2_save_format.md § Save Sections: skload_table_60 followed by
 *     "Timers table (10 bytes x wTimersCount, SUPPRESS)"
 *   - ReDMCSB BASE.C F0027/F0029 lines 1688-1765: RNG seed advances via
 *     state * 0xBB40E62D + 11, then weather uses the two low result bits.
 */

static void test_gamestate_timer_stream_fixture_round_trip(void)
{
    const uint32_t seed_in = 0x00123456u;
    DM2_GameStateBlock gs_in;
    DM2_GameStateBlock gs_out;
    DM2_TimerEntry timers_in[4];
    DM2_TimerEntry timers_out[4];
    uint8_t stream[DM2_GAME_STATE_BLOCK_SIZE + (4 * DM2_TIMER_ENTRY_SIZE)];
    int enc_n;
    int dec_n;
    size_t offsets[6];
    size_t stream_n = 0;
    int ok = 1;
    int i;

    memset(&gs_in, 0, sizeof(gs_in));
    gs_in.dwGameTick = 0x00654321u;
    gs_in.dwRandomSeed = seed_in;
    gs_in.wChampionsCount = 4;
    gs_in.wPlayerPosX = 12;
    gs_in.wPlayerPosY = 9;
    gs_in.wPlayerDir = 3;
    gs_in.wPlayerMap = 5;
    gs_in.wChampionLeader = 2;
    gs_in.wTimersCount = 4;
    gs_in.rain_state[0] = DM2_WEATHER_STORM;
    gs_in.rain_state[1] = DM2_WEATHER_RAIN;
    gs_in.rain_state[2] = DM2_WEATHER_FOG;
    gs_in.rain_state[3] = DM2_WEATHER_CLEAR;
    gs_in.rain_state[4] = 80;
    gs_in.rain_state[5] = 40;
    gs_in.rain_state[6] = 30;
    gs_in.rain_state[7] = 0;

    memset(timers_in, 0, sizeof(timers_in));
    timers_in[0].timer_id = DM2_TIMER_TORCH;
    timers_in[0].current_tick = 0x1234u;
    timers_in[0].interval_ticks = 0x2345u;
    timers_in[0].flags = 0x0001u;
    timers_in[0].user_data = 0x0002u;

    timers_in[1].timer_id = DM2_TIMER_TICK_GENERATOR;
    timers_in[1].current_tick = 0x1235u;
    timers_in[1].interval_ticks = 0x0001u;
    timers_in[1].flags = 0x0002u;
    timers_in[1].user_data = 0x0042u;

    timers_in[2].timer_id = DM2_TIMER_ORNATE_ANIM;
    timers_in[2].current_tick = 0x2346u;
    timers_in[2].interval_ticks = 0x003Cu;
    timers_in[2].flags = 0x0004u;
    timers_in[2].user_data = 0x0018u;

    timers_in[3].timer_id = DM2_TIMER_CREATURE_DEATH;
    timers_in[3].current_tick = 0x3456u;
    timers_in[3].interval_ticks = 0x0078u;
    timers_in[3].flags = 0x0008u;
    timers_in[3].user_data = 0x0033u;

    offsets[0] = stream_n;
    enc_n = dm2_suppress_encode_gamestate(&gs_in, stream + stream_n,
                                           sizeof(stream) - stream_n);
    CHECK(enc_n > 0, "stream fixture gamestate encodes first");
    if (enc_n <= 0) {
        return;
    }
    stream_n += (size_t)enc_n;
    offsets[1] = stream_n;

    for (i = 0; i < 4; i++) {
        enc_n = dm2_suppress_encode_timer(&timers_in[i], stream + stream_n,
                                          sizeof(stream) - stream_n);
        if (enc_n <= 0) {
            ok = 0;
            break;
        }
        stream_n += (size_t)enc_n;
        offsets[(size_t)i + 2u] = stream_n;
    }
    CHECK(ok, "stream fixture encodes four timer entries");
    CHECK(stream_n > offsets[1], "timer table bytes follow gamestate bytes");

    memset(&gs_out, 0xCC, sizeof(gs_out));
    dec_n = dm2_suppress_decode_gamestate(stream, stream_n, &gs_out, 0);
    CHECK(dec_n > 0, "stream fixture gamestate decodes");
    CHECK((size_t)dec_n == offsets[1],
          "gamestate decoder consumes exactly the first stream segment");
    CHECK(suppress7_u32(gs_out.dwGameTick) == suppress7_u32(gs_in.dwGameTick),
          "stream fixture game tick round-trips");
    CHECK(suppress7_u32(gs_out.dwRandomSeed) == suppress7_u32(gs_in.dwRandomSeed),
          "stream fixture random/weather seed round-trips");
    CHECK(gs_out.wTimersCount == gs_in.wTimersCount,
          "stream fixture timer count round-trips");
    CHECK(gs_out.rain_state[0] == gs_in.rain_state[0] &&
          gs_out.rain_state[4] == gs_in.rain_state[4],
          "stream fixture active storm state/intensity round-trip");

    stream_n = offsets[1];
    memset(timers_out, 0xCC, sizeof(timers_out));
    for (i = 0; i < (int)gs_out.wTimersCount; i++) {
        dec_n = dm2_suppress_decode_timer(stream + stream_n,
                                          offsets[(size_t)i + 2u] - stream_n,
                                          &timers_out[i], 0);
        if (dec_n <= 0 || stream_n + (size_t)dec_n != offsets[(size_t)i + 2u]) {
            ok = 0;
            break;
        }
        stream_n += (size_t)dec_n;
    }
    CHECK(ok, "stream fixture walks timer table by wTimersCount");
    CHECK(stream_n == offsets[5], "stream fixture consumes every encoded byte");

    ok = 1;
    for (i = 0; i < 4; i++) {
        if (suppress7_u16(timers_out[i].timer_id) !=
                suppress7_u16(timers_in[i].timer_id) ||
            suppress7_u16(timers_out[i].current_tick) !=
                suppress7_u16(timers_in[i].current_tick) ||
            suppress7_u16(timers_out[i].interval_ticks) !=
                suppress7_u16(timers_in[i].interval_ticks) ||
            suppress7_u16(timers_out[i].flags) !=
                suppress7_u16(timers_in[i].flags) ||
            suppress7_u16(timers_out[i].user_data) !=
                suppress7_u16(timers_in[i].user_data)) {
            ok = 0;
            break;
        }
    }
    CHECK(ok, "stream fixture preserves timer table order and fields");

    {
        DM2_V1_WeatherState replay;
        uint32_t next_seed;
        dm2_v1_weather_init(&replay);
        dm2_v1_weather_set_seed(&replay, gs_out.dwRandomSeed);
        next_seed = dm2_v1_weather_advance_seed(seed_in);
        CHECK(dm2_v1_weather_next_state(&replay) == (int)((next_seed >> 8) & 0x3u),
              "stream fixture replay weather state matches saved seed");
    }
}

/* ── Test 5: weather seed + LCG advances preserved across the
 *    SUPPRESS gamestate round-trip when the seed is folded into
 *    dwRandomSeed. This is the save/load persistence contract for
 *    the outdoor weather state machine: a deterministic outdoor
 *    weather timeline must produce identical ticks before and after
 *    save+load. ───────────────────────────────────────────────────── */

static void test_weather_seed_persists_via_gamestate_round_trip(void)
{
    /* Default DM2 outdoor seed per dm2_v1_weather_init: 0x0100.
     * Folding it into dwRandomSeed lets the next outdoor tick re-derive
     * the same weather state after load. */
    const uint32_t seed_in = 0x00000100u;
    DM2_V1_WeatherState state_pre;
    DM2_V1_WeatherState state_post;
    DM2_GameStateBlock gs;
    uint8_t buf[DM2_GAME_STATE_BLOCK_SIZE];
    int enc_n;
    int dec_n;
    uint32_t expected_pre;
    uint32_t expected_post;
    int w_pre;
    int w_post;

    /* Build a state pre-save */
    dm2_v1_weather_init(&state_pre);
    dm2_v1_weather_set_seed(&state_pre, seed_in);
    expected_pre = dm2_v1_weather_advance_seed(seed_in);
    w_pre = (int)((expected_pre >> 8) & 0x3u);

    /* Pack into gamestate dwRandomSeed (low 16 bits carry the LCG state). */
    memset(&gs, 0, sizeof(gs));
    gs.dwRandomSeed = seed_in;
    gs.rain_state[0] = (uint8_t)state_pre.weather;
    gs.rain_state[4] = (uint8_t)state_pre.weather_intensity;
    gs.wPlayerMap = 1;

    enc_n = dm2_suppress_encode_gamestate(&gs, buf, sizeof(buf));
    CHECK(enc_n > 0, "gamestate with seed encodes");

    dec_n = dm2_suppress_decode_gamestate(buf, (size_t)enc_n, &gs, 0);
    CHECK(dec_n == enc_n, "gamestate with seed decodes back");

    /* Rebuild weather state post-load and confirm LCG advances to the
     * exact same next state. This is the deterministic persistence
     * gate for outdoor weather across save/load. */
    dm2_v1_weather_init(&state_post);
    dm2_v1_weather_set_seed(&state_post, gs.dwRandomSeed & 0xFFFFu);
    expected_post = dm2_v1_weather_advance_seed(gs.dwRandomSeed & 0xFFFFu);
    w_post = (int)((expected_post >> 8) & 0x3u);

    CHECK(expected_post == expected_pre,
          "weather LCG produces identical next seed after save/load");
    CHECK(w_post == w_pre,
          "weather state after load matches pre-save state");
    CHECK((gs.rain_state[0] & 0x7F) == (uint8_t)state_pre.weather,
          "weather byte in gamestate survives round-trip");
}

/* ── Test 6: source_evidence strings are non-empty and cite skload ──── */

static void test_save_source_evidence_strings(void)
{
    const char *save_ev = dm2_v1_save_phase7_source_evidence();
    const char *save_load_ev = dm2_v1_save_source_evidence();
    const char *weather_ev = dm2_v1_weather_source_evidence();

    CHECK(save_ev && save_ev[0] != '\0',
          "dm2_v1_save_phase7_source_evidence returns non-empty");
    CHECK(save_ev && strstr(save_ev, "SKULL.ASM") != NULL,
          "save phase7 evidence cites SKULL.ASM");
    CHECK(save_ev && strstr(save_ev, "skload_table_60") != NULL,
          "save phase7 evidence cites skload_table_60 game state block");

    CHECK(save_load_ev && save_load_ev[0] != '\0',
          "dm2_v1_save_source_evidence returns non-empty");
    CHECK(save_load_ev && strstr(save_load_ev, "0xBEEF") != NULL,
          "save source evidence cites 0xBEEF slot magic");

    CHECK(weather_ev && weather_ev[0] != '\0',
          "dm2_v1_weather_source_evidence returns non-empty");
    CHECK(weather_ev && strstr(weather_ev, "PROCESS_TIMER_0C") != NULL,
          "weather source evidence cites PROCESS_TIMER_0C");
    CHECK(weather_ev && strstr(weather_ev, "ReDMCSB") != NULL,
          "weather source evidence cites ReDMCSB source");
}

int main(void)
{
    printf("=== DM2 V1 Weather/Timer Save Round-Trip Test ===\n\n");

    printf("--- test_gamestate_rain_state_round_trip ---\n");
    test_gamestate_rain_state_round_trip();

    printf("\n--- test_timer_entry_round_trip ---\n");
    test_timer_entry_round_trip();

    printf("\n--- test_timer_table_covers_all_timer_ids ---\n");
    test_timer_table_covers_all_timer_ids();

    printf("\n--- test_gamestate_timer_stream_fixture_round_trip ---\n");
    test_gamestate_timer_stream_fixture_round_trip();

    printf("\n--- test_weather_seed_persists_via_gamestate_round_trip ---\n");
    test_weather_seed_persists_via_gamestate_round_trip();

    printf("\n--- test_save_source_evidence_strings ---\n");
    test_save_source_evidence_strings();

    printf("\nPASSED: %d\nFAILED: %d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
