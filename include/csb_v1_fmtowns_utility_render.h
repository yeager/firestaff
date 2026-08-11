#ifndef FIRESTAFF_CSB_V1_FMTOWNS_UTILITY_RENDER_H
#define FIRESTAFF_CSB_V1_FMTOWNS_UTILITY_RENDER_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_fmtowns_game.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH 320u
#define CSB_V1_FMTOWNS_UTILITY_SCREEN_HEIGHT 200u
#define CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS \
    (CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH * CSB_V1_FMTOWNS_UTILITY_SCREEN_HEIGHT)

/* F31J C06 delegates its first selector to the Towns Shift-JIS text path.
 * Tsugaru capture shows this page is a native 640x400 surface, not the
 * 320x200 F31E editor raster doubled by a host scaler. */
#define CSB_V1_FMTOWNS_UTILITY_JAPANESE_SCREEN_WIDTH 640u
#define CSB_V1_FMTOWNS_UTILITY_JAPANESE_SCREEN_HEIGHT 400u
#define CSB_V1_FMTOWNS_UTILITY_JAPANESE_SCREEN_PIXELS \
    (CSB_V1_FMTOWNS_UTILITY_JAPANESE_SCREEN_WIDTH * \
     CSB_V1_FMTOWNS_UTILITY_JAPANESE_SCREEN_HEIGHT)

typedef struct CSB_V1_FmtownsUtilityRenderReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    uint32_t source_fnv1a;
    uint32_t pixel_fnv1a;
    uint16_t rendered_champion_count;
    uint16_t selected_champion_index;
    uint8_t selected_color_index;
    uint16_t file_picker_first_index;
    uint16_t file_picker_selected_index;
    const char *source_evidence;
} CSB_V1_FmtownsUtilityRenderReceipt;

enum {
    CSB_V1_FMTOWNS_FILE_PICKER_FILE_LIST = 1,
    CSB_V1_FMTOWNS_FILE_PICKER_NEW_DISK = 2,
    CSB_V1_FMTOWNS_FILE_PICKER_CANCEL = 3,
    CSB_V1_FMTOWNS_FILE_PICKER_UP = 4,
    CSB_V1_FMTOWNS_FILE_PICKER_DOWN = 5
};

typedef struct CSB_V1_FmtownsUtilityFilePicker {
    int valid;
    uint16_t first_index;
    uint16_t selected_index;
    uint32_t catalog_fnv1a;
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog;
    const char *source_evidence;
} CSB_V1_FmtownsUtilityFilePicker;

/* CEDT008.C F7083/F7084 file-list state.  The catalogue is the admitted
 * PORTRAIT directory; this state never owns or invents a filename. */
int csb_v1_fmtowns_utility_file_picker_open(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    uint16_t initial_index,
    CSB_V1_FmtownsUtilityFilePicker *out_picker);

/* Apply one source-coordinate mouse command.  The returned command is one
 * of the five C06 ordinals above; a file-list command returns the selected
 * catalogue index, while buttons return -1. */
int csb_v1_fmtowns_utility_file_picker_input(
    CSB_V1_FmtownsUtilityFilePicker *picker,
    int16_t source_x, int16_t source_y,
    int *out_command, int *out_catalog_index);

/* Render the native C06 file-picker surface from the authenticated
 * executable's arrow bitmap, interface font and portrait catalogue. */
int csb_v1_fmtowns_utility_render_file_picker(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_FmtownsUtilityFilePicker *picker,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* Recreate C06_CEDT's first F31E editor frame directly from the selected
 * retail executable and MINI.DAT payload.  Every glyph, menu label, portrait
 * and raster comes from a verified receipt; no fallback font, translated
 * label, placeholder portrait or host artwork is accepted.  F31J is
 * deliberately rejected until its native Shift-JIS/TBIOS text consumer is
 * recovered from runtime evidence. */
int csb_v1_fmtowns_utility_render_editor(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    int edit_field, uint8_t edit_character_index, int text_cursor_visible,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* First F7042 frame: champion and palette index zero. */
int csb_v1_fmtowns_utility_render_initial(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* C06's first modal is not the portrait editor.  F31E CEDTDATA.C G7085 and
 * G7065 ask which saved-game family is to become C0_GAME_SOURCE.  The F31J
 * equivalent remains deliberately closed until its TBIOS glyph path is
 * reproduced rather than replaced with host text. */
int csb_v1_fmtowns_utility_render_game_source_dialog(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityGameSourceReceipt *game_source,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* Render the observed F31J C06 source chooser at its original 640x400
 * geometry.  Japanese glyphs are fetched through the bound TBIOS host, so
 * this function rejects the page unless a verified FMT_FNT.ROM-backed host
 * can supply every Shift-JIS glyph.  The bytes are the exact choices read
 * from the original Tsugaru C06 capture, not translated host strings. */
int csb_v1_fmtowns_utility_render_japanese_game_source_dialog(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* After the CSB branch C06 waits for its selected native game-save medium in
 * drive A:.  This is intentionally a distinct modal: it must not fall
 * through to the retail CD's MINI.DAT bootstrap as though it were a save. */
int csb_v1_fmtowns_utility_render_game_save_medium_dialog(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityGameSourceReceipt *game_source,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* CEDT001.C F7001's native three-choice save dialog.  The editor beneath
 * it and its 5x6 font remain receipt-bound; the choices use CEDTDATA.C
 * G2261 source hit rectangles: GAME, PORTRAIT, CANCEL. */
int csb_v1_fmtowns_utility_render_save_dialog(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

/* CEDT001.C F7004's native three-choice load dialog.  It reuses G2261's
 * GAME/PORTRAIT/CANCEL hit rectangles, but carries G7068's distinct text. */
int csb_v1_fmtowns_utility_render_load_dialog(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_FMTOWNS_UTILITY_RENDER_H */
