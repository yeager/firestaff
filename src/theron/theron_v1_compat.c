/*
 * theron_v1_compat.c — Theron V1 lib compatibility shims
 *
 * Source: THQUEST.ASM (Theron's Quest PC Engine CD ROM, sha256 ...)
 *         ReDMCSB GROUP.C / COMMAND.C / CLIKMENU.C / GAMELOOP.C analogues
 *
 * This file provides definition symbols for the Theron V1 combat +
 * creature API declared in include/theron_v1_combat.h.  These functions
 * are referenced by src/theron/theron_v1_mechanics.c but the actual
 * combat implementation is not yet ported into Firestaff.
 *
 * Until a real implementation lands, these compat shims return safe
 * defaults (0 / NULL / no-op) so any consumer of mechanics.c links
 * cleanly and tests/probes can run.  When the real implementation is
 * added, this file becomes the home of that implementation.
 *
 * V1 invariant: shims MUST preserve the V1 game state — never mutate
 * world->party / world->creatures / world->dungeon without going
 * through the proper API.  The shims below are read-only or no-op.
 *
 * Pass C note (2026-06-17): this file unblocks the Theron V1 lib link
 * (was previously failing on undefined references to champion_attack,
 * champion_die, creature_ai_tick, creature_at, and ~14 others).
 */

#include "theron_v1_combat.h"
#include <stddef.h>

/* ── Creature lifecycle ─────────────────────────────────────────── */

int theron_v1_creature_spawn(Theron_V1_World *world,
    Theron_CreatureType type, int dungeon_id, int level,
    int x, int y)
{
    (void)world; (void)type; (void)dungeon_id; (void)level;
    (void)x; (void)y;
    return -1;  /* not implemented */
}

int theron_v1_creature_kill(Theron_V1_World *world, int creature_id) {
    (void)world; (void)creature_id;
    return 0;
}

int theron_v1_creature_remove(Theron_V1_World *world, int creature_id) {
    (void)world; (void)creature_id;
    return 0;
}

Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *world,
                                           int level, int x, int y)
{
    (void)world; (void)level; (void)x; (void)y;
    return NULL;
}

Theron_V1_Creature *theron_v1_creature_by_id(Theron_V1_World *world, int id) {
    (void)world; (void)id;
    return NULL;
}

int theron_v1_creature_count(const Theron_V1_World *world,
                             int dungeon_id, int level)
{
    (void)world; (void)dungeon_id; (void)level;
    return 0;
}

/* ── Champion attack ────────────────────────────────────────────── */

int theron_v1_champion_attack(Theron_V1_World *world,
    int attacking_slot, int target_creature_id)
{
    (void)world; (void)attacking_slot; (void)target_creature_id;
    return 0;  /* no damage applied (compat shim) */
}

/* ── Creature attack (single creature vs champion) ──────────────── */

Theron_CombatResult theron_v1_creature_attack_champion(
    Theron_V1_World *world, int creature_id, int champion_slot)
{
    (void)world; (void)creature_id; (void)champion_slot;
    return THERON_COMBAT_MISS;  /* compat shim: miss every attack */
}

/* ── Creature AI tick ───────────────────────────────────────────── */

void theron_v1_creature_ai_tick(Theron_V1_World *world) {
    (void)world;
    /* compat shim: real creature AI not yet ported */
}

/* ── Damage calculation helpers ────────────────────────────────── */

int theron_v1_calc_attack_damage(int attack_power,
    const Theron_V1_Champion *defender, Theron_AttackType type)
{
    (void)attack_power; (void)defender; (void)type;
    return 0;
}

int theron_v1_calc_defense(const Theron_V1_Champion *defender,
    Theron_AttackType type)
{
    (void)defender; (void)type;
    return 0;
}

/* ── HP / stamina / mana modification (clamped to valid ranges) ── */

int theron_v1_modify_champion_hp(Theron_V1_Champion *c, int delta) {
    if (!c) return 0;
    int new_hp = c->health + delta;
    if (new_hp < 0) new_hp = 0;
    if (new_hp > c->max_health) new_hp = c->max_health;
    c->health = (int16_t)new_hp;
    return c->health;
}

int theron_v1_modify_champion_stamina(Theron_V1_Champion *c, int delta) {
    if (!c) return 0;
    int new_stamina = c->stamina + delta;
    if (new_stamina < 0) new_stamina = 0;
    if (new_stamina > c->max_stamina) new_stamina = c->max_stamina;
    c->stamina = (int16_t)new_stamina;
    return c->stamina;
}

int theron_v1_modify_champion_mana(Theron_V1_Champion *c, int delta) {
    if (!c) return 0;
    int new_mana = c->mana + delta;
    if (new_mana < 0) new_mana = 0;
    if (new_mana > c->max_mana) new_mana = c->max_mana;
    c->mana = (int16_t)new_mana;
    return c->mana;
}

/* ── Death processing ──────────────────────────────────────────── */

void theron_v1_champion_die(Theron_V1_World *world, int champ_slot) {
    if (!world) return;
    if (champ_slot < 0 || champ_slot >= 4) return;
    Theron_V1_Champion *c = theron_v1_party_getChampion(&world->party, champ_slot);
    if (!c) return;
    c->alive = 0;
    c->health = 0;
}

void theron_v1_creature_die(Theron_V1_World *world, int creature_id) {
    if (!world) return;
    /* compat shim: real creature death not yet ported */
    (void)creature_id;
}

/* ── Drops ──────────────────────────────────────────────────────── */

int theron_v1_drop_loot(Theron_V1_World *world,
    int creature_id, int x, int y)
{
    (void)world; (void)creature_id; (void)x; (void)y;
    return 0;  /* no drops (compat shim) */
}

/* ── Sound ──────────────────────────────────────────────────────── */

int theron_v1_play_sound(Theron_SoundID id) {
    (void)id;
    return 0;
}

int theron_v1_sound_is_valid(Theron_SoundID id) {
    (void)id;
    return 0;
}

const char *theron_v1_combat_source_evidence(void) {
    return
        "Theron V1 Combat Compat — Pass C source-lock\n"
        "Source: THQUEST.ASM T500/T600/T700/T800/T900 (creature + champion combat)\n"
        "Source: ReDMCSB GROUP.C / COMMAND.C / CLIKMENU.C / GAMELOOP.C analogues\n"
        "Source: CSBWin/Resurrect Theron's Quest reimpl\n"
        "Status: compat shims (read-only / no-op) until real combat is ported\n"
        "V1 invariant: shims MUST preserve V1 game state — read-only or no-op\n";
}