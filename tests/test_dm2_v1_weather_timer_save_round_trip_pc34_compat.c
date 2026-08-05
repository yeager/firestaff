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
 * the source-named weather fields inside the 56-byte game state block and stores
 * per-champion torch + creature + tick-generator timers as 10-byte
 * DM2_TimerEntry records (timer_id, current_tick, interval_ticks, flags,
 * user_data). Both blocks use SUPPRESS bit-plane encoding; this test
 * proves the codec keeps every source-owned weather/timer field intact.
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

/* ── Test 1: exact skload_table_60 weather fields round-trip ─────────── */

static void test_gamestate_weather_fields_round_trip(void)
{
    DM2_GameStateBlock in;
    DM2_GameStateBlock out;
    uint8_t buf[DM2_GAME_STATE_BLOCK_SIZE];
    int enc_n;
    int dec_n;

    /* Source-shaped state. SKProject DME.h::skload_table_60 stores these
     * individual weather fields at bytes 40..55; it does not contain a
     * host-side rain_state[8] array. */
    memset(&in, 0, sizeof(in));
    in.dwGameTick        = 0x00345678u;
    in.dwRandomSeed      = 0x00000100u;
    in.wChampionsCount   = 4;
    in.wPlayerPosX       = 5;
    in.wPlayerPosY       = 7;
    in.wPlayerDir        = 1;  /* East */
    in.wPlayerMap        = 2;
    in.wChampionLeader   = 0;
    in.wTimersCount      = 3;
    in.dw22              = 0x12345678u;
    in.dw26              = 0x00030007u;
    in.w30               = 7;
    in.wPlayerThrowCounter = 3;
    in.w34               = 1;
    in.b36               = 1;
    in.b38               = 0x5a;
    in.wRainFlagSomething = 1;
    in.bRainAmbientLightModifier = 1;
    in.bRainDirection    = 3;
    in.bRainStrength     = 80;
    in.bRainLevelForSky  = 0x4d;
    in.bRainLevelForGround = 0x37;
    in.bRainMultiplicator = 3;
    in.wRainStormController = 0x001f;
    in.bRainRelated3     = 0x13;
    in.bRainRelated2     = 2;
    in.dwRainSpecialNextTick = 0x00123456u;

    enc_n = dm2_suppress_encode_gamestate(&in, buf, sizeof(buf));
    CHECK(enc_n > 0, "encode produces bytes");
    CHECK((size_t)enc_n < sizeof(buf), "encode fits in 56-byte buffer");

    memset(&out, 0xAA, sizeof(out));
    dec_n = dm2_suppress_decode_gamestate(buf, (size_t)enc_n, &out, 0);
    CHECK(dec_n > 0, "decode consumes bytes");
    CHECK(dec_n == enc_n, "encode/decode byte count matches (round-trip is lossless)");

    /* _4976_395a selects distinct source bits per field; it is not a generic
     * seven-bit-byte codec. These fixtures stay inside each field's mask. */
    CHECK(out.dwGameTick == in.dwGameTick,
          "dwGameTick source-mask round-trip");
    CHECK(out.dwRandomSeed == in.dwRandomSeed,
          "dwRandomSeed source-mask round-trip");
    CHECK(out.wChampionsCount == in.wChampionsCount,
          "wChampionsCount round-trip");
    CHECK(out.wPlayerPosX == in.wPlayerPosX,
          "wPlayerPosX round-trip");
    CHECK(out.wPlayerPosY == in.wPlayerPosY,
          "wPlayerPosY round-trip");
    CHECK(out.wPlayerDir == in.wPlayerDir,
          "wPlayerDir round-trip");
    CHECK(out.wPlayerMap == in.wPlayerMap,
          "wPlayerMap round-trip");
    CHECK(out.wChampionLeader == in.wChampionLeader,
          "wChampionLeader round-trip");
    CHECK(out.wTimersCount == in.wTimersCount,
          "wTimersCount round-trip");

    CHECK(out.dw22 == in.dw22 && out.dw26 == in.dw26 && out.w30 == in.w30 &&
          out.wPlayerThrowCounter == in.wPlayerThrowCounter && out.w34 == in.w34,
          "source game-state fields at bytes 22..35 round-trip");
    CHECK(out.b36 == in.b36 && out.b38 == in.b38 &&
          out.wRainFlagSomething == in.wRainFlagSomething &&
          out.bRainAmbientLightModifier == in.bRainAmbientLightModifier &&
          out.bRainDirection == in.bRainDirection &&
          out.bRainStrength == in.bRainStrength &&
          out.bRainLevelForSky == in.bRainLevelForSky &&
          out.bRainLevelForGround == in.bRainLevelForGround &&
          out.bRainMultiplicator == in.bRainMultiplicator &&
          out.wRainStormController == in.wRainStormController &&
          out.bRainRelated3 == in.bRainRelated3 && out.bRainRelated2 == in.bRainRelated2 &&
          out.dwRainSpecialNextTick == in.dwRainSpecialNextTick,
          "source weather fields at bytes 40..55 round-trip");
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
    gs_in.bRainDirection = 3;
    gs_in.bRainStrength = 80;
    gs_in.bRainLevelForSky = 3;
    gs_in.bRainLevelForGround = 40;

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
    CHECK((gs_out.dwGameTick & 0x00ffffffu) ==
          (gs_in.dwGameTick & 0x00ffffffu),
          "stream fixture game tick round-trips");
    CHECK((gs_out.dwRandomSeed & 0x0000ffffu) ==
          (gs_in.dwRandomSeed & 0x0000ffffu),
          "stream fixture random/weather seed round-trips");
    CHECK(gs_out.wTimersCount == gs_in.wTimersCount,
          "stream fixture timer count round-trips");
    CHECK(gs_out.bRainDirection == gs_in.bRainDirection &&
          gs_out.bRainStrength == gs_in.bRainStrength,
          "stream fixture rain direction/strength round-trip");

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
    /* This is a decoded source-state fixture, not a startup default.
     * Folding its supplied seed into dwRandomSeed lets the next outdoor tick
     * re-derive the same weather state after load. */
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
    dm2_v1_weather_set(&state_pre, DM2_WEATHER_CLEAR);
    expected_pre = dm2_v1_weather_advance_seed(seed_in);
    w_pre = (int)((expected_pre >> 8) & 0x3u);

    /* Pack into gamestate dwRandomSeed (low 16 bits carry the LCG state). */
    memset(&gs, 0, sizeof(gs));
    gs.dwRandomSeed = seed_in;
    gs.bRainStrength = (uint8_t)state_pre.weather_intensity;
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
    CHECK(gs.bRainStrength == (uint8_t)state_pre.weather_intensity,
          "rain strength byte in gamestate survives round-trip");
}

