#ifndef FIRESTAFF_DM2_V1_SOUND_QUEUE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_SOUND_QUEUE_PC34_COMPAT_H

/*
 * dm2_v1_sound_queue_pc34_compat.h — DM2 V1 source-ordered runtime sound
 * queue (DM2-008).
 *
 * dm2sound.xsndptr2 / v1d2698 is a source-owned dynamic seven-byte runtime
 * queue populated by DM2_SOUND9, NOT a GDAT table; nothing here attempts
 * file materialisation for it.  Every semantic is locked to
 * skproject/SKULLWIN:
 *
 *   c_sound.h:33-40    s_ssound (7 bytes): w_00, b_02, b_03, b_04, w_05
 *   c_sound.h:42-52    s_sfx (12 bytes): s54p_00, ub_04..ub_07, s59_08, ub_0b
 *   xtypes.h:110-120   s_sizee (14 bytes): l_00/s54p_00, barr_04[6], w_0a, w_0c
 *   c_sound.cpp:32-37  table1d14e2[24] bearing table
 *   c_sound.cpp:650-662  DM2_SOUND9 runtime queue mutation
 *   c_sound.cpp:664-673  DM2_QUERY_SND_ENTRY_INDEX (1-based linear search)
 *   c_sound.cpp:256-308  R_928 attenuation/bearing metric
 *   c_sound.cpp:310-319  R_8FE playback precedence
 *   c_sound.cpp:321-339  R_5096A sample-state gate (all-zero table)
 *   c_sound.cpp:342-434  DM2_PLAY_SOUND metric/sort/slot-scan order
 *   c_sound.cpp:633-647  DM2_SOUND8 queue flush order
 *   c_sfx.cpp:138-331    DM2_QUEUE_NOISE_GEN1 queue/change-detection order
 *   c_sfx.cpp:334-345    DM2_QUEUE_NOISE_GEN2 class remap wrapper
 *
 * Fail-closed contract: sample payloads are never resolved from an arbitrary
 * sound id (SKPROJECT-GAP-003); entries whose sample binding is unproven are
 * rejected, the R_1FB7D wall-occlusion clamp rejects without a proven map
 * probe, facing values > 3 reject (the source leaves vw_18/vw_1c
 * uninitialised there), the delayed path receipts its type-0x15 timer as
 * pending instead of simulating DM2_QUEUE_TIMER, and DM2_PLAY_SOUND reports
 * playback explicitly unavailable without mutating the sample-slot table.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_SOUND_SSOUND_QUEUE_CAP 64u /* xsndptr2 module cap */
#define DM2_V1_SOUND_POSITIONAL_CAP 20u   /* c_sfx.cpp:164 (v1d2694 == 20) */
#define DM2_V1_SOUND_IMMEDIATE_CAP 6u     /* c_sfx.cpp:218 (v1d2696 == 6) */
#define DM2_V1_SOUND_DELAYED_SLOT_COUNT 8u /* c_sfx.cpp:303 (8 x s_sizee) */
#define DM2_V1_SOUND_SAMPLE_SLOT_COUNT 64u /* c_sound.cpp:400 (v1dfda4) */
#define DM2_V1_SOUND_PLAY_MAX DM2_V1_SOUND_POSITIONAL_CAP
#define DM2_V1_SOUND_TIMER_TYPE_DELAYED 0x15u /* c_sfx.cpp:327 */

/* s_ssound, c_sound.h:33-40.  w_00 is the sample binding the source owns
 * from level-load GDAT resolution; -1 means unbound (fail-closed). */
typedef struct {
    int16_t w_00;
    int8_t b_02;
    int8_t b_03;
    int8_t b_04;
    int16_t w_05;
} DM2_V1_SoundSsoundEntry;

/* s_sfx, c_sound.h:42-52.  s54p_00 is represented by sample_id (the owning
 * xsndptr2 w_00 word); pointers are never fabricated. */
typedef struct {
    int16_t sample_id;         /* s54p_00 stand-in, -1 unresolved */
    uint8_t ub_04;             /* priority: CUTX8(ecxw) at queue time */
    uint8_t ub_05;             /* volume */
    int8_t ub_06;              /* rotated x delta */
    int8_t ub_07;              /* rotated y delta */
    uint8_t metric_attenuation;/* s59_08.ub_00, written by R_928 */
    uint16_t metric_bearing;   /* s59_08.w_01, written by R_928 */
    uint8_t ub_0b;             /* sort permutation index */
} DM2_V1_SoundSfx;

