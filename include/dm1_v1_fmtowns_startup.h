#ifndef DM1_V1_FMTOWNS_STARTUP_H
#define DM1_V1_FMTOWNS_STARTUP_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_fmtowns_graphics_dat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Receipt for the original FM Towns launcher boundary.  The FM Towns
 * release does not use the PC34 TITLE.DAT/SWOOSH pair: AUTOEXEC starts
 * CONTROL, which owns the TMENU program, and EDM/JDM owns the selected
 * language game.  Keep this as a source-bound admission gate until the
 * Phar Lap P3/TBIOS drawing path is decoded. */
typedef struct {
    int valid;
    DM1_V1_FmtownsLang language;
    uint32_t autoexec_size;
    uint32_t game_program_size;
    uint32_t menu_program_size;
    uint32_t menu_icon_size;
    uint32_t menu_info_size;
    char game_program_name[8];
    char autoexec_md5[33];
    char game_program_md5[33];
    char menu_program_md5[33];
    char menu_icon_md5[33];
    char menu_info_md5[33];
    int menu_info_selects_game;
    int menu_program_symbols_verified;
    int game_program_symbols_verified;
    int menu_p3_header_verified;
    int game_p3_header_verified;
    int game_symbol_table_verified;
    uint32_t game_symbol_table_entry_count;
    uint32_t game_do_title_animation_entry;
    uint32_t game_title_presents_entry;
    uint32_t game_title_dungeon_entry;
    uint32_t game_draw_dmenu_entry;
    uint32_t game_dynamenu_entry;
    uint32_t game_menu_icons_entry;
    uint32_t game_cd_level_song_entry;
    int game_title_animation_plan_verified;
    uint16_t game_title_graphic_index;
    uint16_t game_title_presents_source_y;
    uint16_t game_title_master_source_y;
    uint16_t game_title_zoom_source_width;
    uint16_t game_title_zoom_source_height;
    uint16_t game_title_zoom_start_width;
    uint16_t game_title_zoom_start_height;
    uint16_t game_title_zoom_width_step;
    uint16_t game_title_zoom_height_step;
    uint16_t game_title_zoom_step_count;
    uint16_t game_title_swoosh_rect[4];
    uint16_t game_title_presents_rect[4];
    uint16_t game_title_master_rect[4];
    uint32_t game_p3_header_size;
    uint32_t game_p3_load_image_offset;
    uint32_t game_p3_load_image_size;
    uint32_t game_p3_symbol_table_offset;
    uint32_t game_p3_symbol_table_size;
    uint32_t game_p3_initial_eip;
    uint8_t title_track;
    uint8_t hall_track;
    uint8_t entrance_track;
} DM1_V1_FmtownsStartupReceipt;

/* TMENU.INF is the retained 256-byte TownsOS launcher configuration.  Its
 * two 128-byte records select JDM.EXP and EDM.EXP respectively.  Preserve
 * the labels as raw source bytes until their Shift-JIS/Towns glyph route is
 * decoded; this receipt only exposes the executable and DOS path that the
 * original menu actually launches. */
#define DM1_FMTOWNS_TMENU_ENTRY_COUNT 2u
typedef struct {
    int valid;
    DM1_V1_FmtownsLang language;
    char program_name[13]; /* Twelve-byte 8.3 field, without source padding. */
    char program_path[16]; /* Original `\\JDM.EXP` / `\\EDM.EXP` field. */
    uint8_t title_bytes[9]; /* Source-owned label bytes; not host-decoded. */
} DM1_V1_FmtownsMenuEntry;

typedef struct {
    int valid;
    DM1_V1_FmtownsMenuEntry entries[DM1_FMTOWNS_TMENU_ENTRY_COUNT];
} DM1_V1_FmtownsMenuReceipt;

/* Parse the authenticated TMENU.INF record layout. Returns 1 only for the
 * exact two-launcher layout found on the HMA-240 English/Japanese disc. */
int dm1_v1_fmtowns_menu_receipt(const uint8_t *menu_info, size_t menu_info_size,
                                DM1_V1_FmtownsMenuReceipt *out);

/* Validate the original root startup files from the HMA-240 retail disc.
 * All buffers are caller-owned and are never modified. Returns 1 only for
 * the verified English or Japanese program family, otherwise 0. */
int dm1_v1_fmtowns_startup_receipt(
    const uint8_t *autoexec, size_t autoexec_size,
    const uint8_t *game_program, size_t game_program_size,
    const uint8_t *menu_program, size_t menu_program_size,
    const uint8_t *menu_icon, size_t menu_icon_size,
    const uint8_t *menu_info, size_t menu_info_size,
    DM1_V1_FmtownsStartupReceipt *out);

/* Validate the selected original FM Towns runtime directory without
 * accepting a sibling language executable.  The directory must contain the
 * retained retail root files; no PC34 title asset is consulted. */
int dm1_v1_fmtowns_startup_receipt_from_directory(
    const char *root,
    int japanese,
    DM1_V1_FmtownsStartupReceipt *out);

/* The receipt's program is the native startup owner. This intentionally
 * does not claim that the P3/TBIOS pixel decoder is complete. */
int dm1_v1_fmtowns_startup_receipt_is_native(
    const DM1_V1_FmtownsStartupReceipt *receipt);

/* The native executables remain the source of truth for startup ownership.
 * This gate is deliberately symbol-based: it does not claim that the P3
 * instruction stream or TBIOS pixel calls have been decoded. */
int dm1_v1_fmtowns_startup_receipt_has_native_owners(
    const DM1_V1_FmtownsStartupReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif
