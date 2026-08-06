/* dm2_v1_sound_queue_pc34_compat.c — DM2 V1 source-ordered runtime sound
 * queue (DM2-008).
 *
 * skproject/SKULLWIN source lock:
 *   c_sound.cpp:650-662  DM2_SOUND9 (xsndptr2 runtime queue mutation)
 *   c_sound.cpp:664-673  DM2_QUERY_SND_ENTRY_INDEX (1-based)
 *   c_sound.cpp:256-308  R_928 attenuation/bearing metric
 *   c_sound.cpp:310-319  R_8FE precedence
 *   c_sound.cpp:321-339  R_5096A sample-state gate
 *   c_sound.cpp:342-434  DM2_PLAY_SOUND order
 *   c_sound.cpp:633-647  DM2_SOUND8 flush order
 *   c_sound.cpp:32-37    table1d14e2[24]
 *   c_sfx.cpp:138-331    DM2_QUEUE_NOISE_GEN1
 *   c_sfx.cpp:334-345    DM2_QUEUE_NOISE_GEN2
 *
 * dm2sound.xsndptr2 is a runtime queue, not GDAT-resident: no file
 * materialisation is attempted.  Unavailable audio is explicit, never
 * simulated (SKPROJECT-GAP-003).
 */

#include "dm2_v1_sound_queue_pc34_compat.h"

#include <string.h>

/* table1d14e2[24], c_sound.cpp:32-37 (bearing table, EVIL-shifted users). */
static const uint16_t dm2_v1_sound_bearing_table[24] = {
    0x25a0u, 0x11f0u, 0x0b00u, 0x0730u, 0x0490u, 0x0290u,
    0x00d0u, 0x0000u, 0x0800u, 0x1700u, 0x2600u, 0x3500u,
    0x4400u, 0x5300u, 0x6200u, 0x7100u, 0x8f00u, 0x9e00u,
    0xad00u, 0xbc00u, 0xcb00u, 0xda00u, 0xe900u, 0xf800u
};

static void dm2_v1_sound_queue_clear_receipt(DM2_V1_SoundQueueReceipt *r)
{
    if (r) memset(r, 0, sizeof(*r));
}

static void dm2_v1_sound_queue_clear_play_receipt(DM2_V1_SoundPlayReceipt *r)
{
    if (!r) return;
    memset(r, 0, sizeof(*r));
    for (uint16_t i = 0; i < DM2_V1_SOUND_PLAY_MAX; ++i)
        r->first_free_slot[i] = -1;
}

void dm2_v1_sound_queue_state_init(DM2_V1_SoundQueueState *state,
                                   uint16_t ssound_capacity)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (ssound_capacity > DM2_V1_SOUND_SSOUND_QUEUE_CAP)
        ssound_capacity = DM2_V1_SOUND_SSOUND_QUEUE_CAP;
    state->ssound_capacity = ssound_capacity;
    state->sound_enabled = 1;   /* c_sound::init v1d14be = true */
    state->master_sfx_volume = 7; /* c_sound::init v1dff88 = 7 */
    for (uint16_t i = 0; i < DM2_V1_SOUND_SSOUND_QUEUE_CAP; ++i)
        state->ssound[i].w_05 = -1;
    for (uint16_t i = 0; i < DM2_V1_SOUND_SAMPLE_SLOT_COUNT; ++i)
        state->sample_slots[i] = -1; /* c_sound::init v1dfda4[i] = -1 */
}

uint16_t dm2_v1_sound_queue_query_entry_index(
    const DM2_V1_SoundQueueState *state,
    int8_t cls1, int8_t cls2, int8_t cls3)
{
    /* DM2_QUERY_SND_ENTRY_INDEX, c_sound.cpp:664-673. */
    if (!state) return 0;
    for (uint16_t i = 0; i < state->ssound_count; ++i) {
        const DM2_V1_SoundSsoundEntry *e = &state->ssound[i];
        if (cls1 == e->b_02 && cls2 == e->b_03 && cls3 == e->b_04)
            return (uint16_t)(i + 1u);
    }
    return 0;
}

