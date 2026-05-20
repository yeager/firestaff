/*
 * CTest integration test for DM1 V1 projectile/explosion viewport rendering.
 *
 * Verifies source-locked data tables and rendering queries against ReDMCSB
 * DUNVIEW.C F0115, DUNGEON.C F0142, and DEFS.H constants.
 */

#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define ASSERT_EQ(actual, expected, label) do { \
    int _a = (actual), _e = (expected); \
    if (_a != _e) { \
        fprintf(stderr, "FAIL %s: expected %d, got %d\n", label, _e, _a); \
        ++g_failures; \
    } \
} while(0)

#define ASSERT_NE(actual, unexpected, label) do { \
    int _a = (actual), _u = (unexpected); \
    if (_a == _u) { \
        fprintf(stderr, "FAIL %s: unexpected %d\n", label, _u); \
        ++g_failures; \
    } \
} while(0)

#define ASSERT_GE(actual, lower, label) do { \
    int _a = (actual), _l = (lower); \
    if (_a < _l) { \
        fprintf(stderr, "FAIL %s: %d < %d\n", label, _a, _l); \
        ++g_failures; \
    } \
} while(0)

#define ASSERT_LE(actual, upper, label) do { \
    int _a = (actual), _u = (upper); \
    if (_a > _u) { \
        fprintf(stderr, "FAIL %s: %d > %d\n", label, _a, _u); \
        ++g_failures; \
    } \
} while(0)

#define ASSERT_TRUE(cond, label) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s\n", label); \
        ++g_failures; \
    } \
} while(0)


/* ── Test: Projectile aspect table dimensions ────────────────────── */

static void test_projectile_aspect_table(void) {
    int i;
    printf("  projectile aspect table...\n");
    for (i = 0; i < DM1_PROJECTILE_ASPECT_COUNT; ++i) {
        int t = dm1_v1_projectile_aspect_type(i);
        ASSERT_GE(t, 0, "aspect type >= 0");
        ASSERT_LE(t, 3, "aspect type <= 3");
    }
    /* Out of range returns -1 */
    ASSERT_EQ(dm1_v1_projectile_aspect_type(-1), -1, "aspect type -1");
    ASSERT_EQ(dm1_v1_projectile_aspect_type(14), -1, "aspect type 14");
}


/* ── Test: Projectile graphic index ranges ───────────────────────── */

static void test_projectile_graphic_indices(void) {
    int aspect, dir;
    printf("  projectile graphic indices...\n");
    for (aspect = 0; aspect < DM1_PROJECTILE_ASPECT_COUNT; ++aspect) {
        for (dir = 0; dir < 4; ++dir) {
            int gfx = dm1_v1_projectile_graphic_index(aspect, dir);
            char label[64];
            snprintf(label, sizeof(label), "gfx[%d][%d] range", aspect, dir);
            ASSERT_GE(gfx, DM1_GFX_FIRST_PROJECTILE, label);
            /* Max projectile bitmap = 454 + 31 + 2 = 487, but we only
             * go up to 485 (486 is first explosion).  Verify < 486. */
            ASSERT_LE(gfx, DM1_GFX_FIRST_EXPLOSION - 1, label);
        }
    }
    /* Out of range */
    ASSERT_EQ(dm1_v1_projectile_graphic_index(-1, 0), -1, "gfx[-1]");
    ASSERT_EQ(dm1_v1_projectile_graphic_index(14, 0), -1, "gfx[14]");
}


/* ── Test: Projectile bitmap deltas match aspect type rules ──────── */

