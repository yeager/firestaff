#ifndef FIRESTAFF_DM2_V1_OBJECT_MODEL_H
#define FIRESTAFF_DM2_V1_OBJECT_MODEL_H
/*
 * dm2_v1_object_model.h — DM2 V1 Object/Tile Model
 *
 * DM2 Phase 2: Object types, tile states, world-state objects.
 * DM2 extends the object system from DM1 with:
 *   - 16 thing types (vs DM1's 13)
 *   - Extended door types (normal/dragon/animated mirrored)
 *   - Actuator/sensor system (floor/wall input → effector output)
 *   - Teleporter squares with dedicated GDAT category
 *   - Item affinity (magic/tech/hybrid)
 *
 * Source: ReDMCSB DEFS.H:985-1116 — MAP/TILE/THING types
 * Source: ReDMCSB DUNGEON.C:35-60 — thing data byte counts
 * Source: docs/dm2_actuators.md — actuator/sensor types
 * Source: docs/dm2_special_squares.md — door/teleporter/pit/ladder
 * Source: docs/dm2_le_objects.md — LE object headers
 * Source: SkWinCore.cpp:847-852 — item DB type switch
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── DM2 Thing Types ──────────────────────────────────────────────── */
/*
 * DM2 uses 16 thing types vs DM1's 13.
 * Source: ReDMCSB DUNGEON.C:35 — G0235_auc_Graphic559_ThingDataByteCount[16]
 */
typedef enum {
    DM2_THING_DOOR         = 0,   /* 4 bytes */
    DM2_THING_TELEPORTER   = 1,   /* 6 bytes */
    DM2_THING_TEXT_STRING  = 2,   /* 4 bytes */
    DM2_THING_SENSOR       = 3,   /* 8 bytes */
    DM2_THING_GROUP        = 4,   /* 16 bytes */
    DM2_THING_WEAPON       = 5,   /* 4 bytes */
    DM2_THING_ARMOUR       = 6,   /* 4 bytes */
    DM2_THING_SCROLL       = 7,   /* 4 bytes */
    DM2_THING_POTION       = 8,   /* 4 bytes */
    DM2_THING_CONTAINER    = 9,   /* 8 bytes */
    DM2_THING_JUNK         = 10,  /* 4 bytes */
    DM2_THING_UNUSED_11    = 11,  /* 0 bytes */
    DM2_THING_UNUSED_12    = 12,  /* 0 bytes */
    DM2_THING_UNUSED_13    = 13,  /* 0 bytes */
    DM2_THING_PROJECTILE   = 14, /* 8 bytes */
    DM2_THING_EXPLOSION    = 15, /* 4 bytes */
    DM2_THING_COUNT        = 16
} DM2_ThingType;

/* Thing record byte sizes (source-locked to ReDMCSB DUNGEON.C:35) */
extern const uint8_t dm2_thing_data_byte_count[16];

#define DM2_THING_MAX_DOOR_ORNAMENTS  4
#define DM2_THING_MAX_DOOR_SETS       2

/* ── DM2 Door Types ─────────────────────────────────────────────── */
/*
 * DM2 extends door types with:
 *   - Normal clan door (0x09)
 *   - Dragon door (0x0A) — skullkeep-specific, color key support
 *   - Animated mirrored door (field 0x20) — unused in standard dungeons
 * Source: SKWin.GDAT2.InternalCodes.txt (door fields)
 * Source: docs/dm2_special_squares.md §8
 */
typedef enum {
    DM2_DOOR_TYPE_NORMAL     = 0x09,
    DM2_DOOR_TYPE_DRAGON    = 0x0A,
} DM2_DoorType;

/* Door state machine values (ReDMCSB DEFS.H:1040-1045) */
typedef enum {
    DM2_DOOR_STATE_OPEN             = 0,
    DM2_DOOR_STATE_CLOSED_ONE_FOURTH  = 1,
    DM2_DOOR_STATE_CLOSED_HALF        = 2,
    DM2_DOOR_STATE_CLOSED_THREE_FOURTH = 3,
    DM2_DOOR_STATE_CLOSED            = 4,
    DM2_DOOR_STATE_DESTROYED         = 5,
} DM2_DoorState;

#define DM2_DOOR_STATE_MASK(square)   ((square) & 0x0007)
#define DM2_SET_DOOR_STATE(square, state) \
    (((square) & ~0x0007) | ((state) & 0x0007))

/* ── DM2 Actuator Types (sensor → effector) ─────────────────────── */
/*
 * Actuators are the output/effect side of DM2's trigger system.
 * Sensors (floor/wall squares) detect conditions and activate actuators.
 * Source: docs/dm2_actuators.md
 * Source: SKWIN/defines.h — ACTUATOR_TYPE_* enumeration
 */
