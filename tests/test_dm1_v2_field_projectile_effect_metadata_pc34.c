#include "dm1_v2_field_projectile_effect_metadata_pc34.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int check_fireball_and_lightning_routes(void) {
    const DM1_V2_FieldProjectileEffectMetadata* fireball;
    const DM1_V2_FieldProjectileEffectMetadata* lightning;

    fireball = dm1_v2_field_projectile_effect_metadata_for_dm1_thing((int16_t)0xFF80);
    lightning = dm1_v2_field_projectile_effect_metadata_for_dm1_thing((int16_t)0xFF82);

    CHECK(fireball != 0);
    CHECK(lightning != 0);
    CHECK(fireball->v1ThingType == 15);
    CHECK(fireball->v1EventType == 25);
    CHECK(fireball->route == DM1_V2_EFFECT_ROUTE_EXPLOSION_EVENT);
    CHECK(fireball->family == DM1_V2_EFFECT_FAMILY_FIREBALL);
    CHECK(fireball->presentationOnly == 1);
    CHECK(fireball->mutatesGameplayState == 0);
    CHECK(strstr(fireball->sourceEvidence, "PROJEXPL.C:95-165") != 0);

    CHECK(lightning->family == DM1_V2_EFFECT_FAMILY_LIGHTNING);
    CHECK(lightning->v1EventType == 25);
    CHECK(lightning->presentationOnly == 1);
    CHECK(lightning->mutatesGameplayState == 0);
    return 0;
}

static int check_poison_projectile_and_cloud_routes(void) {
    const DM1_V2_FieldProjectileEffectMetadata* bolt;
    const DM1_V2_FieldProjectileEffectMetadata* cloud;

    bolt = dm1_v2_field_projectile_effect_metadata_for_dm1_thing((int16_t)0xFF86);
    cloud = dm1_v2_field_projectile_effect_metadata_for_dm1_thing((int16_t)0xFF87);

    CHECK(bolt != 0);
    CHECK(cloud != 0);
    CHECK(bolt->route == DM1_V2_EFFECT_ROUTE_PROJECTILE_EVENT);
    CHECK(bolt->v1ThingType == 14);
    CHECK(bolt->family == DM1_V2_EFFECT_FAMILY_POISON);
    CHECK(strstr(bolt->sourceEvidence, "PROJEXPL.C:43-92") != 0);
    CHECK(cloud->route == DM1_V2_EFFECT_ROUTE_EXPLOSION_EVENT);
    CHECK(cloud->v1ThingType == 15);
    CHECK(cloud->family == DM1_V2_EFFECT_FAMILY_POISON);
    CHECK(strstr(cloud->sourceEvidence, "PROJEXPL.C:817-864") != 0);
    CHECK(bolt->presentationOnly == 1 && cloud->presentationOnly == 1);
    CHECK(bolt->mutatesGameplayState == 0 && cloud->mutatesGameplayState == 0);
    return 0;
}

static int check_fluxcage_field_route(void) {
    const DM1_V2_FieldProjectileEffectMetadata* fluxcage;

    fluxcage = dm1_v2_field_projectile_effect_metadata_for_dm1_thing((int16_t)0xFFB2);
    CHECK(fluxcage != 0);
    CHECK(fluxcage->route == DM1_V2_EFFECT_ROUTE_FIELD_EVENT);
    CHECK(fluxcage->v1ThingType == 15);
    CHECK(fluxcage->v1EventType == 24);
    CHECK(fluxcage->family == DM1_V2_EFFECT_FAMILY_FLUXCAGE_FIELD);
    CHECK(fluxcage->presentationOnly == 1);
    CHECK(fluxcage->mutatesGameplayState == 0);
    CHECK(strstr(fluxcage->sourceEvidence, "PROJEXPL.C:987-994") != 0);
    CHECK(strstr(fluxcage->sourceEvidence, "DUNVIEW.C:6816-6831") != 0);
    return 0;
}

static int check_table_is_const_metadata_only(void) {
    const DM1_V2_FieldProjectileEffectMetadata* before;
    const DM1_V2_FieldProjectileEffectMetadata* after;
    size_t i;

    CHECK(dm1_v2_field_projectile_effect_metadata_count() == 7U);
    CHECK(dm1_v2_field_projectile_effect_metadata_at(7U) == 0);
    CHECK(dm1_v2_field_projectile_effect_metadata_for_dm1_thing((int16_t)0x1234) == 0);
    CHECK(strcmp(dm1_v2_field_projectile_effect_family_name(DM1_V2_EFFECT_FAMILY_POISON), "poison") == 0);

    for (i = 0; i < dm1_v2_field_projectile_effect_metadata_count(); ++i) {
        before = dm1_v2_field_projectile_effect_metadata_at(i);
        after = dm1_v2_field_projectile_effect_metadata_for_dm1_thing(before->dm1Thing);
        CHECK(before == after);
        CHECK(before->presentationOnly == 1);
        CHECK(before->mutatesGameplayState == 0);
        CHECK(before->sourceEvidence != 0);
    }
    return 0;
}

int main(void) {
    if (check_fireball_and_lightning_routes()) return 1;
    if (check_poison_projectile_and_cloud_routes()) return 1;
    if (check_fluxcage_field_route()) return 1;
    if (check_table_is_const_metadata_only()) return 1;
    puts("dm1_v2_field_projectile_effect_metadata_pc34: ok");
    return 0;
}
