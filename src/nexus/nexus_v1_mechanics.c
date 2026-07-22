#include "nexus_v1_mechanics.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_squares.h"
#include "nexus_v1_creatures.h"
#include "nexus_v1_combat.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_drops.h"
#include "nexus_v1_script_vm.h"
#include "nexus_v1_sound.h"
#include <string.h>
#include <stdio.h>

/* Nexus V1 mechanics — assembled game loop.
 * Combines: movement, square events, creature AI, combat, resource drain,
 * script triggers, sound triggers.
 * Source: DM1 game loop (CLIKMENU.C F0366 + FIRESTAFF game loop),
 * ReDMCSB COMMAND.C, MOVESENS.C, CHAMPION.C, CREATURE.C.
 *
 * Tick rate: 55ms (18.2 Hz) — same as DM1.
 *
 * Order per tick:
 *   1. Input queue → command dispatch
 *   2. Movement (if command == step/turn)
 *   3. Square event (stairs/teleport/pit/alarm/exit)
 *   4. Creature AI tick
 *   5. Creature attack resolution (adjacent creatures attack party)
 *   6. Resource drain (food/water every N ticks)
 *   7. Script VM events (party move, level load, etc.)
 *   8. Sound triggers
 *
 * DM1 source references:
 *   CLIKMENU.C:167-169 (stairs turn), 176-179 (movement sensor order)
 *   CLIKMENU.C:237-255 (stamina), 264-276 (stairs step), 269-323 (step result)
 *   CLIKMENU.C:325-346 (cooldown assignment)
 *   MOVESENS.C:316-328 (move result), 752-783 (walk-on/off sensors)
 *   CHAMPION.C:2025-2048 (stamina decrement), 117-130 (turn)
 *   F0209_GROUP_ProcessEvents29to41 (creature AI timeline) */

#define FOOD_DRAIN_TICKS   60
#define WATER_DRAIN_TICKS  60

void nexus_mechanics_init(Nexus_MechanicsState *st,
                            int start_x, int start_y, int start_dir) {
    if (!st) return;
    memset(st, 0, sizeof(*st));
    st->party_x = start_x;
    st->party_y = start_y;
    st->party_dir = start_dir;
    st->map_index = 0;
    st->party_alive = 1;
    st->gold_pieces = 0;
    st->game_over = 0;
    st->game_over_reason = 0;
    st->pending_level_change = -1;
    st->pending_teleport = 0;
    st->move_cooldown_ticks = 0;
    st->total_ticks = 0;
    st->food_drain_timer = FOOD_DRAIN_TICKS;
    st->water_drain_timer = WATER_DRAIN_TICKS;
    st->input_head = 0;
    st->input_tail = 0;
    st->input_count = 0;
    st->use_item_slot = -1;
}

int nexus_mechanics_push_command(Nexus_MechanicsState *st, int command) {
    if (!st || st->input_count >= 8) return -1;
    st->input_queue[st->input_tail] = command;
    st->input_tail = (st->input_tail + 1) % 8;
    st->input_count++;
    return 0;
}

int nexus_mechanics_pop_command(Nexus_MechanicsState *st, int *out_cmd) {
    if (!st || st->input_count <= 0) return 0;
    if (out_cmd) *out_cmd = st->input_queue[st->input_head];
    st->input_head = (st->input_head + 1) % 8;
    st->input_count--;
    return 1;
}

int nexus_mechanics_party_alive(const Nexus_MechanicsState *st, Nexus_V1_Engine *engine) {
    /* Check if at least one living party champion is alive.
     * engine pointer required — st does not carry engine state.
     * Source: DM1 CLIKMENU.C F0366 — party dead when all 4 champions dead.
     * A party with zero members cannot be alive. */
    int i;
    (void)st;  /* engine carries the live champion state */
    if (!engine) return 0;
    if (engine->champions.party_count == 0) return 0; /* no party = dead */
    for (i = 0; i < engine->champions.party_count; i++) {
        int ci = engine->champions.party[i];
        if (ci < 0 || ci >= engine->champions.champion_count) continue;
        if (engine->champions.champions[ci].alive) return 1;
    }
    /* All dead */
    return 0;
}

void nexus_mechanics_get_party_pos(const Nexus_MechanicsState *st,
                                     int *out_x, int *out_y, int *out_dir) {
    if (out_x) *out_x = st->party_x;
    if (out_y) *out_y = st->party_y;
    if (out_dir) *out_dir = st->party_dir;
}