typedef enum {
    DM2_ACTUATOR_TYPE_DM1_WALL_SWITCH          = 0x01,
    DM2_ACTUATOR_TYPE_2_STATE_WALL_SWITCH      = 0x17,
    DM2_ACTUATOR_TYPE_WALL_SWITCH              = 0x18,
    DM2_ACTUATOR_TYPE_KEY_HOLE                 = 0x1A,
    DM2_ACTUATOR_TYPE_PUSH_BUTTON_WALL_SWITCH   = 0x46,
    DM2_ACTUATOR_TYPE_SWITCH_SIGN_FOR_CREATURE = 0x26,
    DM2_ACTUATOR_TYPE_MISSILE_SHOOTER          = 0x08,
    DM2_ACTUATOR_TYPE_WEAPON_SHOOTER           = 0x09,
    DM2_ACTUATOR_TYPE_ITEM_SHOOTER             = 0x0E,
    DM2_ACTUATOR_TYPE_COUNTER                  = 0x1D,
    DM2_ACTUATOR_TYPE_TICK_GENERATOR           = 0x1E,
    DM2_ACTUATOR_TYPE_RELAY_1                  = 0x20,
    DM2_ACTUATOR_TYPE_CROSS_MAP                = 0x16,
    DM2_ACTUATOR_TYPE_INVERSE_FLAG             = 0x43,
    DM2_ACTUATOR_TYPE_TEST_FLAG                = 0x44,
    DM2_ACTUATOR_TYPE_CREATURE_GENERATOR       = 0x2E,
    DM2_ACTUATOR_TYPE_ITEM_GENERATOR           = 0x3C,
    DM2_ACTUATOR_TYPE_FLYING_ITEM_CATCHER      = 0x22,
    DM2_ACTUATOR_TYPE_FLYING_ITEM_TELEPORTER   = 0x23,
    DM2_ACTUATOR_TYPE_ITEM_CAPTURE             = 0x47,
    DM2_ACTUATOR_TYPE_ORNATE_ANIMATOR          = 0x2C,
    DM2_ACTUATOR_TYPE_ORNATE_ANIMATOR_2         = 0x32,
    DM2_ACTUATOR_TYPE_ORNATE_STEP_ANIMATOR     = 0x41,
    DM2_ACTUATOR_TYPE_SHOP_PANEL               = 0x3F,
    DM2_ACTUATOR_TYPE_ITEM_WATCHER             = 0x03,
    DM2_ACTUATOR_TYPE_CHARGED_ITEM_WATCHER     = 0x15,
    DM2_ACTUATOR_TYPE_ITEM_RECYCLER            = 0x40,
    DM2_ACTUATOR_TYPE_RESURRECTOR             = 0x7E,
    DM2_ACTUATOR_TYPE_CHAMPION_MIRROR          = 0x7F,
} DM2_ActuatorType;

/* Actuator messages (door action types) */
typedef enum {
    DM2_ACTMSG_OPEN_SET  = 0x00,  /* Force door open */
    DM2_ACTMSG_CLOSE_CLEAR = 0x01,/* Force door closed */
    DM2_ACTMSG_TOGGLE    = 0x02,  /* Toggle current state */
} DM2_ActuatorMsg;

/* ── DM2 Floor Sensor Types ──────────────────────────────────────── */
/*
 * Floor sensors detect conditions and activate actuators.
 * Source: docs/dm2_sensors.md
 * Source: SKWIN/defines.h — ACTUATOR_FLOOR_TYPE__*
 */
typedef enum {
    DM2_FLOOR_SENSOR_EVERYTHING        = 0x01,  /* Any entity */
    DM2_FLOOR_SENSOR_PARTY             = 0x03,  /* Party only */
    DM2_FLOOR_SENSOR_ITEM              = 0x04,  /* Item dropped/placed */
    DM2_FLOOR_SENSOR_CREATURE          = 0x07,  /* Creature steps */
    DM2_FLOOR_SENSOR_ITEM_POSSESSION   = 0x08,  /* Party member with item */
    DM2_FLOOR_SENSOR_CREATURE_KILLER   = 0x0B,  /* Kills creatures */
    DM2_FLOOR_SENSOR_CROSS_MAP         = 0x16,  /* Cross-map teleport */
    DM2_FLOOR_SENSOR_COUNTER           = 0x1D,  /* Counter reaches zero */
    DM2_FLOOR_SENSOR_INFINITE_TICK_GEN = 0x1E,  /* Periodic firing */
    DM2_FLOOR_SENSOR_ARRIVAL_DEPARTURE = 0x21,  /* Party arrive/depart */
    DM2_FLOOR_SENSOR_MISSILE_EXPLOSION = 0x26,  /* Missile lands */
    DM2_FLOOR_SENSOR_ALCOVE_ITEM       = 0x2A,  /* Item in alcove */
    DM2_FLOOR_SENSOR_PARTY_TELEPORTER  = 0x2E,  /* Party teleporter */
    DM2_FLOOR_SENSOR_SHOP              = 0x30,  /* Shop entry */
    DM2_FLOOR_SENSOR_CREATURE_ANIMATOR = 0x3A,  /* Animates creatures */
    DM2_FLOOR_SENSOR_ITEM_TELEPORTER   = 0x3B,  /* Teleports items */
    DM2_FLOOR_SENSOR_ITEM_CAPTURE_CREATURE = 0x49, /* Captures from creature */
} DM2_FloorSensorType;

