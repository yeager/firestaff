#include "nexus_v1_prs3_structure2_intake.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0U;
    if (!path || !out_data || !out_size || strstr(path, "::")) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(file);
        return 0;
    }
    if (fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int default_nexus_dir(char *out, size_t out_size)
{
    const char *dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home;

    if (!out || out_size == 0U) return 0;
    if (dir && dir[0]) {
        return snprintf(out, out_size, "%s", dir) > 0;
    }
    home = getenv("HOME");
    if (!home || !home[0]) return 0;
    return snprintf(out, out_size, "%s/.firestaff/data/nexus", home) > 0;
}

static void test_real_menu_bpk_and_structure2_intake(void)
{
    enum {
        LEVEL_INDEX = 0
    };
    static const char menu_bpk_md5[] = "c2776768ff25287c79013a1452253ca0";
    static const char lev00_md5[] = "603ec9c531a92539babdda84ab09e78e";
    char data_dir[1024];
    char menu_path[1024];
    char level_path[1024];
    uint8_t *menu = NULL;
    uint8_t *level_bytes = NULL;
    size_t menu_size = 0U;
    size_t level_size = 0U;
    Nexus_V1_Level level;
    Nexus_V1_Prs3Structure2IntakeInput input;
    Nexus_V1_Prs3Structure2IntakeReceipt receipt;

    if (!default_nexus_dir(data_dir, sizeof(data_dir)) ||
        snprintf(menu_path, sizeof(menu_path), "%s/MENU.BPK", data_dir) <= 0 ||
        snprintf(level_path, sizeof(level_path), "%s/LEV00.DGN", data_dir) <= 0 ||
        !asset_file_matches_md5(menu_path, menu_bpk_md5) ||
        !asset_file_matches_md5(level_path, lev00_md5)) {
        puts("SKIP: canonical Nexus MENU.BPK/LEV00.DGN corpus not present");
        return;
    }
    CHECK(read_file(menu_path, &menu, &menu_size),
          "canonical MENU.BPK bytes read");
    CHECK(read_file(level_path, &level_bytes, &level_size),
          "canonical LEV00.DGN bytes read");
    if (!menu || !level_bytes) goto done;

    memset(&level, 0, sizeof(level));
    CHECK(nexus_v1_level_load(&level, level_bytes, (int)level_size,
                              LEVEL_INDEX) == 0,
          "canonical LEV00.DGN parses through Structure2 loader");

    memset(&input, 0, sizeof(input));
    input.menu_bpk = menu;
    input.menu_bpk_size = menu_size;
    input.menu_bpk_source_verified = 1;
    input.level = &level;
    input.level_index = LEVEL_INDEX;
    input.level_source_verified = 1;

    CHECK(nexus_v1_prs3_structure2_intake_admit(&input, &receipt) == 0,
          "real MENU.BPK PRS3 and LEV Structure2 intake blocked on decoded PRS3");
    CHECK(receipt.status == NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PRS3 &&
              strcmp(nexus_v1_prs3_structure2_intake_status_name(receipt.status),
                     "blocked-prs3") == 0,
          "intake status is blocked-prs3 because PRS3 is already decoded");
    CHECK(receipt.menu_bpk_archive_bound &&
              receipt.menu_bpk_entry_count == 163U &&
              receipt.menu_bpk_prs3_entry_count == 162U &&
              receipt.menu_bpk_trailer_entry_count == 1U,
          "canonical MENU.BPK directory and PRS3 counts are bound");
    CHECK(!receipt.prs3_framing_bound &&
              receipt.prs3_stream_plan_count == 0U,
          "PRS3 stream plans not evaluated after decoded PRS3 blocks intake");
    CHECK(receipt.palt_trailer_bound &&
              receipt.palt_trailer.valid &&
              receipt.palt_trailer.entry_count == 256U &&
              receipt.palt_trailer.entry_bytes == 512U &&
              receipt.palt_trailer.entry_bytes_fnv1a64 != 0U &&
              !receipt.palt_trailer.palette_format_proven &&
              !receipt.palt_trailer.decoder_promoted,
          "MENU.BPK PALT trailer is retained as opaque palette-source data");
    CHECK(!receipt.structure2_descriptor_bound &&
              !receipt.structure2_payload_envelope_bound &&
              !receipt.structure2_payload_anchor_intake_bound,
          "Structure2 intake not reached after decoded PRS3 blocks intake");
    CHECK(!receipt.structure2_pixel_span_proven &&
              !receipt.structure2_palette_addressing_proven &&
              !receipt.structure2_decoder_permitted &&
              !receipt.can_decode_prs3 &&
              !receipt.can_submit_structure2_pixels &&
              !receipt.can_submit_palette &&
              !receipt.runtime_render_permitted &&
              receipt.no_draw_only &&
              receipt.blocks_real_menu_surface_render &&
              receipt.blocks_real_dgn_mesh_render &&
              !receipt.fallback_visuals_permitted &&
              receipt.prs3_decoded_pixels_emitted == 0U,
          "real intake stays fail-closed for pixels, palette, render, and fallback");

    input.menu_bpk_source_verified = 0;
    CHECK(nexus_v1_prs3_structure2_intake_admit(&input, &receipt) == 0 &&
              receipt.status ==
                  NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_BPK_SOURCE,
          "unverified MENU.BPK source is rejected");
    input.menu_bpk_source_verified = 1;
    input.level_source_verified = 0;
    CHECK(nexus_v1_prs3_structure2_intake_admit(&input, &receipt) == 0 &&
              receipt.status ==
                  NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PRS3,
          "unverified Structure2 source blocked at PRS3 stage by decoded PRS3");

done:
    free(menu);
    free(level_bytes);
}

int main(void)
{
    test_real_menu_bpk_and_structure2_intake();
    if (failures) {
        fprintf(stderr, "Nexus PRS3/Structure2 intake: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus PRS3/Structure2 intake: PASS");
    return 0;
}