/* ── Test 6: skproject weather timer transaction ──────────────────── */

static void test_set_timer_weather_source_transaction(void)
{
    const uint32_t seed = 0x00002d2du;
    const uint32_t expected_seed = dm2_v1_weather_advance_seed(seed);
    const uint8_t expected_weather = (uint8_t)((expected_seed >> 8) & 0x3u);
    DM2_V1_WeatherState state;
    DM2_V1_WeatherTimerReceipt receipt;

    dm2_v1_weather_init(&state);
    dm2_v1_weather_set_seed(&state, seed);
    /* A source-loaded chain has both the saved selector and its seed. */
    dm2_v1_weather_set(&state, DM2_WEATHER_CLEAR);

    CHECK(dm2_v1_weather_set_timer_weather(&state, 0,
                                            DM2_WEATHER_TIMER_INTERVAL_TICKS,
                                            &receipt) == 0,
          "DM2_SET_TIMER_WEATHER rejects indoor transition");
    CHECK(receipt.valid && receipt.source_set_timer_weather &&
          !receipt.source_weather_3df7_0037 && !receipt.due &&
          receipt.seed_before == seed && receipt.seed_after == seed,
          "indoor weather timer receipt preserves source seed");

    CHECK(dm2_v1_weather_set_timer_weather(
              &state, 1, DM2_WEATHER_TIMER_INTERVAL_TICKS - 1u,
              &receipt) == 0,
          "DM2_SET_TIMER_WEATHER waits for exact interval tick");
    CHECK(receipt.valid && receipt.outdoor && !receipt.due &&
          receipt.interval_ticks == DM2_WEATHER_TIMER_INTERVAL_TICKS &&
          receipt.seed_after == seed,
          "pre-interval outdoor receipt is non-mutating");

    CHECK(dm2_v1_weather_set_timer_weather(
              &state, 1, DM2_WEATHER_TIMER_INTERVAL_TICKS,
              &receipt) == 1,
          "DM2_SET_TIMER_WEATHER dispatches weather_3df7_0037");
    CHECK(receipt.valid && receipt.source_set_timer_weather &&
          receipt.source_weather_3df7_0037 && receipt.due &&
          receipt.seed_before == seed &&
          receipt.seed_after == expected_seed &&
          receipt.weather_after == expected_weather &&
          receipt.intensity_after ==
              (uint8_t)state.weather_intensity &&
          receipt.transaction_hash != 0u,
          "weather timer receipt binds skproject transition fields");
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
    CHECK(save_load_ev && strstr(save_load_ev, "SKSave") != NULL,
          "save source evidence cites the original SKSave format");

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

    printf("--- test_gamestate_weather_fields_round_trip ---\n");
    test_gamestate_weather_fields_round_trip();

    printf("\n--- test_timer_entry_round_trip ---\n");
    test_timer_entry_round_trip();

    printf("\n--- test_timer_table_covers_all_timer_ids ---\n");
    test_timer_table_covers_all_timer_ids();

    printf("\n--- test_gamestate_timer_stream_fixture_round_trip ---\n");
    test_gamestate_timer_stream_fixture_round_trip();

    printf("\n--- test_weather_seed_persists_via_gamestate_round_trip ---\n");
    test_weather_seed_persists_via_gamestate_round_trip();

    printf("\n--- test_set_timer_weather_source_transaction ---\n");
    test_set_timer_weather_source_transaction();

    printf("\n--- test_save_source_evidence_strings ---\n");
    test_save_source_evidence_strings();

    printf("\nPASSED: %d\nFAILED: %d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
