/*
 * Real PC-DOS SKSave1 Resume -> WIELD trace gate.
 *
 * This corpus does not contain an original input-to-CD/RAM combat trace.
 * The bounded diagnostic probe may establish that a real calculation was
 * reached, but it must never promote its generated command search into a
 * retail creature-drop claim.  The expected result for the supplied media is
 * therefore the authenticated miss and an untouched drop path.
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exercise_wield_trace_gate(DM2_V1_BootProfile *profile)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    DM2_V1_DungeonData *dungeon =
        profile ? (DM2_V1_DungeonData *)profile->dungeon_data : NULL;
    unsigned char framebuffer[320 * 200];
    DM2_V1_RuntimeWieldAttackReceipt last_wield;
    int saw_wield_calculation = 0;

    memset(&last_wield, 0, sizeof(last_wield));

    if (!dungeon || !dungeon->record_graph_complete) return 0;
    {
        DM2_V1_BootRuntimeInventoryReceipt swap;
        memset(&swap, 0, sizeof(swap));
        if (!dm2_v1_boot_runtime_swap_inventory_slot(
                profile, 2, 7, &swap) || !swap.status) {
            puts("Resume inventory swap did not commit");
            return 0;
        }
        {
            DM2_V1_RuntimeSourceHeroStateReceipt state;
            memset(&state, 0, sizeof(state));
            if (!dm2_v1_runtime_get_source_hero_state(0, &state) ||
                (uint16_t)state.first_item != 0x1407u ||
                dm2_v1_runtime_get_leader_hand_object() != 0x1407u) {
                puts("Resume inventory swap did not update active source hand");
                return 0;
            }
        }
    }
    for (int map = 0; map < dungeon->level_count; ++map) {
        DM2_V1_G1CreatureMapChipRuntimeReceipt materials;
        int base_x = dungeon->level_widths[map] > 1 ? 1 : 0;
        int base_y = dungeon->level_heights[map] > 1 ? 1 : 0;
        dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
        dm2_v1_runtime_set_position(map, base_x, base_y, 0);
        memset(&materials, 0, sizeof(materials));
        if (!dm2_v1_runtime_g1_creature_map_chip_receipt(&materials) ||
            !materials.valid) continue;
        for (int i = 0; i < materials.material_count; ++i) {
            const DM2_V1_G1CreatureMapChipMaterial *material =
                &materials.materials[i];
            int has_drop = 0;
            if (!dm2_v1_creature_drop_slots_loaded(material->creature_type))
                continue;
            for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot)
                has_drop |= dm2_v1_creature_drop_slot_word(
                    material->creature_type, slot) != 0;
            if (!has_drop) continue;
            for (int dir = 0; dir < 4; ++dir) {
                int px = material->x - dx[dir];
                int py = material->y - dy[dir];
                DM2_V1_EngageCommandReceipt attack;
                int hits = 0;
                if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                    py >= dungeon->level_heights[map] ||
                    dm2_v1_dungeon_get_square_type(dungeon, map, px, py) < 0)
                    continue;
                dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
                dm2_v1_runtime_set_position(map, px, py, dir);
                memset(framebuffer, 0, sizeof(framebuffer));
                for (int hero = 0; hero < 4; ++hero) {
                    for (int cycle = 0;
                         cycle < 4 &&
                         dm2_v1_runtime_get_active_champion_index() != hero;
                         ++cycle)
                        if (!dm2_v1_runtime_cycle_action_champion()) break;
                    if (dm2_v1_runtime_get_active_champion_index() != hero)
                        continue;
                    for (int hand = 0; hand < 2; ++hand) {
                        if (!dm2_v1_runtime_activate_action_hand(hero, hand))
                            continue;
                        for (int command = 0; command < 3;
                             ++command) {
                            for (int attempt = 0; attempt < 256; ++attempt) {
                                memset(&attack, 0, sizeof(attack));
                                if (!dm2_v1_runtime_proceed_hand_command(
                                    command, &attack)) {
                                    DM2_V1_RuntimeWieldAttackReceipt detail;
                                    if (dm2_v1_runtime_last_wield_attack_receipt(
                                            &detail)) {
                                        last_wield = detail;
                                        saw_wield_calculation = 1;
                                    }
                                    break;
                                }
                                if (!attack.creature_attacked) break;
                                ++hits;
                                if (dm2_v1_runtime_last_wield_death_deallocated())
                                    break;
                            }
                        }
                    }
                }
                (void)hits;
            }
        }
    }
    if (saw_wield_calculation) {
        printf("SKSave1 WIELD source miss item=%04x creature=%04x power=%d "
               "dex=%d strength=%d skill=%d defense=%d armor=%d mapdiff=%d "
               "partypower=%d "
               "calculation=%d hit=%d miss=%d closed=%d raw=%d final=%d hp=%d\n",
               (unsigned short)last_wield.item_handle,
               (unsigned short)last_wield.creature_record,
               last_wield.command_power, last_wield.hero_dexterity,
               last_wield.hero_strength, last_wield.hero_skill_level,
               last_wield.creature_defense, last_wield.creature_armor,
               last_wield.map_difficulty, last_wield.party_power_level,
               last_wield.calculation_valid, last_wield.calculation_hit,
               last_wield.calculation_miss, last_wield.calculation_fail_closed,
               last_wield.raw_damage,
               last_wield.final_damage, last_wield.hp_applied);
    }
    /* The source search reached a calculation, but the supplied first
     * encounter is a miss.  Without an original input/RNG trace, a later
     * synthetic hit or possession drop would be false parity. */
    return saw_wield_calculation &&
           last_wield.calculation_valid &&
           !last_wield.calculation_hit &&
           last_wield.calculation_miss &&
           !last_wield.calculation_fail_closed &&
           last_wield.final_damage == 0 &&
           last_wield.hp_applied == 0 &&
           !dm2_v1_runtime_last_wield_death_deallocated() &&
           dm2_v1_runtime_last_wield_death_drop_count() == 0;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DOS_ROOT");
    const char *save = getenv("FIRESTAFF_DM2_SKSAVE1");
    DM2_V1_BootStartupLaunch launch;
    int ok = 0;
    int launch_ok;
    int prepare_ok;
    int commit_ok;
    int trace_gate_ok;

    if (!root || !root[0] || !save || !save[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ROOT and FIRESTAFF_DM2_SKSAVE1 are required");
        return 77;
    }
    memset(&launch, 0, sizeof(launch));
    launch_ok = dm2_v1_boot_startup_launch_alloc(root, &launch) &&
        launch.profile != NULL;
    prepare_ok = launch_ok &&
        dm2_v1_boot_prepare_sksave_resume_path(&launch, save);
    commit_ok = prepare_ok &&
        dm2_v1_boot_commit_sksave_resume_session(launch.profile);
    trace_gate_ok = commit_ok && exercise_wield_trace_gate(launch.profile);
    ok = launch_ok && prepare_ok && commit_ok && trace_gate_ok;
    dm2_v1_boot_startup_launch_cleanup(&launch);
    if (!ok) {
        printf("FAIL: SKSave1 WIELD trace gate launch=%d prepare=%d commit=%d gate=%d\n",
               launch_ok, prepare_ok, commit_ok, trace_gate_ok);
        return 1;
    }
    puts("PASS: authentic SKSave1 WIELD source miss keeps creature drop fail-closed");
    return 0;
}
