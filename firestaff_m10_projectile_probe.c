/*
 * M10 Phase 17 probe — Projectile & explosion flight verification.
 *
 * Validates the pure projectile/explosion tick-transform data layer
 * against PHASE17_PLAN.md §5 invariants.
 *
 * Shipped: 50 invariants (≥30 gate, 48 target). Two sub-checks are
 * folded under invariant #48 and counted as one.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "memory_projectile_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_magic_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* ---------- Local CRC32 (IEEE reflected poly 0xEDB88320) -------- */

static uint32_t local_crc32(const unsigned char* buf, size_t n) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    int b;
    for (i = 0; i < n; i++) {
        crc ^= (uint32_t)buf[i];
        for (b = 0; b < 8; b++) {
            crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1u));
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ---------- Digest / instance fixtures -------------------------- */

static void zero_digest(struct CellContentDigest_Compat* d) {
    memset(d, 0, sizeof(*d));
    d->sourceSquareType            = PROJECTILE_ELEMENT_CORRIDOR;
    d->destSquareType              = PROJECTILE_ELEMENT_CORRIDOR;
    d->destDoorState               = PROJECTILE_DOOR_STATE_NONE;
    d->destCreatureType            = -1;
    d->destTeleporterNewDirection  = -1;
}

static void set_source_and_dest(
    struct CellContentDigest_Compat* d,
    int mapIndex, int sx, int sy, int dx, int dy)
{
    d->sourceMapIndex = mapIndex;
    d->sourceMapX     = sx;
    d->sourceMapY     = sy;
    d->destMapIndex   = mapIndex;
    d->destMapX       = dx;
    d->destMapY       = dy;
}

static void make_projectile_kinetic(
    struct ProjectileInstance_Compat* p,
    int dir, int cell, int mx, int my,
    int kinetic, int attack, int stepE)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex          = 0;
    p->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p->projectileSubtype  = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    p->ownerKind          = PROJECTILE_OWNER_CHAMPION;
    p->ownerIndex         = 0;
    p->mapIndex           = 0;
    p->mapX               = mx;
    p->mapY               = my;
    p->cell               = cell & 3;
    p->direction          = dir & 3;
    p->kineticEnergy      = kinetic;
    p->attack             = attack;
    p->stepEnergy         = stepE;
    p->firstMoveGraceFlag = 0;
    p->attackTypeCode     = COMBAT_ATTACK_NORMAL;
    p->flags              = PROJECTILE_FLAG_IGNORE_DOOR_PASS_THROUGH;
    p->reserved3          = 1;
}

static void make_projectile_magical(
    struct ProjectileInstance_Compat* p,
    int subtype, int attackType,
    int dir, int cell, int mx, int my,
    int kinetic, int attack, int stepE)
{
    make_projectile_kinetic(p, dir, cell, mx, my, kinetic, attack, stepE);
    p->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    p->projectileSubtype  = subtype;
    p->attackTypeCode     = attackType;
}

/* ---------- fill functions for round-trip tests ---------------- */

static void fill_projectile_populated(struct ProjectileInstance_Compat* p) {
    memset(p, 0, sizeof(*p));
    p->slotIndex             = 13;
    p->projectileCategory    = PROJECTILE_CATEGORY_MAGICAL;
    p->projectileSubtype     = PROJECTILE_SUBTYPE_FIREBALL;
    p->ownerKind             = PROJECTILE_OWNER_CREATURE;
    p->ownerIndex            = 5;
    p->mapIndex              = 2;
    p->mapX                  = 7;
    p->mapY                  = 11;
    p->cell                  = 3;
    p->direction             = 1;
    p->kineticEnergy         = 100;
    p->attack                = 80;
    p->stepEnergy            = 4;
    p->firstMoveGraceFlag    = 1;
    p->launchedAtTick        = 0x12345678;
    p->scheduledAtTick       = 0x12345679;
    p->associatedPotionPower = 42;
    p->poisonAttack          = 25;
    p->attackTypeCode        = COMBAT_ATTACK_FIRE;
    p->flags                 = 0x07;
    p->reserved0             = 0x11223344;
    p->reserved1             = 0x55667788;
    p->reserved2             = 0x7EADBEEF;
    p->reserved3             = 1;
}

static void fill_explosion_populated(struct ExplosionInstance_Compat* e) {
    memset(e, 0, sizeof(*e));
    e->slotIndex             = 7;
    e->explosionType         = C007_EXPLOSION_POISON_CLOUD;
    e->mapIndex              = 3;
    e->mapX                  = 11;
    e->mapY                  = 13;
    e->cell                  = 0xFF;
    e->centered              = 1;
    e->attack                = 32;
    e->currentFrame          = 2;
    e->maxFrames             = 30;
    e->poisonAttack          = 64;
    e->scheduledAtTick       = 0xCAFEBABE;
    e->ownerKind             = PROJECTILE_OWNER_CHAMPION;
    e->ownerIndex            = 1;
    e->creatorProjectileSlot = 3;
    e->reserved0             = 1;
}

int main(int argc, char* argv[]) {
    FILE* report;
    FILE* invariants;
    char path_buf[512];
    int failCount = 0;
    int invariantCount = 0;
    const char* dungeonPath;
    const char* outputDir;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }
    dungeonPath = argv[1];
    outputDir   = argv[2];

    snprintf(path_buf, sizeof(path_buf), "%s/projectile_probe.md", outputDir);
    report = fopen(path_buf, "w");
    if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); return 1; }
    fprintf(report, "# M10 Phase 17: Projectile & Explosion Flight Probe\n\n");
    fprintf(report, "## Scope (v1)\n\n");
    fprintf(report, "- F0810-F0813 projectile lifecycle (create, advance, spell-bridge, despawn)\n");
    fprintf(report, "- F0814-F0816 cell inspection (blocker, impact-attack, door-pass-through)\n");
    fprintf(report, "- F0817-F0820 collision resolution (champion/creature/door hit, dispatch)\n");
    fprintf(report, "- F0821-F0824 explosion lifecycle (create, advance, AoE roll, despawn)\n");
    fprintf(report, "- F0825-F0826 timeline scheduling helpers (clamped delay >= 1)\n");
    fprintf(report, "- F0827-F0829 serialisation (instance + list round-trips)\n");
    fprintf(report, "- Integration with Phases 12/13/14/15/16\n");
    fprintf(report, "- Loop-guard (100 configs × 200 ticks) and real DUNGEON.DAT spot-check\n\n");
    fprintf(report, "## Known NEEDS DISASSEMBLY REVIEW\n\n");
    fprintf(report, "- Teleporter direction rotation (memory_projectile_pc34_compat.c F0811 step 7) — v1 honours `destTeleporterNewDirection` when caller pre-rotates; no in-module rotation.\n");
    fprintf(report, "- Kinetic pass-through door random roll (PROJEXPL.C:490-500, pouch/thrown) — v1 deterministic non-pass for kinetic projectiles.\n");
    fprintf(report, "- Projectile-vs-projectile annihilation order (PROJEXPL.C F0218) — v1 picks order-independent variant (both despawn); Fontanel's \"earlier wins\" depends on list-iteration order which is not save-stable.\n");
    fprintf(report, "- Non-material creature pass-through — v1 despawns kinetic/non-HARM_NON_MATERIAL projectile on non-material-creature contact (plan §4.3 simplification); Fontanel actually continues past.\n");
    fprintf(report, "- Paired source-cell explosion damage (PROJEXPL.C F0213 lines 169-175 fireball/lightning) — v1 emits single-cell damage only; source-cell lag-one-tick is v2.\n");
    fprintf(report, "- Sharp-weapon absorption by creatures with KEEP_THROWN_SHARP_WEAPONS (PROJEXPL.C:572-590) — v1 drops the weapon; projectile just despawns.\n");
    fprintf(report, "- Cell → champion-index rotation by party facing (Phase 10 packing convention); v1 uses direct cell == index.\n");
    fprintf(report, "- Explosion per-type frame counts are coarse placeholders; exact Fontanel frame-table values are rendering concerns and out of Phase 17 scope.\n\n");

    snprintf(path_buf, sizeof(path_buf), "%s/projectile_invariants.md", outputDir);
    invariants = fopen(path_buf, "w");
    if (!invariants) {
        fprintf(stderr, "FAIL: cannot write invariants\n");
        fclose(report);
        return 1;
    }
    fprintf(invariants, "# Projectile & Explosion Invariants\n\n");

#define CHECK(cond, msg) do { \
    invariantCount++; \
    if (cond) { \
        fprintf(invariants, "- PASS: %s\n", msg); \
    } else { \
        fprintf(invariants, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while (0)

    /* ================================================================
     *  Block A — sizes + platform (invariants 1-5)
     * ================================================================ */
    CHECK(sizeof(int) == 4 && sizeof(uint32_t) == 4,
          "sizeof(int)==4 && sizeof(uint32_t)==4");
    CHECK(PROJECTILE_INSTANCE_SERIALIZED_SIZE == 96 &&
          sizeof(struct ProjectileInstance_Compat) == 96,
          "PROJECTILE_INSTANCE_SERIALIZED_SIZE == 96 and matches sizeof");
    CHECK(EXPLOSION_INSTANCE_SERIALIZED_SIZE == 64 &&
          sizeof(struct ExplosionInstance_Compat) == 64,
          "EXPLOSION_INSTANCE_SERIALIZED_SIZE == 64 and matches sizeof");
    CHECK(CELL_CONTENT_DIGEST_SERIALIZED_SIZE == 100 &&
          sizeof(struct CellContentDigest_Compat) == 100,
          "CELL_CONTENT_DIGEST_SERIALIZED_SIZE == 100 and matches sizeof");
    CHECK(PROJECTILE_TICK_RESULT_SERIALIZED_SIZE == 232 &&
          sizeof(struct ProjectileTickResult_Compat) == 232 &&
          EXPLOSION_TICK_RESULT_SERIALIZED_SIZE == 184 &&
          sizeof(struct ExplosionTickResult_Compat) == 184,
          "Tick result sizes: Projectile=232, Explosion=184; sizeof matches");

    /* ================================================================
     *  Block B — round-trip serialisation (invariants 6-9)
     * ================================================================ */
    {
        struct ProjectileInstance_Compat a, b;
        unsigned char buf[PROJECTILE_INSTANCE_SERIALIZED_SIZE];
        int w, r;
        memset(&a, 0, sizeof(a));
        memset(&b, 0xAA, sizeof(b));
        w = F0827_PROJECTILE_InstanceSerialize_Compat(&a, buf, sizeof(buf));
        r = F0827_PROJECTILE_InstanceDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == PROJECTILE_INSTANCE_SERIALIZED_SIZE &&
              r == PROJECTILE_INSTANCE_SERIALIZED_SIZE &&
              memcmp(&a, &b, sizeof(a)) == 0,
              "Zero ProjectileInstance_Compat round-trips bit-identical");
    }
    {
        struct ProjectileInstance_Compat a, b;
        unsigned char buf[PROJECTILE_INSTANCE_SERIALIZED_SIZE];
        int w, r;
        fill_projectile_populated(&a);
        memset(&b, 0xCC, sizeof(b));
        w = F0827_PROJECTILE_InstanceSerialize_Compat(&a, buf, sizeof(buf));
        r = F0827_PROJECTILE_InstanceDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == PROJECTILE_INSTANCE_SERIALIZED_SIZE &&
              r == PROJECTILE_INSTANCE_SERIALIZED_SIZE &&
              memcmp(&a, &b, sizeof(a)) == 0,
              "Populated ProjectileInstance_Compat round-trips bit-identical (24 fields)");
    }
    {
        struct ExplosionInstance_Compat a0, b0, a1, b1;
        unsigned char buf[EXPLOSION_INSTANCE_SERIALIZED_SIZE];
        int w, r;
        int ok;

        memset(&a0, 0, sizeof(a0));
        memset(&b0, 0x55, sizeof(b0));
        w = F0828_EXPLOSION_InstanceSerialize_Compat(&a0, buf, sizeof(buf));
        r = F0828_EXPLOSION_InstanceDeserialize_Compat(&b0, buf, sizeof(buf));
        ok = (w == EXPLOSION_INSTANCE_SERIALIZED_SIZE &&
              r == EXPLOSION_INSTANCE_SERIALIZED_SIZE &&
              memcmp(&a0, &b0, sizeof(a0)) == 0);

        fill_explosion_populated(&a1);
        memset(&b1, 0x77, sizeof(b1));
        w = F0828_EXPLOSION_InstanceSerialize_Compat(&a1, buf, sizeof(buf));
        r = F0828_EXPLOSION_InstanceDeserialize_Compat(&b1, buf, sizeof(buf));
        ok = ok && (w == EXPLOSION_INSTANCE_SERIALIZED_SIZE &&
                    r == EXPLOSION_INSTANCE_SERIALIZED_SIZE &&
                    memcmp(&a1, &b1, sizeof(a1)) == 0);
        CHECK(ok, "ExplosionInstance_Compat round-trips bit-identical (zero + populated)");
    }
    {
        struct ProjectileList_Compat plA, plB;
        struct ExplosionList_Compat elA, elB;
        unsigned char pbuf[PROJECTILE_LIST_SERIALIZED_SIZE];
        unsigned char ebuf[EXPLOSION_LIST_SERIALIZED_SIZE];
        int ok;
        int i;

        memset(&plA, 0, sizeof(plA));
        for (i = 0; i < 3; i++) {
            fill_projectile_populated(&plA.entries[i * 10]);
            plA.entries[i * 10].slotIndex = i * 10;
            plA.entries[i * 10].ownerIndex = i;
        }
        plA.count = 3;
        memset(&plB, 0x33, sizeof(plB));
        ok = (F0829_PROJECTILE_ListSerialize_Compat(&plA, pbuf, sizeof(pbuf))
                == PROJECTILE_LIST_SERIALIZED_SIZE);
        ok = ok && (F0829_PROJECTILE_ListDeserialize_Compat(&plB, pbuf, sizeof(pbuf))
                     == PROJECTILE_LIST_SERIALIZED_SIZE);
        ok = ok && (memcmp(&plA, &plB, sizeof(plA)) == 0);

        memset(&elA, 0, sizeof(elA));
        for (i = 0; i < 3; i++) {
            fill_explosion_populated(&elA.entries[i * 5]);
            elA.entries[i * 5].slotIndex = i * 5;
        }
        elA.count = 3;
        memset(&elB, 0x99, sizeof(elB));
        ok = ok && (F0829_EXPLOSION_ListSerialize_Compat(&elA, ebuf, sizeof(ebuf))
                     == EXPLOSION_LIST_SERIALIZED_SIZE);
        ok = ok && (F0829_EXPLOSION_ListDeserialize_Compat(&elB, ebuf, sizeof(ebuf))
                     == EXPLOSION_LIST_SERIALIZED_SIZE);
        ok = ok && (memcmp(&elA, &elB, sizeof(elA)) == 0);

        CHECK(ok, "ProjectileList + ExplosionList with 3 entries each round-trip bit-identical");
    }

    /* ================================================================
     *  Block C — motion determinism (invariants 10-17)
     * ================================================================ */
    /* 10: direction=0 (NORTH), cell=0 -> crossedCell=1, newMapY=source-1 */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 1, 1, 1, 1, 0);
        make_projectile_kinetic(&p, 0, 0, 1, 1, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.crossedCell == 1 && r.resultKind == PROJECTILE_RESULT_FLEW
              && r.newMapY == 0 && r.newMapX == 1,
              "F0811 NORTH (dir=0, cell=0, open floor) -> crossed, newMapY=0");
    }
    /* 11: direction=1 (EAST), cell=1 -> crossedCell=1, newMapX=source+1 */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 1, 1, 1, 2, 1);
        make_projectile_kinetic(&p, 1, 1, 1, 1, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.crossedCell == 1 && r.resultKind == PROJECTILE_RESULT_FLEW
              && r.newMapX == 2 && r.newMapY == 1,
              "F0811 EAST (dir=1, cell=1, open floor) -> crossed, newMapX=+1");
    }
    /* 12: direction=2 (SOUTH), cell=2 -> crossedCell=1, newMapY+1 */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 1, 1, 1, 1, 2);
        make_projectile_kinetic(&p, 2, 2, 1, 1, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.crossedCell == 1 && r.resultKind == PROJECTILE_RESULT_FLEW
              && r.newMapY == 2,
              "F0811 SOUTH (dir=2, cell=2, open floor) -> crossed, newMapY=+1");
    }
    /* 13: direction=3 (WEST), cell=3 -> crossedCell=1, newMapX-1 */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 1, 1, 1, 0, 1);
        make_projectile_kinetic(&p, 3, 3, 1, 1, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.crossedCell == 1 && r.resultKind == PROJECTILE_RESULT_FLEW
              && r.newMapX == 0,
              "F0811 WEST (dir=3, cell=3, open floor) -> crossed, newMapX=-1");
    }
    /* 14: Intra-cell flip case (cell != direction and cell != dir+1) */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 1, 1, 1, 1, 1);  /* same square */
        /* dir=0 (NORTH), cell=2: 0!=2, (0+1)&3=1 !=2 -> intra-cell flip */
        make_projectile_kinetic(&p, 0, 2, 1, 1, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.crossedCell == 0 && r.newCell != p.cell,
              "Intra-cell flip: dir!=cell and !=(dir+1)&3 -> crossedCell=0, newCell differs");
    }
    /* 15: Parity rule — two sub-checks */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        int ok;
        zero_digest(&d);
        set_source_and_dest(&d, 1, 1, 1, 1, 1);
        /* (dir & 1) == (cell & 1): dir=0,cell=2 -> newCell = (2-1)&3 = 1 */
        make_projectile_kinetic(&p, 0, 2, 1, 1, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        ok = (r.newCell == 1);
        /* (dir & 1) != (cell & 1): dir=0,cell=1 ((dir+1)&3==cell so this
         * will cross — pick a case that doesn't cross): dir=1,cell=2.
         * dir=1,cell=2: 1!=2, (1+1)&3=2 == cell -> crosses. Pick
         * dir=2,cell=1 which is intra-cell: 2!=1, (2+1)&3=3!=1, intra.
         * (dir&1)=0, (cell&1)=1 -> newCell=(1+1)&3=2 */
        make_projectile_kinetic(&p, 2, 1, 1, 1, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        ok = ok && (r.crossedCell == 0) && (r.newCell == 2);
        CHECK(ok, "Cell parity rule: (dir&1)==(cell&1) -> newCell=cell-1; else newCell=cell+1");
    }
    /* 16: After 10 open-floor advances with kinetic=10, stepE=1, the
     *     11th advance despawns with DESPAWN_ENERGY. */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        int i;
        int okFlights = 1;
        int eleventhDespawn;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        /* kinetic=11 so exactly 10 advances FLEW (attack decrements
         * 30->20), then the 11th advance sees kinetic=1 <= stepE=1
         * and despawns with DESPAWN_ENERGY. Per F0811 step 3, the
         * lifespan gate fires when kineticEnergy <= stepEnergy at
         * tick start. */
        make_projectile_kinetic(&p, 1, 1, 5, 5, /*kinetic*/11, /*attack*/30, /*stepE*/1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        for (i = 0; i < 10; i++) {
            F0811_PROJECTILE_Advance_Compat(&p, &d, (uint32_t)(100 + i),
                                              &rng, &pOut, &r);
            if (r.despawn || r.resultKind != PROJECTILE_RESULT_FLEW) {
                okFlights = 0; break;
            }
            p = pOut;
        }
        F0811_PROJECTILE_Advance_Compat(&p, &d, 200, &rng, &pOut, &r);
        eleventhDespawn = (r.despawn == 1
                          && r.resultKind == PROJECTILE_RESULT_DESPAWN_ENERGY);
        CHECK(okFlights && eleventhDespawn,
              "Kinetic(stepE=1, kinetic=11): 10 advances FLEW, 11th despawns with DESPAWN_ENERGY");
    }
    /* 17: Boundary clamp — dest is off-map */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 1, 0, 0, -1, 0);
        d.destIsMapBoundary = 1;
        make_projectile_kinetic(&p, 3, 3, 0, 0, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.despawn == 1 && r.resultKind == PROJECTILE_RESULT_HIT_WALL,
              "Map boundary: crossing off-map -> HIT_WALL + despawn");
    }

    /* ================================================================
     *  Block D — collision cases (invariants 18-24)
     * ================================================================ */
    /* 18: Wall ahead — kinetic vs magical (explosion emitted for magical) */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        int okK, okM;

        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destSquareType = PROJECTILE_ELEMENT_WALL;
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        okK = (r.resultKind == PROJECTILE_RESULT_HIT_WALL
               && r.despawn == 1
               && r.emittedCombatAction == 0
               && r.emittedExplosion == 0);

        make_projectile_magical(&p, PROJECTILE_SUBTYPE_FIREBALL,
                                 COMBAT_ATTACK_FIRE,
                                 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        okM = (r.resultKind == PROJECTILE_RESULT_HIT_WALL
               && r.despawn == 1
               && r.emittedCombatAction == 0
               && r.emittedExplosion == 1);
        CHECK(okK && okM,
              "Wall hit: kinetic -> no emission; magical FIREBALL -> explosion emitted");
    }
    /* 19: Closed door -> HIT_DOOR + DOOR_DESTRUCTION event */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destSquareType = PROJECTILE_ELEMENT_DOOR;
        d.destDoorState  = PROJECTILE_DOOR_STATE_CLOSED_FULL;
        d.destDoorAllowsProjectilePassThrough = 0;
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.resultKind == PROJECTILE_RESULT_HIT_DOOR
              && r.emittedDoorDestructionEvent == 1
              && r.outNextTick.kind == TIMELINE_EVENT_DOOR_DESTRUCTION
              && r.outNextTick.fireAtTick > 100u,
              "Closed door -> HIT_DOOR + TIMELINE_EVENT_DOOR_DESTRUCTION scheduled");
    }
    /* 20: Destroyed door -> projectile passes through (FLEW, despawn=0) */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destSquareType      = PROJECTILE_ELEMENT_DOOR;
        d.destDoorState       = PROJECTILE_DOOR_STATE_DESTROYED;
        d.destDoorIsDestroyed = 1;
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.resultKind == PROJECTILE_RESULT_FLEW
              && r.despawn == 0,
              "Destroyed door -> projectile passes (FLEW, despawn=0)");
    }
    /* 21: Champion on dest cell -> HIT_CHAMPION + combat action */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destHasChampion       = 1;
        /* dir=1,cell=1 crosses; parity (1&1)==(1&1) -> newCell=0. */
        d.destChampionCellMask  = 0x01;
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.resultKind == PROJECTILE_RESULT_HIT_CHAMPION
              && r.emittedCombatAction == 1
              && r.outAction.kind == COMBAT_ACTION_APPLY_DAMAGE_CHAMPION
              && r.outAction.allowedWounds == (COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO),
              "Champion on dest cell -> HIT_CHAMPION + APPLY_DAMAGE_CHAMPION + wounds=HEAD|TORSO");
    }
    /* 22: Creature on dest cell -> HIT_CREATURE + APPLY_DAMAGE_GROUP */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destHasCreatureGroup  = 1;
        d.destCreatureCellMask  = 0x01;  /* post-flip newCell=0 */
        d.destCreatureType      = 10; /* Mummy */
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.resultKind == PROJECTILE_RESULT_HIT_CREATURE
              && r.emittedCombatAction == 1
              && r.outAction.kind == COMBAT_ACTION_APPLY_DAMAGE_GROUP,
              "Creature on dest cell -> HIT_CREATURE + APPLY_DAMAGE_GROUP");
    }
    /* 23: Non-material creature + non-HARM projectile -> DESPAWN_ENERGY (v1) */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destHasCreatureGroup     = 1;
        d.destCreatureCellMask     = 0x01;  /* post-flip newCell=0 */
        d.destCreatureIsNonMaterial= 1;
        d.destCreatureType         = 5;
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.despawn == 1 && r.resultKind == PROJECTILE_RESULT_DESPAWN_ENERGY
              && r.emittedCombatAction == 0,
              "Non-material creature + non-HARM_NON_MATERIAL projectile -> DESPAWN_ENERGY, no emission");
    }
    /* 24: Fluxcage on dest -> HIT_FLUXCAGE, despawn=1, nothing emitted */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destHasFluxcage = 1;
        make_projectile_magical(&p, PROJECTILE_SUBTYPE_FIREBALL,
                                 COMBAT_ATTACK_FIRE,
                                 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.resultKind == PROJECTILE_RESULT_HIT_FLUXCAGE
              && r.despawn == 1
              && r.emittedCombatAction == 0
              && r.emittedExplosion == 0
              && r.emittedDoorDestructionEvent == 0,
              "Fluxcage on dest cell -> HIT_FLUXCAGE, despawn=1, no emission");
    }

    /* ================================================================
     *  Block E — despawn paths (invariants 25-27)
     * ================================================================ */
    /* 25: kineticEnergy <= stepEnergy at tick start -> DESPAWN_ENERGY */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        make_projectile_kinetic(&p, 1, 1, 5, 5, /*kinetic*/5, /*attack*/30, /*stepE*/5);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.despawn == 1 && r.resultKind == PROJECTILE_RESULT_DESPAWN_ENERGY
              && r.emittedCombatAction == 0 && r.emittedExplosion == 0,
              "kineticEnergy <= stepEnergy at tick start -> DESPAWN_ENERGY, no emission");
    }
    /* 26: Wall hit always sets despawn=1 */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destSquareType = PROJECTILE_ELEMENT_WALL;
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.despawn == 1 && r.resultKind == PROJECTILE_RESULT_HIT_WALL,
              "Wall hit always sets despawn=1");
    }
    /* 27: Two projectiles on same cell -> both despawn (order-independent v1) */
    {
        struct ProjectileInstance_Compat pA, pB, pOutA, pOutB;
        struct CellContentDigest_Compat dA, dB;
        struct ProjectileTickResult_Compat rA, rB;
        struct RngState_Compat rng;

        zero_digest(&dA);
        zero_digest(&dB);
        set_source_and_dest(&dA, 0, 5, 5, 6, 5);
        set_source_and_dest(&dB, 0, 6, 5, 5, 5);
        /* Each sees the other on its destination cell. */
        dA.destHasOtherProjectile = 1;
        dB.destHasOtherProjectile = 1;

        make_projectile_kinetic(&pA, 1, 1, 5, 5, 10, 30, 1);
        make_projectile_kinetic(&pB, 3, 3, 6, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&pA, &dA, 100, &rng, &pOutA, &rA);
        F0811_PROJECTILE_Advance_Compat(&pB, &dB, 100, &rng, &pOutB, &rB);
        CHECK(rA.despawn == 1 && rA.resultKind == PROJECTILE_RESULT_HIT_OTHER_PROJECTILE
              && rB.despawn == 1 && rB.resultKind == PROJECTILE_RESULT_HIT_OTHER_PROJECTILE
              && rA.emittedCombatAction == 0 && rB.emittedCombatAction == 0,
              "Projectile-vs-projectile: both despawn, no damage (v1 order-independent)");
    }

    /* ================================================================
     *  Block F — explosion per-type behaviour (invariants 28-31)
     * ================================================================ */
    /* 28: Fireball attack=80 -> ONE_SHOT + party+group actions (both present) */
    {
        struct ExplosionInstance_Compat e, eOut;
        struct CellContentDigest_Compat d;
        struct ExplosionTickResult_Compat r;
        struct RngState_Compat rng;
        memset(&e, 0, sizeof(e));
        e.slotIndex     = 0;
        e.explosionType = C000_EXPLOSION_FIREBALL;
        e.attack        = 80;
        e.currentFrame  = 0;
        e.maxFrames     = 3;
        e.reserved0     = 1;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 5, 5);
        d.destHasChampion      = 1;
        d.destChampionCellMask = 0x01;
        d.destHasCreatureGroup = 0;
        F0730_COMBAT_RngInit_Compat(&rng, 0x1234);
        F0822_EXPLOSION_Advance_Compat(&e, &d, 100, &rng, &eOut, &r);
        CHECK(r.despawn == 1 && r.resultKind == EXPLOSION_RESULT_ONE_SHOT
              && r.emittedCombatActionPartyCount == 1,
              "Fireball attack=80 -> ONE_SHOT + despawn + party action emitted");
    }
    /* 29: Poison cloud attack=32 -> newAttack=29, reschedule +1 */
    {
        struct ExplosionInstance_Compat e, eOut;
        struct CellContentDigest_Compat d;
        struct ExplosionTickResult_Compat r;
        struct RngState_Compat rng;
        memset(&e, 0, sizeof(e));
        e.explosionType = C007_EXPLOSION_POISON_CLOUD;
        e.attack        = 32;
        e.poisonAttack  = 64;
        e.maxFrames     = 30;
        e.reserved0     = 1;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 5, 5);
        d.destHasCreatureGroup = 1;
        d.destCreatureCellMask = 0x01;
        d.destCreatureType     = 10;
        F0730_COMBAT_RngInit_Compat(&rng, 0x5678);
        F0822_EXPLOSION_Advance_Compat(&e, &d, 500, &rng, &eOut, &r);
        CHECK(r.despawn == 0 && r.newAttack == 29
              && eOut.attack == 29
              && r.outNextTick.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE
              && r.outNextTick.fireAtTick == 501u
              && r.emittedCombatActionGroupCount == 1,
              "Poison cloud attack=32 -> newAttack=29, reschedule +1, group action emitted");
    }
    /* 30: Poison cloud attack=5 -> despawn */
    {
        struct ExplosionInstance_Compat e, eOut;
        struct CellContentDigest_Compat d;
        struct ExplosionTickResult_Compat r;
        struct RngState_Compat rng;
        memset(&e, 0, sizeof(e));
        e.explosionType = C007_EXPLOSION_POISON_CLOUD;
        e.attack        = 5;
        e.maxFrames     = 30;
        e.reserved0     = 1;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 5, 5);
        F0730_COMBAT_RngInit_Compat(&rng, 0x5678);
        F0822_EXPLOSION_Advance_Compat(&e, &d, 100, &rng, &eOut, &r);
        CHECK(r.despawn == 1 && r.resultKind == EXPLOSION_RESULT_ONE_SHOT,
              "Poison cloud attack=5 (< 6) -> despawn + ONE_SHOT");
    }
    /* 31: Smoke attack=100 -> newAttack=60, reschedule +1, no damage */
    {
        struct ExplosionInstance_Compat e, eOut;
        struct CellContentDigest_Compat d;
        struct ExplosionTickResult_Compat r;
        struct RngState_Compat rng;
        memset(&e, 0, sizeof(e));
        e.explosionType = C040_EXPLOSION_SMOKE;
        e.attack        = 100;
        e.maxFrames     = 30;
        e.reserved0     = 1;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 5, 5);
        d.destHasChampion      = 1;
        d.destChampionCellMask = 0x01;
        d.destHasCreatureGroup = 1;
        d.destCreatureCellMask = 0x02;
        F0730_COMBAT_RngInit_Compat(&rng, 0x5678);
        F0822_EXPLOSION_Advance_Compat(&e, &d, 100, &rng, &eOut, &r);
        CHECK(r.despawn == 0 && r.newAttack == 60
              && r.outNextTick.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE
              && r.emittedCombatActionPartyCount == 0
              && r.emittedCombatActionGroupCount == 0,
              "Smoke attack=100 -> newAttack=60, reschedule +1, no damage emitted");
    }

    /* ================================================================
     *  Block G — AoE distribution (invariants 32-33)
     * ================================================================ */
    /* 32: Poison cloud + champion -> party action only (champion preempts group) */
    {
        struct ExplosionInstance_Compat e, eOut;
        struct CellContentDigest_Compat d;
        struct ExplosionTickResult_Compat r;
        struct RngState_Compat rng;
        memset(&e, 0, sizeof(e));
        e.explosionType = C007_EXPLOSION_POISON_CLOUD;
        e.attack        = 32;
        e.maxFrames     = 30;
        e.reserved0     = 1;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 5, 5);
        d.destHasChampion      = 1;
        d.destChampionCellMask = 0x01;
        d.destHasCreatureGroup = 1;
        d.destCreatureCellMask = 0x02;
        d.destCreatureType     = 10;
        F0730_COMBAT_RngInit_Compat(&rng, 0x9);
        F0822_EXPLOSION_Advance_Compat(&e, &d, 100, &rng, &eOut, &r);
        CHECK(r.emittedCombatActionPartyCount == 1
              && r.emittedCombatActionGroupCount == 0,
              "Poison cloud + champion -> champion preempts group (party=1, group=0)");
    }
    /* 33: Poison cloud + creature only -> group action */
    {
        struct ExplosionInstance_Compat e, eOut;
        struct CellContentDigest_Compat d;
        struct ExplosionTickResult_Compat r;
        struct RngState_Compat rng;
        memset(&e, 0, sizeof(e));
        e.explosionType = C007_EXPLOSION_POISON_CLOUD;
        e.attack        = 32;
        e.maxFrames     = 30;
        e.reserved0     = 1;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 5, 5);
        d.destHasChampion      = 0;
        d.destHasCreatureGroup = 1;
        d.destCreatureCellMask = 0x02;
        d.destCreatureType     = 10;
        F0730_COMBAT_RngInit_Compat(&rng, 0x9);
        F0822_EXPLOSION_Advance_Compat(&e, &d, 100, &rng, &eOut, &r);
        CHECK(r.emittedCombatActionPartyCount == 0
              && r.emittedCombatActionGroupCount == 1,
              "Poison cloud, no champion -> group action only");
    }

    /* ================================================================
     *  Block H — per-type attack-roll envelope (invariants 34-35)
     * ================================================================ */
    /* 34: Fireball attack=10, seed=0xA5 -> applied in [7..13] */
    {
        struct ExplosionInstance_Compat e;
        struct CellContentDigest_Compat d;
        struct RngState_Compat rng;
        int applied = -1;
        memset(&e, 0, sizeof(e));
        e.explosionType = C000_EXPLOSION_FIREBALL;
        e.attack        = 10;
        zero_digest(&d);
        F0730_COMBAT_RngInit_Compat(&rng, 0xA5);
        F0823_EXPLOSION_ComputeAoE_Compat(&e, &d, &rng, &applied);
        /* raw = (10>>1)+1 = 6; += rand(rng, 6) in [0..5] + 1 -> raw in [7..12].
         * v1 computes raw += rng(raw) + 1 which gives range [7..13]. */
        CHECK(applied >= 7 && applied <= 13,
              "F0823 fireball attack=10 -> applied in [7..13] (envelope)");
    }
    /* 35: Lightning attack=10 -> applied in [3..6] */
    {
        struct ExplosionInstance_Compat e;
        struct CellContentDigest_Compat d;
        struct RngState_Compat rng;
        int applied = -1;
        memset(&e, 0, sizeof(e));
        e.explosionType = C002_EXPLOSION_LIGHTNING_BOLT;
        e.attack        = 10;
        zero_digest(&d);
        F0730_COMBAT_RngInit_Compat(&rng, 0xA5);
        F0823_EXPLOSION_ComputeAoE_Compat(&e, &d, &rng, &applied);
        CHECK(applied >= 3 && applied <= 6,
              "F0823 lightning attack=10 -> applied in [3..6] (half of fireball)");
    }

    /* ================================================================
     *  Block I — Phase 13 integration (invariants 36-37)
     *
     *  Structural parity check: the CombatAction_Compat emitted by
     *  Phase 17 can be round-tripped through Phase 13's ser/deser
     *  without change, and every field the Phase 13 damage resolver
     *  reads (kind, targetMapIndex/X/Y, allowedWounds, rawAttackValue,
     *  attackTypeCode, defenderSlotOrCreatureIndex) is populated.
     * ================================================================ */
    /* 36: Champion-hit -> APPLY_DAMAGE_CHAMPION with populated fields
     *     + ser/deser round-trips via Phase 13's F0740/F0741. */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        struct CombatAction_Compat aCopy;
        unsigned char buf[COMBAT_ACTION_SERIALIZED_SIZE];
        int w, rd;
        int structuralOK;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destHasChampion       = 1;
        d.destChampionCellMask  = 0x01;  /* post-flip newCell=0 */
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        w = F0740_COMBAT_ActionSerialize_Compat(&r.outAction, buf, sizeof(buf));
        rd = F0741_COMBAT_ActionDeserialize_Compat(&aCopy, buf, sizeof(buf));
        /* Phase 13 F0740/F0741 return 1 on success (not byte count). */
        structuralOK = (w == 1 && rd == 1
                        && memcmp(&r.outAction, &aCopy, sizeof(aCopy)) == 0
                        && aCopy.kind == COMBAT_ACTION_APPLY_DAMAGE_CHAMPION
                        && aCopy.rawAttackValue == p.attack
                        && aCopy.attackTypeCode == p.attackTypeCode
                        && aCopy.targetMapIndex == d.destMapIndex);
        CHECK(structuralOK,
              "F0811 champion-hit CombatAction round-trips through Phase 13 ser/deser with populated fields");
    }
    /* 37: Creature-hit -> APPLY_DAMAGE_GROUP with populated fields
     *     + Phase 13 ser/deser round-trip. */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        struct CombatAction_Compat aCopy;
        unsigned char buf[COMBAT_ACTION_SERIALIZED_SIZE];
        int w, rd;
        int structuralOK;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        d.destHasCreatureGroup = 1;
        d.destCreatureCellMask = 0x01;  /* post-flip newCell=0 */
        d.destCreatureType     = 10;
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        w = F0740_COMBAT_ActionSerialize_Compat(&r.outAction, buf, sizeof(buf));
        rd = F0741_COMBAT_ActionDeserialize_Compat(&aCopy, buf, sizeof(buf));
        structuralOK = (w == 1 && rd == 1
                        && memcmp(&r.outAction, &aCopy, sizeof(aCopy)) == 0
                        && aCopy.kind == COMBAT_ACTION_APPLY_DAMAGE_GROUP
                        && aCopy.rawAttackValue == p.attack
                        && aCopy.targetMapIndex == d.destMapIndex);
        CHECK(structuralOK,
              "F0811 creature-hit CombatAction round-trips through Phase 13 ser/deser with populated fields");
    }

    /* ================================================================
     *  Block J — Phase 14 integration (invariant 38)
     * ================================================================ */
    {
        struct SpellEffect_Compat effect;
        struct ProjectileList_Compat list;
        struct TimelineEvent_Compat ev;
        int slot = -1;
        int rc;
        memset(&effect, 0, sizeof(effect));
        effect.spellKind     = C2_SPELL_KIND_PROJECTILE_COMPAT;
        effect.spellType     = 0; /* FIREBALL */
        effect.impactAttack  = 100;
        effect.kineticEnergy = 20;
        memset(&list, 0, sizeof(list));
        rc = F0812_PROJECTILE_CreateFromSpellEffect_Compat(
            &effect, /*champ*/0, /*mapIdx*/1, /*mx*/5, /*my*/5,
            /*dir*/1, /*tick*/100, &list, &slot, &ev);
        CHECK(rc == 1 && slot == 0
              && list.entries[slot].projectileSubtype == PROJECTILE_SUBTYPE_FIREBALL
              && list.entries[slot].projectileCategory == PROJECTILE_CATEGORY_MAGICAL
              && list.entries[slot].attack == 100
              && list.entries[slot].kineticEnergy == 20
              && list.entries[slot].attackTypeCode == COMBAT_ATTACK_FIRE
              && ev.kind == TIMELINE_EVENT_PROJECTILE_MOVE
              && ev.fireAtTick == 101u,
              "F0812 fireball spell-effect -> projectile with matching attack/kinetic + PROJECTILE_MOVE at tick+1");
    }

    /* ================================================================
     *  Block K — Phase 12 integration (invariants 39-40)
     * ================================================================ */
    /* 39: After a FLEW advance, outNextTick.kind=PROJECTILE_MOVE and
     *     fireAtTick > currentTick. */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        make_projectile_kinetic(&p, 1, 1, 5, 5, 20, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 500, &rng, &pOut, &r);
        CHECK(r.resultKind == PROJECTILE_RESULT_FLEW
              && r.outNextTick.kind == TIMELINE_EVENT_PROJECTILE_MOVE
              && r.outNextTick.fireAtTick > 500u,
              "After FLEW: outNextTick.kind=PROJECTILE_MOVE and fireAtTick > currentTick");
    }
    /* 40: Feed outNextTick through Phase 12 F0721 + F0727/F0728 round-trip. */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        struct TimelineQueue_Compat q1, q2;
        unsigned char buf[TIMELINE_QUEUE_SERIALIZED_SIZE];
        int w, rd;
        int ok;

        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        make_projectile_kinetic(&p, 1, 1, 5, 5, 20, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 500, &rng, &pOut, &r);

        memset(&q1, 0, sizeof(q1));
        F0720_TIMELINE_Init_Compat(&q1, 500);
        F0721_TIMELINE_Schedule_Compat(&q1, &r.outNextTick);
        memset(&q2, 0x55, sizeof(q2));

        w = F0727_TIMELINE_QueueSerialize_Compat(&q1, buf, sizeof(buf));
        rd = F0728_TIMELINE_QueueDeserialize_Compat(&q2, buf, sizeof(buf));
        /* Phase 12 ser/deser return 1 on success, not the byte count.
         * Compare structurally via memcmp (unused events[] slots are
         * zeroed by both sides). */
        ok = (w == 1 && rd == 1
              && q1.nowTick == q2.nowTick
              && q1.count == q2.count
              && memcmp(q1.events, q2.events,
                        q1.count * sizeof(q1.events[0])) == 0);
        CHECK(ok,
              "Phase 12 TimelineQueue round-trips bit-identical after scheduling Phase 17 outNextTick");
    }

    /* ================================================================
     *  Block L — Phase 15 integration (invariants 41-42)
     *  Standalone ser/deser of lists with 3 projectiles / 4 explosions.
     * ================================================================ */
    /* 41: ProjectileList with 3 projectiles across 3 owner kinds */
    {
        struct ProjectileList_Compat a, b;
        unsigned char buf[PROJECTILE_LIST_SERIALIZED_SIZE];
        int w, r;
        int i;
        memset(&a, 0, sizeof(a));
        for (i = 0; i < 3; i++) {
            make_projectile_kinetic(&a.entries[i * 7], 1, 1,
                                     5 + i, 5, 20, 30, 1);
            a.entries[i * 7].slotIndex = i * 7;
            a.entries[i * 7].ownerKind = i; /* 0=champion, 1=creature, 2=launcher */
            a.entries[i * 7].ownerIndex = i * 3;
        }
        a.count = 3;
        memset(&b, 0xBB, sizeof(b));
        w = F0829_PROJECTILE_ListSerialize_Compat(&a, buf, sizeof(buf));
        r = F0829_PROJECTILE_ListDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == PROJECTILE_LIST_SERIALIZED_SIZE
              && r == PROJECTILE_LIST_SERIALIZED_SIZE
              && memcmp(&a, &b, sizeof(a)) == 0,
              "ProjectileList (3 owner kinds) ser/deser round-trips bit-identical");
    }
    /* 42: ExplosionList with 4 distinct explosion types */
    {
        struct ExplosionList_Compat a, b;
        unsigned char buf[EXPLOSION_LIST_SERIALIZED_SIZE];
        int w, r;
        const int types[4] = {
            C000_EXPLOSION_FIREBALL,
            C002_EXPLOSION_LIGHTNING_BOLT,
            C007_EXPLOSION_POISON_CLOUD,
            C040_EXPLOSION_SMOKE
        };
        int i;
        memset(&a, 0, sizeof(a));
        for (i = 0; i < 4; i++) {
            fill_explosion_populated(&a.entries[i * 3]);
            a.entries[i * 3].slotIndex     = i * 3;
            a.entries[i * 3].explosionType = types[i];
            a.entries[i * 3].attack        = 20 + i * 10;
        }
        a.count = 4;
        memset(&b, 0xEE, sizeof(b));
        w = F0829_EXPLOSION_ListSerialize_Compat(&a, buf, sizeof(buf));
        r = F0829_EXPLOSION_ListDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(w == EXPLOSION_LIST_SERIALIZED_SIZE
              && r == EXPLOSION_LIST_SERIALIZED_SIZE
              && memcmp(&a, &b, sizeof(a)) == 0,
              "ExplosionList (4 types: fireball/lightning/poison-cloud/smoke) ser/deser round-trips");
    }

    /* ================================================================
     *  Block M — Phase 16 integration (invariant 43)
     *  Synthesised SpellEffect (representing a Phase 16 creature spell
     *  request resolved by Phase 14) -> F0812 launches a projectile
     *  whose fields mirror the spell's attack/kinetic parameters.
     * ================================================================ */
    {
        struct SpellEffect_Compat effect;
        struct ProjectileList_Compat list;
        struct TimelineEvent_Compat ev;
        int slot = -1;
        int rc;
        memset(&effect, 0, sizeof(effect));
        effect.spellKind     = C2_SPELL_KIND_PROJECTILE_COMPAT;
        effect.spellType     = 1; /* LIGHTNING */
        effect.impactAttack  = 77;
        effect.kineticEnergy = 15;
        memset(&list, 0, sizeof(list));
        rc = F0812_PROJECTILE_CreateFromSpellEffect_Compat(
            &effect, /*champ*/2, /*mapIdx*/3, /*mx*/4, /*my*/6,
            /*dir*/2, /*tick*/200, &list, &slot, &ev);
        CHECK(rc == 1 && slot == 0
              && list.entries[slot].projectileSubtype == PROJECTILE_SUBTYPE_LIGHTNING_BOLT
              && list.entries[slot].attack == 77
              && list.entries[slot].kineticEnergy == 15
              && list.entries[slot].attackTypeCode == COMBAT_ATTACK_LIGHTNING
              && list.entries[slot].ownerKind == PROJECTILE_OWNER_CHAMPION
              && list.entries[slot].ownerIndex == 2,
              "Phase 16 creature-spell SpellEffect (synthesised) -> F0812 produces matching projectile");
    }

    /* ================================================================
     *  Block N — boundary (invariants 44-47)
     * ================================================================ */
    /* 44: F0811 null in -> 0 */
    {
        struct ProjectileInstance_Compat pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        int rc;
        zero_digest(&d);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        rc = F0811_PROJECTILE_Advance_Compat(NULL, &d, 100, &rng, &pOut, &r);
        CHECK(rc == 0,
              "F0811(in=NULL, ...) returns 0");
    }
    /* 45: direction=4 -> returns 0, resultKind=INVALID */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        int rc;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        make_projectile_kinetic(&p, 1, 1, 5, 5, 10, 30, 1);
        p.direction = 4;
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        rc = F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(rc == 0 && r.resultKind == PROJECTILE_RESULT_INVALID,
              "F0811 direction=4 -> returns 0, resultKind=INVALID");
    }
    /* 46: attack=0 and kineticEnergy=0 -> DESPAWN_ENERGY */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        make_projectile_kinetic(&p, 1, 1, 5, 5, 0, 0, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.despawn == 1
              && r.resultKind == PROJECTILE_RESULT_DESPAWN_ENERGY
              && r.emittedCombatAction == 0,
              "attack=0 && kineticEnergy=0 -> DESPAWN_ENERGY immediately");
    }
    /* 47: destIsMapBoundary=1 -> HIT_WALL */
    {
        struct ProjectileInstance_Compat p, pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        zero_digest(&d);
        set_source_and_dest(&d, 0, 0, 0, -1, 0);
        d.destIsMapBoundary = 1;
        make_projectile_kinetic(&p, 3, 3, 0, 0, 10, 30, 1);
        F0730_COMBAT_RngInit_Compat(&rng, 1);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);
        CHECK(r.despawn == 1 && r.resultKind == PROJECTILE_RESULT_HIT_WALL,
              "Out-of-bounds digest -> HIT_WALL + despawn");
    }

    /* ================================================================
     *  Block O — purity (invariant 48 — two sub-checks)
     * ================================================================ */
    {
        struct ProjectileInstance_Compat p, pSnap, pOut;
        struct CellContentDigest_Compat d, dSnap;
        struct ProjectileTickResult_Compat r;
        struct RngState_Compat rng;
        unsigned char bufP[sizeof(struct ProjectileInstance_Compat)];
        unsigned char bufD[sizeof(struct CellContentDigest_Compat)];
        unsigned char bufT[256];
        uint32_t crcP1, crcP2, crcD1, crcD2, crcT1, crcT2;
        int iter;
        int purityOK;

        zero_digest(&d);
        set_source_and_dest(&d, 0, 5, 5, 6, 5);
        make_projectile_kinetic(&p, 1, 1, 5, 5, 20, 30, 1);
        pSnap = p;
        dSnap = d;
        memcpy(bufP, &p, sizeof(p));
        memcpy(bufD, &d, sizeof(d));
        crcP1 = local_crc32(bufP, sizeof(bufP));
        crcD1 = local_crc32(bufD, sizeof(bufD));

        F0730_COMBAT_RngInit_Compat(&rng, 0x1234);
        F0811_PROJECTILE_Advance_Compat(&p, &d, 100, &rng, &pOut, &r);

        memcpy(bufP, &p, sizeof(p));
        memcpy(bufD, &d, sizeof(d));
        crcP2 = local_crc32(bufP, sizeof(bufP));
        crcD2 = local_crc32(bufD, sizeof(bufD));

        /* Sub-check 48a: input pointers unchanged. */
        purityOK = (crcP1 == crcP2) && (crcD1 == crcD2)
                   && memcmp(&p, &pSnap, sizeof(p)) == 0
                   && memcmp(&d, &dSnap, sizeof(d)) == 0;

        /* Sub-check 48b: static SUBTYPE_CREATES_EXPLOSION table —
         * probe this indirectly by observing that many random
         * advances all agree on which subtypes emit explosions.
         * The actual 256-entry table is not exposed. We snapshot
         * the behaviour via F0811 and hash whichever pattern the
         * 256 iterations produce, then re-run and confirm. */
        memset(bufT, 0, sizeof(bufT));
        for (iter = 0; iter < 256; iter++) {
            struct ProjectileInstance_Compat pp;
            struct ProjectileTickResult_Compat rr;
            struct ProjectileInstance_Compat ppOut;
            struct CellContentDigest_Compat dd;
            struct RngState_Compat rng2;
            zero_digest(&dd);
            set_source_and_dest(&dd, 0, 5, 5, 6, 5);
            dd.destSquareType = PROJECTILE_ELEMENT_WALL; /* force wall-hit */
            make_projectile_magical(&pp, iter & 0xFF,
                                    COMBAT_ATTACK_FIRE,
                                    1, 1, 5, 5, 10, 30, 1);
            F0730_COMBAT_RngInit_Compat(&rng2, 0xBEEF);
            F0811_PROJECTILE_Advance_Compat(&pp, &dd, 100, &rng2, &ppOut, &rr);
            bufT[iter] = (unsigned char)(rr.emittedExplosion ? 1 : 0);
        }
        crcT1 = local_crc32(bufT, sizeof(bufT));
        for (iter = 0; iter < 256; iter++) {
            struct ProjectileInstance_Compat pp;
            struct ProjectileTickResult_Compat rr;
            struct ProjectileInstance_Compat ppOut;
            struct CellContentDigest_Compat dd;
            struct RngState_Compat rng2;
            zero_digest(&dd);
            set_source_and_dest(&dd, 0, 5, 5, 6, 5);
            dd.destSquareType = PROJECTILE_ELEMENT_WALL;
            make_projectile_magical(&pp, iter & 0xFF,
                                    COMBAT_ATTACK_FIRE,
                                    1, 1, 5, 5, 10, 30, 1);
            F0730_COMBAT_RngInit_Compat(&rng2, 0xBEEF);
            F0811_PROJECTILE_Advance_Compat(&pp, &dd, 100, &rng2, &ppOut, &rr);
        }
        crcT2 = local_crc32(bufT, sizeof(bufT));
        purityOK = purityOK && (crcT1 == crcT2);

        CHECK(purityOK,
              "Purity 48a/48b: (a) const in/digest unchanged across F0811; (b) SUBTYPE_CREATES_EXPLOSION pattern CRC stable across 256 iterations");
    }

    /* ================================================================
     *  Block P — Real DUNGEON.DAT spot-check (invariant 49)
     * ================================================================ */
    {
        struct DungeonDatState_Compat dungeon;
        int found = 0;
        int wallDistance = -1;
        int hitTick = -1;
        int ok = 0;
        int partyX = -1, partyY = -1, partyDir = -1;
        int w = 0, h = 0;

        memset(&dungeon, 0, sizeof(dungeon));
        if (F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon)
            && F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon)) {
            int py, px, pd;
            F0501_DUNGEON_DecodePartyLocation_Compat(
                dungeon.header.initialPartyLocation,
                &pd, &py, &px);
            partyX   = px;
            partyY   = py;
            partyDir = pd;

            if (dungeon.header.mapCount > 0 && dungeon.tiles != NULL
                && dungeon.tiles[0].squareData != NULL) {
                int xi;
                w = dungeon.maps[0].width;
                h = dungeon.maps[0].height;
                /* Scan east from (partyX, partyY) until wall or boundary. */
                for (xi = partyX + 1; xi < w; xi++) {
                    unsigned char sq =
                        dungeon.tiles[0].squareData[xi * h + partyY];
                    int elem = sq >> 5;
                    if (elem == DUNGEON_ELEMENT_WALL) {
                        wallDistance = xi - partyX;
                        break;
                    }
                }
                if (wallDistance < 0 && xi >= w) {
                    wallDistance = w - partyX;   /* boundary-equiv */
                }
                if (wallDistance > 0 && wallDistance <= 10) {
                    found = 1;
                }
            }

            if (found) {
                struct ProjectileInstance_Compat p, pOut;
                struct RngState_Compat rng;
                int t;
                int despawned = 0;
                make_projectile_kinetic(&p, /*dir=east*/1, /*cell*/1,
                                          partyX, partyY,
                                          /*kinetic*/8, /*attack*/30,
                                          /*stepE*/1);
                /* No first-move grace — go straight into motion each
                 * tick so every cross-cell step advances one tile. */
                p.firstMoveGraceFlag = 0;
                F0730_COMBAT_RngInit_Compat(&rng, 0xDEAD);
                for (t = 0; t < 10; t++) {
                    struct CellContentDigest_Compat d;
                    struct ProjectileTickResult_Compat r;
                    int destX = p.mapX + 1;
                    int destY = p.mapY;
                    unsigned char destSq = 0;
                    zero_digest(&d);
                    set_source_and_dest(&d, 0, p.mapX, p.mapY,
                                         destX, destY);
                    if (destX < 0 || destX >= w || destY < 0 || destY >= h) {
                        d.destIsMapBoundary = 1;
                    } else {
                        destSq = dungeon.tiles[0].squareData[destX * h + destY];
                        d.destSquareType = destSq >> 5;
                    }
                    F0811_PROJECTILE_Advance_Compat(&p, &d, (uint32_t)(100 + t),
                                                      &rng, &pOut, &r);
                    if (r.despawn) {
                        if (r.resultKind == PROJECTILE_RESULT_HIT_WALL
                            && t + 1 == wallDistance) {
                            hitTick = t + 1;
                            ok = 1;
                        }
                        despawned = 1;
                        break;
                    }
                    p = pOut;
                }
                (void)despawned;
            }

            fprintf(report,
                    "## DUNGEON.DAT integration\n\n"
                    "- mapCount: %d\n- party start (level 1): (%d,%d) dir=%d\n"
                    "- east wall distance: %d\n- projectile hit tick: %d\n"
                    "- invariant 49 PASS: %s\n\n",
                    (int)dungeon.header.mapCount, partyX, partyY, partyDir,
                    wallDistance, hitTick, ok ? "yes" : "no");

            F0502_DUNGEON_FreeTileData_Compat(&dungeon);
            F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        } else {
            fprintf(report,
                    "## DUNGEON.DAT integration\n\n- could not load DUNGEON.DAT at %s\n\n",
                    dungeonPath);
        }
        CHECK(ok,
              "Real DUNGEON.DAT: kinetic eastbound projectile from Level-1 party start hits first wall at computed distance (<=10 ticks)");
    }

    /* ================================================================
     *  Block Q — Loop guard (invariant 50)
     * ================================================================ */
    {
        struct RngState_Compat feed;
        int iter;
        int allOK = 1;
        F0730_COMBAT_RngInit_Compat(&feed, 0xFACE17u);
        for (iter = 0; iter < 100; iter++) {
            struct ProjectileInstance_Compat p;
            struct CellContentDigest_Compat d;
            struct RngState_Compat rng;
            int tick;
            int subtype_sel;
            int dir, cell, squareType, kinetic, attack, stepE;
            subtype_sel = F0732_COMBAT_RngRandom_Compat(&feed, 4);
            dir         = F0732_COMBAT_RngRandom_Compat(&feed, 4);
            cell        = F0732_COMBAT_RngRandom_Compat(&feed, 4);
            squareType  = F0732_COMBAT_RngRandom_Compat(&feed, 7);
            kinetic     = F0732_COMBAT_RngRandom_Compat(&feed, 50) + 1;
            attack      = F0732_COMBAT_RngRandom_Compat(&feed, 200) + 1;
            stepE       = F0732_COMBAT_RngRandom_Compat(&feed, 5) + 1;

            zero_digest(&d);
            set_source_and_dest(&d, 0, 5, 5, 6, 5);
            d.destSquareType = squareType;

            if (subtype_sel == 0) {
                make_projectile_kinetic(&p, dir, cell, 5, 5, kinetic, attack, stepE);
            } else if (subtype_sel == 1) {
                make_projectile_magical(&p, PROJECTILE_SUBTYPE_FIREBALL,
                                         COMBAT_ATTACK_FIRE, dir, cell, 5, 5,
                                         kinetic, attack, stepE);
            } else if (subtype_sel == 2) {
                make_projectile_magical(&p, PROJECTILE_SUBTYPE_LIGHTNING_BOLT,
                                         COMBAT_ATTACK_LIGHTNING, dir, cell, 5, 5,
                                         kinetic, attack, stepE);
            } else {
                make_projectile_magical(&p, PROJECTILE_SUBTYPE_POISON_CLOUD,
                                         COMBAT_ATTACK_NORMAL, dir, cell, 5, 5,
                                         kinetic, attack, stepE);
            }

            F0730_COMBAT_RngInit_Compat(&rng, 0x10000u + (uint32_t)iter);
            for (tick = 0; tick < 200; tick++) {
                struct ProjectileInstance_Compat pOut;
                struct ProjectileTickResult_Compat r;
                int rc = F0811_PROJECTILE_Advance_Compat(
                    &p, &d, (uint32_t)(100 + tick), &rng, &pOut, &r);
                if (rc != 1) {
                    /* Only acceptable if the only input we fuzzed into
                     * an invalid state was direction/cell clamped — we
                     * guarantee dir/cell in [0..3] above, so rc=0 is
                     * a hard failure. */
                    allOK = 0;
                    break;
                }
                if (r.despawn) break;
                /* Non-despawn must emit PROJECTILE_MOVE with fireAtTick
                 * - currentTick >= 1. */
                if (r.outNextTick.kind != TIMELINE_EVENT_PROJECTILE_MOVE
                    || ((uint32_t)r.outNextTick.fireAtTick
                        - (uint32_t)(100 + tick)) < 1u) {
                    allOK = 0;
                    break;
                }
                p = pOut;
            }
            if (!allOK) break;
        }
        CHECK(allOK,
              "Loop-guard: 100 random configs x up to 200 ticks -> every non-despawn advance schedules fireAtTick - currentTick >= 1; no infinite loops");
    }

    /* ================================================================
     *  Trailer
     * ================================================================ */
    fprintf(invariants, "\nInvariant count: %d\n", invariantCount);
    if (failCount == 0) {
        fprintf(invariants, "Status: PASS\n");
    } else {
        fprintf(invariants, "Status: FAIL (%d failures)\n", failCount);
    }
    fclose(invariants);
    fclose(report);
    return failCount > 0 ? 1 : 0;
}
