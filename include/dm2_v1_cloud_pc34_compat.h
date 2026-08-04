/*
 * dm2_v1_cloud_pc34_compat.h — DM2 cloud/weather system.
 *
 * Source: skproject c_cloud.cpp (4 functions).
 * Cloud effects: actuator scan, creation, damage calculation, timer processing.
 */

#ifndef FIRESTAFF_DM2_V1_CLOUD_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CLOUD_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Cloud type table (table1d26c8) — 8 entries, indexed by cloud_type & 0x7F
 * Bit flags per cloud type:
 *   bit 0: add random bonus damage
 *   bit 1: affects door-record creatures (record type 0)
 *   bit 2: affects party (record == 0xFFFF)
 *   bit 3: affects creature-type records (record type 4)
 * ======================================================================== */

static const uint8_t dm2_cloud_type_table[8] = {
    0x00, /* 0: no effect */
    0x0F, /* 1: all flags */
    0x04, /* 2: party only */
    0x0F, /* 3: all flags */
    0x0F, /* 4: all flags */
    0x0F, /* 5: all flags */
    0x0D, /* 6: random + party + creature */
    0x0F, /* 7: all flags */
};

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_CreateCloudReceipt {
    bool    created;
    int16_t record_index;  /* allocated record, or -1 on failure */
} DM2_V1_CreateCloudReceipt;

typedef struct DM2_V1_CalcCloudDamageReceipt {
    int16_t damage;
} DM2_V1_CalcCloudDamageReceipt;

typedef struct DM2_V1_ProcessCloudReceipt {
    bool requeued;    /* true if timer was re-queued (cloud still alive) */
    bool deallocated; /* true if cloud record was freed */
} DM2_V1_ProcessCloudReceipt;

/* ========================================================================
 * Timer struct for cloud timers (type 0x19)
 * ======================================================================== */

typedef struct DM2_V1_CloudTimer {
    uint32_t ticks_and_map;
    uint8_t  type;       /* 0x19 for cloud */
    uint8_t  actor;
    int16_t  value_a;    /* encoded x,y */
    int16_t  value_b;    /* cloud record index */
} DM2_V1_CloudTimer;

/* ========================================================================
 * Callback struct — external dependencies for the cloud system
 * ======================================================================== */