static void test_projectile_bitmap_deltas(void) {
    int i;
    printf("  projectile bitmap deltas...\n");

    /* Type 3 (no back, no rotation) always delta=0 */
    for (i = 10; i <= 13; ++i) {
        ASSERT_EQ(dm1_v1_projectile_aspect_type(i), 3, "type3 aspect");
        ASSERT_EQ(dm1_v1_projectile_bitmap_delta(i, 0), 0, "type3 delta dir0");
        ASSERT_EQ(dm1_v1_projectile_bitmap_delta(i, 1), 0, "type3 delta dir1");
        ASSERT_EQ(dm1_v1_projectile_bitmap_delta(i, 2), 0, "type3 delta dir2");
        ASSERT_EQ(dm1_v1_projectile_bitmap_delta(i, 3), 0, "type3 delta dir3");
    }

    /* Type 2 (no back, rotation): delta=1 for perpendicular, 0 for parallel */
    ASSERT_EQ(dm1_v1_projectile_aspect_type(3), 2, "lightning type2");
    ASSERT_EQ(dm1_v1_projectile_bitmap_delta(3, 0), 0, "type2 delta dir0");
    ASSERT_EQ(dm1_v1_projectile_bitmap_delta(3, 1), 1, "type2 delta dir1");
    ASSERT_EQ(dm1_v1_projectile_bitmap_delta(3, 2), 0, "type2 delta dir2");
    ASSERT_EQ(dm1_v1_projectile_bitmap_delta(3, 3), 1, "type2 delta dir3");

    /* Type 0 (has back, rotation): delta=2 for perpendicular */
    ASSERT_EQ(dm1_v1_projectile_aspect_type(0), 1, "arrow type");
    /* arrow is actually type 1 due to GraphicInfo 0x0011 & 3 = 1; skip */
}


/* ── Test: Specific projectile subtype->aspect mapping ───────────── */

static void test_projectile_subtype_mapping(void) {
    printf("  projectile subtype mapping...\n");
    ASSERT_EQ(dm1_v1_projectile_subtype_to_aspect(PROJECTILE_SUBTYPE_FIREBALL),
              DM1_PROJ_ASPECT_FIREBALL, "fireball->10");
    ASSERT_EQ(dm1_v1_projectile_subtype_to_aspect(PROJECTILE_SUBTYPE_SLIME),
              DM1_PROJ_ASPECT_SLIME, "slime->12");
    ASSERT_EQ(dm1_v1_projectile_subtype_to_aspect(PROJECTILE_SUBTYPE_LIGHTNING_BOLT),
              DM1_PROJ_ASPECT_LIGHTNING_BOLT, "lightning->3");
    ASSERT_EQ(dm1_v1_projectile_subtype_to_aspect(PROJECTILE_SUBTYPE_POISON_BOLT),
              DM1_PROJ_ASPECT_POISON, "poison_bolt->13");
    ASSERT_EQ(dm1_v1_projectile_subtype_to_aspect(PROJECTILE_SUBTYPE_POISON_CLOUD),
              DM1_PROJ_ASPECT_POISON, "poison_cloud->13");
    ASSERT_EQ(dm1_v1_projectile_subtype_to_aspect(PROJECTILE_SUBTYPE_HARM_NON_MATERIAL),
              DM1_PROJ_ASPECT_DEFAULT, "harm->11");
    ASSERT_EQ(dm1_v1_projectile_subtype_to_aspect(PROJECTILE_SUBTYPE_KINETIC_ARROW),
              0, "kinetic->0");
}


/* ── Test: Projectile depth scaling ──────────────────────────────── */

static void test_projectile_scale(void) {
    printf("  projectile depth scaling...\n");
    /* D0 closest: scale = 32 (native) regardless of sub-cell */
    ASSERT_EQ(dm1_v1_projectile_scale_units(0, -1), 32, "D0 scale");
    ASSERT_EQ(dm1_v1_projectile_scale_units(0, 0), 32, "D0 cell0");
    ASSERT_EQ(dm1_v1_projectile_scale_units(0, 3), 32, "D0 cell3");

    /* Greater depth: scale decreases */
    ASSERT_TRUE(dm1_v1_projectile_scale_units(1, 2) > dm1_v1_projectile_scale_units(2, 2),
                "D1 > D2 scale");
    ASSERT_TRUE(dm1_v1_projectile_scale_units(2, 2) > dm1_v1_projectile_scale_units(3, 2),
                "D2 > D3 scale");

    /* Front row cells (2,3) get slightly larger scale than back (0,1) */
    ASSERT_GE(dm1_v1_projectile_scale_units(2, 2),
              dm1_v1_projectile_scale_units(2, 0),
              "front >= back scale D2");
}


/* ── Test: Flip flags basic sanity ───────────────────────────────── */

