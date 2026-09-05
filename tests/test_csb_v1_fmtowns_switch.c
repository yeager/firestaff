#include "csb_v1_fmtowns_switch.h"
#include "csb_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, text) do { \
    if (condition) ++passed; else { ++failed; printf("FAIL: %s\\n", text); } \
} while (0)

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes = NULL;
    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

int main(int argc, char **argv)
{
    const char *path = argc == 2 ? argv[1] : getenv("FIRESTAFF_CSB_FMTOWNS_SWITCH");
    const char *data_dir;
    const char *home;
    char inferred_path[1024];
    char default_archive[1024];
    CSB_V1_BootStartupLaunch_PC34 launch;
    CSB_V1_FmtownsSwitchReceipt receipt;
    CSB_V1_FmtownsItemDecodeReceipt page;
    CSB_V1_FmtownsSwitchInputReceipt click;
    uint8_t pixels[CSB_FMTOWNS_SWITCH_PIXELS];
    uint8_t *bytes;
    size_t byte_count;
    size_t index;
    int packed_launch = 0;

    memset(&launch, 0, sizeof(launch));

    /* The native CLI and M11 real-media tests already use this selected F31
     * root.  Reuse it when a stand-alone SWITCHTW path was not supplied so a
     * normal real-data lane proves the actual switch-page decoder instead of
     * reporting a misleading skip.  An explicit argv/env path remains useful
     * for isolated files and takes precedence. */
    if (!path && (data_dir = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR")) &&
        data_dir[0] != '\0' &&
        snprintf(inferred_path, sizeof(inferred_path), "%s/SWITCHTW.EXP",
                 data_dir) > 0 && strlen(inferred_path) < sizeof(inferred_path)) {
        path = inferred_path;
    }
    bytes = path ? read_file(path, &byte_count) : NULL;
    data_dir = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    home = getenv("HOME");
    if (!bytes && (!data_dir || data_dir[0] == '\0') && home && home[0] != '\0' &&
        snprintf(default_archive, sizeof(default_archive),
                 "%s/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip",
                 home) > 0 && strlen(default_archive) < sizeof(default_archive)) {
        data_dir = default_archive;
    }
    if (!bytes && data_dir && data_dir[0] != '\0' &&
        csb_v1_boot_startup_launch_alloc_with_variant_pc34(
            data_dir, NULL, NULL, NULL, NULL, CSB_V1_VARIANT_FMTOWNS_EN,
            &launch) && launch.profile && launch.profile->fmtowns_switch_bytes &&
        launch.profile->fmtowns_switch_size > 0u) {
        byte_count = launch.profile->fmtowns_switch_size;
        bytes = (uint8_t *)malloc(byte_count);
        if (bytes) {
            memcpy(bytes, launch.profile->fmtowns_switch_bytes, byte_count);
            packed_launch = 1;
        }
    }
    if (!bytes) {
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        puts("SKIP: original SWITCHTW.EXP unavailable in loose or packed F31 media");
        return 77;
    }
    CHECK(csb_v1_fmtowns_switch_parse(bytes, byte_count, &receipt),
          "recognises the complete F31E/F31J resource sequence");
    CHECK(receipt.valid && receipt.japanese_page.width == 320u &&
          receipt.english_page.height == 200u, "keeps both original pages");
    CHECK(receipt.japanese_page_byte_count == 7314u &&
          receipt.english_page_byte_count == 6541u,
          "keeps SWITCHDA.C stream boundaries");
    CHECK(receipt.japanese_page_offset < receipt.english_page_offset,
          "preserves executable resource ordering");
    CHECK(receipt.language_buttons[CSB_FMTOWNS_SWITCH_JAPANESE].source_byte_count ==
              304u &&
          receipt.language_buttons[CSB_FMTOWNS_SWITCH_ENGLISH].source_byte_count ==
              108u,
          "keeps the distinct fourth-button streams for both languages");
    CHECK(receipt.palette_offset < receipt.japanese_page_offset &&
          receipt.palette_byte_count == 68u && receipt.palette[8].red6 == 0x3fu &&
          receipt.palette[8].green6 == 0x3fu && receipt.palette[8].blue6 == 0u,
          "binds C26_SWITCH from the original executable");
    for (index = 0u; index < CSB_FMTOWNS_SWITCH_BUTTON_COUNT; ++index) {
        CHECK(receipt.buttons[index].image.valid &&
              receipt.buttons[index].width != 0u && receipt.buttons[index].height != 0u,
              "decodes a source-owned button");
    }
    CHECK(csb_v1_fmtowns_switch_decode_page(bytes, byte_count, &receipt,
                                            CSB_FMTOWNS_SWITCH_ENGLISH,
                                            pixels, sizeof(pixels), &page),
          "decodes the original English page");
    CHECK(page.pixel_fnv1a != 0u && page.pixel_fnv1a != receipt.japanese_page.pixel_fnv1a,
          "English page remains distinct from Japanese page");
    CHECK(csb_v1_fmtowns_switch_compose_page(
              bytes, byte_count, &receipt, CSB_FMTOWNS_SWITCH_ENGLISH,
              pixels, sizeof(pixels)),
          "composes the English page with its source-owned buttons");
    CHECK(csb_v1_fmtowns_switch_route_click(&receipt, CSB_FMTOWNS_SWITCH_ENGLISH,
                                            52, 15, 1, &click) && click.valid &&
          click.button_index == 0u && click.source_exit_status == 4u &&
          click.action == CSB_FMTOWNS_SWITCH_ACTION_STORY,
          "English first button routes to AUTOEXEC Story exit 4");
    CHECK(csb_v1_fmtowns_switch_route_click(&receipt, CSB_FMTOWNS_SWITCH_JAPANESE,
                                            57, 59, 1, &click) &&
          click.source_exit_status == 2u &&
          click.action == CSB_FMTOWNS_SWITCH_ACTION_UTILITY,
          "Japanese second button routes to utility exit 2");
    CHECK(csb_v1_fmtowns_switch_route_click(&receipt, CSB_FMTOWNS_SWITCH_ENGLISH,
                                            50, 150, 1, &click) &&
          click.source_exit_status == 0u &&
          click.action == CSB_FMTOWNS_SWITCH_ACTION_TOGGLE_LANGUAGE,
          "fourth button toggles language without an exit status");
    CHECK(!csb_v1_fmtowns_switch_route_click(&receipt, CSB_FMTOWNS_SWITCH_ENGLISH,
                                             0, 0, 1, &click),
          "does not invent a hit outside source button rectangles");
    CHECK(!csb_v1_fmtowns_switch_parse(bytes, 100u, &receipt),
          "rejects truncated executable");
    free(bytes);
    if (packed_launch) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
    printf("csb_v1_fmtowns_switch: %d/%d assertions passed\n", passed, passed + failed);
    return failed ? 1 : 0;
}
