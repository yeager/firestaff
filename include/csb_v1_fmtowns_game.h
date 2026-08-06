#ifndef FIRESTAFF_CSB_V1_FMTOWNS_GAME_H
#define FIRESTAFF_CSB_V1_FMTOWNS_GAME_H

#include <stdint.h>

#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_switch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_FMTOWNS_GAME_MUSIC_MAP_COUNT 10u
#define CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH 32u
#define CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT 32u
#define CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES \
    (CSB_V1_FMTOWNS_GAME_MUSIC_MAP_COUNT * \
     CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH * \
     CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT)

/*
 * FM Towns Game-program admission.
 *
 * SWITCHTW.EXP does not enter a shared PC startup program.  AUTOEXEC.BAT
 * maps its Japanese/English Game exits to CHTWJ.EXP/CHTWE.EXP respectively.
 * ReDMCSB Toolchains/Common/Source/COMPILE.H:367-385 identifies both as
 * C03_GAME and binds them to the distinct F31J/F31E media sets; STARTUP1.C
 * line 163 then owns the F0435 load loop and ENTRANCE.C line 85 owns F0807.
 */
typedef struct CSB_V1_FmtownsGameHandoffReceipt {
    int valid;
    int executable_verified;
    int language_matches_profile;
    int game_program_is_c03_game;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t executable_size;
    uint32_t executable_fnv1a;
    char executable_name[16];
    char executable_path[512];
    char graphics_md5[33];
    char dungeon_md5[33];
    int music_table_verified;
    uint32_t music_table_source_offset;
    uint32_t music_table_size;
    uint32_t music_table_fnv1a;
    const char *source_evidence;
} CSB_V1_FmtownsGameHandoffReceipt;

/* AUTOEXEC.BAT exit 2/5 enters a different C06_CEDT program. This receipt
 * admits that program only; it does not pretend its editor UI is C03_GAME. */
typedef struct CSB_V1_FmtownsUtilityHandoffReceipt {
    int valid;
    int executable_verified;
    int language_matches_profile;
    int utility_program_is_c06_cedt;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t executable_size;
    uint32_t executable_fnv1a;
    char executable_name[16];
    char executable_path[512];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityHandoffReceipt;

/* Admit precisely the F31E/F31J executable selected by SWITCHTW.  A valid
 * CSB profile alone is deliberately insufficient: this gate also checks the
 * exact retail program identity before the entrance/HUD session is opened. */
int csb_v1_fmtowns_game_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsGameHandoffReceipt *out_receipt);

int csb_v1_fmtowns_utility_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityHandoffReceipt *out_receipt);

/* Return the native F31 music selector (zero means no selector) for one
 * source map square. The value is the exact byte passed to F0719_PlayMusicTrack
 * by ReDMCSB MUSIC.C F0743; it is not a synthesized physical CD track number. */
int csb_v1_fmtowns_game_music_track_at(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint32_t map_index,
    uint32_t map_x,
    uint32_t map_y,
    uint8_t *out_track);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_FMTOWNS_GAME_H */