/* s_sizee delayed slot, xtypes.h:110-120. */
typedef struct {
    int32_t l_00;              /* 1 = occupied */
    uint8_t barr_04[6];        /* cls1, cls2, cls3, map, x, y */
    int16_t w_0a;              /* vw_24 (ecxw) */
    int16_t w_0c;              /* volume */
} DM2_V1_SoundDelayedSlot;

typedef struct {
    DM2_V1_SoundSsoundEntry ssound[DM2_V1_SOUND_SSOUND_QUEUE_CAP];
    uint16_t ssound_count;     /* v1d2698 */
    uint16_t ssound_capacity;  /* dm2_dballochandler.v1e0ad6 */
    uint16_t sample_binding_count; /* v1d269a, set by DM2_482b_0684 */
    DM2_V1_SoundSfx positional[DM2_V1_SOUND_POSITIONAL_CAP]; /* xsndptr5 */
    uint16_t positional_count; /* v1d2694 */
    DM2_V1_SoundSfx immediate[DM2_V1_SOUND_IMMEDIATE_CAP];   /* xsndptr3 */
    uint16_t immediate_count;  /* v1d2696 */
    DM2_V1_SoundDelayedSlot delayed[DM2_V1_SOUND_DELAYED_SLOT_COUNT];
    int32_t sample_slots[DM2_V1_SOUND_SAMPLE_SLOT_COUNT]; /* v1dfda4 */
    int sound_enabled;         /* v1d14be */
    int master_sfx_volume;     /* v1dff88, 0..7 */
} DM2_V1_SoundQueueState;

/* Explicit replacements for the ddat globals DM2_QUEUE_NOISE_GEN1 reads.
 * occlusion_probe is the R_1FB7D stand-in (c_sfx.cpp:88-135): it must return
 * the source's occlusion distance, or a negative value when the target is
 * inaudible.  NULL probe => occlusion unavailable (fail-closed when the
 * manhattan distance exceeds 1). */
typedef struct {
    int16_t current_map;       /* ddat.v1d3248 */
    int16_t gate_map_a;        /* ddat.v1e0282 */
    int16_t gate_map_b;        /* ddat.v1e027c */
    int half_volume;           /* ddat.v1e0238 != 0 */
    uint16_t facing;           /* ddat.v1e0258 */
    int16_t party_x;           /* ddat.v1e0270 */
    int16_t party_y;           /* ddat.v1e0272 */
    int16_t map_origin_dx;     /* c_sfx.cpp:180 barr_04[0x2] difference */
    int16_t map_origin_dy;     /* c_sfx.cpp:181 barr_04[0x3] difference */
    int32_t gametick;          /* timdat.gametick, delayed timer receipt */
    int16_t (*occlusion_probe)(void *ctx, int16_t x, int16_t y);
    void *occlusion_ctx;
} DM2_V1_SoundQueueEnv;

typedef struct {
    int valid;
    int rejected_map_gate;
    int rejected_positional_full;
    int rejected_immediate_full;
    int rejected_query_miss;
    int rejected_sample_unresolved;
    int rejected_duplicate;
    int rejected_facing_unproven;
    int rejected_occlusion_unavailable;
    int rejected_occlusion_blocked;
    int rejected_delayed_full;
    int queued_positional;
    int queued_immediate;
    int play_sound_requested;  /* argw3 == 0: DM2_PLAY_SOUND(1, entry) */
    int delayed_slot_reserved;
    int timer_queue_pending;   /* type 0x15 timer NOT queued by this module */
    int occlusion_scaled;
    uint16_t queue_index;
    uint16_t delayed_slot_index;
    int16_t sample_id;
    int16_t occlusion_distance;
    uint8_t priority;
    uint8_t volume;
    int8_t x;
    int8_t y;
    uint8_t timer_type;
    uint8_t timer_actor;
    int16_t timer_a;
    int32_t timer_mticks;
} DM2_V1_SoundQueueReceipt;