static void test_projectile_flip_flags(void) {
    printf("  projectile flip flags...\n");
    /* Type 3 (fireball etc): no flipping ever */
    ASSERT_EQ(dm1_v1_projectile_flip_flags(10, 0, 0, 0, 0), 0, "fireball no flip");
    ASSERT_EQ(dm1_v1_projectile_flip_flags(10, 1, 1, 5, 3), 0, "fireball no flip r");
    ASSERT_EQ(dm1_v1_projectile_flip_flags(10, 3, 2, 7, 2), 0, "fireball no flip l");

    /* Flip flags are in range [0, 3] (2 bits) */
    {
        int a, d, c;
        for (a = 0; a < DM1_PROJECTILE_ASPECT_COUNT; ++a) {
            for (d = 0; d < 4; ++d) {
                for (c = 0; c < 4; ++c) {
                    int f = dm1_v1_projectile_flip_flags(a, d, c, 10, 7);
                    ASSERT_GE(f, 0, "flip >= 0");
                    ASSERT_LE(f, 3, "flip <= 3");
                }
            }
        }
    }
}


/* ── Test: Explosion type->aspect mapping ────────────────────────── */

static void test_explosion_type_to_aspect(void) {
    printf("  explosion type to aspect...\n");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_FIREBALL),
              DM1_EXPLOSION_ASPECT_FIRE, "fireball->fire");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_LIGHTNING_BOLT),
              DM1_EXPLOSION_ASPECT_FIRE, "lightning->fire");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_REBIRTH_STEP2),
              DM1_EXPLOSION_ASPECT_FIRE, "rebirth2->fire");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_POISON_BOLT),
              DM1_EXPLOSION_ASPECT_POISON, "poison_bolt->poison");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_POISON_CLOUD),
              DM1_EXPLOSION_ASPECT_POISON, "poison_cloud->poison");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_SMOKE),
              DM1_EXPLOSION_ASPECT_SMOKE, "smoke->smoke");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_HARM_NON_MATERIAL),
              DM1_EXPLOSION_ASPECT_SPELL, "harm->spell");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_OPEN_DOOR),
              DM1_EXPLOSION_ASPECT_SPELL, "open_door->spell");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_SLIME),
              DM1_EXPLOSION_ASPECT_SPELL, "slime->spell");
    /* Fluxcage and rebirth step1 return -1 */
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_FLUXCAGE), -1, "fluxcage->-1");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(DM1_EXPLOSION_REBIRTH_STEP1), -1, "rebirth1->-1");
    ASSERT_EQ(dm1_v1_explosion_type_to_aspect(-1), -1, "neg->-1");
}


/* ── Test: Explosion aspect->graphic mapping ─────────────────────── */

static void test_explosion_aspect_to_graphic(void) {
    printf("  explosion aspect to graphic...\n");
    ASSERT_EQ(dm1_v1_explosion_aspect_to_graphic(0), 486, "fire->486");
    ASSERT_EQ(dm1_v1_explosion_aspect_to_graphic(1), 487, "spell->487");
    ASSERT_EQ(dm1_v1_explosion_aspect_to_graphic(2), 488, "poison->488");
    ASSERT_EQ(dm1_v1_explosion_aspect_to_graphic(3), 488, "smoke->488");
    ASSERT_EQ(dm1_v1_explosion_aspect_to_graphic(-1), -1, "neg->-1");
}


/* ── Test: Explosion size classes from attack ────────────────────── */

static void test_explosion_size_class(void) {
    printf("  explosion size class...\n");
    ASSERT_EQ(dm1_v1_explosion_size_class(0), 0, "attack 0 -> small");
    ASSERT_EQ(dm1_v1_explosion_size_class(31), 0, "attack 31 -> small");
    ASSERT_EQ(dm1_v1_explosion_size_class(32), 1, "attack 32 -> medium");
    ASSERT_EQ(dm1_v1_explosion_size_class(63), 1, "attack 63 -> medium");
    ASSERT_EQ(dm1_v1_explosion_size_class(96), 1, "attack 96 -> medium");
    ASSERT_EQ(dm1_v1_explosion_size_class(127), 1, "attack 127 -> medium (127>>5=3, not >3)");
    ASSERT_EQ(dm1_v1_explosion_size_class(128), 2, "attack 128 -> large");
    ASSERT_EQ(dm1_v1_explosion_size_class(255), 2, "attack 255 -> large");
}


