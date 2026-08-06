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

#define CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT 6u
#define CSB_V1_FMTOWNS_UTILITY_MENU_POOL_CAPACITY 76u
#define CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT 16u
#define CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_RECORD_BYTES 68u

/* ReDMCSB DEFS.H command ordinals consumed by CEDT006.C's C06 loop. */
typedef enum CSB_V1_FmtownsUtilityMenuAction {
    CSB_V1_FMTOWNS_UTILITY_ACTION_NONE = 0,
    CSB_V1_FMTOWNS_UTILITY_ACTION_REVERT = 8,
    CSB_V1_FMTOWNS_UTILITY_ACTION_UNDO = 9,
    CSB_V1_FMTOWNS_UTILITY_ACTION_LOAD_CHAMPIONS = 11,
    CSB_V1_FMTOWNS_UTILITY_ACTION_SAVE_CHAMPIONS = 12,
    CSB_V1_FMTOWNS_UTILITY_ACTION_MAKE_NEW_ADVENTURE = 13,
    CSB_V1_FMTOWNS_UTILITY_ACTION_QUIT = 17
} CSB_V1_FmtownsUtilityMenuAction;

typedef struct CSB_V1_FmtownsUtilityMenuHitBox {
    CSB_V1_FmtownsUtilityMenuAction action;
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} CSB_V1_FmtownsUtilityMenuHitBox;

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
    /* The CD's MINI.DAT is an authenticated F31 bootstrap resource.  It is
     * deliberately recorded separately from a user save: it must not enter
     * the Atari/Amiga GAMEBLOCK Resume decoder. */
    int startup_mini_verified;
    uint32_t startup_mini_size;
    uint32_t startup_mini_fnv1a;
    char startup_mini_path[512];
    /* F0435 reads and authenticates the F31 512-byte header with the CSB
     * key at word 29.  This proves the selected seed is a native C5-format
     * FM Towns save header; it does not yet decode its saved-game body. */
    int startup_mini_header_verified;
    uint16_t startup_mini_header_key;
    uint8_t startup_mini_header_format_id;
    uint16_t startup_mini_header_platform;
    uint16_t startup_mini_header_dungeon_id;
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
    /* Bounded Phar Lap level-1 P3 envelope from the verified C06 image.
     * This exposes the real native entry point without claiming to emulate
     * its TBIOS menu, editor pixels, or save transactions. */
    int p3_header_verified;
    uint32_t p3_header_size;
    uint32_t p3_load_image_offset;
    uint32_t p3_load_image_size;
    uint32_t p3_initial_eip;
    /* CEDT027.C's C09_ICON table is retained from this exact hash-verified
     * F31 executable: 16 indexed RGB6 entries followed by its 0xFF sentinel.
     * It is not a host palette approximation. */
    int icon_palette_verified;
    uint32_t icon_palette_file_offset;
    uint8_t icon_palette_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityHandoffReceipt;

/* C06's first menu is retained as source bytes.  Japanese remains Shift-JIS
 * until the native Towns text path is decoded; callers must not replace it
 * with translated host strings. */
typedef struct CSB_V1_FmtownsUtilityMenuReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t source_virtual_offset;
    uint32_t source_file_offset;
    uint32_t source_size;
    uint32_t source_fnv1a;
    uint16_t label_offsets[CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT];
    uint8_t source_bytes[CSB_V1_FMTOWNS_UTILITY_MENU_POOL_CAPACITY];
    int icon_palette_verified;
    uint32_t icon_palette_file_offset;
    uint8_t icon_palette_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityMenuReceipt;

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

int csb_v1_fmtowns_utility_menu_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityMenuReceipt *out_receipt);

/* Decode the original C06 mouse target in its 320x200 source coordinate
 * space. ReDMCSB CEDTDATA.C lines 128-165 defines these F31E/F31J boxes,
 * and CEDT006.C lines 1401-1529 dispatches the resulting command ordinal.
 * Edges are inclusive, matching MOUSE2_INPUT. */
int csb_v1_fmtowns_utility_menu_action_at(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    int16_t source_x, int16_t source_y,
    CSB_V1_FmtownsUtilityMenuHitBox *out_hit_box);

/* Copy the receipt-bound F31 C06 editor C09_ICON palette in native six-bit
 * RGB. CEDT018.C first blacks the curtain, applies C09_ICON, then restores
 * it; CEDT027.C defines the table. A receipt is required so the live route
 * never falls back to a hand-copied host palette. */
int csb_v1_fmtowns_utility_icon_palette_rgb6(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    uint8_t out_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3]);

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