/* ── DM2 Item DB Types ───────────────────────────────────────────── */
/*
 * DM2 stores items in 4 database categories:
 *   dbWeapon (5), dbCloth (6), dbScroll (7), dbMisc (10)
 * Source: SkWinCore.cpp:847-852, DME.h
 */
typedef enum {
    DM2_DB_WEAPON  = 5,
    DM2_DB_CLOTH   = 6,
    DM2_DB_SCROLL  = 7,
    DM2_DB_MISC    = 10,
} DM2_DbType;

/* Item affinity (tech/magic hybrid system) */
typedef enum {
    DM2_ITEM_AFFINITY_MAGIC   = 0,
    DM2_ITEM_AFFINITY_TECH    = 1,
    DM2_ITEM_AFFINITY_HYBRID  = 2,
} DM2_ItemAffinity;

/* ── DM2 Object State Structures ─────────────────────────────────── */

/* Door object state (4 bytes, thing type 0) */
typedef struct {
    uint8_t  type;          /* DM2_DOOR_TYPE_* */
    uint8_t  state;         /* DM2_DOOR_STATE_* */
    uint8_t  door_set;      /* 0 or 1 */
    uint8_t  flags;         /* bit 0=locked, bit 1=visible, etc. */
    int16_t  target_x;      /* target map X (if cross-map) */
    int16_t  target_y;      /* target map Y */
    int16_t  target_level;  /* target map index */
} DM2_Object_Door;

/* Teleporter object state (6 bytes, thing type 1) */
typedef struct {
    int16_t  target_x;
    int16_t  target_y;
    int16_t  target_level;
    uint8_t  flags;
    uint8_t  teleporter_type; /* GDAT_CATEGORY_TELEPORTERS type */
} DM2_Object_Teleporter;

/* Sensor object state (8 bytes, thing type 3) */
typedef struct {
    uint8_t  sensor_type;   /* DM2_FLOOR_SENSOR_* or wall actuator */
    uint8_t  target_type;   /* DM2_ACTUATOR_TYPE_* */
    uint16_t target_id;     /* actuator target ID */
    int16_t  target_map_x;
    int16_t  target_map_y;
    int16_t  target_level;
} DM2_Object_Sensor;

/* Projectile object state (8 bytes, thing type 14) */
typedef struct {
    int16_t  x;
    int16_t  y;
    int16_t  dx;
    int16_t  dy;
    uint8_t  projectile_type;
    uint8_t  flags;
} DM2_Object_Projectile;

/* ── DM2 World Object Model ───────────────────────────────────────── */

/* Full object record for any thing type */
typedef struct {
    DM2_ThingType type;
    int           level;      /* map index */
    int           x;
    int           y;
    uint8_t       data[16];   /* raw thing data bytes */
    /* Parsed fields union */
    union {
        DM2_Object_Door       door;
        DM2_Object_Teleporter teleporter;
        DM2_Object_Sensor     sensor;
        DM2_Object_Projectile projectile;
    } u;
} DM2_ObjectRecord;

/* Object model — holds all objects for a dungeon level */
typedef struct {
    int           object_count;
    DM2_ObjectRecord *objects;
} DM2_ObjectModel;

/* Load thing records from dungeon raw data.
 * Source: ReDMCSB DUNGEON.C:35-60 — G0235/G0236 thing tables */
int dm2_v1_object_model_load(DM2_ObjectModel *model,
                             const uint8_t *dungeon_raw,
                             int dungeon_size,
                             int level);
void dm2_v1_object_model_free(DM2_ObjectModel *model);

/* Tile utility functions (also usable standalone) */
int dm2_v1_tile_get_type(const uint8_t *tile_raw);
int dm2_v1_tile_is_walkable(int tile_type);
int dm2_v1_tile_get_door_state(uint16_t tile_raw);

const char *dm2_v1_object_model_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_OBJECT_MODEL_H */