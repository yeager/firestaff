/* test_dm2_v1_sound_queue_pc34_compat.c — DM2-008 verification.
 *
 * Verifies the source-ordered runtime sound queue against
 * skproject/SKULLWIN c_sound.cpp / c_sfx.cpp:
 *   - DM2_SOUND9 / DM2_QUERY_SND_ENTRY_INDEX queue order (1-based)
 *   - DM2_QUEUE_NOISE_GEN1 gates: map, caps, query, unresolved sample,
 *     change detection, facing rotation, R_1FB7D occlusion clamp
 *   - DM2_QUEUE_NOISE_GEN2 class remap
 *   - R_928 metric vectors (incl. negative x, y == 0 branches)
 *   - DM2_PLAY_SOUND bubble-sort permutation + 64-slot scan early return
 *   - DM2_SOUND8 flush order
 *   - delayed path: 8-slot scan, type-0x15 timer receipted pending
 *   - unavailable audio is explicit, never simulated
 */

#include "dm2_v1_sound_queue_pc34_compat.h"

#include <assert.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static int16_t probe_far(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return 4;
}

static int16_t probe_near(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return 0;
}

static int16_t probe_blocked(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return -1;
}

static void make_env(DM2_V1_SoundQueueEnv *env)
{
    memset(env, 0, sizeof(*env));
    env->current_map = 2;
    env->gate_map_a = 2;
    env->gate_map_b = 5;
    env->facing = 0;
    env->party_x = 10;
    env->party_y = 10;
    env->gametick = 1000;
}