int dm2_v1_sound_queue_sound9(DM2_V1_SoundQueueState *state,
                              int8_t cls1, int8_t cls2, int8_t cls3,
                              int16_t sample_id,
                              uint16_t *out_index)
{
    /* DM2_SOUND9, c_sound.cpp:650-662. */
    if (out_index) *out_index = 0;
    if (!state) return 0;
    if (dm2_v1_sound_queue_query_entry_index(state, cls1, cls2, cls3) != 0u)
        return 0;
    if (state->ssound_count >= state->ssound_capacity ||
        state->ssound_count >= DM2_V1_SOUND_SSOUND_QUEUE_CAP)
        return 0;
    /* c_sound.cpp:656-661 assigns only the class triple and w_05 = -1.
     * The old Firestaff compatibility path copied sample_id into w_00 here,
     * fabricating c_gdatfile.cpp::DM2_482b_0684's later ownership of both a
     * GDAT raw index and a sndptr4 slot. */
    (void)sample_id;
    state->ssound[state->ssound_count].w_00 = -1;
    state->ssound[state->ssound_count].b_02 = cls1;
    state->ssound[state->ssound_count].b_03 = cls2;
    state->ssound[state->ssound_count].b_04 = cls3;
    state->ssound[state->ssound_count].w_05 = -1;
    state->ssound_count++;
    if (out_index) *out_index = state->ssound_count;
    return 1;
}

void dm2_v1_sound_queue_r928_metric(DM2_V1_SoundSfx *sfx)
{
    /* R_928, c_sound.cpp:256-308. */
    int32_t x;
    int32_t y;
    uint32_t divisor;
    if (!sfx) return;
    x = (int32_t)sfx->ub_06;
    y = (int32_t)sfx->ub_07;
    divisor = (uint32_t)(x * x + y * y) + 8u;
    sfx->metric_attenuation =
        (uint8_t)((((uint32_t)sfx->ub_05 << 8) / divisor) >> 5);
    if (x == 0) {
        sfx->metric_bearing = 0x8000u;
        return;
    }
    if (y == 0) {
        /* c_sound.cpp:273-278: table[0xf+0x8] for x >= 0, table[0x8] else. */
        sfx->metric_bearing = (x >= 0) ? dm2_v1_sound_bearing_table[23]
                                       : dm2_v1_sound_bearing_table[8];
        return;
    }
    {
        int neg_x = 0;
        uint16_t ratio;
        if (x < 0) {
            x = -x;
            neg_x = 1;
        }
        if (y < 0) y = -y;
        ratio = (uint16_t)(((uint32_t)x << 11) / (uint32_t)y);
        /* c_sound.cpp:292-306: scan terminates by index 7 (table[7] == 0). */
        for (uint16_t i = 0; i < 8u; ++i) {
            if (ratio >= dm2_v1_sound_bearing_table[i]) {
                sfx->metric_bearing =
                    neg_x ? dm2_v1_sound_bearing_table[8u + i]
                          : dm2_v1_sound_bearing_table[8u + 15u - i];
                return;
            }
        }
        sfx->metric_bearing =
            neg_x ? dm2_v1_sound_bearing_table[15]
                  : dm2_v1_sound_bearing_table[16];
    }
}

int dm2_v1_sound_queue_r8fe_precedes(const DM2_V1_SoundSfx *lhs,
                                     const DM2_V1_SoundSfx *rhs)
{
    /* R_8FE, c_sound.cpp:310-319. */
    if (!lhs || !rhs) return 0;
    if (lhs->ub_04 > rhs->ub_04) return 1;
    if (lhs->ub_04 == rhs->ub_04)
        return lhs->metric_attenuation >= rhs->metric_attenuation;
    return 0;
}