typedef struct DM2_V1_CloudCallbacks {
    void *ctx;

    /* Record pool */
    int16_t  (*alloc_new_record)(void *ctx, int record_type);
    uint8_t *(*get_address_of_record)(void *ctx, uint16_t record);
    int16_t  (*get_next_record_link)(void *ctx, uint16_t record);
    void     (*dealloc_record)(void *ctx, uint16_t record_index);

    /* Tile access */
    uint16_t (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    uint8_t  (*get_tile_value)(void *ctx, int16_t x, int16_t y);
    uint8_t *(*get_address_of_tile_record)(void *ctx, int16_t x, int16_t y);

    /* Record list manipulation */
    void (*append_record_to)(void *ctx, uint16_t record, int16_t x, int16_t y);
    void (*cut_record_from)(void *ctx, uint16_t record, int16_t x, int16_t y);

    /* Timer queue */
    void (*queue_timer)(void *ctx, DM2_V1_CloudTimer *timer);

    /* Party state */
    int16_t party_map;
    int16_t party_x;
    int16_t party_y;
    int16_t current_map;
    int16_t view_map;      /* ddat.v1e0266 — map shown on viewport */
    uint32_t game_tick;

    /* Viewport dirty flag */
    uint8_t *viewport_dirty;  /* pointer to ddat.v1e0390.b_00 */

    /* Attack functions */
    void (*attack_party)(void *ctx, int16_t damage, int16_t flags, int16_t mode);
    void (*attack_creature)(void *ctx, uint16_t creature_rec, int16_t x,
                            int16_t y, int16_t attack_type, int16_t chance,
                            int16_t damage);
    void (*attack_door)(void *ctx, int16_t x, int16_t y, int16_t damage,
                        int16_t mode, int16_t extra);

    /* Creature queries */
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    int16_t (*query_1c9a_0958)(void *ctx, uint16_t creature_rec);
    uint8_t *(*query_creature_ai_spec_from_record)(void *ctx, uint8_t creature_type);
    uint8_t *(*query_creature_ai_spec_from_type)(void *ctx, uint16_t creature_type);
    int16_t (*query_creature_ai_spec_flags)(void *ctx, uint16_t creature_type);
    int16_t (*apply_creature_poison_resistance)(void *ctx, uint16_t creature_type,
                                                uint16_t damage);

    /* Actuator invocation */
    void (*invoke_actuator)(void *ctx, uint8_t *record, int16_t action, int16_t value);
    void (*invoke_message)(void *ctx, int16_t x, int16_t y, int16_t dir,
                           int16_t action, int32_t tick);

    /* Noise/sound */
    void (*queue_noise_gen2)(void *ctx, uint8_t type, uint8_t cloud_type,
                             uint8_t a, uint8_t b,
                             int16_t x, int16_t y,
                             int16_t c, int16_t d, int16_t intensity);

    /* Random */
    int16_t (*rand16)(void *ctx, int16_t max);
    bool    (*randbit)(void *ctx);

    /* Creature direction query for 1c9a_03cf scan */
    int16_t (*query_1c9a_03cf)(void *ctx, int16_t *out_x, int16_t *out_y,
                                uint16_t direction);

    /* Min/Max helpers */
    int16_t (*min16)(int16_t a, int16_t b);
    int16_t (*max16)(int16_t a, int16_t b);
} DM2_V1_CloudCallbacks;

/* ========================================================================
 * Function declarations
 * ======================================================================== */

/*
 * DM2_075f_0182 — Scan actuators on a tile for cloud-related effects.
 * Source: c_cloud.cpp DM2_075f_0182
 *
 * cloud_spell:  spell/cloud source identifier (e.g. 0xFF80..0xFFE4)
 * tile_x/y:     tile coordinates
 * intensity:    cloud intensity/damage level
 */
int32_t dm2_v1_cloud_actuator_scan(const DM2_V1_CloudCallbacks *cb,
                                   int32_t cloud_spell,
                                   int16_t tile_x, int16_t tile_y,
                                   int16_t intensity);

/*
 * DM2_CREATE_CLOUD — Create a cloud record with timer and damage.
 * Source: c_cloud.cpp DM2_CREATE_CLOUD
 *
 * cloud_spell:  spell/cloud source identifier
 * strength:     cloud strength (damage base)
 * x_encoded:    x coordinate (or encoded x|y if > 0xFF)
 * y_encoded:    y coordinate (or encoded x|y if > 0xFF)
 * direction:    cloud direction (0xFF = omnidirectional)
 */
DM2_V1_CreateCloudReceipt dm2_v1_create_cloud(const DM2_V1_CloudCallbacks *cb,
                                               int32_t cloud_spell,
                                               int16_t strength,
                                               int16_t x_encoded,
                                               int16_t y_encoded,
                                               int16_t direction);

/*
 * DM2_CALC_CLOUD_DAMAGE — Calculate cloud damage against a target.
 * Source: c_cloud.cpp DM2_CALC_CLOUD_DAMAGE
 *
 * cloud_record:  cloud record index
 * target_record: target record (0xFFFF = party, else creature/door)
 */
DM2_V1_CalcCloudDamageReceipt dm2_v1_calc_cloud_damage(
    const DM2_V1_CloudCallbacks *cb,
    uint16_t cloud_record,
    int32_t target_record);

/*
 * DM2_PROCESS_CLOUD — Timer-driven cloud processing.
 * Source: c_cloud.cpp DM2_PROCESS_CLOUD
 *
 * Handles damage application, decay, and timer re-queue or deallocation.
 */
DM2_V1_ProcessCloudReceipt dm2_v1_process_cloud(const DM2_V1_CloudCallbacks *cb,
                                                 DM2_V1_CloudTimer *timer);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CLOUD_PC34_COMPAT_H */
