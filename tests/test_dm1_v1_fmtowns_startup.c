/* Real-media DM1 FM Towns startup receipt.
 *
 * The original HMA-240 archive is consumed as ZIP -> CUE -> BIN -> ISO9660
 * in RAM.  Do not replace this with a developer-maintained extracted tree. */

#include "dm1_v1_fmtowns_startup.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int extract_retail_file(const uint8_t *image, size_t image_size,
                               const FmtownsDiscProbeResult *disc,
                               const char *name, uint8_t **out,
                               size_t *out_size)
{
    const FmtownsIsoEntry *entry;
    if (!image || !disc || !name || !out || !out_size) return 0;
    entry = fmtowns_disc_find(disc, name);
    return entry && fmtowns_disc_extract_alloc(image, image_size,
                                                FMTOWNS_SECTOR_2048, entry,
                                                out, out_size) == 0;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    uint8_t *cue = NULL, *image = NULL;
    uint8_t *autoexec = NULL, *edm = NULL, *tmenu = NULL, *icons = NULL;
    uint8_t *info = NULL;
    size_t cue_size = 0u, image_size = 0u, autoexec_size = 0u, edm_size = 0u;
    size_t tmenu_size = 0u, icons_size = 0u, info_size = 0u;
    char image_member[256];
    FmtownsDiscProbeResult disc;
    DM1_V1_FmtownsStartupReceipt receipt;
    DM1_V1_FmtownsMenuReceipt menu_receipt;
    FILE *file = NULL;
    int result = 1;

    if (!archive || !archive[0] || !(file = fopen(archive, "rb"))) {
        if (file) fclose(file);
        puts("SKIP: FIRESTAFF_DM1_FMTOWNS_ARCHIVE is not staged");
        return 77;
    }
    fclose(file);

    if (firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cue_size) != 0 ||
        !cue || !fmtowns_cue_parse_image_member((const char *)cue, cue_size,
                                                 image_member,
                                                 sizeof(image_member)) ||
        firestaff_zip_extract_by_suffix(archive, image_member, &image,
                                        &image_size) != 0 || !image ||
        fmtowns_disc_probe(image, image_size, FMTOWNS_SECTOR_2048, &disc) != 0 ||
        !disc.valid || strcmp(disc.volume_id, "DUNGEON") != 0 ||
        !extract_retail_file(image, image_size, &disc, "AUTOEXEC.BAT",
                             &autoexec, &autoexec_size) ||
        !extract_retail_file(image, image_size, &disc, "EDM.EXP", &edm,
                             &edm_size) ||
        !extract_retail_file(image, image_size, &disc, "TMENU.EXP", &tmenu,
                             &tmenu_size) ||
        !extract_retail_file(image, image_size, &disc, "TMENU.ICN", &icons,
                             &icons_size) ||
        !extract_retail_file(image, image_size, &disc, "TMENU.INF", &info,
                             &info_size)) {
        fputs("FAIL: cannot read original FM Towns startup material in RAM\n",
              stderr);
        goto done;
    }

    if (!dm1_v1_fmtowns_startup_receipt(autoexec, autoexec_size, edm, edm_size,
                                        tmenu, tmenu_size, icons, icons_size,
                                        info, info_size, &receipt) ||
        !dm1_v1_fmtowns_startup_receipt_is_native(&receipt) ||
        !dm1_v1_fmtowns_startup_receipt_has_native_owners(&receipt) ||
        receipt.language != DM1_FMTOWNS_LANG_EN ||
        strcmp(receipt.game_program_name, "EDM.EXP") ||
        receipt.game_p3_header_size != 0x180u ||
        receipt.game_p3_load_image_offset != 0x200u ||
        receipt.game_p3_initial_eip != 0x42a48u ||
        !receipt.game_symbol_table_verified ||
        receipt.game_symbol_table_entry_count != 1174u ||
        receipt.game_do_title_animation_entry != 0xc3b0u ||
        receipt.game_title_presents_entry != 0x28f4au ||
        receipt.game_title_dungeon_entry != 0x28f4cu ||
        receipt.game_draw_dmenu_entry != 0x4620u ||
        receipt.game_dynamenu_entry != 0x2418cu ||
        receipt.game_menu_icons_entry != 0x2415cu ||
        receipt.game_cd_level_song_entry != 0x211d8u ||
        !receipt.game_title_animation_plan_verified ||
        receipt.game_title_graphic_index != 1u ||
        receipt.game_title_presents_source_y != 137u ||
        receipt.game_title_master_source_y != 80u ||
        receipt.game_title_zoom_step_count != 18u ||
        receipt.game_title_zoom_width_step != 16u ||
        receipt.game_title_zoom_height_step != 4u ||
        receipt.game_title_swoosh_rect[2] != 0u ||
        receipt.game_title_swoosh_rect[3] != 56u ||
        receipt.game_title_presents_rect[2] != 90u ||
        receipt.game_title_presents_rect[3] != 105u ||
        receipt.game_title_master_rect[2] != 118u ||
        receipt.game_title_master_rect[3] != 174u ||
        receipt.game_action_name_count != 44u ||
        strcmp(receipt.game_action_names[6], "PUNCH") ||
        strcmp(receipt.game_action_names[8], "WAR CRY") ||
        strcmp(receipt.game_action_names[43], "FUSE") ||
        receipt.title_track != 2u || receipt.hall_track != 3u ||
        receipt.entrance_track != 5u) {
        fputs("FAIL: original FM Towns startup receipt\n", stderr);
        goto done;
    }
    if (!dm1_v1_fmtowns_menu_receipt(info, info_size, &menu_receipt) ||
        !menu_receipt.valid || !menu_receipt.entries[0].valid ||
        !menu_receipt.entries[1].valid ||
        menu_receipt.entries[0].language != DM1_FMTOWNS_LANG_JP ||
        menu_receipt.entries[1].language != DM1_FMTOWNS_LANG_EN ||
        strcmp(menu_receipt.entries[0].program_name, "JDM     .EXP") ||
        strcmp(menu_receipt.entries[1].program_name, "EDM     .EXP") ||
        strcmp(menu_receipt.entries[0].program_path, "\\JDM.EXP") ||
        strcmp(menu_receipt.entries[1].program_path, "\\EDM.EXP")) {
        fputs("FAIL: original TMENU.INF launch records\n", stderr);
        goto done;
    }
    result = 0;
    puts("PASS: original FM Towns ZIP reaches native startup and menu owners in RAM");

done:
    free(cue); free(image); free(autoexec); free(edm); free(tmenu);
    free(icons); free(info);
    return result;
}
