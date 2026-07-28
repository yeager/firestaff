#ifndef FIRESTAFF_CSB_V1_ATARI_ST_ANIMATION_ASSETS_H
#define FIRESTAFF_CSB_V1_ATARI_ST_ANIMATION_ASSETS_H

#include <stddef.h>
#include <stdint.h>

enum {
    CSB_V1_ATARI_ST_ANIMATION_WIDTH = 320,
    CSB_V1_ATARI_ST_ANIMATION_HEIGHT = 200,
    CSB_V1_ATARI_ST_ANIMATION_RGBA_BYTES =
        CSB_V1_ATARI_ST_ANIMATION_WIDTH * CSB_V1_ATARI_ST_ANIMATION_HEIGHT * 4,
    CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES =
        CSB_V1_ATARI_ST_ANIMATION_WIDTH * CSB_V1_ATARI_ST_ANIMATION_HEIGHT,
    CSB_V1_ATARI_ST_ANIMATION_MAX_PRESENTED_FRAMES = 4,
    CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS = 4
};

typedef struct {
    int valid;
    uint16_t data_item_count;
    uint16_t script_instruction_count;
    uint16_t palette_load_count;
    uint16_t image_load_count;
    uint16_t sound_load_count;
    uint16_t invalid_load_count;
} CSB_V1_AtariStAnimationAssetReceipt;

typedef struct {
    int valid;
    uint32_t executed_instruction_count;
    uint32_t waited_vbl_count;
    uint16_t fade_count;
    uint16_t expand_count;
    uint16_t blit_count;
    uint16_t present_count;
    uint16_t played_sound_count;
    uint16_t last_presented_image_item;
    uint16_t last_presented_palette_item;
    uint16_t final_active_image_item;
    uint16_t final_palette_item;
    uint16_t presented_image_items[CSB_V1_ATARI_ST_ANIMATION_MAX_PRESENTED_FRAMES];
    uint16_t presented_palette_items[CSB_V1_ATARI_ST_ANIMATION_MAX_PRESENTED_FRAMES];
    uint32_t presented_vbls[CSB_V1_ATARI_ST_ANIMATION_MAX_PRESENTED_FRAMES];
    uint16_t played_sound_items[CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS];
    uint16_t played_sound_periods[CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS];
    uint32_t played_sound_vbls[CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS];
    uint16_t failed_opcode;
    uint16_t failed_instruction_index;
} CSB_V1_AtariStAnimationTraceReceipt;

/* Decode one documented Atari ST P4B1 palette: 16 big-endian ST RGB words.
 * Each output entry is RGB888. */
int csb_v1_atari_st_animation_decode_p4b1_palette(
    const uint8_t *bytes, size_t byte_count, uint8_t out_rgb[16][3]);

/* Verify every Load item in an original Atari ST ANIMATE.SCR against the
 * corresponding 87-item ANIMATE.DAT family. No substitute assets are used. */
int csb_v1_atari_st_animation_validate_assets(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    CSB_V1_AtariStAnimationAssetReceipt *out_receipt);

/* Execute the documented ANIMATE.SCR control flow against its original
 * ANIMATE.DAT item families. This is a semantic player trace: it validates
 * slot ownership, loops, fades, waits, expands, blits, present calls and
 * sound calls without inventing a PC34 presentation sequence. */
int csb_v1_atari_st_animation_trace_script(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Resolve and rasterize the final Atari ST animation screen through the
 * original script state. The selected IMG1/P4B1 pair must come from the
 * ANIMATE.SCR sequence; callers do not supply substitute item numbers. */
int csb_v1_atari_st_animation_render_final_rgba(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint8_t *out_rgba, size_t out_rgba_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Rasterize one actual Set-screen instruction from the original script.
 * presentation_index is zero-based and bounded by the trace's present_count. */
int csb_v1_atari_st_animation_render_presented_rgba(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint16_t presentation_index, uint8_t *out_rgba, size_t out_rgba_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Decode a source-selected presentation directly to its original 4-bit pixel
 * indices and P4B1 RGB palette. This is the M11 host-palette handoff. */
int csb_v1_atari_st_animation_decode_presented_indexed(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint16_t presentation_index,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Replay the documented image-memory portion of ANIMATE.SCR through a source
 * VBlank. This owns IMG1 expansion, empty image copies, display coordinates,
 * and transparent blits into the active Atari screen; it never synthesizes a
 * substitute PC34 surface. */
int csb_v1_atari_st_animation_decode_frame_at_vbl_indexed(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint32_t target_vbl,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Discover, materialize and render the final original Atari ST animation
 * frame from one data root. This is the launcher-facing route and accepts
 * loose files or supported archive entries without caller-supplied assets. */
int csb_v1_atari_st_animation_render_final_from_root_rgba(
    const char *search_root, const char *cache_root, uint8_t *out_rgba,
    size_t out_rgba_size, CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

int csb_v1_atari_st_animation_render_presented_from_root_rgba(
    const char *search_root, const char *cache_root,
    uint16_t presentation_index, uint8_t *out_rgba, size_t out_rgba_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

int csb_v1_atari_st_animation_decode_presented_from_root_indexed(
    const char *search_root, const char *cache_root,
    uint16_t presentation_index,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

int csb_v1_atari_st_animation_decode_frame_at_vbl_from_root_indexed(
    const char *search_root, const char *cache_root, uint32_t target_vbl,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Read the original script's complete timing/ownership trace from one data
 * root.  The final waited VBlank is the source-owned handoff point where
 * ANIM.C launches FTLCODE; no PC34 title sequence is implied. */
int csb_v1_atari_st_animation_trace_from_root(
    const char *search_root, const char *cache_root,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Copy one SND1 payload selected by a real Play sound instruction. The
 * caller supplies storage; no filename or item number is accepted here, so
 * a host cannot replace an animation sound with arbitrary data. */
int csb_v1_atari_st_animation_copy_played_sound_from_root(
    const char *search_root, const char *cache_root, uint16_t sound_index,
    uint8_t *out_bytes, size_t out_capacity, size_t *out_size,
    uint16_t *out_period, uint32_t *out_vbl,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt);

/* Render a full-screen original IMG1 item with an original P4B1 palette into
 * a 320x200 RGBA frame. image_item must be a documented full-screen image
 * and palette_item must be a documented 32-byte palette item. */
int csb_v1_atari_st_animation_render_rgba(
    const char *animate_dat_path, uint16_t image_item, uint16_t palette_item,
    uint8_t *out_rgba, size_t out_rgba_size);

#endif
