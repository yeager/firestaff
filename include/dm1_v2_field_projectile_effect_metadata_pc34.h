#ifndef FIRESTAFF_DM1_V2_FIELD_PROJECTILE_EFFECT_METADATA_PC34_H
#define FIRESTAFF_DM1_V2_FIELD_PROJECTILE_EFFECT_METADATA_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_EFFECT_FAMILY_FIREBALL = 0,
    DM1_V2_EFFECT_FAMILY_LIGHTNING,
    DM1_V2_EFFECT_FAMILY_POISON,
    DM1_V2_EFFECT_FAMILY_FLUXCAGE_FIELD,
    DM1_V2_EFFECT_FAMILY_SLIME,   /* ReDMCSB DEFS.H:421 — 0xFF81 */
    DM1_V2_EFFECT_FAMILY_SMOKE    /* ReDMCSB DEFS.H:421 — 0xFFA8 */
} DM1_V2_FieldProjectileEffectFamily;

typedef enum {
    DM1_V2_EFFECT_ROUTE_PROJECTILE_EVENT = 0,
    DM1_V2_EFFECT_ROUTE_EXPLOSION_EVENT,
    DM1_V2_EFFECT_ROUTE_FIELD_EVENT
} DM1_V2_FieldProjectileEffectRoute;

typedef struct {
    int16_t dm1Thing;
    int v1ThingType;
    int v1EventType;
    DM1_V2_FieldProjectileEffectRoute route;
    DM1_V2_FieldProjectileEffectFamily family;
    int presentationOnly;
    int mutatesGameplayState;
    const char* sourceEvidence;
} DM1_V2_FieldProjectileEffectMetadata;

size_t dm1_v2_field_projectile_effect_metadata_count(void);
const DM1_V2_FieldProjectileEffectMetadata*
dm1_v2_field_projectile_effect_metadata_at(size_t index);
const DM1_V2_FieldProjectileEffectMetadata*
dm1_v2_field_projectile_effect_metadata_for_dm1_thing(int16_t dm1Thing);
const char* dm1_v2_field_projectile_effect_family_name(
    DM1_V2_FieldProjectileEffectFamily family);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_FIELD_PROJECTILE_EFFECT_METADATA_PC34_H */
