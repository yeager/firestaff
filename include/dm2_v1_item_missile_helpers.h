#ifndef FIRESTAFF_DM2_V1_ITEM_MISSILE_HELPERS_H
#define FIRESTAFF_DM2_V1_ITEM_MISSILE_HELPERS_H

/*
 * dm2_v1_item_missile_helpers.h — DM2 item/missile query helpers at
 * source-level parity with skproject.
 *
 * Source boundary:
 *
 *   c_querydb.cpp:949   DM2_IS_MISSILE_VALID_TO_LAUNCHER — the launcher
 *                       held in party.hero[eax].item[edx] and the
 *                       candidate missile (ebx) are both resolved
 *                       through DM2_QUERY_GDAT_DBSPEC_WORD_VALUE index
 *                       5.  The launcher word must carry bit 0x8000,
 *                       the missile word must not, and the launcher
 *                       word must share a set bit with the missile
 *                       word masked to 0x7fff.
 *   c_item.cpp:22       DM2_RETRIEVE_ITEM_BONUS — DBSPEC word at the
 *                       caller-supplied index; zero yields 0; bit
 *                       0x4000 clear gates on the ebx selector (0 =
 *                       the word must also carry bit 0x8000), bit
 *                       0x4000 set gates the ecx mode to -2, 2 or 3;
 *                       the result is the sign-extended low byte,
 *                       negated when the mode word is negative.
 *   c_querydb.cpp:1449  DM2_GET_MISSILE_REF_OF_MINION — filtered walk
 *                       of the minion record's word@2 chain; a node
 *                       qualifies when its handle bits 10-13 equal 14
 *                       (the missile DB index) and the filter is the
 *                       0xffff wildcard or equals the node's own
 *                       word@2.
 *   c_querydb.cpp:4562  DM2_IS_ITEM_HAND_ACTIVABLE — moneybox and
 *                       chest containers are activable outright;
 *                       otherwise the (cls1, cls2) key (or 0x16 /
 *                       herotype when the item ref is the 0xffff
 *                       "hero's own actions" sentinel) drives a scan
 *                       of command slots 8..11, collecting up to three
 *                       action entries that pass the loadable, cmdstr,
 *                       charge/pouch and skill-level gates.  A map
 *                       container is always activable.
 *
 * Fail-closed contract: every out-of-source datum (DBSPEC words,
 * record links, class bytes, cmdstr entries, charges, skill levels)
 * enters as caller-owned facts or callbacks.  Missing callbacks,
 * unresolvable records and bounded-walk overruns reject through the
 * receipt rather than being synthesised.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_ITEM_MISSILE_NULL_REF 0xffffu
#define DM2_V1_ITEM_MISSILE_END_MARKER 0xfffeu

/* DM2_QUERY_GDAT_DBSPEC_WORD_VALUE index used by the launcher test. */
#define DM2_V1_ITEM_MISSILE_DBSPEC_LAUNCHER_INDEX 5u
/* Launcher marker bit in DBSPEC word 5. */
#define DM2_V1_ITEM_MISSILE_LAUNCHER_BIT 0x8000u
/* Ammunition-class bits of DBSPEC word 5. */
#define DM2_V1_ITEM_MISSILE_CLASS_MASK 0x7fffu

/* Record handle bits 10-13 selecting the missile/projectile DB. */
#define DM2_V1_ITEM_MISSILE_RECORD_DB_INDEX 0xeu

/* Bound for the minion missile chain walk (fail-closed guard). */
#define DM2_V1_ITEM_MISSILE_MAX_CHAIN 256u

/* ddat.v1e0b40 holds at most three collected hand actions. */
#define DM2_V1_ITEM_HAND_ACTIVABLE_MAX_ACTIONS 3u
/* Command slots scanned by DM2_IS_ITEM_HAND_ACTIVABLE. */
#define DM2_V1_ITEM_HAND_ACTIVABLE_FIRST_CMD 8u
#define DM2_V1_ITEM_HAND_ACTIVABLE_END_CMD 12u
/* cls1 substituted when the item ref is the 0xffff sentinel. */
#define DM2_V1_ITEM_HAND_ACTIVABLE_HERO_CLS1 0x16u

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int result;
    int blocked;
    /* Bounded-walk / scan bookkeeping (0 when not applicable). */
    int steps;
    int action_count;
    const char *symbol;
    const char *source_path;
} DM2_V1_ItemMissileReceipt;

/* ------------------------------------------------------------------ */
/* DM2_IS_MISSILE_VALID_TO_LAUNCHER (c_querydb.cpp:949)                */
/* ------------------------------------------------------------------ */

typedef struct {
    /* party.hero[hero].item[hand_slot] — the held launcher. */
    uint16_t launcher_ref;
    /* the ebx candidate missile item. */
    uint16_t missile_ref;
    /* DM2_QUERY_GDAT_DBSPEC_WORD_VALUE(launcher_ref, 5). */
    uint16_t launcher_dbspec_word5;
    /* DM2_QUERY_GDAT_DBSPEC_WORD_VALUE(missile_ref, 5). */
    uint16_t missile_dbspec_word5;
} DM2_V1_MissileLauncherFacts;

