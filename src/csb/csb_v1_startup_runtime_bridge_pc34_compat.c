#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "firestaff/csb/v1/startup_entrance_pointer_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <string.h>

int csb_v1_runtime_apply_startup_sequence_plan_from_state_facts_with_receipts_pc34(
    CSB_V1_RuntimeProfile *profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_runtime_exec_receipt,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_runtime_apply_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt)
{
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 local_exec_receipt;
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *exec_receipt =
        out_runtime_exec_receipt ? out_runtime_exec_receipt
                                 : &local_exec_receipt;

    csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(exec_receipt);
    if (out_runtime_apply_receipt) {
        csb_v1_startup_runtime_apply_receipt_init_pc34(
            out_runtime_apply_receipt);
    }
    if (out_state_receipt) {
        csb_v1_startup_command_state_receipt_init_pc34(out_state_receipt);
    }
    if (!profile || !startup_plan ||
        !csb_v1_runtime_apply_startup_sequence_plan_pc34(
            profile,
            startup_plan,
            resume_path,
            exec_receipt)) {
        return 0;
    }
    return csb_v1_startup_apply_runtime_plan_from_facts_with_receipts_pc34(
        title_active,
        title_frame,
        title_source_step,
        entrance_active,
        entrance_source_step,
        entrance_dismissed,
        credits_active,
        credits_remaining_ticks,
        opening_active,
        opening_delay_ticks,
        opening_step,
        pending_command,
        startup_plan,
        exec_receipt->resume_available,
        exec_receipt->resume_loaded,
        out_outcome,
        out_runtime_apply_receipt,
        out_state_receipt);
}

int csb_v1_runtime_apply_startup_sequence_plan_from_boot_profile_facts_with_receipts_pc34(
    void *boot_profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_runtime_exec_receipt,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_runtime_apply_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    if (!profile) {
        if (out_runtime_exec_receipt) {
            csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(
                out_runtime_exec_receipt);
        }
        if (out_runtime_apply_receipt) {
            csb_v1_startup_runtime_apply_receipt_init_pc34(
                out_runtime_apply_receipt);
        }
        if (out_state_receipt) {
            csb_v1_startup_command_state_receipt_init_pc34(
                out_state_receipt);
        }
        return 0;
    }

    return csb_v1_runtime_apply_startup_sequence_plan_from_state_facts_with_receipts_pc34(
        &profile->runtime,
        startup_plan,
        resume_path,
        title_active,
        title_frame,
        title_source_step,
        entrance_active,
        entrance_source_step,
        entrance_dismissed,
        credits_active,
        credits_remaining_ticks,
        opening_active,
        opening_delay_ticks,
        opening_step,
        pending_command,
        out_runtime_exec_receipt,
        out_outcome,
        out_runtime_apply_receipt,
        out_state_receipt);
}

int csb_v1_runtime_m11_mirror_receipt_from_boot_profile_pc34(
    const void *boot_profile,
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    if (!profile) {
        if (out_receipt) {
            csb_v1_runtime_m11_mirror_receipt_init_pc34(out_receipt);
        }
        return 0;
    }
    return csb_v1_runtime_m11_mirror_receipt_from_profile_pc34(
        &profile->runtime,
        out_receipt);
}

int csb_v1_runtime_util_render_plan_from_boot_profile_facts_pc34(
    int selected_action_index,
    int imported_champion_count,
    const void *boot_profile,
    const char *prompt_override,
    int preview_active,
    CSB_V1_UtilRenderPlan *out_plan)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return csb_v1_util_flow_render_plan_from_runtime_profile_facts(
        selected_action_index,
        imported_champion_count,
        profile ? &profile->runtime : NULL,
        prompt_override,
        preview_active,
        out_plan);
}

void csb_v1_runtime_util_startup_host_action_receipt_init_pc34(
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_util_flow_apply_receipt_init(&receipt->util_receipt);
    csb_v1_util_flow_state_receipt_init(&receipt->util_state_receipt);
    csb_v1_startup_entrance_host_action_receipt_init_pc34(
        &receipt->entrance_receipt);
}

