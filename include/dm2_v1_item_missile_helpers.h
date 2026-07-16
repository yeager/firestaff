#ifndef FIRESTAFF_DM2_V1_ITEM_MISSILE_HELPERS_H
#define FIRESTAFF_DM2_V1_ITEM_MISSILE_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_ITEM_MISSILE_NULL_REF 0xffffu
#define DM2_V1_ITEM_HAND_ACTIVABLE_MASK 0x0001u

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int result;
    int blocked;
    const char *symbol;
    const char *source_path;
} DM2_V1_ItemMissileReceipt;

typedef struct {
    uint16_t item_ref;
    uint16_t dbspec_word;
    int16_t base_bonus;
    int16_t runtime_delta;
} DM2_V1_ItemBonusFacts;

typedef struct {
    uint16_t launcher_ref;
    uint16_t missile_ref;
    uint8_t launcher_accepts_missiles;
    uint8_t missile_class;
    uint8_t required_missile_class;
} DM2_V1_MissileLauncherFacts;

void dm2_v1_item_missile_receipt_clear(
    DM2_V1_ItemMissileReceipt *receipt);

int dm2_v1_IS_ITEM_HAND_ACTIVABLE(
    uint16_t item_ref,
    uint16_t dbspec_word,
    DM2_V1_ItemMissileReceipt *out_receipt);

int16_t dm2_v1_RETRIEVE_ITEM_BONUS(
    const DM2_V1_ItemBonusFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt);

uint16_t dm2_v1_GET_MISSILE_REF_OF_MINION(
    const uint16_t *missile_refs,
    size_t missile_ref_count,
    size_t slot_index,
    DM2_V1_ItemMissileReceipt *out_receipt);

int dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(
    const DM2_V1_MissileLauncherFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt);

const char *dm2_v1_item_missile_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ITEM_MISSILE_HELPERS_H */