/* ── Test: Explosion pattern graphic index (D0C path) ────────────── */

static void test_explosion_pattern_graphic(void) {
    printf("  explosion pattern graphic index...\n");
    /* Fire small: 489 + 0*3 + 0 = 489 */
    ASSERT_EQ(dm1_v1_explosion_pattern_graphic_index(DM1_EXPLOSION_FIREBALL, 0), 489,
              "fire small");
    /* Fire medium: 489 + 0*3 + 1 = 490 */
    ASSERT_EQ(dm1_v1_explosion_pattern_graphic_index(DM1_EXPLOSION_FIREBALL, 32), 490,
              "fire medium");
    /* Fire large: 489 + 0*3 + 2 = 491 */
    ASSERT_EQ(dm1_v1_explosion_pattern_graphic_index(DM1_EXPLOSION_FIREBALL, 200), 491,
              "fire large");
    /* Spell small: 489 + 1*3 + 0 = 492 */
    ASSERT_EQ(dm1_v1_explosion_pattern_graphic_index(DM1_EXPLOSION_HARM_NON_MATERIAL, 0), 492,
              "spell small");
    /* Poison medium: 489 + 2*3 + 1 = 496 */
    ASSERT_EQ(dm1_v1_explosion_pattern_graphic_index(DM1_EXPLOSION_POISON_CLOUD, 64), 496,
              "poison medium");
    /* Smoke uses poison graphics: 489 + 2*3 + 0 = 495 */
    ASSERT_EQ(dm1_v1_explosion_pattern_graphic_index(DM1_EXPLOSION_SMOKE, 0), 495,
              "smoke small (=poison)");
    /* Fluxcage returns -1 */
    ASSERT_EQ(dm1_v1_explosion_pattern_graphic_index(DM1_EXPLOSION_FLUXCAGE, 100), -1,
              "fluxcage->-1");
}


/* ── Test: Explosion base scale decreases with depth ─────────────── */

static void test_explosion_base_scale(void) {
    printf("  explosion base scale...\n");
    ASSERT_EQ(dm1_v1_explosion_base_scale(0), 32, "D0 scale");
    ASSERT_TRUE(dm1_v1_explosion_base_scale(0) > dm1_v1_explosion_base_scale(1),
                "D0 > D1");
    ASSERT_TRUE(dm1_v1_explosion_base_scale(1) > dm1_v1_explosion_base_scale(2),
                "D1 > D2");
    ASSERT_TRUE(dm1_v1_explosion_base_scale(2) > dm1_v1_explosion_base_scale(3),
                "D2 > D3");
}


/* ── Test: Smoke detection ───────────────────────────────────────── */

static void test_smoke_detection(void) {
    printf("  smoke detection...\n");
    ASSERT_EQ(dm1_v1_explosion_is_smoke(DM1_EXPLOSION_SMOKE), 1, "smoke=1");
    ASSERT_EQ(dm1_v1_explosion_is_smoke(DM1_EXPLOSION_FIREBALL), 0, "fire=0");
    ASSERT_EQ(dm1_v1_explosion_is_smoke(DM1_EXPLOSION_POISON_CLOUD), 0, "poison=0");
}


/* ── Test: F0115 draw order verification ─────────────────────────── */

static void test_draw_order(void) {
    printf("  F0115 draw order...\n");
    {
        int correct[] = {
            DM1_F0115_LAYER_FLOOR_ORNAMENTS,
            DM1_F0115_LAYER_FLOOR_ITEMS,
            DM1_F0115_LAYER_CREATURES,
            DM1_F0115_LAYER_PROJECTILES,
            DM1_F0115_LAYER_EXPLOSIONS,
            DM1_F0115_LAYER_FLUXCAGE_FIELD
        };
        ASSERT_EQ(dm1_v1_verify_f0115_draw_order(correct, 6), 1, "correct order");
    }
    {
        /* Swap projectiles and explosions — should fail */
        int wrong[] = {0, 1, 2, 4, 3, 5};
        ASSERT_EQ(dm1_v1_verify_f0115_draw_order(wrong, 6), 0, "wrong order");
    }
    {
        /* Partial ordering: items before creatures before projectiles */
        int partial[] = {1, 2, 3};
        ASSERT_EQ(dm1_v1_verify_f0115_draw_order(partial, 3), 1, "partial order");
    }
}