int main(void)
{
    DM2_V1_SoundQueueState st;
    DM2_V1_SoundQueueEnv env;
    DM2_V1_SoundQueueReceipt r;
    (void)r;
    DM2_V1_SoundPlayReceipt pr;
    (void)pr;
    uint16_t idx;
    (void)idx;

    /* ── init order (c_sound::init + DM2_SOUND6) ── */
    dm2_v1_sound_queue_state_init(&st, 4);
    assert(st.ssound_capacity == 4);
    assert(st.ssound_count == 0);
    assert(st.positional_count == 0 && st.immediate_count == 0);
    assert(st.sound_enabled == 1 && st.master_sfx_volume == 7);
    assert(st.ssound[0].w_05 == -1);
    assert(st.sample_slots[63] == -1);
    make_env(&env);

    /* ── DM2_SOUND9 + QUERY (c_sound.cpp:650-673) ── */
    assert(dm2_v1_sound_queue_sound9(&st, 1, 2, 3, 7, &idx) == 1);
    assert(idx == 1);
    assert(dm2_v1_sound_queue_query_entry_index(&st, 1, 2, 3) == 1);
    assert(dm2_v1_sound_queue_query_entry_index(&st, 1, 2, 4) == 0);
    assert(dm2_v1_sound_queue_sound9(&st, 1, 2, 3, 7, &idx) == 0); /* dup */
    assert(dm2_v1_sound_queue_sound9(&st, 4, 5, 6, -1, &idx) == 1);
    assert(dm2_v1_sound_queue_sound9(&st, 7, 8, 9, 11, &idx) == 1);
    assert(dm2_v1_sound_queue_sound9(&st, 3, 1, 4, 12, &idx) == 1);
    assert(dm2_v1_sound_queue_sound9(&st, 5, 5, 5, 13, &idx) == 0); /* full */

    /* Model only the postcondition of c_gdatfile.cpp::DM2_482b_0684 for
     * the source-ordered GEN1 cases below. DM2_SOUND9 must not create this
     * binding. The data-free queue test has no authenticated DYN4 owner. */
    st.ssound[0].w_00 = 7;
    st.ssound[0].w_05 = 0x1234;
    st.ssound[2].w_00 = 11;
    st.ssound[2].w_05 = 0x1235;
    st.ssound[3].w_00 = 12;
    st.ssound[3].w_05 = 0x1236;

    /* ── GEN1 gate order (c_sfx.cpp:156-169) ── */
    /* map gate: delay > 0 with foreign map rejects before anything else */
    env.current_map = 9;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 0, 10, 11, 12, 1,
                                         &env, &r) == 0);
    assert(r.rejected_map_gate);
    env.current_map = 2;

    /* query miss */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 9, 9, 9, 0, 10, 11, 12, 1,
                                         &env, &r) == 0);
    assert(r.rejected_query_miss);

    /* unresolved sample: (4,5,6) queued with w_00 == -1 */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 4, 5, 6, 0, 10, 11, 12, 1,
                                         &env, &r) == 0);
    assert(r.rejected_sample_unresolved);

    /* facing > 3: source leaves vw_18/vw_1c uninitialised; fail-closed */
    env.facing = 4;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 0, 10, 11, 12, 1,
                                         &env, &r) == 0);
    assert(r.rejected_facing_unproven);
    env.facing = 0;

    /* distance > 1 without an occlusion probe: fail-closed */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 0, 10, 12, 13, 1,
                                         &env, &r) == 0);
    assert(r.rejected_occlusion_unavailable);

    /* occlusion probe blocked (wordrg18 < 0) */
    env.occlusion_probe = probe_blocked;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 0, 10, 12, 13, 1,
                                         &env, &r) == 0);
    assert(r.rejected_occlusion_blocked);

    /* ── facing rotation, all four cases (c_sfx.cpp:183-207) ── */
    env.occlusion_probe = probe_near; /* no rescale: 0 > dist is false */
    /* case 0: vw_18 = x-px, vw_1c = py-y */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 3, 10, 12, 13, 1,
                                         &env, &r) == 1);
    assert(r.queued_positional && r.x == 2 && r.y == -3);
    assert(r.priority == 3 && r.volume == 10 && r.sample_id == 7);
    /* case 1: vw_18 = y-py, vw_1c = x-px */
    env.facing = 1;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 3, 10, 12, 13, 1,
                                         &env, &r) == 1);
    assert(r.x == 3 && r.y == 2);
    /* case 2: vw_18 = px-x, vw_1c = y-py */
    env.facing = 2;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 3, 10, 12, 13, 1,
                                         &env, &r) == 1);
    assert(r.x == -2 && r.y == 3);
    /* case 3: vw_18 = py-y, vw_1c = px-x */
    env.facing = 3;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 3, 10, 12, 13, 1,
                                         &env, &r) == 1);
    assert(r.x == -3 && r.y == -2);
    env.facing = 0;
    assert(st.positional_count == 4);

    /* ── change detection (c_sfx.cpp:223-238) ── */
    /* same proven sample 7 + same rotated pos (2,-3) => suppressed */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 3, 10, 12, 13, 1,
                                         &env, &r) == 0);
    assert(r.rejected_duplicate);
    /* same position but a different proven sample queues fine */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 7, 8, 9, 3, 10, 12, 13, 1,
                                         &env, &r) == 1);
    assert(r.sample_id == 11);
    assert(st.positional_count == 5);

    /* ── volume halving (c_sfx.cpp:170-171) ── */
    env.half_volume = 1;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 7, 8, 9, 3, 10, 14, 15, 1,
                                         &env, &r) == 1);
    assert(r.volume == 5);
    env.half_volume = 0;

    /* ── R_1FB7D rescale (c_sfx.cpp:239-281) ── */
    /* facing 0, x=12,y=11 => vw_18=2, vw_1c=-1, dist=3; probe 4 > 3:
     * factor=(4<<10)/3=1365; vw_18=(2*1365+0x200)>>10=3;
     * vw_1c=-((1*1365+0x200)>>10)=-1 */
    env.occlusion_probe = probe_far;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 3, 10, 12, 11, 1,
                                         &env, &r) == 1);
    assert(r.occlusion_scaled && r.occlusion_distance == 4);
    assert(r.x == 3 && r.y == -1);
    /* negative x rescale: x=8,y=11 => vw_18=-2, vw_1c=-1 */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 7, 8, 9, 3, 10, 8, 11, 1,
                                         &env, &r) == 1);
    assert(r.x == -3 && r.y == -1);
    env.occlusion_probe = probe_near;

    /* ── immediate queue (argw3 < 0), cap 6 (c_sfx.cpp:216-222) ── */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 1, 10, 11, 10, -1,
                                         &env, &r) == 1);
    assert(r.queued_immediate && st.immediate_count == 1);

    /* ── scratch immediate play (argw3 == 0, c_sfx.cpp:297) ── */
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 2, 10, 11, 11, 0,
                                         &env, &r) == 1);
    assert(r.play_sound_requested);
    assert(st.positional_count == 8); /* scratch write did not increment */

    /* ── GEN2 remap (c_sfx.cpp:334-345) ── */
    /* (1,2,3) queued => cls2 stays; same pos as existing sample 7 entry at
     * (2,-3) => duplicate */
    assert(dm2_v1_sound_queue_noise_gen2(&st, 1, 2, 3, 99, 12, 13, 3, 10,
                                         1, &env, &r) == 0);
    assert(r.rejected_duplicate);
    /* (3,1,4) not queued under cls2=99 => routed via cls_alt=1: (3,1,4)
     * was sound9'd, so GEN1(3,1,4,...) succeeds */
    assert(dm2_v1_sound_queue_noise_gen2(&st, 3, 99, 4, 1, 20, 20, 5, 10,
                                         1, &env, &r) == 1);
    assert(r.queued_positional && r.sample_id == 12);

    /* ── delayed path (c_sfx.cpp:300-330) ── */
    env.map_origin_dx = 1;
    env.map_origin_dy = -2;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 6, 30, 40, 50, 2,
                                         &env, &r) == 1);
    assert(r.delayed_slot_reserved && r.delayed_slot_index == 0);
    assert(r.timer_queue_pending);
    assert(r.timer_type == 0x15 && r.timer_actor == 6 && r.timer_a == 0);
    assert(r.timer_mticks == 1000 + 2 - 1);
    assert(st.delayed[0].l_00 == 1);
    assert(st.delayed[0].barr_04[0] == 1 && st.delayed[0].barr_04[1] == 2);
    assert(st.delayed[0].barr_04[2] == 3 && st.delayed[0].barr_04[3] == 2);
    assert(st.delayed[0].w_0a == 6 && st.delayed[0].w_0c == 30);
    /* fill the remaining delayed slots, then reject */
    for (int i = 1; i < 8; ++i)
        assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 6, 30, 40, 50,
                                             2, &env, &r) == 1);
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 6, 30, 40, 50, 2,
                                         &env, &r) == 0);
    assert(r.rejected_delayed_full);
    env.map_origin_dx = 0;
    env.map_origin_dy = 0;

    /* ── positional cap (c_sfx.cpp:164-165) ── */
    while (st.positional_count < DM2_V1_SOUND_POSITIONAL_CAP) {
        assert(dm2_v1_sound_queue_noise_gen1(&st, 7, 8, 9, 1, 10, 30, 30,
                                             1, &env, &r) == 1);
        /* change detection suppresses identical repeats; shift position */
        env.party_y--;
    }
    env.party_y = 10;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 0, 10, 11, 12, 1,
                                         &env, &r) == 0);
    assert(r.rejected_positional_full);

    /* ── R_928 metric vectors (c_sound.cpp:256-308) ── */
    {
        DM2_V1_SoundSfx s;
        memset(&s, 0, sizeof(s));
        s.ub_05 = 200; s.ub_06 = 0; s.ub_07 = 4;
        dm2_v1_sound_queue_r928_metric(&s);
        assert(s.metric_bearing == 0x8000u);
        assert(s.metric_attenuation ==
               (uint8_t)(((200u << 8) / (16u + 8u)) >> 5));

        s.ub_06 = 5; s.ub_07 = 0;
        dm2_v1_sound_queue_r928_metric(&s);
        assert(s.metric_bearing == 0xf800u); /* table[23] */

        s.ub_06 = -5; s.ub_07 = 0;
        dm2_v1_sound_queue_r928_metric(&s);
        assert(s.metric_bearing == 0x0800u); /* table[8] */

        /* x=3,y=4: divisor 33, att=(51200/33)>>5=48; ratio=(3<<11)/4=1536
         * first table entry <= 1536 is index 4 (0x0490) =>
         * bearing table[8+15-4] = table[19] = 0xbc00 */
        s.ub_05 = 200; s.ub_06 = 3; s.ub_07 = 4;
        dm2_v1_sound_queue_r928_metric(&s);
        assert(s.metric_attenuation == 48u);
        assert(s.metric_bearing == 0xbc00u);

        /* negative x mirror: table[8+4] = table[12] = 0x4400 */
        s.ub_06 = -3;
        dm2_v1_sound_queue_r928_metric(&s);
        assert(s.metric_bearing == 0x4400u);
    }

    /* ── DM2_PLAY_SOUND order (c_sound.cpp:342-434) ── */
    {
        DM2_V1_SoundSfx e[3];
        memset(e, 0, sizeof(e));
        /* priorities: e0=1, e1=5, e2=5; e1 attenuation > e2 attenuation */
        e[0].ub_04 = 1; e[0].ub_05 = 10; e[0].ub_06 = 40; e[0].ub_07 = 40;
        e[1].ub_04 = 5; e[1].ub_05 = 200; e[1].ub_06 = 1; e[1].ub_07 = 0;
        e[2].ub_04 = 5; e[2].ub_05 = 10; e[2].ub_06 = 40; e[2].ub_07 = 40;
        dm2_v1_sound_queue_state_init(&st, 4);
        assert(dm2_v1_sound_queue_play_sound(&st, e, 3, &pr) == 1);
        assert(pr.valid && pr.entries_processed == 3);
        assert(pr.playback_unavailable);
        assert(!pr.no_free_sample_slot);
        /* sorted: e1 (pri 5, louder), e2 (pri 5), e0 (pri 1) */
        assert(pr.permutation[0] == 1);
        assert(pr.permutation[1] == 2);
        assert(pr.permutation[2] == 0);
        /* no slot mutation (fail-closed): every scan sees slot 0 free */
        assert(pr.first_free_slot[0] == 0 && pr.first_free_slot[1] == 0);
        assert(st.sample_slots[0] == -1);

        /* stability: equal priority + equal attenuation keeps input order */
        e[0].ub_04 = 5; e[0].ub_05 = 200; e[0].ub_06 = 1; e[0].ub_07 = 0;
        e[1].ub_04 = 5; e[1].ub_05 = 200; e[1].ub_06 = 1; e[1].ub_07 = 0;
        e[2].ub_04 = 5; e[2].ub_05 = 200; e[2].ub_06 = 1; e[2].ub_07 = 0;
        assert(dm2_v1_sound_queue_play_sound(&st, e, 3, &pr) == 1);
        assert(pr.permutation[0] == 0 && pr.permutation[1] == 1 &&
               pr.permutation[2] == 2);

        /* gate: sound disabled / zero volume / zero count */
        st.sound_enabled = 0;
        assert(dm2_v1_sound_queue_play_sound(&st, e, 3, &pr) == 0);
        assert(pr.gate_rejected);
        st.sound_enabled = 1;
        st.master_sfx_volume = 0;
        assert(dm2_v1_sound_queue_play_sound(&st, e, 3, &pr) == 0);
        assert(pr.gate_rejected);
        st.master_sfx_volume = 7;
        assert(dm2_v1_sound_queue_play_sound(&st, e, 0, &pr) == 0);
        assert(pr.gate_rejected);

        /* no free sample slot: source ends the whole pass (cpp:411-413) */
        for (int i = 0; i < 64; ++i)
            st.sample_slots[i] = 40; /* sample_state(40) == 10: busy */
        assert(dm2_v1_sound_queue_play_sound(&st, e, 3, &pr) == 0);
        assert(pr.no_free_sample_slot && pr.entries_processed == 0);
    }

    /* ── sample-state gate (c_sound.cpp:321-339) ── */
    assert(dm2_v1_sound_queue_sample_state(0) == 1);
    assert(dm2_v1_sound_queue_sample_state(31) == 1);
    assert(dm2_v1_sound_queue_sample_state(32) == 10);
    assert(dm2_v1_sound_queue_sample_state(-1) == 10);

    /* ── DM2_SOUND8 flush (c_sound.cpp:633-647) ── */
    dm2_v1_sound_queue_state_init(&st, 8);
    make_env(&env);
    env.occlusion_probe = probe_near;
    assert(dm2_v1_sound_queue_sound9(&st, 1, 2, 3, 7, &idx) == 1);
    /* Post-DM2_482b_0684 source state; see the equivalent note above. */
    st.ssound[0].w_00 = 7;
    st.ssound[0].w_05 = 0x1234;
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 4, 10, 12, 13, 1,
                                         &env, &r) == 1);
    assert(dm2_v1_sound_queue_noise_gen1(&st, 1, 2, 3, 2, 10, 14, 15, 1,
                                         &env, &r) == 1);
    assert(st.positional_count == 2);
    assert(dm2_v1_sound_queue_sound8_flush(&st, 0, &pr) == 1);
    assert(pr.entries_processed == 2 && pr.playback_unavailable);
    /* sorted by priority: ecxw 4 entry first */
    assert(pr.permutation[0] == 0 && pr.permutation[1] == 1);
    assert(st.positional_count == 0);
    /* empty queue flush: no play attempt, count reset */
    assert(dm2_v1_sound_queue_sound8_flush(&st, 1, &pr) == 0);
    assert(!pr.valid);

    assert(dm2_v1_sound_queue_source_evidence() != 0);

    return 0;
}
