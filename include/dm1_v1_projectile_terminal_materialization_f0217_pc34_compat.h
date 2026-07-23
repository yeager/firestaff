#ifndef FIRESTAFF_DM1_V1_PROJECTILE_TERMINAL_MATERIALIZATION_F0217_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_TERMINAL_MATERIALIZATION_F0217_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_projectile_damage_receipt_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB PROJEXPL.C F0217 terminal handoff. This consumes the completed
 * F0213-F0226 receipts and only admits their already-published PC34 C14/C15
 * records; allocation, linking, drawing, and deletion remain with owners. */
typedef struct {
    const struct DungeonDatState_Compat *dungeon;
    const struct DungeonThings_Compat *things;
    const DM1_ProjectileTerminationSourceReceiptPc34 *termination;
    const DM1_ProjectileMaterializationReceiptPc34 *f0215;
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *c14Material;
    const DM1_V1_F0115SourceMaterialHandoffPc34 *associatedMaterial;
    const DM1_ProjectileDamageSourceReceiptPc34 *damage;
    const DM1_C15C25PublicationReceiptPc34 *c15Publication;
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *c15Material;
    const DM1_ProjectileImpactWorldReceiptPc34 *impactWorld;
} DM1_ProjectileTerminalMaterializationInputF0217Pc34;

typedef struct {
    int valid;
    int shouldDeleteC14;
    int shouldMaterializeAssociatedThing;
    int shouldConsumeAssociatedPotion;
    int shouldAdvanceC15;
    unsigned short projectileThing;
    unsigned short associatedThing;
    unsigned short c15Thing;
    uint32_t rawC14FNV1a;
    uint32_t rawAssociatedThingFNV1a;
    uint32_t rawC15FNV1a;
    uint32_t associatedMaterialFNV1a;
    const char *sourceAnchor;
} DM1_ProjectileTerminalMaterializationReceiptF0217Pc34;

/* Returns one for a handled admission. A missing, stale, or synthetic source
 * returns valid=0 and leaves the terminal F0217 mutation unadmitted. */
int dm1_v1_projectile_terminal_materialization_f0217_pc34(
    const DM1_ProjectileTerminalMaterializationInputF0217Pc34 *input,
    DM1_ProjectileTerminalMaterializationReceiptF0217Pc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
