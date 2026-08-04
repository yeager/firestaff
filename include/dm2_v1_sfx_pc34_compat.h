#ifndef FIRESTAFF_DM2_V1_SFX_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_SFX_PC34_COMPAT_H

/*
 * dm2_v1_sfx_pc34_compat.h -- DM2 sound effects processing module.
 *
 * Source: skproject c_sfx.cpp (9 functions).
 * All public functions use callback-based architecture.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define DM2_V1_SFX_MAX_SAMPLES       16
#define DM2_V1_SFX_SAMPLE_REC_FREQ   8000
#define DM2_V1_SFX_MAX_DELAYED       8
#define DM2_V1_SFX_MAX_QUEUED        20
#define DM2_V1_SFX_MAX_QUEUED_NEG    6

/* ========================================================================
 * Sound entry structs
 * ======================================================================== */

/* s_sfx / s_sizee mapped entry: 14-byte sound descriptor */
typedef struct DM2_V1_SfxEntry {
    int32_t  l_00;          /* pointer/flag */
    int8_t   barr_04[6];    /* category, type, subtype, map, x, y */
    int16_t  w_0a;          /* volume parameter */
    int16_t  w_0c;          /* distance parameter */
} DM2_V1_SfxEntry;

/* Sound index entry (xsndptr2) */
typedef struct DM2_V1_SndIndexEntry {
    int16_t  w_00;
} DM2_V1_SndIndexEntry;

/* ========================================================================
 * SFX state
 * ======================================================================== */

typedef struct DM2_V1_SfxState {
    int      sample_index;
    int16_t  queued_count;       /* v1d2694 */
    int16_t  queued_neg_count;   /* v1d2696 */
} DM2_V1_SfxState;

/* ========================================================================
 * Callback struct
 * ======================================================================== */

typedef struct DM2_V1_SfxCallbacks {
    /* Query sound entry index from GDAT. Returns 0 if not found. */
    int16_t (*query_snd_entry_index)(void *ctx, int8_t cat, int8_t type, int8_t sub);

    /* Play a sound buffer */
    void (*play_sound)(void *ctx, int mode, void *sfx_data);

    /* Queue a timer */
    void (*queue_timer)(void *ctx, const void *timer_data);

    /* Map/level state */
    int16_t (*get_current_map)(void *ctx);
    int16_t (*get_party_map)(void *ctx);          /* v1e0282 */
    int16_t (*get_party_alt_map)(void *ctx);      /* v1e027c */
    int16_t (*get_party_x)(void *ctx);            /* v1e0270 */
    int16_t (*get_party_y)(void *ctx);            /* v1e0272 */
    int16_t (*get_party_dir)(void *ctx);          /* v1e0258 */
    int16_t (*get_view_map1)(void *ctx);          /* v1d62a4 */
    int16_t (*get_view_map2)(void *ctx);          /* v1d62a6 */
    uint8_t *(*get_view_data1)(void *ctx);        /* v1e08cc */
    uint8_t *(*get_view_data2)(void *ctx);        /* v1e08c8 */
    int16_t (*get_view_width1)(void *ctx);        /* v1e08d2 */
    int16_t (*get_view_width2)(void *ctx);        /* v1e08d0 */
    int16_t (*get_map_height)(void *ctx);

    /* Sound system state */
    void    *(*get_snd_ptr4)(void *ctx);          /* sndptr4 base */
    DM2_V1_SndIndexEntry *(*get_snd_index)(void *ctx); /* xsndptr2 */
    void    *(*get_snd_queue_pos)(void *ctx);     /* xsndptr5 */
    void    *(*get_snd_queue_neg)(void *ctx);     /* xsndptr3 */
    DM2_V1_SfxEntry *(*get_delayed_sounds)(void *ctx); /* sndptr1 */

    /* Level geometry pointers for sound occlusion */
    void    *(*get_level_sizee)(void *ctx);        /* v1e03c0 */
    void    *(*get_level_sizee_array)(void *ctx);  /* v1e03c8 */

    /* v1e0238 flag for distance halving */
    int16_t (*get_distance_halve_flag)(void *ctx);

    /* Game tick */
    int32_t (*get_game_tick)(void *ctx);

    void *ctx;
} DM2_V1_SfxCallbacks;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_SfxSoundDistanceReceipt {
    bool     valid;
    int32_t  distance;
} DM2_V1_SfxSoundDistanceReceipt;

typedef struct DM2_V1_SfxQueueNoiseReceipt {
    bool     queued;
} DM2_V1_SfxQueueNoiseReceipt;

/* ========================================================================
 * Public functions
 * ======================================================================== */

/*
 * Calculate sound distance/occlusion from a map position.
 * Source: R_1FB7D in c_sfx.cpp.
 */
DM2_V1_SfxSoundDistanceReceipt dm2_v1_sfx_calc_sound_distance(
    const DM2_V1_SfxCallbacks *cb,
    int16_t x, int16_t y);

/*
 * Queue a positional sound effect (GEN1).
 * Source: DM2_QUEUE_NOISE_GEN1 in c_sfx.cpp.
 */
DM2_V1_SfxQueueNoiseReceipt dm2_v1_sfx_queue_noise_gen1(
    const DM2_V1_SfxCallbacks *cb,
    DM2_V1_SfxState *state,
    int8_t cat, int8_t type, int8_t sub, int16_t vol,
    int16_t dist, int16_t x, int16_t y, int16_t mode);

/*
 * Queue a positional sound effect (GEN2 wrapper).
 * Source: DM2_QUEUE_NOISE_GEN2 in c_sfx.cpp.
 */
DM2_V1_SfxQueueNoiseReceipt dm2_v1_sfx_queue_noise_gen2(
    const DM2_V1_SfxCallbacks *cb,
    DM2_V1_SfxState *state,
    int8_t cat, int8_t type, int8_t sub, int8_t fallback_type,
    int16_t x, int16_t y, int16_t mode, int16_t vol, int16_t dist);

/*
 * Compute volume and pan from distance and direction.
 * Extracted from c_sfx::do_sound logic.
 */
void dm2_v1_sfx_calc_volume_pan(int8_t vol_level, int8_t dx, int8_t dy,
                                int16_t *out_volume, int16_t *out_pan);

/*
 * Initialize SFX state.
 */
void dm2_v1_sfx_state_init(DM2_V1_SfxState *state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SFX_PC34_COMPAT_H */
