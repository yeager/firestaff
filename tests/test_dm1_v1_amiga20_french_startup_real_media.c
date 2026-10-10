#include "dm1_v1_amiga_entrance_f0441.h"
#include "dm1_v1_amiga_title_f0437.h"
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "dm1_v1_amiga_graphics_dat.h"
#include "firestaff_7z_extract.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_zip_extract.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dungeon_decompressor_ftl.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *graphics;
    size_t graphics_bytes;
    uint8_t *swsh;
    size_t swsh_bytes;
    uint8_t *dungeon;
    size_t dungeon_bytes;
} SelectedMedia;

static int selected_adf_member(const char *name, const uint8_t *bytes,
                               size_t byte_count, void *user_data) {
    SelectedMedia *media = (SelectedMedia *)user_data;
    uint8_t **target = NULL;
    size_t *target_bytes = NULL;
    if (strcmp(name, "graphics.dat") == 0 || strcmp(name, "GRAPHICS.DAT") == 0) {
        target = &media->graphics;
        target_bytes = &media->graphics_bytes;
    } else if (strcmp(name, "swoosh") == 0 || strcmp(name, "SWSH") == 0) {
        target = &media->swsh;
        target_bytes = &media->swsh_bytes;
    } else if (strcmp(name, "dungeon.dat") == 0 ||
               strcmp(name, "DUNGEON.DAT") == 0 ||
               strcmp(name, "Dungeon.dat") == 0) {
        target = &media->dungeon;
        target_bytes = &media->dungeon_bytes;
    }
    if (target) {
        if (*target || !byte_count) return -1;
        *target = (uint8_t *)malloc(byte_count);
        if (!*target) return -1;
        memcpy(*target, bytes, byte_count);
        *target_bytes = byte_count;
    }
    return 0;
}

static void media_free(SelectedMedia *media) {
    free(media->graphics);
    free(media->swsh);
    free(media->dungeon);
    memset(media, 0, sizeof(*media));
}

static int dungeon_world_from_original(const uint8_t *source,
                                      size_t source_bytes,
                                      size_t trim_before_normalize) {
    uint8_t *bytes = NULL;
    size_t byte_count = 0u;
    struct GameWorld_Compat world;
    int ok = 0;
    memset(&world, 0, sizeof(world));
    if (source_bytes >= 8u && source[0] == 0x81u && source[1] == 0x04u) {
        byte_count = ((size_t)source[2] << 24u) |
                     ((size_t)source[3] << 16u) |
                     ((size_t)source[4] << 8u) | source[5];
        if (!byte_count || byte_count > 1024u * 1024u) return 0;
        bytes = (uint8_t *)calloc(1u, byte_count);
        if (!bytes || !ftl_decompress_dungeon(
                source + 8u, source_bytes - 8u, bytes, (long)byte_count)) {
            free(bytes);
            return 0;
        }
    } else {
        byte_count = source_bytes;
        bytes = (uint8_t *)malloc(byte_count);
        if (!bytes) return 0;
        memcpy(bytes, source, byte_count);
    }
    if (trim_before_normalize > byte_count) goto cleanup;
    byte_count -= trim_before_normalize;
    if (!M11_GameView_NormalizeDm1BigEndianDungeon(&bytes, &byte_count) ||
        !F0882_WORLD_InitFromDungeonDatBuffer_Compat(
            bytes, (int)byte_count, 0xF1A5u, &world)) goto cleanup;
    F0883_WORLD_Free_Compat(&world);
    ok = 1;
cleanup:
    free(bytes);
    return ok;
}

