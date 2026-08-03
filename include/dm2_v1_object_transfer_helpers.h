#ifndef FIRESTAFF_DM2_V1_OBJECT_TRANSFER_HELPERS_H
#define FIRESTAFF_DM2_V1_OBJECT_TRANSFER_HELPERS_H

/*
 * dm2_v1_object_transfer_helpers.h — DM2 object-transfer helpers at
 * source-level parity with skproject.
 *
 * Source boundary:
 *
 *   c_hero.cpp:2485   DM2_REMOVE_POSSESSION — the edx slot selects
 *                     party.hero[eax].item[slot] for slot < 30 and
 *                     party.hand_container[(2 * slot - 60) / 2], i.e.
 *                     slot - 30, otherwise.  The slot is cleared to
 *                     0xffff and its former content returned; an
 *                     already-empty slot returns immediately with no
 *                     side effects.  A non-empty removal refreshes the
 *                     right-hand squad panel when the hero is the
 *                     current active hero, the slot is hand 0/1 and it
 *                     matches party.curactmode, and then runs
 *                     DM2_PROCESS_ITEM_BONUS(hero, removed, slot, -1).
 *   c_item.cpp:1146   DM2_PUT_OBJECT_INTO_CONTAINER — the pending drop
 *                     target ddat.v1d6700 is consumed (cleared to
 *                     0xffff); each of the eight party.hand_container
 *                     slots that is occupied is cleared and its record
 *                     appended to the target container's chain through
 *                     DM2_APPEND_RECORD_TO(record, target + 2, -1, 0).
 *   c_hero.cpp:3643   DM2_LOAD_PROJECTILE_TO_HAND — clears the hand's
 *                     cooldown, returns on a dead hero or a hand index
 *                     outside 0..1, captures and clears handcmd and
 *                     handdefenseclass, then reloads per captured
 *                     command: 0x20 (missile) equips into the opposite
 *                     hand, 0x2a (scroll/spell) into this hand.  Both
 *                     search item[12] first, then item[12]'s chest
 *                     chain, then item slots 7..9; the 0x2a path also
 *                     falls back to removing item[12] itself.
 *
 * Fail-closed contract: hero item arrays, hand containers and record
 * chains stay caller-owned; every out-of-module effect (record append,
 * record cut, equip, item-bonus processing, panel refresh) is either a
 * caller-supplied callback or a receipted caller responsibility.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_OBJECT_TRANSFER_NULL 0xffffu
#define DM2_V1_OBJECT_TRANSFER_END_MARKER 0xfffeu
#define DM2_V1_OBJECT_TRANSFER_MAX_LINKS 64u

/* party.hero[].item[] slots addressed directly by REMOVE_POSSESSION. */
#define DM2_V1_OBJECT_TRANSFER_HERO_ITEM_SLOTS 30u
/* party.hand_container[] slots (slot indices 30..37). */
#define DM2_V1_OBJECT_TRANSFER_HAND_CONTAINER_SLOTS 8u
/* The chest/quiver possession slot searched by the reload paths. */
#define DM2_V1_OBJECT_TRANSFER_CHEST_SLOT 12u
/* Inclusive pouch slot range scanned by the reload fallback. */
#define DM2_V1_OBJECT_TRANSFER_POUCH_FIRST_SLOT 7u
#define DM2_V1_OBJECT_TRANSFER_POUCH_LAST_SLOT 9u

/* Captured handcmd values driving the reload. */
#define DM2_V1_OBJECT_TRANSFER_HANDCMD_MISSILE 0x20
#define DM2_V1_OBJECT_TRANSFER_HANDCMD_SPELL 0x2a

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    int mutated;
    uint16_t object_ref;
    uint16_t container_ref;
    uint16_t previous_ref;
    uint16_t next_ref;
    uint16_t new_head_ref;
    uint16_t hand_ref;
    /* REMOVE_POSSESSION */
    uint16_t removed_ref;      /* the source return value */
    int16_t slot_index;
    int ui_refresh_needed;     /* DM2_DISPLAY_RIGHT_PANEL_SQUAD_HANDS */
    int item_bonus_pending;    /* caller must run DM2_PROCESS_ITEM_BONUS */
    /* PUT_OBJECT_INTO_CONTAINER */
    int moved_count;
    unsigned moved_slot_mask;
    /* LOAD_PROJECTILE_TO_HAND */
    int handcmd;               /* the captured hero->handcmd[hand] */
    int path;                  /* DM2_V1_LoadProjectilePath */
    int equipped;              /* an equip callback ran */
    int16_t equip_hand;
    const char *symbol;
    const char *source_path;
} DM2_V1_ObjectTransferReceipt;

typedef struct {
    uint16_t ref;
    uint16_t next_ref;
} DM2_V1_ObjectTransferLink;

/* ------------------------------------------------------------------ */
/* DM2_REMOVE_POSSESSION (c_hero.cpp:2485)                             */
/* ------------------------------------------------------------------ */

/* Caller-owned, mutable backing storage for the two possession
 * arrays the source slot index selects between. */
typedef struct {
    uint16_t *hero_items;          /* party.hero[hero].item[] */
    size_t hero_item_count;        /* >= 30 for full slot coverage */
    uint16_t *hand_container;      /* party.hand_container[] */
    size_t hand_container_count;   /* >= 8 for full slot coverage */
    int16_t hero_index;            /* the eax hero */
    int16_t cur_act_hero;          /* party.curacthero */
    int16_t cur_act_mode;          /* party.curactmode */
} DM2_V1_PossessionSlots;

/* Returns the removed object ref, or DM2_V1_OBJECT_TRANSFER_NULL when
 * the slot was already empty or the call failed closed. */
