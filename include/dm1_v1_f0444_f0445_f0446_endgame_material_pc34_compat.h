#ifndef FIRESTAFF_DM1_V1_F0444_F0445_F0446_ENDGAME_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0444_F0445_F0446_ENDGAME_MATERIAL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0444_GRAPHIC_THE_END_PC34 = 6,
    DM1_V1_F0444_GRAPHIC_CHAMPION_PORTRAITS_PC34 = 26,
    DM1_V1_F0444_GRAPHIC_CHAMPION_MIRROR_PC34 = 346,
    DM1_V1_F0444_GRAPHIC_CREDITS_PC34 = 5,
    DM1_V1_F0446_MUSIC_GAME_WON_PC34 = 2,
    DM1_V1_F0444_CREDITS_PALETTE_BYTE_COUNT_PC34 = 48
};

typedef struct DM1_V1_F0444EndgameGraphicPc34 {
    int graphic_index;
    const uint8_t *indexed_pixels;
    size_t indexed_pixel_byte_count;
    uint32_t graphics_dat_record_fingerprint;
    int decoded_from_original_graphics_dat;
    int raw_record_verified;
    int no_synthetic_surface;
} DM1_V1_F0444EndgameGraphicPc34;

typedef struct DM1_V1_F0444EndgameMaterialRequestPc34 {
    const DM1_V1_F0444EndgameGraphicPc34 *the_end;
    const DM1_V1_F0444EndgameGraphicPc34 *champion_portraits;
    const DM1_V1_F0444EndgameGraphicPc34 *champion_mirror;
    const DM1_V1_F0444EndgameGraphicPc34 *credits;
    const uint8_t *credits_palette_rgb6;
    size_t credits_palette_byte_count;
    uint32_t credits_palette_fingerprint;
    int original_credits_palette_verified;
    int no_host_font;
    int no_synthetic_credits;
} DM1_V1_F0444EndgameMaterialRequestPc34;

typedef struct DM1_V1_F0444EndgameMaterialReceiptPc34 {
    int accepted;
    int the_end_bound;
    int champion_portraits_bound;
    int champion_mirror_bound;
    int credits_bound;
    int credits_palette_bound;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0444EndgameMaterialReceiptPc34;

typedef struct DM1_V1_F0445FuseResourceRequestPc34 {
    const uint8_t *raw_dungeon_bytes;
    size_t raw_dungeon_byte_count;
    uint32_t dungeon_fingerprint;
    uint32_t timeline_fingerprint;
    int original_pc34_dungeon_verified;
    int original_timeline_verified;
    int pending_audio_route_verified;
    int no_synthetic_world;
} DM1_V1_F0445FuseResourceRequestPc34;

typedef struct DM1_V1_F0445FuseResourceReceiptPc34 {
    int accepted;
    int dungeon_bound;
    int timeline_bound;
    int pending_audio_route_bound;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0445FuseResourceReceiptPc34;

typedef struct DM1_V1_F0446FuseMaterialRequestPc34 {
    const DM1_V1_F0445FuseResourceReceiptPc34 *f0445_resource_receipt;
    const DM1_V1_F0444EndgameMaterialReceiptPc34 *f0444_material_receipt;
    const uint8_t *victory_music_bytes;
    size_t victory_music_byte_count;
    uint32_t victory_music_fingerprint;
    int music_track_id;
    int original_music_verified;
    int no_synthetic_music;
    int no_lifecycle_execution;
} DM1_V1_F0446FuseMaterialRequestPc34;

typedef struct DM1_V1_F0446FuseMaterialReceiptPc34 {
    int accepted;
    int f0445_resource_consumed;
    int f0444_material_consumed;
    int victory_music_bound;
    int victory_music_track_id;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0446FuseMaterialReceiptPc34;

int dm1_v1_f0444_endgame_material_admission_pc34(
    const DM1_V1_F0444EndgameMaterialRequestPc34 *request,
    DM1_V1_F0444EndgameMaterialReceiptPc34 *out_receipt);

int dm1_v1_f0445_fuse_resource_admission_pc34(
    const DM1_V1_F0445FuseResourceRequestPc34 *request,
    DM1_V1_F0445FuseResourceReceiptPc34 *out_receipt);

int dm1_v1_f0446_fuse_material_admission_pc34(
    const DM1_V1_F0446FuseMaterialRequestPc34 *request,
    DM1_V1_F0446FuseMaterialReceiptPc34 *out_receipt);

const char *dm1_v1_f0444_f0445_f0446_endgame_material_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0444_F0445_F0446_ENDGAME_MATERIAL_PC34_COMPAT_H */