typedef struct {
    int valid;
    int gate_rejected;
    int no_free_sample_slot;
    int playback_unavailable;  /* always 1: no verified sample backend */
    uint16_t entries_processed;
    uint8_t permutation[DM2_V1_SOUND_PLAY_MAX];
    uint8_t attenuation[DM2_V1_SOUND_PLAY_MAX];
    uint16_t bearing[DM2_V1_SOUND_PLAY_MAX];
    int16_t first_free_slot[DM2_V1_SOUND_PLAY_MAX];
} DM2_V1_SoundPlayReceipt;

void dm2_v1_sound_queue_state_init(DM2_V1_SoundQueueState *state,
                                   uint16_t ssound_capacity);

/* DM2_SOUND9, c_sound.cpp:650-662. The source initialises w_05 to -1 and
 * does not bind w_00 here. `sample_id` exists only to preserve Firestaff's
 * older public call shape and is ignored; DM2_482b_0684 owns later binding. */
int dm2_v1_sound_queue_sound9(DM2_V1_SoundQueueState *state,
                              int8_t cls1, int8_t cls2, int8_t cls3,
                              int16_t sample_id,
                              uint16_t *out_index);

/* DM2_QUERY_SND_ENTRY_INDEX, c_sound.cpp:664-673: 1-based, 0 when absent. */
uint16_t dm2_v1_sound_queue_query_entry_index(
    const DM2_V1_SoundQueueState *state,
    int8_t cls1, int8_t cls2, int8_t cls3);

/* DM2_QUEUE_NOISE_GEN1, c_sfx.cpp:138-331.  delay_mode is argw3:
 * <0 immediate queue, 0 immediate play scratch write, 1 positional queue,
 * >1 delayed slot + pending type-0x15 timer. */
int dm2_v1_sound_queue_noise_gen1(DM2_V1_SoundQueueState *state,
                                  int8_t cls1, int8_t cls2, int8_t cls3,
                                  int16_t ecxw,
                                  int16_t volume, int16_t x, int16_t y,
                                  int16_t delay_mode,
                                  const DM2_V1_SoundQueueEnv *env,
                                  DM2_V1_SoundQueueReceipt *out_receipt);

/* DM2_QUEUE_NOISE_GEN2, c_sfx.cpp:334-345. */
int dm2_v1_sound_queue_noise_gen2(DM2_V1_SoundQueueState *state,
                                  int8_t cls1, int8_t cls2, int8_t cls3,
                                  int8_t cls_alt,
                                  int16_t x, int16_t y,
                                  int16_t ecxw, int16_t volume,
                                  int16_t delay_mode,
                                  const DM2_V1_SoundQueueEnv *env,
                                  DM2_V1_SoundQueueReceipt *out_receipt);

/* R_928, c_sound.cpp:256-308: exact attenuation/bearing metric. */
void dm2_v1_sound_queue_r928_metric(DM2_V1_SoundSfx *sfx);

/* R_8FE, c_sound.cpp:310-319: playback precedence. */
int dm2_v1_sound_queue_r8fe_precedes(const DM2_V1_SoundSfx *lhs,
                                     const DM2_V1_SoundSfx *rhs);

/* R_5096A, c_sound.cpp:321-339: 10 for index >= 32, else 1 (all-zero source
 * sample-state table). */
int dm2_v1_sound_queue_sample_state(int32_t sample_index);

/* DM2_PLAY_SOUND, c_sound.cpp:342-434: gate, per-entry R_928, permutation
 * bubble sort, 64-slot free-sample scan with early return.  Playback is
 * reported unavailable; the slot table is never mutated (fail-closed). */
int dm2_v1_sound_queue_play_sound(DM2_V1_SoundQueueState *state,
                                  DM2_V1_SoundSfx *entries,
                                  uint16_t count,
                                  DM2_V1_SoundPlayReceipt *out_receipt);

/* DM2_SOUND8, c_sound.cpp:633-647: flush a queue through DM2_PLAY_SOUND and
 * reset its count. */
int dm2_v1_sound_queue_sound8_flush(DM2_V1_SoundQueueState *state,
                                    int immediate,
                                    DM2_V1_SoundPlayReceipt *out_receipt);

const char *dm2_v1_sound_queue_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SOUND_QUEUE_PC34_COMPAT_H */