uint16_t dm2_v1_REMOVE_POSSESSION(
    DM2_V1_PossessionSlots *slots,
    int16_t slot_index,
    DM2_V1_ObjectTransferReceipt *out_receipt);

/* ------------------------------------------------------------------ */
/* DM2_PUT_OBJECT_INTO_CONTAINER (c_item.cpp:1146)                     */
/* ------------------------------------------------------------------ */

/* DM2_APPEND_RECORD_TO(record, container + 2, -1, 0).  Returns 1 on
 * success; 0 fails the batch closed at that slot. */
typedef int (*DM2_V1_ObjectTransferAppend)(void *context,
                                           uint16_t container_ref,
                                           uint16_t object_ref);

/* Consumes `*pending_target_ref` (ddat.v1d6700), clearing it, and
 * moves every occupied hand-container slot onto that container.
 * Returns 1 when the batch ran (including the no-target early return),
 * 0 fail-closed. */
int dm2_v1_PUT_OBJECT_INTO_CONTAINER(
    uint16_t *pending_target_ref,
    uint16_t *hand_container,
    size_t hand_container_count,
    DM2_V1_ObjectTransferAppend append,
    void *append_context,
    DM2_V1_ObjectTransferReceipt *out_receipt);

/* The bounded single-object append primitive backing the batch: plans
 * the tail insertion of `object_ref` on the caller-owned chain rooted
 * at `container_head_ref`.  Kept exposed for callers that model the
 * chain as an explicit link table rather than through the append
 * callback. */
int dm2_v1_object_transfer_append_to_chain(
    const DM2_V1_ObjectTransferLink *links,
    size_t link_count,
    uint16_t container_ref,
    uint16_t container_head_ref,
    uint16_t object_ref,
    uint16_t *out_new_head_ref,
    uint16_t *out_previous_tail_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt);

/* ------------------------------------------------------------------ */
/* DM2_LOAD_PROJECTILE_TO_HAND (c_hero.cpp:3643)                       */
/* ------------------------------------------------------------------ */

typedef enum {
    DM2_V1_LOAD_PROJECTILE_PATH_NONE = 0,
    DM2_V1_LOAD_PROJECTILE_PATH_HERO_DEAD,
    DM2_V1_LOAD_PROJECTILE_PATH_BAD_HAND,
    DM2_V1_LOAD_PROJECTILE_PATH_UNHANDLED_CMD,
    DM2_V1_LOAD_PROJECTILE_PATH_TARGET_HAND_BUSY,
    DM2_V1_LOAD_PROJECTILE_PATH_FROM_CHEST_SLOT,   /* item[12] itself */
    DM2_V1_LOAD_PROJECTILE_PATH_FROM_CHEST_CHAIN,  /* inside item[12] */
    DM2_V1_LOAD_PROJECTILE_PATH_FROM_POUCH,        /* item[7..9] */
    DM2_V1_LOAD_PROJECTILE_PATH_EXHAUSTED
} DM2_V1_LoadProjectilePath;

typedef struct {
    /* DM2_IS_MISSILE_VALID_TO_LAUNCHER(hero, hand_slot, candidate). */
    int (*is_missile_valid_to_launcher)(void *context,
                                        int16_t hero_index,
                                        int16_t hand_slot,
                                        uint16_t candidate_ref);
    /* DM2_2759_0e93(hero, candidate, handcmd) — the 0x2a predicate. */
    int (*is_item_valid_for_command)(void *context,
                                     int16_t hero_index,
                                     uint16_t candidate_ref,
                                     int16_t handcmd);
    int (*is_container_chest)(void *context, uint16_t item_ref);
    /* word_at(record_of(chest) + 2, 0) — the chest chain head. */
    int (*chest_chain_head)(void *context, uint16_t chest_ref,
                            uint16_t *out_ref);
    /* DM2_GET_NEXT_RECORD_LINK. */
    int (*next_record_link)(void *context, uint16_t record_ref,
                            uint16_t *out_ref);
    /* DM2_CUT_RECORD_FROM(record, chest + 2, -1, 0). */
    int (*cut_record_from_chest)(void *context, uint16_t chest_ref,
                                 uint16_t record_ref);
    /* DM2_EQUIP_ITEM_TO_HAND(hero, item, hand). */
    int (*equip_item_to_hand)(void *context, int16_t hero_index,
                              uint16_t item_ref, int16_t hand_slot);
    void *context;
} DM2_V1_LoadProjectileCallbacks;

typedef struct {
    DM2_V1_PossessionSlots slots;  /* hero items + hand containers */
    int16_t hand_slot;             /* the edx hand (0 or 1) */
    int16_t hero_cur_hp;           /* hero->curHP */
    int16_t *handcooldown;         /* hero->handcooldown[2], mutated */
    int16_t *handcmd;              /* hero->handcmd[2], mutated */
    int16_t *handdefenseclass;     /* hero->handdefenseclass[2], mutated */
} DM2_V1_LoadProjectileToHandInput;

/* Returns 1 when an item was equipped into a hand, 0 for every source
 * early return and for fail-closed input. */
int dm2_v1_LOAD_PROJECTILE_TO_HAND(
    DM2_V1_LoadProjectileToHandInput *input,
    const DM2_V1_LoadProjectileCallbacks *callbacks,
    DM2_V1_ObjectTransferReceipt *out_receipt);

void dm2_v1_object_transfer_receipt_clear(
    DM2_V1_ObjectTransferReceipt *receipt);

const char *dm2_v1_object_transfer_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_OBJECT_TRANSFER_HELPERS_H */