/* ── Test: Projectile aspect data cross-check with m11_game_view ─── */

static void test_aspect_data_cross_check(void) {
    /* Cross-check FirstNativeBitmapRelativeIndex against the known
     * kFirstNative array from m11_game_view.c. */
    static const unsigned char kExpectedFirstNative[14] = {
        0,3,6,9,11,14,17,20,23,26,28,29,30,31
    };
    static const unsigned short kExpectedGraphicInfo[14] = {
        0x0011,0x0011,0x0010,0x0112,0x0011,0x0010,0x0010,
        0x0011,0x0011,0x0012,0x0103,0x0103,0x0103,0x0103
    };
    int i;
    printf("  aspect data cross-check...\n");
    for (i = 0; i < DM1_PROJECTILE_ASPECT_COUNT; ++i) {
        char label[64];
        snprintf(label, sizeof(label), "firstNative[%d]", i);
        ASSERT_EQ((int)DM1_ProjectileAspects[i].firstNativeBitmapRelativeIndex,
                  (int)kExpectedFirstNative[i], label);
        snprintf(label, sizeof(label), "graphicInfo[%d]", i);
        ASSERT_EQ((int)DM1_ProjectileAspects[i].graphicInfo,
                  (int)kExpectedGraphicInfo[i], label);
    }
}


/* ── Test: Full projectile graphic index for known spell subtypes ── */

static void test_spell_graphic_indices(void) {
    int gfx;
    printf("  spell graphic indices...\n");
    /* Fireball (aspect 10, type 3) flying forward: delta=0 */
    gfx = dm1_v1_projectile_graphic_index(10, 0);
    ASSERT_EQ(gfx, 454 + 28, "fireball fwd");
    /* Fireball flying right: type 3, still delta=0 */
    gfx = dm1_v1_projectile_graphic_index(10, 1);
    ASSERT_EQ(gfx, 454 + 28, "fireball right");
    /* Lightning (aspect 3, type 2) flying forward: delta=0 */
    gfx = dm1_v1_projectile_graphic_index(3, 0);
    ASSERT_EQ(gfx, 454 + 9, "lightning fwd");
    /* Lightning flying right: type 2, delta=1 */
    gfx = dm1_v1_projectile_graphic_index(3, 1);
    ASSERT_EQ(gfx, 454 + 10, "lightning right");
}


static struct ProjectileInstance_Compat make_travel_projectile(int category, int subtype) {
    struct ProjectileInstance_Compat p;
    memset(&p, 0, sizeof(p));
    p.slotIndex = 7;
    p.projectileCategory = category;
    p.projectileSubtype = subtype;
    p.ownerKind = PROJECTILE_OWNER_CHAMPION;
    p.mapIndex = 0;
    p.mapX = 2;
    p.mapY = 2;
    p.cell = 0;
    p.direction = 0;
    p.kineticEnergy = 40;
    p.attack = 24;
    p.stepEnergy = 4;
    p.reserved3 = 1;
    return p;
}

static struct CellContentDigest_Compat make_travel_digest(int destSquareType) {
    struct CellContentDigest_Compat d;
    memset(&d, 0, sizeof(d));
    d.sourceMapIndex = 0;
    d.sourceMapX = 2;
    d.sourceMapY = 2;
    d.sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    d.destMapIndex = 0;
    d.destMapX = 2;
    d.destMapY = 1;
    d.destSquareType = destSquareType;
    d.destDoorState = PROJECTILE_DOOR_STATE_NONE;
    d.destTeleporterNewDirection = -1;
    return d;
}

