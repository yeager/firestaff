
#ifndef FIRESTAFF_DM2_V1_PRESSURE_PLATE_H
#define FIRESTAFF_DM2_V1_PRESSURE_PLATE_H

/*
 * dm2_v1_pressure_plate.h - DM2 V1 Pressure Plate Parity
 *
 * DM2 Phase 4 mechanics parity: pressure plates (also called "sensors"
 * in DM1 parlance, "weight plates" or "triggers" colloquially).
 *
 * In DM2, pressure plates are floor sensors that detect when the party
 * (or specific items) is standing on them, and they activate a target
 * actuator (typically a door or pit) when the condition is met.
 *
 * Source-lock anchors (DM2 decompilation via skproject):
 *   skproject/SKULLWIN/c_sensor.cpp       - pressure plate sensor logic
 *   skproject/SKULLWIN/c_actuator.cpp     - target actuation (door/pit)
 *   skproject/SKWIN/SkGlobal.cpp:1112-1170 - dPressurePlatesTable
 *   skproject/SKWIN/DME.h:1456-1504       - pressure_plate_descriptor_t
 *   ReDMCSB MOVESENS.C:1000-1100          - F0268_SENSOR_AddEvent (DM1 parity)
 *   ReDMCSB docs/dm2_sensors.md           - DM2 sensor types
 *
 * Pressure plate kinds in DM2:
 *   - WEIGHT (party weight triggers when on plate)
 *   - ITEM   (specific item on plate)
 *   - TIME   (periodic timer)
 *
 * Targets: door toggle, pit toggle, message display, message-only.
 *
 * DM2 difference vs DM1:
 *   - DM1 sensors (ReDMCSB SENSOR.C) detect party/items/creatures.
 *   - DM2 adds weight thresholds (party must weigh >= threshold).
 *   - DM2 supports time-based triggering (DM1 only has action-triggered).
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ───────────────────────────────────────────────────── */
#define DM2_PLATE_MAX_PLATES         16     /* max active plates tracked */
#define DM2_PLATE_MAX_TRIGGERED      64     /* max history entries */
#define DM2_PLATE_NUM_BUILTIN         5     /* built-in plate catalog */

/* Plate kinds */
typedef enum {
    DM2_PLATE_KIND_WEIGHT     = 1,    /* party weight threshold */
    DM2_PLATE_KIND_ITEM       = 2,    /* specific item on plate */
    DM2_PLATE_KIND_TIME       = 3,    /* periodic timer */
    DM2_PLATE_KIND_PARTY      = 4,    /* any party member */
    DM2_PLATE_KIND_CREATURE   = 5,    /* any creature */
} DM2_PlateKind;

/* Target actuator kinds (mirror dm2_v1_door_mechanics) */
typedef enum {
    DM2_PLATE_TARGET_DOOR_TOGGLE  = 1,
    DM2_PLATE_TARGET_DOOR_OPEN    = 2,
    DM2_PLATE_TARGET_DOOR_CLOSE   = 3,
    DM2_PLATE_TARGET_PIT_TOGGLE   = 4,
    DM2_PLATE_TARGET_MESSAGE      = 5,
    DM2_PLATE_TARGET_CREATURE_SPAWN = 6,
} DM2_PlateTarget;

/* Pressure plate descriptor */
typedef struct {
    int   plate_id;
    int   kind;          /* DM2_PLATE_KIND_* */
    int   map_x;
    int   map_y;
    int   map_level;
    /* Trigger condition (depends on kind) */
    int   weight_threshold;     /* WEIGHT: party weight >= triggers */
    int   required_item_id;     /* ITEM: 0 = any item */
    int   time_period_ms;       /* TIME: re-fire period in ms */
    /* Target */
    int   target_kind;          /* DM2_PLATE_TARGET_* */
    int   target_x;
    int   target_y;
    int   target_level;
    /* Behavior */
    int   fire_once;            /* 1 = single shot, 0 = repeating */
    int   one_way;              /* 1 = fires on enter, resets on leave */
    int   enabled;              /* 1 = active, 0 = disabled */
} DM2_V1_PressurePlate;

/* Per-plate state */
typedef struct {
    int   plate_id;
    int   active;          /* currently triggered */
    int   fired_count;     /* total fires observed */
    int   party_present;   /* party is on plate */
    int   item_present;    /* required item is on plate */
    int   last_fire_ms;    /* last fire time (ms) */
} DM2_V1_PlateState;

/* Result codes */
typedef enum {
    DM2_PLATE_RESULT_OK = 0,
    DM2_PLATE_RESULT_NOT_FOUND,
    DM2_PLATE_RESULT_DISABLED,
    DM2_PLATE_RESULT_NO_PARTY,
    DM2_PLATE_RESULT_INSUFFICIENT_WEIGHT,
    DM2_PLATE_RESULT_WRONG_ITEM,
    DM2_PLATE_RESULT_NOT_TIME_YET,
    DM2_PLATE_RESULT_ALREADY_FIRED,  /* fire_once + already fired */
} DM2_PlateResult;

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_plate_reset_state(void);
void dm2_v1_plate_set_party_weight(int weight);
void dm2_v1_plate_set_party_position(int x, int y, int level);
void dm2_v1_plate_set_item_on_floor(int item_id, int x, int y, int level);

/* ── Catalog ────────────────────────────────────────────────────── */
int  dm2_v1_plate_get_builtin_count(void);
const DM2_V1_PressurePlate *dm2_v1_plate_get_builtin(int plate_id);
int  dm2_v1_plate_lookup_index(int plate_id);

/* ── Activation / reset API ─────────────────────────────────────── */
int  dm2_v1_plate_activate(int plate_id);              /* mark party/item present */
int  dm2_v1_plate_deactivate(int plate_id);           /* party/item left */
int  dm2_v1_plate_check(int plate_id, int now_ms);    /* tick-based evaluator */
int  dm2_v1_plate_force_fire(int plate_id);           /* manual override */
int  dm2_v1_plate_reset_fire_count(int plate_id);     /* re-arm fire_once */

/* ── State / door state mutators ────────────────────────────────── */
int  dm2_v1_plate_get_state_for(int plate_id);        /* 0=inactive, 1=active */
int  dm2_v1_plate_get_fire_count(int plate_id);
int  dm2_v1_plate_get_door_state_after_fire(int plate_id); /* 0..5 */
const DM2_V1_PlateState *dm2_v1_plate_get_state(int plate_id);
const char *dm2_v1_plate_get_target_message(int plate_id);

/* ── Observability ──────────────────────────────────────────────── */
int  dm2_v1_plate_fire_total(void);
int  dm2_v1_plate_active_count(void);

const char *dm2_v1_pressure_plate_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PRESSURE_PLATE_H */