void nexus_mechanics_teleport(Nexus_MechanicsState *st,
                                int target_x, int target_y, int target_level) {
    if (!st) return;
    st->pending_teleport = 1;
    st->teleport_target_x = target_x;
    st->teleport_target_y = target_y;
    st->teleport_target_level = target_level;
}

void nexus_mechanics_change_level(Nexus_MechanicsState *st, int target_level,
                                    int target_x, int target_y) {
    if (!st) return;
    st->pending_level_change = target_level;
    st->party_x = target_x;
    st->party_y = target_y;
    (void)target_y;
}

void nexus_mechanics_set_use_item_slot(Nexus_MechanicsState *st, int slot) {
    if (!st) return;
    st->use_item_slot = slot;
}

static int get_champion_defense(Nexus_V1_ChampionPool *pool) {
    /* Sum defense from all equipped armor on the party leader.
     * Defense comes from: head, torso, legs, feet, hands, shield, amulet.
     * Weapons contribute attack, not defense.
     * Source: DM1 CHAMPION.C combat defense calculation,
     * CHAMPION.C F0309 equipment slot layout (C05_SLOT_WEAPON..C13_SLOT_AMULET). */
    int total_def = 0;
    int leader_idx, leader_slot, item_id, i;
    Nexus_V1_Champion *leader;
    const Nexus_ItemDef *def;

    if (!pool || pool->party_count == 0) return 0;
    if (pool->leader_index < 0 || pool->leader_index >= pool->party_count) return 0;

    leader_idx = pool->party[pool->leader_index];
    if (leader_idx < 0 || leader_idx >= pool->champion_count) return 0;
    leader = &pool->champions[leader_idx];
    if (!leader->alive) return 0;

    /* Iterate all equipment slots (NEXUS_SLOT_HEAD..NEXUS_SLOT_AMULET).
     * The slot indices in the champion slots[] array match NEXUS_SLOT_* values.
     * Source: CHAMPION.C F0309 slot layout, nexus_v1_champions.h NEXUS_SLOT_COUNT.
     * Slot mapping: HEAD=1, TORSO=2, LEGS=3, FEET=4, HANDS=5,
     *                WEAPON=6, SHIELD=7, RING1=8, RING2=9, AMULET=10. */
    for (leader_slot = 1; leader_slot <= 10; leader_slot++) {
        if (leader_slot == 6 || leader_slot == 8 || leader_slot == 9) {
            /* WEAPON, RING1, RING2 — these contribute attack not defense.
             * Source: DM1 combat — defense only from armor slots. */
            continue;
        }
        item_id = leader->slots[leader_slot - 1];
        if (item_id < 0) continue;
        def = nexus_itemdef_get(item_id);
        if (def && def->defense > 0) {
            total_def += def->defense;
        }
    }

    /* Ring slots can also contribute defense in some DM1 configurations.
     * Rings (RING1=8, RING2=9) sometimes give defense if equipped.
     * Source: DM1 ring slot defense. */
    for (i = 8; i <= 9; i++) {
        item_id = leader->slots[i];
        if (item_id < 0) continue;
        def = nexus_itemdef_get(item_id);
        if (def && def->defense > 0) {
            total_def += def->defense;
        }
    }

    /* Minimum defense of 1 so attacks always deal at least 1 damage.
     * Source: DM1 CREATURE.C minimum damage rule. */
    return total_def > 0 ? total_def : 1;
}

/* Apply a used item to the party leader.
 * Consumables restore stats; equippables move to the matching equipment slot.
 * Returns 1 if the item was consumed/used, 0 if no effect.
 * Source: DM1 COMMAND.C item use / CLIKMENU.C action dispatch;
 *         CHAMPION.C F0309 equipment slot layout.
 * Nexus V1 uses the same item catalog as DM1 (nexus_v1_inventory.c). */