static void test_projectile_travel_blockers(void) {
    struct ProjectileInstance_Compat p;
    struct ProjectileInstance_Compat next;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat result;
    int blocker = -1;

    printf("  projectile travel blockers...\n");

    p = make_travel_projectile(PROJECTILE_CATEGORY_MAGICAL, PROJECTILE_SUBTYPE_FIREBALL);
    d = make_travel_digest(PROJECTILE_ELEMENT_WALL);
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "wall inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_WALL, "wall inspect blocker");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 100, NULL, &next, &result), 1, "wall advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_WALL, "wall hit result");
    ASSERT_EQ(result.despawn, 1, "wall hit despawns");
    ASSERT_EQ(result.emittedExplosion, 1, "fireball wall hit creates explosion");

    d = make_travel_digest(PROJECTILE_ELEMENT_FAKEWALL);
    d.destFakeWallIsImaginaryOrOpen = 0;
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "closed fakewall inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_WALL, "closed fakewall blocker");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 101, NULL, &next, &result), 1, "closed fakewall advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_WALL, "closed fakewall hit result");
    ASSERT_EQ(result.despawn, 1, "closed fakewall despawns");

    d.destFakeWallIsImaginaryOrOpen = 1;
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "open fakewall inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_OPEN, "open fakewall no blocker");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 102, NULL, &next, &result), 1, "open fakewall advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_FLEW, "open fakewall flies");
    ASSERT_EQ(result.despawn, 0, "open fakewall keeps projectile");

    d = make_travel_digest(PROJECTILE_ELEMENT_STAIRS);
    d.sourceSquareType = PROJECTILE_ELEMENT_STAIRS;
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "stairs inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_STAIRS, "stairs-to-stairs blocker");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 103, NULL, &next, &result), 1, "stairs advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_WALL, "stairs-to-stairs impacts as wall");
    ASSERT_EQ(result.despawn, 1, "stairs-to-stairs despawns");

    p = make_travel_projectile(PROJECTILE_CATEGORY_KINETIC, PROJECTILE_SUBTYPE_KINETIC_ARROW);
    d = make_travel_digest(PROJECTILE_ELEMENT_DOOR);
    d.destDoorState = PROJECTILE_DOOR_STATE_CLOSED_HALF;
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "closed door inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_CLOSED_DOOR, "closed door blocker");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 104, NULL, &next, &result), 1, "closed door advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_DOOR, "closed door hit result");
    ASSERT_EQ(result.despawn, 1, "closed door despawns");
    ASSERT_EQ(result.emittedDoorDestructionEvent, 1, "closed door emits attack event");

    d.destDoorState = PROJECTILE_DOOR_STATE_CLOSED_ONE_FOURTH;
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "one-fourth door inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_OPEN, "one-fourth door no blocker");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 105, NULL, &next, &result), 1, "one-fourth door advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_FLEW, "one-fourth door flies");
    ASSERT_EQ(result.despawn, 0, "one-fourth door keeps projectile");

    p = make_travel_projectile(PROJECTILE_CATEGORY_MAGICAL, PROJECTILE_SUBTYPE_HARM_NON_MATERIAL);
    d.destDoorState = PROJECTILE_DOOR_STATE_CLOSED_FULL;
    d.destDoorAllowsProjectilePassThrough = 1;
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "pass-through door inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_CLOSED_DOOR, "pass-through door still classified closed");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 106, NULL, &next, &result), 1, "pass-through door advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_FLEW, "allowed magical projectile flies through");
    ASSERT_EQ(result.despawn, 0, "allowed magical projectile not consumed");

    p = make_travel_projectile(PROJECTILE_CATEGORY_MAGICAL, PROJECTILE_SUBTYPE_OPEN_DOOR);
    d.destDoorHasButton = 0;
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 107, NULL, &next, &result), 1, "open-door projectile advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_DOOR, "open-door projectile still hits pass-through door");
    ASSERT_EQ(result.despawn, 1, "open-door projectile consumed");
    ASSERT_EQ(result.emittedDoorToggleEvent, 0, "open-door no-button emits no toggle");
    ASSERT_EQ(result.emittedDoorDestructionEvent, 0, "open-door never emits destruction");
    ASSERT_EQ(result.emittedSoundCode, 4, "open-door impact thud");

    d.destDoorHasButton = 1;
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 108, NULL, &next, &result), 1, "open-door button advance ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_DOOR, "open-door button hits door");
    ASSERT_EQ(result.emittedDoorToggleEvent, 1, "open-door button emits toggle");
    ASSERT_EQ(result.emittedDoorDestructionEvent, 0, "open-door button no destruction");
    ASSERT_EQ(result.outNextTick.kind, TIMELINE_EVENT_SENSOR_DELAYED, "open-door toggle event kind");
    ASSERT_EQ((int)result.outNextTick.fireAtTick, 109, "open-door toggle tick+1");
    ASSERT_EQ(result.outNextTick.mapX, d.destMapX, "open-door toggle x");
    ASSERT_EQ(result.outNextTick.mapY, d.destMapY, "open-door toggle y");
    ASSERT_EQ(result.outNextTick.aux0, 10, "open-door toggle C10_EVENT_DOOR");
    ASSERT_EQ(result.outNextTick.aux1, 2, "open-door toggle C02_EFFECT_TOGGLE");

    d.destDoorState = PROJECTILE_DOOR_STATE_OPEN;
    d.destDoorAllowsProjectilePassThrough = 0;
    ASSERT_EQ(F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker), 1, "open door inspect ok");
    ASSERT_EQ(blocker, PROJECTILE_BLOCKER_OPEN, "open door normal blocker open");
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 109, NULL, &next, &result), 1, "open-door spell versus open door ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_DOOR, "open-door spell impacts open door");
    ASSERT_EQ(result.emittedDoorToggleEvent, 1, "open-door spell toggles open button door");
    ASSERT_EQ(result.despawn, 1, "open-door spell open door consumed");

    d.destDoorState = PROJECTILE_DOOR_STATE_CLOSED_ONE_FOURTH;
    d.destDoorHasButton = 0;
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 110, NULL, &next, &result), 1, "open-door spell versus one-fourth door ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_HIT_DOOR, "open-door spell impacts one-fourth door");
    ASSERT_EQ(result.emittedDoorToggleEvent, 0, "one-fourth no-button emits no toggle");
    ASSERT_EQ(result.despawn, 1, "open-door spell one-fourth consumed");

    d.destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
    d.destDoorHasButton = 1;
    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&p, &d, 111, NULL, &next, &result), 1, "open-door spell versus destroyed door ok");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_FLEW, "open-door spell passes destroyed door");
    ASSERT_EQ(result.despawn, 0, "destroyed door keeps open-door projectile flying");
}