/* ------------------------------------------------------------------ */
/* DM2_RETRIEVE_ITEM_BONUS (c_item.cpp:22)                             */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t item_ref;      /* eax */
    uint8_t dbspec_index;   /* edx low byte: DBSPEC word index */
    uint16_t dbspec_word;   /* the queried DBSPEC word value */
    int32_t select_flag;    /* ebx: 0 enforces the 0x8000 gate */
    int16_t mode;           /* ecx: -2/2/3 gate plus result sign */
} DM2_V1_ItemBonusFacts;

/* ------------------------------------------------------------------ */
/* DM2_GET_MISSILE_REF_OF_MINION (c_querydb.cpp:1449)                  */
/* ------------------------------------------------------------------ */

/* Caller-owned record access.  Both hooks return 1 on success, 0 when
 * the record cannot be resolved (the walk then fails closed). */
typedef struct {
    int (*record_word)(void *context, uint16_t record_ref,
                       unsigned offset, uint16_t *out_word);
    int (*next_link)(void *context, uint16_t record_ref,
                     uint16_t *out_next_ref);
    void *context;
    unsigned max_steps; /* 0 -> DM2_V1_ITEM_MISSILE_MAX_CHAIN */
} DM2_V1_RecordChainAccess;

/* ------------------------------------------------------------------ */
/* DM2_IS_ITEM_HAND_ACTIVABLE (c_querydb.cpp:4562)                     */
/* ------------------------------------------------------------------ */

/* One collected ddat.v1e0b40 entry. */
typedef struct {
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cmd;
} DM2_V1_HandActionEntry;

/* Caller-owned query surface.  Every hook mirrors one skproject
 * callee; a NULL hook that the scan needs fails the call closed. */
typedef struct {
    int (*is_container_moneybox)(void *context, uint16_t item_ref);
    int (*is_container_chest)(void *context, uint16_t item_ref);
    int (*is_container_map)(void *context, uint16_t item_ref);
    uint8_t (*query_cls1)(void *context, uint16_t item_ref);
    uint8_t (*query_cls2)(void *context, uint16_t item_ref);
    uint8_t (*hero_type)(void *context, int16_t hero_index);
    /* DM2_QUERY_GDAT_ENTRY_IF_LOADABLE(cls1, cls2, 5, cmd). */
    int (*gdat_entry_if_loadable)(void *context, uint8_t cls1,
                                  uint8_t cls2, uint8_t group,
                                  uint8_t cmd);
    /* DM2_QUERY_CMDSTR_ENTRY(cls1, cls2, cmd, field). */
    int16_t (*cmdstr_entry)(void *context, uint8_t cls1, uint8_t cls2,
                            uint8_t cmd, uint8_t field);
    /* DM2_query_2759_01fe(cmdstr field 2 value, item_ref). */
    int (*action_applies_to_item)(void *context, int16_t action_word,
                                  uint16_t item_ref);
    /* DM2_ADD_ITEM_CHARGE(item_ref, 0) — the current charge. */
    int16_t (*item_charge)(void *context, uint16_t item_ref);
    /* DM2_FIND_POUCH_OR_SCABBARD_POSSESSION_POS(hero, slot). */
    int16_t (*find_pouch_or_scabbard_pos)(void *context,
                                          int16_t hero_index,
                                          int16_t slot_index);
    /* DM2_QUERY_PLAYER_SKILL_LV(hero, skill, 1). */
    int16_t (*player_skill_level)(void *context, int16_t hero_index,
                                  int16_t skill, int16_t mode);
    void *context;
} DM2_V1_HandActivableCallbacks;

void dm2_v1_item_missile_receipt_clear(
    DM2_V1_ItemMissileReceipt *receipt);

/* Returns 1 when at least one hand action is available (or the item is
 * a moneybox, chest or map container), 0 otherwise / fail-closed.
 * `out_actions` (when non-NULL) receives up to
 * DM2_V1_ITEM_HAND_ACTIVABLE_MAX_ACTIONS collected entries; the count
 * is reported through `out_action_count` and the receipt. */
int dm2_v1_IS_ITEM_HAND_ACTIVABLE(
    const DM2_V1_HandActivableCallbacks *callbacks,
    int16_t hero_index,
    uint16_t item_ref,
    int16_t slot_index,
    DM2_V1_HandActionEntry *out_actions,
    size_t out_action_capacity,
    size_t *out_action_count,
    DM2_V1_ItemMissileReceipt *out_receipt);

int16_t dm2_v1_RETRIEVE_ITEM_BONUS(
    const DM2_V1_ItemBonusFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt);

/* Returns the qualifying missile record handle, or
 * DM2_V1_ITEM_MISSILE_NULL_REF for the source NULL return. */
uint16_t dm2_v1_GET_MISSILE_REF_OF_MINION(
    const DM2_V1_RecordChainAccess *chain,
    uint16_t minion_ref,
    uint16_t filter_word,
    DM2_V1_ItemMissileReceipt *out_receipt);

int dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(
    const DM2_V1_MissileLauncherFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt);

const char *dm2_v1_item_missile_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ITEM_MISSILE_HELPERS_H */
