#ifndef FIRESTAFF_CSB_V1_STARTUP_PRESENTATION_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_PRESENTATION_RECEIPT_PC34_COMPAT_H

/* One source-bound receipt for the CSB startup presentation spine.
 * It composes existing owners; it never draws, decodes, mixes, or generates
 * a substitute frame. ReDMCSB: SWSH.C F0908-F0910, TITLE.C F0437,
 * ENTRANCE.C F0438/F0442/F0806/F0807, PANEL.C F0346/F0347. */

#include "csb_v1_f0908_f0909_f0910_swsh_sound_pc34_compat.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_STARTUP_PRESENTATION_PRESENTS_STEP_PC34 = 1,
    CSB_V1_STARTUP_PRESENTATION_CHAOS_FIRST_STEP_PC34 = 2,
    CSB_V1_STARTUP_PRESENTATION_CHAOS_LAST_STEP_PC34 = 21,
    CSB_V1_STARTUP_PRESENTATION_STRIKES_STEP_PC34 = 22,
    CSB_V1_STARTUP_PRESENTATION_TITLE_TOTAL_TICKS_PC34 = 101,
    CSB_V1_STARTUP_PRESENTATION_CHAOS_HOLD_VBLANKS_PC34 = 2,
    CSB_V1_STARTUP_PRESENTATION_ENTRANCE_DOOR_STEPS_PC34 = 31
};

typedef struct CSB_V1_StartupPresentationCadenceFacts_PC34 {
    int valid;
    unsigned int presents_source_step;
    unsigned int chaos_first_source_step;
    unsigned int chaos_last_source_step;
    unsigned int chaos_hold_vblanks;
    unsigned int strikes_source_step;
    unsigned int title_total_ticks;
    unsigned int entrance_door_steps;
    int no_host_timing_padding;
    int no_synthetic_palette_program;
} CSB_V1_StartupPresentationCadenceFacts_PC34;

typedef struct CSB_V1_StartupEntranceAudioFacts_PC34 {
    int valid;
    int entrance_music_started_after_title;
    int entrance_music_active_through_door_open;
    int source_music_route_bound;
    int no_synthetic_audio;
    int no_legacy_audio_wrapper;
    uint32_t source_audio_hash;
} CSB_V1_StartupEntranceAudioFacts_PC34;

typedef struct CSB_V1_StartupPackagePresentationReceipt_PC34 {
    int valid;
    int real_package_consumed;
    int c001_c005_c017_c040_bound;
    int source_pixels_and_palettes_bound;
    int source_cadence_bound;
    int source_audio_bound;
    int title_to_entrance_to_hud_same_session;
    int no_legacy_wrappers;
    int no_fallback_routes;
    unsigned int session_generation;
    unsigned int source_tick;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
    uint32_t presentation_receipt_hash;
    const char *source_evidence;
} CSB_V1_StartupPackagePresentationReceipt_PC34;

void csb_v1_startup_package_presentation_receipt_init_pc34(
    CSB_V1_StartupPackagePresentationReceipt_PC34 *receipt);

/* Returns 1 only when all inputs describe one authenticated package session.
 * A stale phase hash, synthetic/wrapper route, altered source cadence, or
 * missing source audio leaves the output cleared. */
int csb_v1_startup_package_presentation_receipt_from_source_pc34(
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 *title_opening,
    const CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 *hud_runtime,
    const CSB_V1_StartupPresentationCadenceFacts_PC34 *cadence,
    const CSB_V1_SwshSoundInitReceipt_PC34 *swoosh_init,
    const CSB_V1_SwshSoundPlayReceipt_PC34 *swoosh_play,
    const CSB_V1_SwshSoundReleaseReceipt_PC34 *swoosh_release,
    const CSB_V1_StartupEntranceAudioFacts_PC34 *entrance_audio,
    CSB_V1_StartupPackagePresentationReceipt_PC34 *out_receipt);

const char *csb_v1_startup_package_presentation_receipt_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
