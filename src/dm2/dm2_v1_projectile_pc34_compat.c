/* dm2_v1_projectile_pc34_compat.c — DM2 V1 Projectile Routing
 *
 * Phase 5 (creature/combat parity) source-lock.
 *
 * DM2's creature attack projectiles are routed through the DM1
 * projectile engine (memory_projectile_pc34_compat.c F0810-F0820).
 * This module is the DM2->DM1 bridge: it maps DM2 creature AI attack
 * flags + world coordinates to a DM1 ProjectileCreateInput_Compat and
 * invokes F0810_PROJECTILE_Create_Compat.
 *
 * Source-lock anchors:
 *   SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)
 *   SKULL.ASM:11100-11200  (projectile routing — SKWin SKULL.ASM source-locked region)
 *   ReDMCSB PROJEXPL.C:76-92       (F0212: projectile is linked live, first move +1 tick)
 *   ReDMCSB PROJEXPL.C:689-690     (F0219: C48 ignore-impacts-first-movement)
 *   ReDMCSB GROUP.C:2376-2387      (F0209: visible same row/column triggers F0207 attack)
 *   ReDMCSB GROUP.C:1695-1770      (F0207: creature attack projectile payload)
 *   skproject/SKWIN/SkWinCore.cpp:10479-10561  (AI_W30_TURNS_MISSILE check)
 *   skproject/SKULLWIN/c_creature.cpp           (DM2_PROCEED_CCM, CCM 0x0d/0x15)
 *   memory_projectile_pc34_compat.h             (F0810-F0820 contract)
 */

#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_creature.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

/* ── Module state ──────────────────────────────────────────────────── */

/* Local projectile list owned by this module.  In production, the M11
 * game loop owns the projectile list — but the DM2 V1 runtime path is
 * decoupled from M11 (DM2 has its own runtime), so this module owns a
 * standalone projectile list.  Production wire-up: M11 dispatches DM2
 * creature attacks and forwards the inputs to this module's projectile
 * list, then drains it back into M11's renderer. */
static struct ProjectileList_Compat s_projectile_list;
static int s_initialized = 0;

/* Observability counters (monotonic, reset by dm2_v1_projectile_reset_counters). */
static int s_dispatch_count = 0;
static int s_spell_count = 0;
static int s_bomb_count = 0;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

static void ensure_init(void) {
    if (s_initialized) return;
    memset(&s_projectile_list, 0, sizeof(s_projectile_list));
    s_initialized = 1;
}

/* ── Public API: lifecycle ────────────────────────────────────────── */

void dm2_v1_projectile_reset_counters(void) {
    s_dispatch_count = 0;
    s_spell_count = 0;
    s_bomb_count = 0;
}

/* ── Public API: pick category+subtype from attack flags ─────────── */

/* Source: skproject/SKULLWIN/c_creature.cpp projectile dispatch.
 * Maps AI_ATTACK_FLAGS__* bits to (PROJECTILE_CATEGORY_*, subtype).
 * Returns 1 if the attack flags resolve to a projectile (KINETIC or
 * MAGICAL), 0 if the attack is melee-only (caller should not dispatch
 * a projectile and instead use the existing melee attack path). */
