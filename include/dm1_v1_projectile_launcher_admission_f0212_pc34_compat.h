#ifndef FIRESTAFF_DM1_V1_PROJECTILE_LAUNCHER_ADMISSION_F0212_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_LAUNCHER_ADMISSION_F0212_PC34_COMPAT_H

#include "dm1_v1_c14_layout_pc34_compat.h"
#include "dm1_v1_c14_c15_graphics_catalog_pc34_compat.h"
#include "dm1_v1_throw_shoot_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB PROJEXPL.C F0212 launcher admission. It composes existing raw
 * F0810 source, F0142 aspect, C14 reservation, and decoder-catalog owners;
 * it neither allocates a replacement C14 nor supplies a substitute surface. */
typedef struct {
    const struct DungeonThings_Compat *things;
    const struct ProjectileCreateInput_Compat *createInput;
    const DM1_ProjectileCreateSourceReceiptPc34 *createSource;
    const DM1_C14PoolReservationPc34 *c14Reservation;
    int associatedThingType;
    int associatedThingSubtype;
    int weaponProjectileAspectOrdinal;
    const DM1_V1_F0115SourceMaterialHandoffPc34 *associatedMaterial;
} DM1_ProjectileLauncherPlanInputF0212Pc34;

typedef struct {
    int valid;
    unsigned short associatedThing;
    unsigned short reservedC14Thing;
    int expectedGraphicIndex;
    int objectAspectIndex;
    uint32_t rawAssociatedThingFNV1a;
    uint32_t createInputFNV1a;
    uint32_t associatedMaterialFNV1a;
    const char *sourceAnchor;
} DM1_ProjectileLauncherPlanF0212Pc34;

typedef struct {
    const DM1_ProjectileLauncherPlanF0212Pc34 *plan;
    const DM1_C14PoolReservationPc34 *c14Reservation;
    const DM1_V1_C14C15GraphicsCatalogPc34 *graphicsCatalog;
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *c14Material;
} DM1_ProjectileLauncherRuntimeInputF0212Pc34;

typedef struct {
    int valid;
    int shouldPublishRuntimeProjectile;
    unsigned short c14Thing;
    unsigned short associatedThing;
    uint32_t rawC14FNV1a;
    uint32_t rawAssociatedThingFNV1a;
    uint32_t graphicsPixelsFNV1a;
    const char *sourceAnchor;
} DM1_ProjectileLauncherRuntimeReceiptF0212Pc34;

int dm1_v1_projectile_launcher_plan_f0212_pc34(
    const DM1_ProjectileLauncherPlanInputF0212Pc34 *input,
    DM1_ProjectileLauncherPlanF0212Pc34 *outPlan);

int dm1_v1_projectile_launcher_runtime_admit_f0212_pc34(
    const DM1_ProjectileLauncherRuntimeInputF0212Pc34 *input,
    DM1_ProjectileLauncherRuntimeReceiptF0212Pc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