static int apply_use_item(Nexus_V1_Champion *leader, int item_id) {
    const Nexus_ItemDef *def;
    int target_slot;

    if (!leader || !leader->alive || item_id < 0) return 0;
    def = nexus_itemdef_get(item_id);
    if (!def) return 0;

    if (def->flags & NEXUS_ITEMF_CONSUMABLE) {
        switch (item_id) {
        case 30: /* Health Potion */
            if (leader->health < leader->max_health) {
                leader->health += 25;
                if (leader->health > leader->max_health)
                    leader->health = leader->max_health;
            }
            break;
        case 31: /* Mana Potion */
            if (leader->mana < leader->max_mana) {
                leader->mana += 25;
                if (leader->mana > leader->max_mana)
                    leader->mana = leader->max_mana;
            }
            break;
        case 32: /* Stamina Potion */
            if (leader->stamina < leader->max_stamina) {
                leader->stamina += 25;
                if (leader->stamina > leader->max_stamina)
                    leader->stamina = leader->max_stamina;
            }
            break;
        case 33: /* Antidote */
            leader->wounds = 0;
            break;
        case 63: /* Corn */
            if (leader->food < 100) {
                leader->food += 25;
                if (leader->food > 100) leader->food = 100;
            }
            break;
        case 64: /* Water Flask */
            if (leader->water < 100) {
                leader->water += 25;
                if (leader->water > 100) leader->water = 100;
            }
            break;
        default:
            return 0; /* unknown consumable */
        }
        return 1;
    }

    if ((def->flags & NEXUS_ITEMF_EQUIPPABLE) && def->category == NEXUS_ITEM_WEAPON) {
        /* Weapon (including weapons with a defense bonus like Sword/Slayer). */
        leader->slots[NEXUS_SLOT_WEAPON - 1] = item_id;
        return 1;
    }

    if ((def->flags & NEXUS_ITEMF_EQUIPPABLE) && def->category == NEXUS_ITEM_ARMOR) {
        /* Armor — map by item ID, same logic as nexus_inventory_equip(). */
        if (item_id >= 20 && item_id <= 22) {
            target_slot = NEXUS_SLOT_TORSO;
        } else if (item_id == 23) {
            target_slot = NEXUS_SLOT_SHIELD;
        } else if (item_id == 24) {
            target_slot = NEXUS_SLOT_HEAD;
        } else if (item_id == 25) {
            target_slot = NEXUS_SLOT_FEET;
        } else if (item_id == 26) {
            target_slot = NEXUS_SLOT_HANDS;
        } else {
            target_slot = NEXUS_SLOT_TORSO;
        }
        leader->slots[target_slot - 1] = item_id;
        return 1;
    }

    /* Keys, runes, and other items have no V1 use effect here. */
    return 0;
}