int dm2_v1_projectile_pick_category(uint16_t attack_flags,
    int *out_category, int *out_subtype)
{
    if (!out_category || !out_subtype) return 0;

    /* KINETIC ranged (Archer Guard, Rocky, etc.) — AI_ATTACK_FLAGS__SHOOT.
     * Source: SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE routing). */
    if (attack_flags & AI_ATTACK_FLAGS__SHOOT) {
        *out_category = PROJECTILE_CATEGORY_KINETIC;
        *out_subtype  = DM2_PROJ_SUBTYPE_KINETIC_ARROW;
        return 1;
    }
    /* DM2 bombs (CCM 0x0d SHOOT_ITEM for bomb-throwing creatures). */
    if (attack_flags & AI_ATTACK_FLAGS__PUSH_BACK) {
        /* Push-back flag also used by DM2 bomb-throwers (Vexirk, etc.).
         * Source: skproject/SKULLWIN/c_creature.cpp bomb dispatch. */
        *out_category = PROJECTILE_CATEGORY_KINETIC;
        *out_subtype  = DM2_PROJ_SUBTYPE_BOMB;
        return 1;
    }
    /* MAGICAL — spell attacks (CCM 0x15 CAST_SPELL).
     * Priority: DISPELL > FIREBALL > LIGHTNING > POISON_CLOUD > POISON_BOLT > POISON_BLOB.
     * Source: skproject/SKWIN/SkWinCore.cpp:27038-27096. */
    if (attack_flags & AI_ATTACK_FLAGS__DISPELL) {
        *out_category = PROJECTILE_CATEGORY_MAGICAL;
        *out_subtype  = DM2_PROJ_SUBTYPE_MAGICAL_DISPELL;
        return 1;
    }
    if (attack_flags & AI_ATTACK_FLAGS__FIREBALL) {
        *out_category = PROJECTILE_CATEGORY_MAGICAL;
        *out_subtype  = DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL;
        return 1;
    }
    if (attack_flags & AI_ATTACK_FLAGS__LIGHTNING) {
        *out_category = PROJECTILE_CATEGORY_MAGICAL;
        *out_subtype  = DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING;
        return 1;
    }
    if (attack_flags & AI_ATTACK_FLAGS__POISON_CLOUD) {
        *out_category = PROJECTILE_CATEGORY_MAGICAL;
        *out_subtype  = DM2_PROJ_SUBTYPE_MAGICAL_POISON_CLOUD;
        return 1;
    }
    if (attack_flags & AI_ATTACK_FLAGS__POISON_BOLT) {
        *out_category = PROJECTILE_CATEGORY_MAGICAL;
        *out_subtype  = DM2_PROJ_SUBTYPE_MAGICAL_POISON_BOLT;
        return 1;
    }
    if (attack_flags & AI_ATTACK_FLAGS__POISON_BLOB) {
        *out_category = PROJECTILE_CATEGORY_MAGICAL;
        *out_subtype  = DM2_PROJ_SUBTYPE_MAGICAL_POISON_BLOB;
        return 1;
    }
    /* Melee-only — no projectile. */
    *out_category = -1;
    *out_subtype = -1;
    return 0;
}

/* ── Internal: compute direction from creature → target ─────────────
 * DM2 creature directions are V1 cardinals (0=N, 1=E, 2=S, 3=W).
 * Source: skproject/SKWIN/DME.h:1505-1560, ReDMCSB GROUP.C:1695-1770. */
static int compute_direction(int from_x, int from_y, int to_x, int to_y) {
    int dx = to_x - from_x;
    int dy = to_y - from_y;
    if (dx == 0 && dy == 0) return 0; /* fallback: N */
    /* Use the dominant axis. */
    if (abs(dx) >= abs(dy)) {
        return (dx > 0) ? 1 : 3; /* E or W */
    } else {
        return (dy > 0) ? 2 : 0; /* S or N */
    }
}

/* ── Internal: create a projectile via F0810 ─────────────────────── */
static DM2_V1_ProjectileDispatchResult create_projectile(
    int creature_instance_id,
    int category, int subtype,
    int target_world_x, int target_world_y,
    int target_map_index)
{
    DM2_V1_ProjectileDispatchResult r;
    memset(&r, 0, sizeof(r));
    r.slot_index = -1;
    r.category = category;
    r.subtype = subtype;

    if (creature_instance_id < 0
        || creature_instance_id >= DM2_MAX_CREATURE_INSTANCES) {
        return r;
    }
    const DM2_V1_CreatureInstance *c =
        dm2_v1_creature_get_instance(creature_instance_id);
    if (!c) return r;
    if (!c->alive) return r;

    ensure_init();

    /* Look up AI spec for AttackStrength. */
    const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(c->ai_index);

    /* Build F0810 input.  Source: skproject/SKWIN/SkWinCore.cpp:27038-27096,
     * memory_projectile_pc34_compat.h ProjectileCreateInput_Compat. */
    struct ProjectileCreateInput_Compat input;
    memset(&input, 0, sizeof(input));
    input.category = category;
    input.subtype = subtype;
    input.ownerKind = PROJECTILE_OWNER_CREATURE;
    input.ownerIndex = creature_instance_id;
    input.mapIndex = target_map_index;
    input.mapX = target_world_x;
    input.mapY = target_world_y;
    input.cell = 0;  /* center cell by default */
    input.direction = compute_direction(
        c->world_x, c->world_y, target_world_x, target_world_y);
    input.kineticEnergy = 100;  /* DM1 default for creature-launched */
    input.attack = spec ? (int)spec->AttackStrength : 10;
    if (input.attack < 1) input.attack = 1;
    input.stepEnergy = 8;  /* ReDMCSB GROUP.C:1695-1770 step energy */
    input.currentTick = 0;  /* DM2 V1 runtime uses game_tick */
    input.poisonAttack = (subtype == PROJECTILE_SUBTYPE_POISON_CLOUD)
                         ? input.attack : 0;
    input.attackTypeCode = 0;
    input.potionPower = 0;
    input.firstMoveGraceFlag = 1;  /* ReDMCSB PROJEXPL.C:689-690 (C48) */

    struct TimelineEvent_Compat firstMove;
    memset(&firstMove, 0, sizeof(firstMove));
    int slot = -1;
    if (!F0810_PROJECTILE_Create_Compat(&input, &s_projectile_list,
                                         &slot, &firstMove)) {
        return r;  /* rejected (list full, invalid input) */
    }
    r.accepted = 1;
    r.slot_index = slot;
    r.owner_kind = PROJECTILE_OWNER_CREATURE;
    r.owner_index = creature_instance_id;
    r.first_move_event_type = (int)firstMove.kind;
    r.first_move_event_tick = (int)firstMove.fireAtTick;
    s_dispatch_count++;
    return r;
}

