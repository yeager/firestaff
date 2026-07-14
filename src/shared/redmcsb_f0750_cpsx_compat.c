#include "redmcsb_f0750_cpsx_compat.h"

void redmcsb_f0750_shutdown_startup2_compat(
    redmcsb_f0750_startup2_target_compat target,
    bool game_won,
    const redmcsb_f0750_startup2_services_compat *services)
{
    services->pause_music(services->context);

    switch (target) {
    case REDMCSB_F0750_STARTUP2_DOS:
        services->release_ems_handle(services->context);
        services->free_base_memory(services->context);
        services->terminate_process(services->context, game_won ? 1 : 0);
        return;

    case REDMCSB_F0750_STARTUP2_FMTOWNS:
        services->free_base_memory(services->context);
        if (game_won) {
            services->enable_screen_update(services->context);
            services->play_animation(services->context, "ending.anm");
        }
        services->restore_towns(services->context);
        services->terminate_process(services->context, 0);
        return;

    case REDMCSB_F0750_STARTUP2_PC98:
        services->free_base_memory(services->context);
        if (game_won) {
            services->install_ending_interrupt(services->context, "anim ending.anm");
            services->terminate_process(services->context, 1);
            return;
        }
        services->terminate_process(services->context, 0);
        return;
    }
}

void redmcsb_f0750_shutdown_amiga_compat(
    bool game_won,
    bool copy_protection_enabled,
    const redmcsb_f0750_amiga_services_compat *services)
{
    if (game_won) {
        services->finish_game_won(services->context);
    } else {
        services->finish_game_not_won(services->context);
    }
    services->wait_blit(services->context);
    if (copy_protection_enabled) {
        while (services->is_copy_protection_reading(services->context)) {
        }
    }
    services->close_display(services->context);
    services->deinitialize_input(services->context);
    services->free_amiga_stuff(services->context);
}

const char *redmcsb_f0750_cpsx_source_evidence(void)
{
    return "ReDMCSB STARTUP2.C:293-332 implements the I34E/I34M, "
           "F31E/F31J, and P31J shutdown bodies: music pause first, then "
           "their target-specific EMS/base-memory, ending-animation, "
           "restore, and process-termination calls. AMIGINIT.C:673-690 "
           "implements the Amiga body: F1125/F1126, WaitBlit, optional "
           "copy-protection wait, F1045, F0538_INPUT_Deinitialize, and "
           "F1068_FreeAmigaStuff. ENDGAME.C:1020 calls F0750_CPSX before "
           "longjmp. DEFS.H:9459-9461 declares it void.";
}
