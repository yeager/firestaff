/*
 * test_dm1_v1_tab07_subtype_creates_explosion_pc34_compat.c
 *
 * Source-locked to ReDMCSB PROJEXPL.C:459 (F0219/F0220 area):
 *   L0505_B_CreateExplosionOnImpact =
 *     (L0510_i_ProjectileAssociatedThingType == C15_THING_TYPE_EXPLOSION)
 *     && (L0486_T_ProjectileAssociatedThing != C0xFF81_THING_EXPLOSION_SLIME)
 *     && (L0486_T_ProjectileAssociatedThing != C0xFF86_THING_EXPLOSION_POISON_BOLT);
 *
 * TAB-07 (DM1 V1 functional-divergence-report.md):
 *   "Phase17_SubtypeCreatesExplosion (per-subtype explosion flag)
 *    is referenced but not visible [in the audit].  Cannot
 *    confirm source-lock."
 *
 * Firestaff's compat layer restates the predicate as a per-subtype
 * lookup table (since each projectile spell maps to one of the
 * 7 DM1 explosion thing types).  This test verifies the
 * F0820_PROJECTILE_ResolveCollision_Compat outExplosion is set
 * (i.e. an ExplosionInstance_Compat is emitted) for each subtype
 * that ReDMCSB aligns with the creates-explosion predicate.
 *
 *  T1  Fireball (0x80)         -> emits outExplosion
 *  T2  Lightning bolt (0x82)   -> emits outExplosion
 *  T3  Harm-non-material (0x83) -> emits outExplosion
 *  T4  Poison bolt (0x86)      -> emits outExplosion
 *  T5  Poison cloud (0x87)     -> emits outExplosion
 *  T6  Slime (0x81)            -> does NOT emit outExplosion
 *                                 (ReDMCSB exclusion 0xFF81)
 *  T7  Open door (0x84)        -> does NOT emit outExplosion
 *  T8  Smoke (0xA8)            -> does NOT emit outExplosion
 *  T9  Unmapped subtype (0x00) -> does NOT emit outExplosion
 *
 * Source-locked to ReDMCSB PROJEXPL.C:459 and DEFS.H explosion
 * type constants.
 */

#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

/* Helper: return 1 if outExplosion was emitted.  F0820 sets
 * emittedExplosion = 1 in addition to populating outExplosion.
 * Slot 0 is a valid slot, so we can't use slotIndex != -1 as
 * a sentinel (F0820 memsets outExplosion to 0, not -1). */
static int explosion_emitted(const struct ProjectileTickResult_Compat* r) {
    return r->emittedExplosion ? 1 : 0;
}

int main(void) {
    struct ProjectileInstance_Compat proj;
    struct CellContentDigest_Compat digest;
    struct RngState_Compat rng;
    struct ProjectileTickResult_Compat result;
    int rc;

    /* Set up a deterministic cell digest (empty corridor). */
    memset(&digest, 0, sizeof(digest));
    digest.sourceMapIndex = 0;
    digest.sourceMapX = 8;
    digest.sourceMapY = 8;
    digest.sourceSquareType = 0; /* CORRIDOR */

    /* Use a deterministic RNG. */
    rng.seed = 0xDEADBEEFu;

    /* T1: Fireball. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    proj.kineticEnergy = 100;
    proj.attack = 50;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1; /* default: not emitted */
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(rc >= 0, "T1: Fireball F0820 returns >= 0");
    CHECK(explosion_emitted(&result) == 1,
          "T1: Fireball emits outExplosion");

    /* T2: Lightning bolt. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 1,
          "T2: Lightning bolt emits outExplosion");

    /* T3: Harm-non-material. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 1,
          "T3: Harm-non-material emits outExplosion");

    /* T4: Poison bolt. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 1,
          "T4: Poison bolt emits outExplosion");

    /* T5: Poison cloud. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 1,
          "T5: Poison cloud emits outExplosion");

    /* T6: Slime (ReDMCSB exclusion 0xFF81). */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_SLIME;
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 0,
          "T6: Slime does NOT emit outExplosion (ReDMCSB exclusion)");

    /* T7: Open door. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_OPEN_DOOR;
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 0,
          "T7: Open door does NOT emit outExplosion");

    /* T8: Smoke. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = PROJECTILE_SUBTYPE_SMOKE;
    proj.projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 0,
          "T8: Smoke does NOT emit outExplosion");

    /* T9: Unmapped subtype. */
    memset(&proj, 0, sizeof(proj));
    proj.projectileSubtype = 0x00; /* unmapped */
    proj.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    memset(&result, 0, sizeof(result));
    result.outExplosion.slotIndex = -1;
    rc = F0820_PROJECTILE_ResolveCollision_Compat(
        &proj, &digest, 3, 0u, &rng, &result);
    CHECK(explosion_emitted(&result) == 0,
          "T9: Unmapped subtype does NOT emit outExplosion");

    printf("PASS: TAB-07 Phase17_SubtypeCreatesExplosion pin (9 scenarios)\n");
    return 0;
}