int csb_v1_runtime_save_game_to_path_from_boot_profile_pc34(
    const void *boot_profile,
    const char *path,
    uint32_t *out_game_time)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile || !path) {
        return -1;
    }
    result = csb_v1_runtime_save_game_to_path(&profile->runtime, path);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_runtime_load_game_from_path_from_boot_profile_pc34(
    void *boot_profile,
    const char *path,
    uint32_t *out_game_time)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile || !path) {
        return -1;
    }
    result = csb_v1_runtime_load_game_from_path(&profile->runtime, path);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_runtime_tick_from_boot_profile_pc34(
    void *boot_profile,
    uint32_t *out_game_time)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile) {
        return 0;
    }
    result = csb_v1_runtime_tick_v1(&profile->runtime);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_runtime_object_icon_index_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_object_icon_index(&profile->runtime, thing)
                   : -1;
}

int csb_v1_runtime_object_action_set_index_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile
               ? csb_v1_runtime_object_action_set_index(&profile->runtime,
                                                        thing)
               : 0;
}

uint16_t csb_v1_runtime_object_allowed_slots_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_object_allowed_slots(&profile->runtime,
                                                         thing)
                   : 0u;
}

int csb_v1_runtime_object_name_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing,
    char *out,
    size_t out_size)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    if (!profile) {
        if (out && out_size > 0u) {
            out[0] = '\0';
        }
        return 0;
    }
    return csb_v1_runtime_object_name(&profile->runtime,
                                      thing,
                                      out,
                                      out_size);
}

int csb_v1_runtime_read_container_slots_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short container_thing,
    unsigned short out_slots[8])
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_read_container_slots(&profile->runtime,
                                                         container_thing,
                                                         out_slots)
                   : -1;
}

int csb_v1_runtime_write_container_slots_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short container_thing,
    const unsigned short slots[8])
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_write_container_slots(&profile->runtime,
                                                          container_thing,
                                                          slots)
                   : 0;
}

int csb_v1_runtime_set_thing_next_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short thing,
    unsigned short next_thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_set_thing_next(&profile->runtime,
                                                   thing,
                                                   next_thing)
                   : 0;
}

int csb_v1_runtime_write_inventory_slot_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int csb_slot,
    unsigned short thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;

    if (!profile) return 1;
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= runtime->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        csb_slot < 0 ||
        csb_slot >= CSB_V1_SLOT_COUNT) {
        return 0;
    }
    /* CSBWin Character.cpp::SetPossession runs EquipFilter once for the
     * removed RN (timer function 1) and once for the added RN (0), before it
     * writes the possession. Unsupported or non-authenticated DSA data is
     * deliberately not substituted; the established slot write still owns
     * its non-DSA runtime path. */
    (void)csb_v1_runtime_execute_csbwin_equip_filter(
        runtime, champion_index, csb_slot,
        runtime->party_state.Champions[champion_index].Slots[csb_slot], thing);
    runtime->party_state.Champions[champion_index].Slots[csb_slot] = thing;
    return 1;
}

int csb_v1_runtime_write_leader_hand_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;

    if (!profile) return 1;
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (thing == 0xfffeu) thing = 0xffffu;
    if (runtime->csbwin_gameblock2_summary_valid) {
        /* CSBWin SaveGame.cpp::LoadGame first sends ReadGame after
         * GAMEBLOCK2.objectInHand is restored. CSBCode.cpp::TAG0138ec then
         * sends ResumeSavedGame before ObjectToCursor. Both are notifications:
         * no DSA output can alter this source-owned hand restoration. */
        (void)csb_v1_runtime_execute_csbwin_cursor_read_game_filter(
            runtime, thing);
        (void)csb_v1_runtime_execute_csbwin_cursor_resume_saved_game_filter(
            runtime, thing);
    }
    runtime->party_state.LeaderHandThing = thing;
    if (runtime->csbwin_gameblock2_summary_valid) {
        runtime->csbwin_object_in_hand = thing;
    }
    return 1;
}