static void test_poison_cloud_party_damage_over_time(void) {
    struct ExplosionInstance_Compat explosion;
    struct ExplosionInstance_Compat next;
    struct CellContentDigest_Compat digest;
    struct ExplosionTickResult_Compat result;

    printf("  poison cloud party damage over time...\n");

    memset(&explosion, 0, sizeof(explosion));
    explosion.slotIndex = 3;
    explosion.explosionType = C007_EXPLOSION_POISON_CLOUD;
    explosion.mapIndex = 0;
    explosion.mapX = 4;
    explosion.mapY = 5;
    explosion.cell = EXPLOSION_CELL_CENTERED;
    explosion.centered = 1;
    explosion.attack = 64;
    explosion.currentFrame = 2;
    explosion.ownerKind = PROJECTILE_OWNER_CHAMPION;
    explosion.ownerIndex = 1;

    memset(&digest, 0, sizeof(digest));
    digest.destMapIndex = 0;
    digest.destMapX = 4;
    digest.destMapY = 5;
    digest.destHasChampion = 1;

    ASSERT_EQ(F0822_EXPLOSION_Advance_Compat(&explosion, &digest, 250, NULL, &next, &result),
              1, "poison cloud advance ok");
    ASSERT_EQ(result.emittedCombatActionPartyCount, 1, "poison cloud damages party");
    ASSERT_EQ(result.outActionParty.allowedWounds, 0, "poison cloud party damage has no wound mask");
    ASSERT_EQ(result.outActionParty.rawAttackValue, 2, "poison cloud attack>>5 without rng");
    ASSERT_EQ(result.resultKind, EXPLOSION_RESULT_ADVANCED_FRAME, "poison cloud continues");
    ASSERT_EQ(result.despawn, 0, "poison cloud does not despawn while attack >= 6");
    ASSERT_EQ(next.attack, 61, "poison cloud attack decays by 3");
    ASSERT_EQ(result.outNextTick.kind, TIMELINE_EVENT_EXPLOSION_ADVANCE, "poison cloud schedules next tick");
    ASSERT_EQ((int)result.outNextTick.fireAtTick, 251, "poison cloud next tick +1");
}


