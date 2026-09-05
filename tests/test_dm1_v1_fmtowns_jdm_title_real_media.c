#include "dm1_v1_fmtowns_startup.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* These assertions execute the original-media checks, including parser
 * calls; keep them active in Release test builds as well. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int extract(const uint8_t *image, size_t image_size,
                   const FmtownsDiscProbeResult *disc, const char *name,
                   uint8_t **bytes, size_t *size)
{
    const FmtownsIsoEntry *entry = fmtowns_disc_find(disc, name);
    return entry && fmtowns_disc_extract_alloc(image, image_size,
        FMTOWNS_SECTOR_2048, entry, bytes, size) == 0;
}

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM1_FMTOWNS_ZIP");
    char default_zip[1024];
    uint8_t *image = NULL, *autoexec = NULL, *jdm = NULL, *menu = NULL;
    uint8_t *icon = NULL, *info = NULL, *dungeon = NULL;
    size_t image_size = 0, autoexec_size = 0, jdm_size = 0, menu_size = 0;
    size_t icon_size = 0, info_size = 0, dungeon_size = 0;
    FmtownsDiscProbeResult disc;
    DM1_V1_FmtownsStartupReceipt receipt;
    struct DungeonDatState_Compat dungeon_state;
    struct DungeonThings_Compat dungeon_things;
    int ok = 0;

    if (!zip || !zip[0]) {
        const char *home = getenv("HOME");
        if (!home || snprintf(default_zip, sizeof(default_zip),
                "%s/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip",
                home) >= (int)sizeof(default_zip)) {
            puts("SKIP: no DM1 FM Towns retail ZIP path");
            return 77;
        }
        zip = default_zip;
    }
    if (firestaff_zip_extract_by_suffix(zip, ".bin", &image, &image_size) != 0) {
        puts("SKIP: DM1 FM Towns retail ZIP is not available");
        return 77;
    }
    if (fmtowns_disc_probe(image, image_size, FMTOWNS_SECTOR_2048, &disc) != 0 ||
        !extract(image, image_size, &disc, "AUTOEXEC.BAT", &autoexec, &autoexec_size) ||
        !extract(image, image_size, &disc, "JDM.EXP", &jdm, &jdm_size) ||
        !extract(image, image_size, &disc, "TMENU.EXP", &menu, &menu_size) ||
        !extract(image, image_size, &disc, "TMENU.ICN", &icon, &icon_size) ||
        !extract(image, image_size, &disc, "TMENU.INF", &info, &info_size) ||
        !extract(image, image_size, &disc, "JDATA/DUNGEON.DAT",
                 &dungeon, &dungeon_size)) goto done;

    assert(dm1_v1_fmtowns_startup_receipt(autoexec, autoexec_size,
        jdm, jdm_size, menu, menu_size, icon, icon_size, info, info_size,
        &receipt));
    assert(receipt.valid && receipt.language == DM1_FMTOWNS_LANG_JP);
    assert(strcmp(receipt.game_program_name, "JDM.EXP") == 0);
    assert(receipt.game_p3_initial_eip == 0x42cb4u);
    assert(!receipt.game_symbol_table_verified);
    assert(receipt.game_program_symbols_verified);
    assert(receipt.game_do_title_animation_entry == 0xc428u);
    assert(receipt.game_title_presents_entry == 0x291beu);
    assert(receipt.game_title_dungeon_entry == 0x291c0u);
    assert(receipt.game_title_animation_plan_verified);
    assert(receipt.game_title_palettes_verified);
    assert(receipt.game_title_presents_palette_rgb6[15][0] == 63u);
    assert(receipt.game_title_presents_palette_rgb6[15][1] == 63u);
    assert(receipt.game_title_presents_palette_rgb6[15][2] == 63u);
    assert(receipt.game_title_zoom_palette_rgb6[3][0] == 47u);
    assert(receipt.game_title_zoom_palette_rgb6[3][1] == 39u);
    assert(receipt.game_title_zoom_palette_rgb6[3][2] == 15u);
    assert(receipt.game_title_zoom_palette_rgb6[15][0] == 63u);
    assert(receipt.game_title_zoom_palette_rgb6[15][1] == 0u);
    assert(receipt.game_title_zoom_palette_rgb6[15][2] == 0u);
    assert(dungeon_size == 33931u);
    memset(&dungeon_state, 0, sizeof(dungeon_state));
    memset(&dungeon_things, 0, sizeof(dungeon_things));
    /* The ordinary F0434 reader must continue to reject a missing checksum. */
    assert(!F0504_DUNGEON_LoadTailBuffer_Compat(
        dungeon, (int)dungeon_size, &dungeon_state, &dungeon_things));
    dungeon[dungeon_size - 1u] ^= 1u;
    assert(!F0504J_DUNGEON_LoadTailBufferFmTownsJp_Compat(
        dungeon, (int)dungeon_size, &dungeon_state, &dungeon_things));
    dungeon[dungeon_size - 1u] ^= 1u;
    assert(F0504J_DUNGEON_LoadTailBufferFmTownsJp_Compat(
        dungeon, (int)dungeon_size, &dungeon_state, &dungeon_things));
    assert(dungeon_state.loaded && dungeon_state.tilesLoaded);
    assert(dungeon_state.header.mapCount == 14u);
    assert(dungeon_state.header.textDataWordCount == 2004u);
    assert(dungeon_state.header.rawMapDataByteCount == 12347u);
    F0504_DUNGEON_FreeThingData_Compat(&dungeon_things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon_state);
    ok = 1;
done:
    free(image); free(autoexec); free(jdm); free(menu); free(icon); free(info);
    free(dungeon);
    if (!ok) return 1;
    puts("PASS: retail JDM title/palette plus hash-bound JDATA dungeon");
    return 0;
}