int dm2_v1_sound_queue_sample_state(int32_t sample_index)
{
    /* R_5096A, c_sound.cpp:321-339 (source table all zero). */
    if (sample_index < 0 || sample_index >= 32) return 10;
    return 1;
}

int dm2_v1_sound_queue_play_sound(DM2_V1_SoundQueueState *state,
                                  DM2_V1_SoundSfx *entries,
                                  uint16_t count,
                                  DM2_V1_SoundPlayReceipt *out_receipt)
{
    /* DM2_PLAY_SOUND, c_sound.cpp:342-434. */
    DM2_V1_SoundPlayReceipt receipt;
    dm2_v1_sound_queue_clear_play_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    for (uint16_t i = 0; i < DM2_V1_SOUND_PLAY_MAX; ++i)
        receipt.first_free_slot[i] = -1;
    receipt.playback_unavailable = 1;
    if (!state || !entries || count == 0u ||
        !state->sound_enabled || state->master_sfx_volume == 0) {
        /* c_sound.cpp:351: !v1d14be || v1dff88 == 0 || eaxw == 0. */
        receipt.gate_rejected = 1;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (count > DM2_V1_SOUND_PLAY_MAX)
        count = DM2_V1_SOUND_PLAY_MAX;

    /* c_sound.cpp:353-361: per-entry metric + permutation init. */
    for (uint16_t i = 0; i < count; ++i) {
        dm2_v1_sound_queue_r928_metric(&entries[i]);
        entries[i].ub_0b = (uint8_t)i;
    }

    /* c_sound.cpp:362-384: bubble-sort the ub_0b permutation by R_8FE. */
    {
        int changed;
        do {
            uint8_t vw_0c;
            changed = 0;
            vw_0c = entries[0].ub_0b;
            for (uint16_t i = 1; i < count; ++i) {
                uint8_t vw_08 = entries[i].ub_0b;
                if (dm2_v1_sound_queue_r8fe_precedes(&entries[vw_0c],
                                                     &entries[vw_08])) {
                    vw_0c = vw_08;
                } else {
                    entries[i - 1u].ub_0b = vw_08;
                    entries[i].ub_0b = vw_0c;
                    changed = 1;
                }
            }
        } while (changed);
    }

    /* c_sound.cpp:385-433: sorted order, 64-slot free-sample scan. */
    for (uint16_t i = 0; i < count; ++i) {
        const DM2_V1_SoundSfx *e = &entries[entries[i].ub_0b];
        int32_t free_slot = -1;
        for (uint16_t s = 0; s < DM2_V1_SOUND_SAMPLE_SLOT_COUNT; ++s) {
            int32_t handle = state->sample_slots[s];
            if (handle == -1 ||
                dm2_v1_sound_queue_sample_state(handle) == 1) {
                free_slot = (int32_t)s;
                break;
            }
        }
        if (free_slot == -1) {
            /* c_sound.cpp:411-413: no free slot ends the whole pass. */
            receipt.no_free_sample_slot = 1;
            receipt.valid = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        /* do_sound (c_sfx.cpp:47-77) needs the resolved sample payload and a
         * verified backend; neither is proven, so the slot table is left
         * untouched and playback is receipted unavailable (fail-closed). */
        receipt.permutation[i] = entries[i].ub_0b;
        receipt.attenuation[i] = e->metric_attenuation;
        receipt.bearing[i] = e->metric_bearing;
        receipt.first_free_slot[i] = (int16_t)free_slot;
        receipt.entries_processed++;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_sound_queue_noise_gen1(DM2_V1_SoundQueueState *state,
                                  int8_t cls1, int8_t cls2, int8_t cls3,
                                  int16_t ecxw,
                                  int16_t volume, int16_t x, int16_t y,
                                  int16_t delay_mode,
                                  const DM2_V1_SoundQueueEnv *env,
                                  DM2_V1_SoundQueueReceipt *out_receipt)
{
    /* DM2_QUEUE_NOISE_GEN1, c_sfx.cpp:138-331. */
    DM2_V1_SoundQueueReceipt receipt;
    uint16_t query_index;
    int16_t sample_id;
    dm2_v1_sound_queue_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state || !env) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    /* c_sfx.cpp:156-163: delayed requests are map-gated. */
    if (delay_mode > 0 && env->current_map != env->gate_map_a &&
        env->current_map != env->gate_map_b) {
        receipt.rejected_map_gate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    /* c_sfx.cpp:164-165: positional cap gates every mode. */
    if (state->positional_count >= DM2_V1_SOUND_POSITIONAL_CAP) {
        receipt.rejected_positional_full = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    /* c_sfx.cpp:166-169: the runtime queue must already own the class. */
    query_index =
        dm2_v1_sound_queue_query_entry_index(state, cls1, cls2, cls3);
    if (query_index == 0u) {
        receipt.rejected_query_miss = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    sample_id = state->ssound[query_index - 1u].w_00;
    if (sample_id < 0) {
        /* sndptr4 + (w_00 << 4) cannot be proven without the source GDAT
         * sample binding; queueing an unresolved sample is rejected. */
        receipt.rejected_sample_unresolved = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.sample_id = sample_id;

    /* c_sfx.cpp:170-171. */
    if (env->half_volume)
        volume = (int16_t)(volume >> 1);

    if (delay_mode <= 1) {
        int16_t vw_18 = 0;
        int16_t vw_1c = 0;
        DM2_V1_SoundSfx *target;
        uint16_t target_count;

        /* c_sfx.cpp:177-182: cross-map origin adjustment. */
        if (delay_mode > 0) {
            x = (int16_t)(x + env->map_origin_dx);
            y = (int16_t)(y + env->map_origin_dy);
        }

        /* c_sfx.cpp:183-207: party-facing rotation.  The source leaves
         * vw_18/vw_1c uninitialised when v1e0258 > 3; fail-closed instead. */
        if (env->facing > 3u) {
            receipt.rejected_facing_unproven = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        switch (env->facing) {
        case 0:
            vw_18 = (int16_t)(x - env->party_x);
            vw_1c = (int16_t)(env->party_y - y);
            break;
        case 1:
            vw_18 = (int16_t)(y - env->party_y);
            vw_1c = (int16_t)(x - env->party_x);
            break;
        case 2:
            vw_18 = (int16_t)(env->party_x - x);
            vw_1c = (int16_t)(y - env->party_y);
            break;
        default:
            vw_18 = (int16_t)(env->party_y - y);
            vw_1c = (int16_t)(env->party_x - x);
            break;
        }

        /* c_sfx.cpp:211-222: target queue selection. */
        if (delay_mode >= 0) {
            target = state->positional;
            target_count = state->positional_count;
        } else {
            if (state->immediate_count >= DM2_V1_SOUND_IMMEDIATE_CAP) {
                receipt.rejected_immediate_full = 1;
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
            target = state->immediate;
            target_count = state->immediate_count;
        }

        /* c_sfx.cpp:223-238: change detection.  A queued entry with the same
         * proven sample and the same rotated position suppresses the
         * request.  Unresolved samples (-1) can never prove equality, so
         * they never suppress (fail-closed).  The source's secondary
         * s54p_00->s54p_00 comparison (c_sfx.cpp:228) needs sample-record
         * ownership that is not proven and stays absent. */
        for (uint16_t i = 0; i < target_count; ++i) {
            const DM2_V1_SoundSfx *q = &target[i];
            if (q->sample_id >= 0 && q->sample_id == sample_id &&
                q->ub_06 == (int8_t)vw_18 && q->ub_07 == (int8_t)vw_1c) {
                receipt.rejected_duplicate = 1;
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
        }

        /* c_sfx.cpp:239-281: R_1FB7D wall-occlusion clamp. */
        {
            int32_t dist = (vw_18 >= 0 ? vw_18 : -vw_18) +
                           (vw_1c >= 0 ? vw_1c : -vw_1c);
            if (dist > 1) {
                int16_t occ;
                if (!env->occlusion_probe) {
                    receipt.rejected_occlusion_unavailable = 1;
                    if (out_receipt) *out_receipt = receipt;
                    return 0;
                }
                occ = env->occlusion_probe(env->occlusion_ctx, x, y);
                if (occ < 0) {
                    receipt.rejected_occlusion_blocked = 1;
                    if (out_receipt) *out_receipt = receipt;
                    return 0;
                }
                receipt.occlusion_distance = occ;
                if ((int32_t)occ > dist) {
                    /* c_sfx.cpp:247-280: 10-bit fixed-point rescale. */
                    int32_t factor = ((int32_t)occ << 10) / dist;
                    if (vw_18 >= 0) {
                        if (vw_18 > 0)
                            vw_18 = (int16_t)((vw_18 * factor + 0x200) >> 10);
                    } else {
                        vw_18 = (int16_t)(-((-vw_18 * factor + 0x200) >> 10));
                    }
                    if (vw_1c >= 0) {
                        if (vw_1c > 0)
                            vw_1c = (int16_t)((vw_1c * factor + 0x200) >> 10);
                    } else {
                        vw_1c = (int16_t)(-((-vw_1c * factor + 0x200) >> 10));
                    }
                    receipt.occlusion_scaled = 1;
                }
            }
        }

        /* c_sfx.cpp:282-297: append in source field order. */
        target[target_count].sample_id = sample_id;
        target[target_count].ub_04 = (uint8_t)ecxw;
        target[target_count].ub_05 = (uint8_t)volume;
        target[target_count].ub_06 = (int8_t)vw_18;
        target[target_count].ub_07 = (int8_t)vw_1c;
        target[target_count].metric_attenuation = 0;
        target[target_count].metric_bearing = 0;
        target[target_count].ub_0b = 0;
        receipt.queue_index = target_count;
        receipt.priority = (uint8_t)ecxw;
        receipt.volume = (uint8_t)volume;
        receipt.x = (int8_t)vw_18;
        receipt.y = (int8_t)vw_1c;
        if (delay_mode != 0) {
            if (delay_mode <= 0) {
                state->immediate_count++;
                receipt.queued_immediate = 1;
            } else {
                state->positional_count++;
                receipt.queued_positional = 1;
            }
        } else {
            /* c_sfx.cpp:297: scratch write + DM2_PLAY_SOUND(1, entry); the
             * caller runs dm2_v1_sound_queue_play_sound on the scratch slot. */
            receipt.play_sound_requested = 1;
        }
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }

    /* c_sfx.cpp:300-330: delayed path, 8-slot s_sizee scan. */
    {
        uint16_t slot;
        int found = 0;
        for (slot = 0; slot < DM2_V1_SOUND_DELAYED_SLOT_COUNT; ++slot) {
            if (state->delayed[slot].l_00 == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            receipt.rejected_delayed_full = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        state->delayed[slot].l_00 = 1;
        state->delayed[slot].barr_04[0] = (uint8_t)cls1;
        state->delayed[slot].barr_04[1] = (uint8_t)cls2;
        state->delayed[slot].barr_04[2] = (uint8_t)cls3;
        state->delayed[slot].barr_04[3] = (uint8_t)env->current_map;
        state->delayed[slot].barr_04[4] = (uint8_t)x;
        state->delayed[slot].barr_04[5] = (uint8_t)y;
        state->delayed[slot].w_0a = ecxw;
        state->delayed[slot].w_0c = volume;
        receipt.delayed_slot_reserved = 1;
        receipt.delayed_slot_index = slot;
        receipt.sample_id = sample_id;
        receipt.priority = (uint8_t)ecxw;
        receipt.volume = (uint8_t)volume;
        /* c_sfx.cpp:326-330: the source queues a type-0x15 timer here.  The
         * DM2_QUEUE_TIMER binding is not owned by this module, so the timer
         * is receipted pending; the slot stays occupied exactly like the
         * source until a proven timer releases it via DM2_PROCESS_SOUND. */
        receipt.timer_queue_pending = 1;
        receipt.timer_type = DM2_V1_SOUND_TIMER_TYPE_DELAYED;
        receipt.timer_actor = (uint8_t)ecxw;
        receipt.timer_a = (int16_t)slot;
        receipt.timer_mticks = env->gametick + (int32_t)delay_mode - 1;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
}

int dm2_v1_sound_queue_noise_gen2(DM2_V1_SoundQueueState *state,
                                  int8_t cls1, int8_t cls2, int8_t cls3,
                                  int8_t cls_alt,
                                  int16_t x, int16_t y,
                                  int16_t ecxw, int16_t volume,
                                  int16_t delay_mode,
                                  const DM2_V1_SoundQueueEnv *env,
                                  DM2_V1_SoundQueueReceipt *out_receipt)
{
    /* DM2_QUEUE_NOISE_GEN2, c_sfx.cpp:334-345. */
    int8_t effective_cls2 = cls_alt;
    if (dm2_v1_sound_queue_query_entry_index(state, cls1, cls2, cls3) != 0u)
        effective_cls2 = cls2;
    return dm2_v1_sound_queue_noise_gen1(state, cls1, effective_cls2, cls3,
                                         ecxw, volume, x, y, delay_mode,
                                         env, out_receipt);
}

int dm2_v1_sound_queue_sound8_flush(DM2_V1_SoundQueueState *state,
                                    int immediate,
                                    DM2_V1_SoundPlayReceipt *out_receipt)
{
    /* DM2_SOUND8, c_sound.cpp:633-647. */
    int played = 0;
    int attempted = 0;
    if (!state) {
        dm2_v1_sound_queue_clear_play_receipt(out_receipt);
        return 0;
    }
    if (immediate) {
        if (state->immediate_count > 0u) {
            attempted = 1;
            played = dm2_v1_sound_queue_play_sound(
                state, state->immediate, state->immediate_count, out_receipt);
        }
        state->immediate_count = 0;
    } else {
        if (state->positional_count > 0u) {
            attempted = 1;
            played = dm2_v1_sound_queue_play_sound(
                state, state->positional, state->positional_count,
                out_receipt);
        }
        state->positional_count = 0;
    }
    if (!attempted)
        dm2_v1_sound_queue_clear_play_receipt(out_receipt);
    return played;
}

const char *dm2_v1_sound_queue_source_evidence(void)
{
    return
        "DM2 V1 sound queue — source-ordered runtime queue (DM2-008)\n"
        "skproject/SKULLWIN/c_sound.cpp:650-662 DM2_SOUND9 queue mutation\n"
        "skproject/SKULLWIN/c_sound.cpp:664-673 DM2_QUERY_SND_ENTRY_INDEX\n"
        "skproject/SKULLWIN/c_sound.cpp:256-308 R_928 metric\n"
        "skproject/SKULLWIN/c_sound.cpp:310-319 R_8FE precedence\n"
        "skproject/SKULLWIN/c_sound.cpp:321-339 R_5096A sample state\n"
        "skproject/SKULLWIN/c_sound.cpp:342-434 DM2_PLAY_SOUND order\n"
        "skproject/SKULLWIN/c_sound.cpp:633-647 DM2_SOUND8 flush\n"
        "skproject/SKULLWIN/c_sfx.cpp:138-331 DM2_QUEUE_NOISE_GEN1\n"
        "skproject/SKULLWIN/c_sfx.cpp:334-345 DM2_QUEUE_NOISE_GEN2\n"
        "xsndptr2 is a runtime queue, not GDAT-resident; no file "
        "materialisation. Playback/sample backend explicitly unavailable "
        "(SKPROJECT-GAP-003).\n";
}
