#include "dm1_v2_field_projectile_effect_metadata_pc34.h"

/* DM1 V2 field/projectile presentation metadata.
 *
 * Source-lock anchors:
 * - ReDMCSB DEFS.H:421-430 names the special projectile-associated explosion
 *   thing values used by fireball, lightning, poison bolt/cloud and fluxcage.
 * - ReDMCSB PROJEXPL.C:43-92 owns projectile creation and C48/C49 movement
 *   events; PROJEXPL.C:95-165 owns explosion thing creation and C25 events.
 * - ReDMCSB PROJEXPL.C:817-864 owns explosion damage/state processing, while
 *   PROJEXPL.C:987-994 owns the C24 fluxcage removal event.
 * - ReDMCSB DUNVIEW.C:6816-6831 draws fields after the source object,
 *   creature, projectile and explosion route for the current square.
 *
 * V2 uses this table only to choose presentation families. The authoritative
 * projectile, explosion and field event state remains in the V1 source-shaped
 * runtime routes above; every entry below is explicitly non-mutating (mutatesGameplayState == 0). */

#define DM1_THING_TYPE_PROJECTILE 14
#define DM1_THING_TYPE_EXPLOSION 15
#define DM1_EVENT_REMOVE_FLUXCAGE 24
#define DM1_EVENT_EXPLOSION 25

#define DM1_THING_EXPLOSION_FIREBALL ((int16_t)0xFF80)
#define DM1_THING_EXPLOSION_LIGHTNING_BOLT ((int16_t)0xFF82)
#define DM1_THING_EXPLOSION_POISON_BOLT ((int16_t)0xFF86)
#define DM1_THING_EXPLOSION_POISON_CLOUD ((int16_t)0xFF87)
#define DM1_THING_EXPLOSION_FLUXCAGE ((int16_t)0xFFB2)

static const DM1_V2_FieldProjectileEffectMetadata kEffectMetadata[] = {
    {
        DM1_THING_EXPLOSION_FIREBALL,
        DM1_THING_TYPE_EXPLOSION,
        DM1_EVENT_EXPLOSION,
        DM1_V2_EFFECT_ROUTE_EXPLOSION_EVENT,
        DM1_V2_EFFECT_FAMILY_FIREBALL,
        1,
        0,
        "DEFS.H:421-428; PROJEXPL.C:95-165; PROJEXPL.C:823-831"
    },
    {
        DM1_THING_EXPLOSION_LIGHTNING_BOLT,
        DM1_THING_TYPE_EXPLOSION,
        DM1_EVENT_EXPLOSION,
        DM1_V2_EFFECT_ROUTE_EXPLOSION_EVENT,
        DM1_V2_EFFECT_FAMILY_LIGHTNING,
        1,
        0,
        "DEFS.H:421-428; PROJEXPL.C:95-165; PROJEXPL.C:823-827"
    },
    {
        DM1_THING_EXPLOSION_POISON_BOLT,
        DM1_THING_TYPE_PROJECTILE,
        DM1_EVENT_EXPLOSION,
        DM1_V2_EFFECT_ROUTE_PROJECTILE_EVENT,
        DM1_V2_EFFECT_FAMILY_POISON,
        1,
        0,
        "DEFS.H:421-428; PROJEXPL.C:43-92; PROJEXPL.C:574-585"
    },
    {
        DM1_THING_EXPLOSION_POISON_CLOUD,
        DM1_THING_TYPE_EXPLOSION,
        DM1_EVENT_EXPLOSION,
        DM1_V2_EFFECT_ROUTE_EXPLOSION_EVENT,
        DM1_V2_EFFECT_FAMILY_POISON,
        1,
        0,
        "DEFS.H:421-428; PROJEXPL.C:585; PROJEXPL.C:817-864"
    },
    {
        DM1_THING_EXPLOSION_FLUXCAGE,
        DM1_THING_TYPE_EXPLOSION,
        DM1_EVENT_REMOVE_FLUXCAGE,
        DM1_V2_EFFECT_ROUTE_FIELD_EVENT,
        DM1_V2_EFFECT_FAMILY_FLUXCAGE_FIELD,
        1,
        0,
        "DEFS.H:421-430; PROJEXPL.C:987-994; DUNVIEW.C:6816-6831"
    }
};

size_t dm1_v2_field_projectile_effect_metadata_count(void) {
    return sizeof(kEffectMetadata) / sizeof(kEffectMetadata[0]);
}

const DM1_V2_FieldProjectileEffectMetadata*
dm1_v2_field_projectile_effect_metadata_at(size_t index) {
    if (index >= dm1_v2_field_projectile_effect_metadata_count()) return 0;
    return &kEffectMetadata[index];
}

const DM1_V2_FieldProjectileEffectMetadata*
dm1_v2_field_projectile_effect_metadata_for_dm1_thing(int16_t dm1Thing) {
    size_t i;
    for (i = 0; i < dm1_v2_field_projectile_effect_metadata_count(); ++i) {
        if ((uint16_t)kEffectMetadata[i].dm1Thing == (uint16_t)dm1Thing) {
            return &kEffectMetadata[i];
        }
    }
    return 0;
}

const char* dm1_v2_field_projectile_effect_family_name(
    DM1_V2_FieldProjectileEffectFamily family) {
    switch (family) {
        case DM1_V2_EFFECT_FAMILY_FIREBALL: return "fireball";
        case DM1_V2_EFFECT_FAMILY_LIGHTNING: return "lightning";
        case DM1_V2_EFFECT_FAMILY_POISON: return "poison";
        case DM1_V2_EFFECT_FAMILY_FLUXCAGE_FIELD: return "fluxcage-field";
        default: return "unknown";
    }
}