int main(int argc, char **argv) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE");
    const char *a20e_archive = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    static const char french_adf[] =
        "Floppy Disks ADF/Dungeon Master for Amiga v2.0 (French) Cracked.adf";
    uint8_t *adf = NULL, *inner_zip = NULL, *english_adf = NULL;
    uint8_t *mutated = NULL;
    size_t adf_bytes = 0u, inner_zip_bytes = 0u, english_adf_bytes = 0u;
    SelectedMedia french = {0}, english = {0};
    DM1_V1_AmigaTitleF0437Receipt title;
    DM1_V1_AmigaEntranceF0441Receipt entrance;
    char frenchDungeonMd5[33];
    int result = 1;

    /* Test-only transport conversion: stream the selected original ADF to
     * stdout so the CLI/M12 shell test can wrap those exact bytes in a
     * private ZIP. No extracted disk is written or used by the runtime. */
    if (argc == 3 && strcmp(argv[1], "--emit-french-adf") == 0) {
        if (!firestaff_7z_read_member(argv[2], french_adf,
                                      &adf, &adf_bytes) ||
            adf_bytes != 901120u ||
            fwrite(adf, 1u, adf_bytes, stdout) != adf_bytes) {
            free(adf);
            fputs("FAIL: could not stream the authentic A20F ADF\n", stderr);
            return 1;
        }
        free(adf);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "--check-virtual") == 0) {
        char virtualPath[2048];
        uint8_t *graphics = NULL;
        size_t graphicsBytes = 0u;
        DM1_V1_AmigaGraphicsReceipt graphicsReceipt;
        if (snprintf(virtualPath, sizeof(virtualPath),
                     "%s::Dungeon Master v2.0 (1988)(FTL).zip::"
                     "Dungeon Master v2.0 (1988)(FTL).adf::GRAPHICS.DAT",
                     argv[2]) >= (int)sizeof(virtualPath) ||
            !asset_read_virtual_path_alloc(virtualPath, &graphics,
                                           &graphicsBytes) ||
            dm1_v1_amiga_graphics_receipt(graphics, graphicsBytes,
                                           &graphicsReceipt) != 0 ||
            graphicsReceipt.version != DM1_AMIGA_VER_2_0 ||
            graphicsReceipt.lang != DM1_AMIGA_LANG_FR) {
            free(graphics);
            fputs("FAIL: nested authentic French ADF virtual read rejected\n",
                  stderr);
            return 1;
        }
        free(graphics);
        puts("PASS: nested authentic French ADF virtual read");
        return 0;
    }
    if (argc != 1) {
        fputs("usage: test_dm1_v1_amiga20_french_startup_real_media [--emit-french-adf <archive> | --check-virtual <archive>]\n", stderr);
        return 2;
    }

    if (!archive || !*archive || !a20e_archive || !*a20e_archive) {
        puts("SKIP: authentic A20F and A20E archives are not configured");
        return 77;
    }
    {
        FILE *file = fopen(archive, "rb");
        if (!file) {
            puts("SKIP: authentic A20F archive is not staged");
            return 77;
        }
        fclose(file);
        file = fopen(a20e_archive, "rb");
        if (!file) {
            puts("SKIP: authentic A20E archive is not staged");
            return 77;
        }
        fclose(file);
    }
    if (!firestaff_7z_read_member(archive, french_adf, &adf, &adf_bytes) ||
        firestaff_amiga_adf_visit_ofs_files(
            adf, adf_bytes, selected_adf_member, &french) < 0 ||
        !french.graphics || !french.swsh || !french.dungeon ||
        firestaff_zip_extract_by_suffix(
            a20e_archive, "Dungeon Master v2.0 (1988)(FTL).zip",
            &inner_zip, &inner_zip_bytes) != 0 ||
        firestaff_zip_extract_memory_by_suffix(
            inner_zip, inner_zip_bytes,
            "Dungeon Master v2.0 (1988)(FTL).adf",
            &english_adf, &english_adf_bytes) != 0 ||
        firestaff_amiga_adf_visit_ofs_files(
            english_adf, english_adf_bytes,
            selected_adf_member, &english) < 0 ||
        !english.graphics || !english.swsh || !english.dungeon) {
        fputs("FAIL: could not read the selected authentic A20F/A20E ADFs\n", stderr);
        goto cleanup;
    }
    if (!m12_bytes_md5_hex(french.dungeon, french.dungeon_bytes,
                            frenchDungeonMd5) ||
        strcmp(frenchDungeonMd5,
               "c0e1bf1b9c5578681879d624bb28e663") != 0) {
        fputs("FAIL: authentic A20F Dungeon.dat receipt changed\n", stderr);
        goto cleanup;
    }
    if (!dungeon_world_from_original(french.dungeon,
                                     french.dungeon_bytes, 0u)) {
        fputs("FAIL: authentic A20F full-length dungeon did not initialize a DM1 world\n",
              stderr);
        goto cleanup;
    }
    if (!dungeon_world_from_original(english.dungeon, english.dungeon_bytes,
                                     2u)) {
        fputs("FAIL: authentic A20E dungeon checksum/runtime route changed\n",
              stderr);
        goto cleanup;
    }
    if (!dm1_v1_amiga_title_f0437_receipt(
            french.graphics, french.graphics_bytes,
            french.swsh, french.swsh_bytes, &title) ||
        strcmp(title.graphics_md5, "dd373954b3fb127db7387946131ea322") != 0 ||
        strcmp(title.executable_md5, "1038138978975415571a878bb08f54be") != 0 ||
        title.c001_width != 320u || title.c001_height != 200u ||
        title.source_zoom_steps != 18u ||
        !dm1_v1_amiga_entrance_f0441_receipt(
            french.graphics, french.graphics_bytes,
            french.swsh, french.swsh_bytes, &entrance) ||
        strcmp(entrance.graphics_md5, title.graphics_md5) != 0 ||
        strcmp(entrance.executable_md5, title.executable_md5) != 0 ||
        entrance.screen_width != 320u || entrance.screen_height != 200u ||
        entrance.left_door_width != 128u || entrance.left_door_height != 161u ||
        entrance.right_door_width != 128u || entrance.right_door_height != 161u ||
        entrance.opening_steps != 31u || entrance.switch_delay_ticks != 20u ||
        !entrance.mouse_input_only || !entrance.screen_nonzero_pixels ||
        !entrance.left_door_nonzero_pixels || !entrance.right_door_nonzero_pixels) {
        fputs("FAIL: authentic A20F F0437/F0441 source receipts rejected\n", stderr);
        goto cleanup;
    }
    if (dm1_v1_amiga_title_f0437_receipt(
            french.graphics, french.graphics_bytes,
            english.swsh, english.swsh_bytes, &title) ||
        dm1_v1_amiga_title_f0437_receipt(
            english.graphics, english.graphics_bytes,
            french.swsh, french.swsh_bytes, &title) ||
        dm1_v1_amiga_entrance_f0441_receipt(
            french.graphics, french.graphics_bytes,
            english.swsh, english.swsh_bytes, &entrance)) {
        fputs("FAIL: crossed authentic A20F/A20E files were admitted\n", stderr);
        goto cleanup;
    }
    mutated = (uint8_t *)malloc(french.graphics_bytes);
    if (!mutated) goto cleanup;
    memcpy(mutated, french.graphics, french.graphics_bytes);
    mutated[french.graphics_bytes - 1u] ^= 1u;
    if (dm1_v1_amiga_title_f0437_receipt(
            mutated, french.graphics_bytes,
            french.swsh, french.swsh_bytes, &title)) {
        fputs("FAIL: modified A20F GRAPHICS.DAT was admitted\n", stderr);
        goto cleanup;
    }
    free(mutated);
    mutated = (uint8_t *)malloc(french.swsh_bytes);
    if (!mutated) goto cleanup;
    memcpy(mutated, french.swsh, french.swsh_bytes);
    mutated[french.swsh_bytes - 1u] ^= 1u;
    if (dm1_v1_amiga_title_f0437_receipt(
            french.graphics, french.graphics_bytes,
            mutated, french.swsh_bytes, &title) ||
        dm1_v1_amiga_entrance_f0441_receipt(
            french.graphics, french.graphics_bytes,
            mutated, french.swsh_bytes, &entrance)) {
        fputs("FAIL: modified A20F SWSH was admitted\n", stderr);
        goto cleanup;
    }
    puts("PASS: authentic A20F title and entrance receipts; crossed and modified media rejected");
    result = 0;
cleanup:
    free(mutated);
    media_free(&french);
    media_free(&english);
    free(adf);
    free(english_adf);
    free(inner_zip);
    return result;
}