/* ── Public API: dispatch ranged/creature attack ─────────────────── */
DM2_V1_ProjectileDispatchResult dm2_v1_projectile_dispatch(
    int creature_instance_id,
    int target_world_x, int target_world_y,
    int target_map_index)
{
    DM2_V1_ProjectileDispatchResult r;
    memset(&r, 0, sizeof(r));
    r.slot_index = -1;
    if (creature_instance_id < 0
        || creature_instance_id >= DM2_MAX_CREATURE_INSTANCES) {
        return r;
    }
    const DM2_V1_CreatureInstance *c =
        dm2_v1_creature_get_instance(creature_instance_id);
    if (!c) return r;
    if (!c->alive) return r;
    const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(c->ai_index);
    int category = 0, subtype = 0;
    if (!dm2_v1_projectile_pick_category(spec->AttacksSpells,
                                          &category, &subtype)) {
        /* Melee-only — caller should fall back to melee attack path. */
        r.category = -1;
        r.subtype = -1;
        return r;
    }
    return create_projectile(creature_instance_id, category, subtype,
                             target_world_x, target_world_y, target_map_index);
}

/* ── Public API: dispatch creature spell (CCM 0x15) ─────────────── */
DM2_V1_ProjectileDispatchResult dm2_v1_projectile_dispatch_spell(
    int creature_instance_id,
    int spell_subtype,
    int target_world_x, int target_world_y,
    int target_map_index)
{
    DM2_V1_ProjectileDispatchResult r;
    memset(&r, 0, sizeof(r));
    r.slot_index = -1;
    if (creature_instance_id < 0
        || creature_instance_id >= DM2_MAX_CREATURE_INSTANCES) {
        return r;
    }
    const DM2_V1_CreatureInstance *c =
        dm2_v1_creature_get_instance(creature_instance_id);
    if (!c) return r;
    if (!c->alive) return r;
    r = create_projectile(creature_instance_id,
                           PROJECTILE_CATEGORY_MAGICAL, spell_subtype,
                           target_world_x, target_world_y, target_map_index);
    if (r.accepted) s_spell_count++;
    return r;
}

/* ── Public API: dispatch bomb (DM2 new, area-effect) ────────────── */
DM2_V1_ProjectileDispatchResult dm2_v1_projectile_dispatch_bomb(
    int creature_instance_id,
    int target_world_x, int target_world_y,
    int target_map_index)
{
    DM2_V1_ProjectileDispatchResult r;
    memset(&r, 0, sizeof(r));
    r.slot_index = -1;
    if (creature_instance_id < 0
        || creature_instance_id >= DM2_MAX_CREATURE_INSTANCES) {
        return r;
    }
    const DM2_V1_CreatureInstance *c =
        dm2_v1_creature_get_instance(creature_instance_id);
    if (!c) return r;
    if (!c->alive) return r;
    r = create_projectile(creature_instance_id,
                           PROJECTILE_CATEGORY_KINETIC,
                           DM2_PROJ_SUBTYPE_BOMB,
                           target_world_x, target_world_y, target_map_index);
    if (r.accepted) s_bomb_count++;
    return r;
}

/* ── Public API: observability ────────────────────────────────────── */
int dm2_v1_projectile_dispatch_count(void) { return s_dispatch_count; }
int dm2_v1_projectile_spell_dispatch_count(void) { return s_spell_count; }
int dm2_v1_projectile_bomb_dispatch_count(void) { return s_bomb_count; }

/* ── Phase 5 expansion: projectile drain to M11 ────────────────────
 *
 * Drains the DM2 projectile list into a flat array of framebuffer-ready
 * DM2_V1_DrainedProjectile entries.  M11's render path can iterate over
 * this array each frame and draw fireballs / lightning / arrows in the
 * V1 viewport.
 *
 * Pixel coordinate formula (simplified, matches skproject c_render.cpp):
 *   pixel_x = 32 + (map_x - map_y) * 32 + (cell_offset_x * 16)
 *   pixel_y = (map_x + map_y) * 8 + (cell_offset_y * 8)
 * (For DM2 V1 320x200 viewport, isometric tile rendering.)
 *
 * Returns number of projectiles drained.
 */
static int drain_pixel_x(int map_x, int map_y, int cell) {
    int cell_x = cell & 1;  /* 0 = left, 1 = right within cell */
    return 32 + (map_x - map_y) * 32 + (cell_x ? 16 : 0);
}
static int drain_pixel_y(int map_x, int map_y, int cell) {
    int cell_y = (cell >> 1) & 1;
    return (map_x + map_y) * 8 + (cell_y ? 8 : 0);
}

int dm2_v1_projectile_drain_to_m11(DM2_V1_DrainedProjectile *out_list,
                                    int max_count)
{
    if (!out_list || max_count <= 0) return 0;
    ensure_init();
    int drained = 0;
    /* Phase 5+ extension: walk the full capacity range (not just
     * s_projectile_list.count) because F0813 despawn leaves holes
     * in the slot table.  The step helper (dm2_v1_projectile_step)
     * despawns depleted slots via F0813, which clears reserved3 but
     * may leave earlier slots still occupied.  We filter on
     * reserved3 (the F0810 occupied sentinel) so holes are skipped
     * and live slots anywhere in the table are drained.
     *
     * Source-lock: same F0810/F0813 contract.  This is a pure read-
     * side fix; behavior on a freshly-dispatched list (no F0813
     * holes) is unchanged. */
    for (int i = 0; i < PROJECTILE_LIST_CAPACITY
                && drained < max_count; i++) {
        const struct ProjectileInstance_Compat *p =
            &s_projectile_list.entries[i];
        if (p->reserved3 == 0) continue;  /* empty slot */
        DM2_V1_DrainedProjectile *out = &out_list[drained];
        out->slot_index = p->slotIndex;
        out->category = p->projectileCategory;
        out->subtype = p->projectileSubtype;
        out->owner_kind = p->ownerKind;
        out->owner_index = p->ownerIndex;
        out->map_x = p->mapX;
        out->map_y = p->mapY;
        out->direction = p->direction;
        out->pixel_x = drain_pixel_x(p->mapX, p->mapY, p->cell);
        out->pixel_y = drain_pixel_y(p->mapX, p->mapY, p->cell);
        out->frame = (p->launchedAtTick / 4) % 8;
        out->active = 1;
        drained++;
    }
    return drained;
}

/* ── Phase 5 expansion: synthetic dispatch for tests ─────────────── */
int dm2_v1_projectile_dispatch_synthetic(int category, int subtype,
                                          int map_x, int map_y,
                                          int map_index, int direction)
{
    ensure_init();
    struct ProjectileCreateInput_Compat input;
    memset(&input, 0, sizeof(input));
    input.category = category;
    input.subtype = subtype;
    input.ownerKind = PROJECTILE_OWNER_CREATURE;
    input.ownerIndex = 0;
    input.mapIndex = map_index;
    input.mapX = map_x;
    input.mapY = map_y;
    input.cell = 0;
    input.direction = direction;
    input.kineticEnergy = 100;
    input.attack = 10;
    input.stepEnergy = 8;
    input.currentTick = 0;
    input.firstMoveGraceFlag = 1;
    struct TimelineEvent_Compat firstMove;
    memset(&firstMove, 0, sizeof(firstMove));
    int slot = -1;
    if (!F0810_PROJECTILE_Create_Compat(&input, &s_projectile_list,
                                         &slot, &firstMove)) {
        return -1;
    }
    s_dispatch_count++;
    return slot;
}

int dm2_v1_projectile_active_count(void) {
    ensure_init();
    int n = 0;
    for (int i = 0; i < s_projectile_list.count
                && i < PROJECTILE_LIST_CAPACITY; i++) {
        if (s_projectile_list.entries[i].slotIndex >= 0) n++;
    }
    return n;
}

/* ── Phase 5 expansion: read-only slot snapshot ──────────────────
 * Returns a copy of the live slot's identity + world-position fields.
 * Used by the projectile-vs-creature collision module to read slot
 * data without coupling to the dispatch module's internals.
 *
 * Source: same F0810 payload contract; the snapshot is purely a
 * read-side helper, no behavioral change to dispatch. */
int dm2_v1_projectile_get_slot(int slot_index,
                                DM2_V1_ProjectileSlotSnapshot *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (slot_index < 0 || slot_index >= PROJECTILE_LIST_CAPACITY) return 0;
    ensure_init();
    const struct ProjectileInstance_Compat *p = &s_projectile_list.entries[slot_index];
    /* Use the F0810 occupied sentinel (reserved3 == 1) rather than
     * slotIndex, because F0813 only sets slotIndex = -1 (and the
     * list's initial memset-zero state has slotIndex = 0).  Without
     * this check, get_slot returns 1 for every entry in the cap. */
    if (p->reserved3 == 0) return 0;
    out->slotIndex          = p->slotIndex;
    out->projectileCategory = p->projectileCategory;
    out->projectileSubtype  = p->projectileSubtype;
    out->ownerKind          = p->ownerKind;
    out->ownerIndex         = p->ownerIndex;
    out->mapIndex           = p->mapIndex;
    out->mapX               = p->mapX;
    out->mapY               = p->mapY;
    out->cell               = p->cell;
    out->direction          = p->direction;
    out->attack             = p->attack;
    /* Phase 5+ extension: expose energy fields needed by the per-tick
     * missile step helper.  See header for source-lock anchors. */
    out->kineticEnergy      = p->kineticEnergy;
    out->stepEnergy         = p->stepEnergy;
    out->firstMoveGraceFlag = p->firstMoveGraceFlag;
    out->scheduledAtTick    = p->scheduledAtTick;
    return 1;
}

/* ── Phase 5+ extension: per-tick step energy consumption ────────
 *
 * Apply the STEP_MISSILE energy-decay + despawn boundary to one slot.
 * See header for full contract.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563 m_7CE0/m_7D2A
 *         (DM2_STEP_MISSILE: RG4L = kineticEnergy, RG1L = stepEnergy,
 *          if RG4L <= RG1L → despawn; else RG4L -= RG1L).
 *         ReDMCSB PROJEXPL.C:689-690 (F0219 first-move grace). */
int dm2_v1_projectile_consume_step_energy(int slot_index,
                                           int *out_post_energy,
                                           int *out_was_graced) {
    if (out_post_energy) *out_post_energy = 0;
    if (out_was_graced)  *out_was_graced = 0;
    if (slot_index < 0 || slot_index >= PROJECTILE_LIST_CAPACITY) return 0;
    ensure_init();
    struct ProjectileInstance_Compat *p = &s_projectile_list.entries[slot_index];
    if (p->reserved3 == 0 || p->slotIndex < 0) return 0;

    int ke = p->kineticEnergy;
    int se = p->stepEnergy;
    if (ke <= 0 || se <= 0) {
        /* No energy or no step-cost: treat as depleted.  Mirrors the
         * F0813 safety net for zero-energy entries that should have
         * been cleaned up but slipped through. */
        F0813_PROJECTILE_Despawn_Compat(&s_projectile_list, slot_index);
        return 0;
    }

    /* Boundary rule from skproject m_7CE0: kineticEnergy <= stepEnergy
     * → despawn (no partial step).  Uses <= not == so a slot that
     * arrives with energy == step consumes itself this tick. */
    if (ke <= se) {
        F0813_PROJECTILE_Despawn_Compat(&s_projectile_list, slot_index);
        return 0;
    }

    /* First-move grace (ReDMCSB PROJEXPL.C:689-690 F0219 C48):
     * honor the grace flag on the very first step, then clear it so
     * subsequent steps apply the real decrement. */
    if (p->firstMoveGraceFlag) {
        p->firstMoveGraceFlag = 0;
        if (out_post_energy) *out_post_energy = ke;  /* unchanged */
        if (out_was_graced)  *out_was_graced = 1;
        return 1;
    }

    /* Real decrement.  Clamp at 0 for safety. */
    int post = ke - se;
    if (post < 0) post = 0;
    p->kineticEnergy = post;
    if (out_post_energy) *out_post_energy = post;
    if (out_was_graced)  *out_was_graced = 0;
    return 1;
}