static void test_harm_non_material_materializer_attack_only(void) {
    int attack = -1;
    struct RngState_Compat rng;
    struct RngState_Compat expectedRng;
    int expectedAttack;

    printf("  harm non-material Materializer/Zytaz attack-only damage...\n");

    ASSERT_EQ(F0830_EXPLOSION_HarmNonMaterialCreatureAttack_Compat(
                  72, 19, 1, 0x00, NULL, &attack),
              1, "materializer idle helper ok");
    ASSERT_EQ(attack, 0, "party-map idle Materializer takes no harm-non-material damage");

    ASSERT_EQ(F0830_EXPLOSION_HarmNonMaterialCreatureAttack_Compat(
                  72, 19, 1, 0x80, NULL, &attack),
              1, "materializer attacking helper ok");
    ASSERT_EQ(attack, 63, "attacking Materializer uses base minus one-eighth without rng");

    F0730_COMBAT_RngInit_Compat(&rng, 5);
    F0730_COMBAT_RngInit_Compat(&expectedRng, 5);
    expectedAttack = 63 + F0732_COMBAT_RngRandom_Compat(&expectedRng, 19)
                         + F0732_COMBAT_RngRandom_Compat(&expectedRng, 4);
    ASSERT_EQ(F0830_EXPLOSION_HarmNonMaterialCreatureAttack_Compat(
                  72, 19, 1, 0x80, &rng, &attack),
              1, "materializer attacking rng helper ok");
    ASSERT_EQ(attack, expectedAttack, "attacking Materializer damage applies +random(2*(attack>>3)+1)+random(4)");
    ASSERT_EQ((int)rng.seed, (int)expectedRng.seed, "attacking Materializer consumes exactly two rng rolls");

    ASSERT_EQ(F0830_EXPLOSION_HarmNonMaterialCreatureAttack_Compat(
                  72, 8, 1, 0x00, NULL, &attack),
              1, "non-material non-Materializer helper ok");
    ASSERT_EQ(attack, 72, "other non-material creatures use normal harm-non-material damage");

    ASSERT_EQ(F0830_EXPLOSION_HarmNonMaterialCreatureAttack_Compat(
                  72, 19, 0, 0x00, NULL, &attack),
              1, "off-party-map Materializer helper ok");
    ASSERT_EQ(attack, 72, "off-party-map Materializer uses normal damage path");

    ASSERT_EQ(F0830_EXPLOSION_HarmNonMaterialCreatureAttack_Compat(
                  72, 19, 1, 0x80, NULL, NULL),
              0, "null out attack rejected");
}



/* ── Main ────────────────────────────────────────────────────────── */

int main(void) {
    printf("DM1 V1 Projectile/Explosion Render Tests\n");

    test_projectile_aspect_table();
    test_projectile_graphic_indices();
    test_projectile_bitmap_deltas();
    test_projectile_subtype_mapping();
    test_projectile_scale();
    test_projectile_flip_flags();
    test_explosion_type_to_aspect();
    test_explosion_aspect_to_graphic();
    test_explosion_size_class();
    test_explosion_pattern_graphic();
    test_explosion_base_scale();
    test_smoke_detection();
    test_draw_order();
    test_aspect_data_cross_check();
    test_spell_graphic_indices();
    test_projectile_travel_blockers();
    test_poison_cloud_party_damage_over_time();
    test_harm_non_material_materializer_attack_only();

    if (g_failures == 0) {
        printf("All tests passed.\n");
        return 0;
    } else {
        fprintf(stderr, "%d test failure(s).\n", g_failures);
        return 1;
    }
}