int csb_v1_runtime_throw_leader_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    unsigned short leader_thing,
    unsigned short *out_restored_action_hand,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;
    CSB_V1_Champion *champion;
    unsigned short saved_action_hand;

    if (out_restored_action_hand) *out_restored_action_hand = 0xffffu;
    if (!profile || leader_thing == 0xffffu || leader_thing == 0xfffeu) {
        return 0;
    }
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= runtime->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }

    champion = &runtime->party_state.Champions[champion_index];
    saved_action_hand = champion->Slots[CSB_V1_SLOT_ACTION_HAND];
    champion->Slots[CSB_V1_SLOT_ACTION_HAND] = leader_thing;
    if (!csb_v1_runtime_throw_action_hand(runtime,
                                          champion_index,
                                          out_projectile_slot)) {
        champion->Slots[CSB_V1_SLOT_ACTION_HAND] = saved_action_hand;
        if (out_restored_action_hand) {
            *out_restored_action_hand = saved_action_hand;
        }
        return 0;
    }
    champion->Slots[CSB_V1_SLOT_ACTION_HAND] = saved_action_hand;
    if (out_restored_action_hand) {
        *out_restored_action_hand = saved_action_hand;
    }
    return 1;
}

int csb_v1_runtime_write_champion_vitals_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int current_health,
    int current_stamina,
    int current_mana)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;
    CSB_V1_Champion *champion;

    if (!profile) return 1;
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= runtime->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    champion = &runtime->party_state.Champions[champion_index];
    champion->CurrentHealth = (int16_t)current_health;
    champion->CurrentStamina = (int16_t)current_stamina;
    champion->CurrentMana = (int16_t)current_mana;
    return 1;
}

int csb_v1_runtime_throw_action_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_throw_action_hand(&profile->runtime,
                                                      champion_index,
                                                      out_projectile_slot)
                   : 0;
}

int csb_v1_runtime_shoot_ready_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_shoot_ready_hand(&profile->runtime,
                                                     champion_index,
                                                     out_projectile_slot)
                   : 0;
}

int csb_v1_runtime_refill_ready_hand_after_shoot_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_source_slot,
    unsigned short *out_thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_refill_ready_hand_after_shoot(
                         &profile->runtime,
                         champion_index,
                         out_source_slot,
                         out_thing)
                   : 0;
}

int csb_v1_runtime_spawn_champion_projectile_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int action_index,
    int projectile_subtype,
    int projectile_category,
    int kinetic_energy,
    int attack,
    int attack_type_code,
    int step_energy,
    unsigned short associated_thing,
    int poison_attack,
    int potion_power,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_spawn_champion_projectile(
                         &profile->runtime,
                         champion_index,
                         action_index,
                         projectile_subtype,
                         projectile_category,
                         kinetic_energy,
                         attack,
                         attack_type_code,
                         step_energy,
                         associated_thing,
                         poison_attack,
                         potion_power,
                         out_projectile_slot)
                   : 0;
}

int csb_v1_runtime_perform_melee_action_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int action_index)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_perform_melee_action(&profile->runtime,
                                                         champion_index,
                                                         action_index,
                                                         NULL)
                   : 0;
}

int csb_v1_runtime_trigger_front_wall_ornament_click_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short leader_hand_thing,
    unsigned short *out_leader_hand_thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;
    int dx = 0;
    int dy = 0;
    int queued;

    if (out_leader_hand_thing) *out_leader_hand_thing = leader_hand_thing;
    if (!profile) return 0;
    runtime = &profile->runtime;
    /* F0275 is a live dungeon mutation, not a boot-time coordinate shim.
     * Keep the front-wall bridge bound to the loaded dungeon/level that owns
     * the runtime party before deriving its source square. */
    if (!runtime->dungeon_handle ||
        csb_v1_dungeon_get_current() != runtime->dungeon_handle ||
        csb_v1_dungeon_get_current_level() != runtime->current_level) {
        return 0;
    }
    switch (runtime->party_dir & 3) {
        case 0: dy = -1; break;
        case 1: dx = 1; break;
        case 2: dy = 1; break;
        case 3: dx = -1; break;
        default: break;
    }
    runtime->party_state.LeaderHandThing = leader_hand_thing;
    queued = csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
        runtime,
        runtime->party_x + dx,
        runtime->party_y + dy,
        0);
    if (queued <= 0) return queued;
    if (out_leader_hand_thing) {
        *out_leader_hand_thing = runtime->party_state.LeaderHandThing;
    }
    return queued;
}