/* ── Phase 5 expansion: despawn helper ────────────────────────────
 * Routes through F0813_PROJECTILE_Despawn_Compat against the
 * module-owned s_projectile_list.  Returns 1 if the slot was freed,
 * 0 if the slot was already empty or invalid. */
int dm2_v1_projectile_despawn(int slot_index) {
    ensure_init();
    return F0813_PROJECTILE_Despawn_Compat(&s_projectile_list, slot_index);
}

/* ── Phase 5+ extension: test-only list accessor + slot-energy override ─
 *
 * Both helpers are gated on FIRESTAFF_DM2_PROJECTILE_TESTING=1 (the
 * CMake target that compiles the step test defines this).  Production
 * builds do not see these symbols; tests need them to drive the
 * step helper with non-default kineticEnergy/stepEnergy values
 * (the public dispatch API always sets ke=100, se=8 at F0810 time).
 *
 * Source: same F0810/F0813 contract as above.  These are pure test
 * plumbing — they do not affect runtime behavior. */
#ifdef FIRESTAFF_DM2_PROJECTILE_TESTING

void *dm2_v1_projectile_list_handle_for_test(void) {
    ensure_init();
    return (void *)&s_projectile_list;
}

int dm2_v1_projectile_test_set_slot_energy(int slot_index,
                                             int kinetic_energy,
                                             int step_energy,
                                             int first_grace) {
    if (slot_index < 0 || slot_index >= PROJECTILE_LIST_CAPACITY) return 0;
    ensure_init();
    struct ProjectileInstance_Compat *p = &s_projectile_list.entries[slot_index];
    if (p->reserved3 == 0 || p->slotIndex < 0) return 0;
    p->kineticEnergy      = kinetic_energy;
    p->stepEnergy         = step_energy;
    p->firstMoveGraceFlag = first_grace ? 1 : 0;
    return 1;
}

/* Test-only full-list reset.  Clears all entries (slots become
 * empty via reserved3 = 0) and resets list count to 0.  Production
 * builds do not see this symbol. */
void dm2_v1_projectile_test_reset_list(void) {
    ensure_init();
    memset(&s_projectile_list, 0, sizeof(s_projectile_list));
    s_initialized = 1;
    s_dispatch_count = 0;
    s_spell_count = 0;
    s_bomb_count = 0;
}

#endif /* FIRESTAFF_DM2_PROJECTILE_TESTING */

const char *dm2_v1_projectile_source_evidence(void) {
    return
        "DM2 V1 Projectile Routing — Phase 5 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)\n"
        "Source: SKULL.ASM:11100-11200  (projectile routing, SKWin source-locked region)\n"
        "Source: ReDMCSB PROJEXPL.C:76-92       (F0212: projectile live, first move +1 tick)\n"
        "Source: ReDMCSB PROJEXPL.C:689-690     (F0219: C48 ignore-impacts-first-movement)\n"
        "Source: ReDMCSB GROUP.C:2376-2387      (F0209: visible row/column triggers F0207 attack)\n"
        "Source: ReDMCSB GROUP.C:1695-1770      (F0207: creature attack projectile payload)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:10479-10561  (AI_W30_TURNS_MISSILE check)\n"
        "Source: skproject/SKULLWIN/c_creature.cpp           (DM2_PROCEED_CCM, CCM 0x0d/0x15)\n"
        "Source: memory_projectile_pc34_compat.h             (F0810-F0820 contract)\n"
        "DM2 difference: 12 AI_ATTACK_FLAGS__* bits vs DM1's 8\n"
        "DM2 new: bombs (CCM 0x0d SHOOT_ITEM with AI_ATTACK_FLAGS__PUSH_BACK + area-effect)\n"
        "DM2 reuse: F0810_PROJECTILE_Create_Compat from DM1's PROJECT.C / PROJEXPL.C engine\n"
        "V1 invariant: V1 projectile list state is preserved; DM2 projectiles are in their own list\n"
        "Phase 5 read-side: dm2_v1_projectile_get_slot() exposes a non-aliasing snapshot\n"
        "  for the collision resolver (src/dm2/dm2_v1_projectile_creature_collision_pc34_compat.c)\n";
}
