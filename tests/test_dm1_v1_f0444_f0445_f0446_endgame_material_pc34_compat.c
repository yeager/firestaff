#include "dm1_v1_f0444_f0445_f0446_endgame_material_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static DM1_V1_F0444EndgameGraphicPc34 make_graphic(int index, const uint8_t *pixels)
{
    DM1_V1_F0444EndgameGraphicPc34 graphic;
    memset(&graphic, 0, sizeof(graphic));
    graphic.graphic_index = index;
    graphic.indexed_pixels = pixels;
    graphic.indexed_pixel_byte_count = 8u;
    graphic.graphics_dat_record_fingerprint = (uint32_t)(index + 1);
    graphic.decoded_from_original_graphics_dat = 1;
    graphic.raw_record_verified = 1;
    graphic.no_synthetic_surface = 1;
    return graphic;
}

int main(void)
{
    const uint8_t pixels[8] = {1};
    const uint8_t dungeon[8] = {2};
    const uint8_t music[8] = {3};
    uint8_t palette[DM1_V1_F0444_CREDITS_PALETTE_BYTE_COUNT_PC34] = {0};
    DM1_V1_F0444EndgameGraphicPc34 the_end = make_graphic(6, pixels);
    DM1_V1_F0444EndgameGraphicPc34 portraits = make_graphic(26, pixels);
    DM1_V1_F0444EndgameGraphicPc34 mirror = make_graphic(346, pixels);
    DM1_V1_F0444EndgameGraphicPc34 credits = make_graphic(5, pixels);
    DM1_V1_F0444EndgameMaterialRequestPc34 f0444;
    DM1_V1_F0444EndgameMaterialReceiptPc34 f0444_receipt;
    DM1_V1_F0445FuseResourceRequestPc34 f0445;
    DM1_V1_F0445FuseResourceReceiptPc34 f0445_receipt;
    DM1_V1_F0446FuseMaterialRequestPc34 f0446;
    DM1_V1_F0446FuseMaterialReceiptPc34 f0446_receipt;

    palette[0] = 63u;
    memset(&f0444, 0, sizeof(f0444));
    f0444.the_end = &the_end;
    f0444.champion_portraits = &portraits;
    f0444.champion_mirror = &mirror;
    f0444.credits = &credits;
    f0444.credits_palette_rgb6 = palette;
    f0444.credits_palette_byte_count = sizeof(palette);
    f0444.credits_palette_fingerprint = 1u;
    f0444.original_credits_palette_verified = 1;
    f0444.no_host_font = 1;
    f0444.no_synthetic_credits = 1;
    CHECK(dm1_v1_f0444_endgame_material_admission_pc34(&f0444, &f0444_receipt));
    CHECK(f0444_receipt.accepted && f0444_receipt.the_end_bound &&
          f0444_receipt.champion_mirror_bound && f0444_receipt.credits_palette_bound);

    memset(&f0445, 0, sizeof(f0445));
    f0445.raw_dungeon_bytes = dungeon;
    f0445.raw_dungeon_byte_count = sizeof(dungeon);
    f0445.dungeon_fingerprint = 2u;
    f0445.timeline_fingerprint = 3u;
    f0445.original_pc34_dungeon_verified = 1;
    f0445.original_timeline_verified = 1;
    f0445.pending_audio_route_verified = 1;
    f0445.no_synthetic_world = 1;
    CHECK(dm1_v1_f0445_fuse_resource_admission_pc34(&f0445, &f0445_receipt));
    CHECK(f0445_receipt.accepted && f0445_receipt.dungeon_bound &&
          f0445_receipt.timeline_bound && f0445_receipt.pending_audio_route_bound);

    memset(&f0446, 0, sizeof(f0446));
    f0446.f0445_resource_receipt = &f0445_receipt;
    f0446.f0444_material_receipt = &f0444_receipt;
    f0446.victory_music_bytes = music;
    f0446.victory_music_byte_count = sizeof(music);
    f0446.victory_music_fingerprint = 4u;
    f0446.music_track_id = 2;
    f0446.original_music_verified = 1;
    f0446.no_synthetic_music = 1;
    f0446.no_lifecycle_execution = 1;
    CHECK(dm1_v1_f0446_fuse_material_admission_pc34(&f0446, &f0446_receipt));
    CHECK(f0446_receipt.accepted && f0446_receipt.f0445_resource_consumed &&
          f0446_receipt.f0444_material_consumed && f0446_receipt.victory_music_track_id == 2);

    f0446.no_synthetic_music = 0;
    CHECK(!dm1_v1_f0446_fuse_material_admission_pc34(&f0446, &f0446_receipt));
    f0446.no_synthetic_music = 1;
    the_end.raw_record_verified = 0;
    CHECK(!dm1_v1_f0444_endgame_material_admission_pc34(&f0444, &f0444_receipt));
    CHECK(strstr(dm1_v1_f0444_f0445_f0446_endgame_material_source_evidence_pc34(), "F0446") != NULL);

    printf("test_dm1_v1_f0444_f0445_f0446_endgame_material_pc34_compat: %d assertions, %d failures\\n", assertions, failures);
    return failures == 0 ? 0 : 1;
}