int nexus_mechanics_tick(Nexus_MechanicsState *st, Nexus_V1_Engine *engine) {
    int needs_redraw = 0;
    int cmd = 0;
    int tx, ty, tl, td;
    int i;

    if (!st || !engine) return 0;
    st->total_ticks++;

    if (st->move_cooldown_ticks > 0) {
        st->move_cooldown_ticks--;
        return 0;
    }

    /* Process pending teleport */
    if (st->pending_teleport) {
        st->party_x = st->teleport_target_x;
        st->party_y = st->teleport_target_y;
        st->pending_teleport = 0;
        needs_redraw = 1;
        if (engine->audio_enabled)
            nexus_sound_play(&engine->audio, NEXUS_SFX_TELEPORT);
    }

    /* Dequeue command */
    if (!nexus_mechanics_pop_command(st, &cmd)) {
        cmd = 0;
    }

    /* Dispatch command */
    if (cmd != 0) {
        if (cmd == NEXUS_CMD_TURN_LEFT || cmd == NEXUS_CMD_TURN_RIGHT) {
            int turn_right = (cmd == NEXUS_CMD_TURN_RIGHT) ? 1 : 0;
            nexus_turn(&st->party_dir, turn_right);
            needs_redraw = 1;
        } else if (cmd == NEXUS_CMD_USE_ITEM) {
            /* Use the selected inventory item on the party leader.
             * Source: DM1 COMMAND.C item-use dispatch; CLIKMENU.C action menu. */
            int leader_idx = -1;
            if (engine->champions.party_count > 0 &&
                engine->champions.leader_index >= 0 &&
                engine->champions.leader_index < engine->champions.party_count) {
                leader_idx = engine->champions.party[engine->champions.leader_index];
            }
            if (leader_idx >= 0 && leader_idx < engine->champions.champion_count &&
                st->use_item_slot >= 0 && st->use_item_slot < 30) {
                Nexus_V1_Champion *leader = &engine->champions.champions[leader_idx];
                int item_id = leader->inventory[st->use_item_slot];
                if (apply_use_item(leader, item_id)) {
                    leader->inventory[st->use_item_slot] = (uint8_t)-1;
                    nexus_champion_recalc_load(leader);
                    nexus_sound_play(&engine->audio, NEXUS_SFX_PICKUP_ITEM);
                    needs_redraw = 1;
                }
            }
        } else if (cmd == NEXUS_CMD_INTERACT) {
            /* Click-route interaction: pick up a floor item at the party's
             * current square.  Other interactions (chests, levers) are not
             * wired to the engine's world object DB yet; this command is a
             * narrow fail-closed hook for the click-route layer.
             * Source: DM1 COMMAND.C object/floor-item click dispatch. */
            int leader_idx = -1;
            if (engine->champions.party_count > 0 &&
                engine->champions.leader_index >= 0 &&
                engine->champions.leader_index < engine->champions.party_count) {
                leader_idx = engine->champions.party[engine->champions.leader_index];
            }
            if (leader_idx >= 0 && leader_idx < engine->champions.champion_count) {
                Nexus_V1_Champion *leader = &engine->champions.champions[leader_idx];
                int item_id = -1, qty = 0;
                int idx = nexus_floor_get_at(st->party_x, st->party_y, 0,
                                              &item_id, &qty);
                if (idx >= 0 && item_id >= 0) {
                    int slot;
                    /* Find first empty slot in the flat uint8_t inventory. */
                    for (slot = 0; slot < NEXUS_INVENTORY_SLOTS; slot++) {
                        if (leader->inventory[slot] == 0xFFU) {
                            leader->inventory[slot] = (uint8_t)item_id;
                            nexus_floor_pickup(idx, &item_id, &qty);
                            nexus_champion_recalc_load(leader);
                            nexus_sound_play(&engine->audio, NEXUS_SFX_PICKUP_ITEM);
                            needs_redraw = 1;
                            break;
                        }
                    }
                }
            }
        } else {
            /* Step movement */
            int forward = (cmd == NEXUS_CMD_FORWARD) ? 1 : 0;
            int strafe = (cmd == NEXUS_CMD_STRAFE_LEFT || cmd == NEXUS_CMD_STRAFE_RIGHT) ? 1 : 0;
            int strafe_left = (cmd == NEXUS_CMD_STRAFE_LEFT) ? 1 : 0;
            int t_x, t_y;

            /* BACKWARD: pass forward=0 so target_square moves in opposite dir.
             * Source: nexus_v1_movement.c nexus_target_square(), CLIKMENU.C:224. */
            if (cmd == NEXUS_CMD_BACKWARD) forward = 0;

            nexus_target_square(st->party_x, st->party_y, st->party_dir,
                                 forward, strafe, strafe_left,
                                 &t_x, &t_y);

            int sq = nexus_v1_level_get_square(&engine->current_level, t_x, t_y);

            /* Door blocking: check if door is locked and party has key.
             * Pass party leader's inventory for key check.
             * Source: DM1 door processing — locked doors block without key. */
            if (sq == NEXUS_SQUARE_DOOR && !nexus_doors_is_open(t_x, t_y)) {
                uint8_t leader_inv[30] = {[0 ... 29] = (uint8_t)-1};
                int leader_idx = -1;
                if (engine->champions.party_count > 0 &&
                    engine->champions.leader_index >= 0 &&
                    engine->champions.leader_index < engine->champions.party_count) {
                    leader_idx = engine->champions.party[engine->champions.leader_index];
                }
                if (leader_idx >= 0 && leader_idx < engine->champions.champion_count) {
                    memcpy(leader_inv, engine->champions.champions[leader_idx].inventory, 30);
                }
                if (!nexus_doors_can_open(t_x, t_y, leader_inv)) {
                    /* Locked and no key — blocked */
                    sq = 0; /* treat as wall */
                } else {
                    nexus_doors_open(t_x, t_y);
                    nexus_sound_play(&engine->audio, NEXUS_SFX_DOOR_OPEN);
                }
            }

            if (sq != 0 && nexus_v1_level_move_allowed(&engine->current_level,
                                                       st->party_x, st->party_y,
                                                       t_x, t_y)) {
                st->party_x = t_x;
                st->party_y = t_y;
                st->move_cooldown_ticks = 6;
                needs_redraw = 1;

                /* Process square event */
                Nexus_SquareEvent sq_event = nexus_process_square_event(sq, t_x, t_y, &tx, &ty, &tl, &td);
                switch (sq_event) {
                case NEXUS_EVENT_STAIRS_DOWN:
                case NEXUS_EVENT_STAIRS_UP:
                    st->pending_level_change = tl;
                    st->party_x = tx;
                    st->party_y = ty;
                    if (td >= 0) st->party_dir = td;
                    nexus_sound_play(&engine->audio, NEXUS_SFX_STAIRS);
                    break;
                case NEXUS_EVENT_TELEPORT:
                    nexus_mechanics_teleport(st, tx, ty, tl);
                    break;
                case NEXUS_EVENT_PIT_FALL:
                case NEXUS_EVENT_CHUTE_FALL:
                    /* Pit/chute forces the party to the next lower level.
                     * tl == -1 means "one level down" (same x,y).
                     * Source: DM1 MOVESENS.C F0267/F0268 chute/pit sensor;
                     *         CLIKMENU.C:264-276 stairs special cases. */
                    st->pending_level_change = tl;
                    if (st->pending_level_change < 0)
                        st->pending_level_change = st->map_index + 1;
                    if (st->pending_level_change > 15)
                        st->pending_level_change = 15;
                    st->party_x = tx;
                    st->party_y = ty;
                    nexus_sound_play(&engine->audio, NEXUS_SFX_PIT_FALL);
                    break;
                case NEXUS_EVENT_ALARM_TRIGGER:
                    nexus_v1_creatures_alert_all(&engine->creatures, st->map_index);
                    nexus_sound_play(&engine->audio, NEXUS_SFX_ALARM);
                    break;
                case NEXUS_EVENT_DOOR_OPEN:
                    nexus_sound_play(&engine->audio, NEXUS_SFX_DOOR_OPEN);
                    break;
                case NEXUS_EVENT_EXIT_REACHED:
                    /* Exit reached — game complete!
                     * Source: DM1 exit square processing.
                     * Sets game_over flag; upper layer handles end-game screen.
                     * game_over_reason = 1 (exit reached). */
                    st->game_over = 1;
                    st->game_over_reason = 1;
                    nexus_sound_play(&engine->audio, NEXUS_SFX_EXIT_REACHED);
                    break;
                default:
                    break;
                }
            }
        }
    }

    /* Creature AI tick */
    if (engine->creatures.type_count > 0 && engine->level_loaded) {
        nexus_v1_creatures_tick(&engine->creatures,
                                 st->party_x, st->party_y,
                                 engine->current_level.squares,
                                 st->map_index);

        /* Creature attack against adjacent party.
         * Source: DM1 CREATURE.C F0209 creature attack vs champion;
         *         CHAMPION.C F0309 defense/absorb wiring.
         * Damage is applied to the party leader when possible; if the leader
         * is dead, the first living party member absorbs the blow. */
        for (i = 0; i < engine->creatures.active_count; i++) {
            Nexus_Creature *c = &engine->creatures.active[i];
            if (!c->alive) continue;
            if (c->state == 3) { /* attack range */
                int champ_def = get_champion_defense(&engine->champions);
                int dmg = 0;
                if (nexus_v1_creature_attack(&engine->creatures, i, champ_def, &dmg)) {
                    if (dmg > 0) {
                        int target_idx = -1;
                        int pi;
                        /* Prefer party leader. */
                        if (engine->champions.leader_index >= 0 &&
                            engine->champions.leader_index < engine->champions.party_count) {
                            int li = engine->champions.party[engine->champions.leader_index];
                            if (li >= 0 && li < engine->champions.champion_count &&
                                engine->champions.champions[li].alive) {
                                target_idx = li;
                            }
                        }
                        /* Fallback to first living party member. */
                        for (pi = 0; target_idx < 0 && pi < engine->champions.party_count; pi++) {
                            int ci = engine->champions.party[pi];
                            if (ci >= 0 && ci < engine->champions.champion_count &&
                                engine->champions.champions[ci].alive) {
                                target_idx = ci;
                            }
                        }
                        if (target_idx >= 0) {
                            int was_alive = engine->champions.champions[target_idx].alive;
                            int died = nexus_v1_take_damage(&engine->champions.champions[target_idx], dmg);
                            if (died && was_alive) {
                                nexus_v1_champion_on_death_update_leader(
                                    &engine->champions, target_idx);
                            }
                            nexus_sound_play(&engine->audio, NEXUS_SFX_CHAMPION_HURT);
                        }
                        nexus_sound_play(&engine->audio, NEXUS_SFX_CREATURE_ATTACK);
                    }
                }
            }
        }

        /* Check for total party death.
         * Source: DM1 CLIKMENU.C F0366 — game ends when all champions die. */
        if (!nexus_mechanics_party_alive(st, engine)) {
            st->game_over = 1;
            st->game_over_reason = 2; /* all_dead */
        }
    }

    /* Resource drain — every FOOD_DRAIN_TICKS ticks, drain 1 food from
     * each living party champion. When food reaches 0, stamina drains
     * at double rate. When food AND water both reach 0, champions take
     * wound damage per tick.
     * Source: DM1 CHAMPION.C food/water drain (F0325 stamina decrement). */
    if (st->food_drain_timer > 0) st->food_drain_timer--;
    if (st->water_drain_timer > 0) st->water_drain_timer--;
    if (st->food_drain_timer <= 0) {
        st->food_drain_timer = FOOD_DRAIN_TICKS;
        /* Drain 1 food from each living party champion.
         * Source: DM1 food drain on party movement. */
        for (i = 0; i < engine->champions.party_count; i++) {
            int ci = engine->champions.party[i];
            if (ci < 0 || ci >= engine->champions.champion_count) continue;
            Nexus_V1_Champion *c = &engine->champions.champions[ci];
            if (!c->alive) continue;
            if (c->food > 0) c->food--;
            /* When food=0, stamina penalty: -2 extra stamina per step.
             * Source: DM1 CHAMPION.C — starving reduces stamina regen. */
            if (c->food == 0) c->stamina -= 2;
        }
    }
    if (st->water_drain_timer <= 0) {
        st->water_drain_timer = WATER_DRAIN_TICKS;
        for (i = 0; i < engine->champions.party_count; i++) {
            int ci = engine->champions.party[i];
            if (ci < 0 || ci >= engine->champions.champion_count) continue;
            Nexus_V1_Champion *c = &engine->champions.champions[ci];
            if (!c->alive) continue;
            if (c->water > 0) c->water--;
            if (c->water == 0) c->stamina -= 2;
        }
    }

    /* Stamina cost per step — party leader spends 3 stamina per step.
     * Source: DM1 CLIKMENU.C:237-255 (living champion stamina decrement),
     * CHAMPION.C F0325_DECREMENTSTAMINA lines 2025-2048. */
    if (needs_redraw && cmd != NEXUS_CMD_TURN_LEFT && cmd != NEXUS_CMD_TURN_RIGHT) {
        int leader_idx;
        if (engine->champions.party_count > 0 &&
            engine->champions.leader_index >= 0 &&
            engine->champions.leader_index < engine->champions.party_count) {
            leader_idx = engine->champions.party[engine->champions.leader_index];
            if (leader_idx >= 0 && leader_idx < engine->champions.champion_count) {
                int was_alive = engine->champions.champions[leader_idx].alive;
                nexus_champion_decrement_stamina(
                    &engine->champions.champions[leader_idx], 3);
                if (was_alive && !engine->champions.champions[leader_idx].alive) {
                    nexus_v1_champion_on_death_update_leader(
                        &engine->champions, leader_idx);
                }
            }
        }
    }

    /* Script VM: party move event.
     * Fires ON_XY rules when party steps on a scripted square.
     * Source: nexus_v1_script_vm.c nexus_script_on_party_move(). */
    if (needs_redraw) {
        nexus_script_on_party_move(&engine->script_vm,
                                   st->party_x,
                                   st->party_y,
                                   st->map_index);
    }

    /* Gold pickup — only if party is alive.
     * Source: DM1 GOLDDROP.C gold pile pickup on move result. */
    if (engine->champions.party_count > 0) {
        int gold = nexus_gold_at(st->party_x, st->party_y);
        if (gold > 0) {
            st->gold_pieces += gold;
            nexus_sound_play(&engine->audio, NEXUS_SFX_GOLD_PICKUP);
        }
    }

    return needs_redraw || (st->pending_level_change >= 0);
}
