#include "dm1_v1_f0444_f0445_f0446_endgame_material_pc34_compat.h"

#include <string.h>

static int dm1_v1_nonzero_bytes_pc34(const uint8_t *bytes, size_t count)
{
    size_t index;
    for (index = 0u; index < count; ++index) {
        if (bytes[index] != 0u) return 1;
    }
    return 0;
}

static int dm1_v1_endgame_graphic_valid_pc34(
    const DM1_V1_F0444EndgameGraphicPc34 *graphic, int expected_index)
{
    return graphic && graphic->graphic_index == expected_index &&
        graphic->indexed_pixels && graphic->indexed_pixel_byte_count > 0u &&
        graphic->graphics_dat_record_fingerprint != 0u &&
        graphic->decoded_from_original_graphics_dat && graphic->raw_record_verified &&
        graphic->no_synthetic_surface &&
        dm1_v1_nonzero_bytes_pc34(graphic->indexed_pixels, graphic->indexed_pixel_byte_count);
}

static int dm1_v1_rgb6_palette_valid_pc34(const uint8_t *bytes, size_t count)
{
    size_t index;
    if (!bytes || count != DM1_V1_F0444_CREDITS_PALETTE_BYTE_COUNT_PC34 ||
        !dm1_v1_nonzero_bytes_pc34(bytes, count)) return 0;
    for (index = 0u; index < count; ++index) {
        if (bytes[index] > 63u) return 0;
    }
    return 1;
}

int dm1_v1_f0444_endgame_material_admission_pc34(
    const DM1_V1_F0444EndgameMaterialRequestPc34 *request,
    DM1_V1_F0444EndgameMaterialReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request ||
        !dm1_v1_endgame_graphic_valid_pc34(
            request->the_end, DM1_V1_F0444_GRAPHIC_THE_END_PC34) ||
        !dm1_v1_endgame_graphic_valid_pc34(
            request->champion_portraits, DM1_V1_F0444_GRAPHIC_CHAMPION_PORTRAITS_PC34) ||
        !dm1_v1_endgame_graphic_valid_pc34(
            request->champion_mirror, DM1_V1_F0444_GRAPHIC_CHAMPION_MIRROR_PC34) ||
        !dm1_v1_endgame_graphic_valid_pc34(
            request->credits, DM1_V1_F0444_GRAPHIC_CREDITS_PC34) ||
        request->credits_palette_fingerprint == 0u ||
        !request->original_credits_palette_verified || !request->no_host_font ||
        !request->no_synthetic_credits ||
        !dm1_v1_rgb6_palette_valid_pc34(
            request->credits_palette_rgb6, request->credits_palette_byte_count)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->the_end_bound = 1;
        out_receipt->champion_portraits_bound = 1;
        out_receipt->champion_mirror_bound = 1;
        out_receipt->credits_bound = 1;
        out_receipt->credits_palette_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0444_f0445_f0446_endgame_material_source_evidence_pc34();
    }
    return 1;
}

int dm1_v1_f0445_fuse_resource_admission_pc34(
    const DM1_V1_F0445FuseResourceRequestPc34 *request,
    DM1_V1_F0445FuseResourceReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !request->raw_dungeon_bytes || request->raw_dungeon_byte_count == 0u ||
        request->dungeon_fingerprint == 0u || request->timeline_fingerprint == 0u ||
        !request->original_pc34_dungeon_verified || !request->original_timeline_verified ||
        !request->pending_audio_route_verified || !request->no_synthetic_world ||
        !dm1_v1_nonzero_bytes_pc34(request->raw_dungeon_bytes, request->raw_dungeon_byte_count)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->dungeon_bound = 1;
        out_receipt->timeline_bound = 1;
        out_receipt->pending_audio_route_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0444_f0445_f0446_endgame_material_source_evidence_pc34();
    }
    return 1;
}

int dm1_v1_f0446_fuse_material_admission_pc34(
    const DM1_V1_F0446FuseMaterialRequestPc34 *request,
    DM1_V1_F0446FuseMaterialReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !request->f0445_resource_receipt ||
        !request->f0445_resource_receipt->accepted ||
        !request->f0444_material_receipt || !request->f0444_material_receipt->accepted ||
        !request->victory_music_bytes || request->victory_music_byte_count == 0u ||
        request->victory_music_fingerprint == 0u ||
        request->music_track_id != DM1_V1_F0446_MUSIC_GAME_WON_PC34 ||
        !request->original_music_verified || !request->no_synthetic_music ||
        !request->no_lifecycle_execution ||
        !dm1_v1_nonzero_bytes_pc34(request->victory_music_bytes, request->victory_music_byte_count)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->f0445_resource_consumed = 1;
        out_receipt->f0444_material_consumed = 1;
        out_receipt->victory_music_bound = 1;
        out_receipt->victory_music_track_id = DM1_V1_F0446_MUSIC_GAME_WON_PC34;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0444_f0445_f0446_endgame_material_source_evidence_pc34();
    }
    return 1;
}

const char *dm1_v1_f0444_f0445_f0446_endgame_material_source_evidence_pc34(void)
{
    return "ReDMCSB ENDGAME.C:440-680 F0444 consumes original C006 THE END, "
           "C026 portraits, C346 mirror, C005 credits, and G0019 palette; "
           "ENDGAME.C:742-759 F0445 consumes timeline/dungeon/pending-audio state; "
           "ENDGAME.C:924-961 F0446 selects C2_MUSIC_GAME_WON before F0444(TRUE).";
}
