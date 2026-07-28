#ifndef NEXUS_V1_UI_SURFACES_H
#define NEXUS_V1_UI_SURFACES_H
#include <stdint.h>
#include <stddef.h>
#include "nexus_v1_bpk_archive.h"

/* Nexus V1 UI / Title Surface Renderer
 * ===================================
 * Parses and renders DM Nexus Saturn UI screens and surface assets:
 *   TITLE.CG   — title screen color graphics (164 KB)
 *   WARNING.BIN — warning/disclaimer screen  (99 KB)
 *   GAMEOVER.BIN — game over screen         (101 KB)
 *   FACE.BIN    — champion portrait sprites (44 KB, 24 entries)
 *   STABG.BIN   — status-area background  (52 KB)
 *   FONT256.S2D — Saturn SCR font           (already nexus_v1_saturn_font.c)
 *
 * Rendering entry points:
 *   nexus_ui_render_title()    — blit TITLE.CG to framebuffer
 *   nexus_ui_render_warning()  — blit WARNING.BIN
 *   nexus_ui_render_gameover() — blit GAMEOVER.BIN
 *   nexus_ui_render_portrait() — blit FACE.BIN entry
 *   nexus_ui_render_stabg()    — blit STABG.BIN as status area bg
 *
 * Source-lock references:
 *   ReDMCSB BLIT.C      — F0132 blit rect (F0132)
 *   ReDMCSB PANEL.C    — panel draw (F0120-F0125)
 *   ReDMCSB CEDTINCK.C — CEDT font/text render
 *   Saturn SDK         — VDP1 bitmap surfaces, VDP2 background layers
 *   docs/NEXUS_FILE_CLASSIFICATION.md  — file sizes / formats
 *
 * Missing, truncated, or unsupported media remains unavailable. No solid
 * placeholder, generated surface, or substitute palette is permitted. */

/* ── Surface descriptor ───────────────────────────────────────── */
#define NEXUS_UI_MAX_SURFACES 8

typedef struct {
    uint8_t  *data;       /* indexed pixels (320×200) or texture bitmap */
    int       w, h;        /* dimensions */
    uint8_t   pal_start;  /* first palette slot for this surface */
    uint8_t   pal_count;  /* number of palette entries */
    int       owns_data;  /* 1=calloc'd, 0=borrowed ref */
    const char *source;   /* e.g. "TITLE.CG" */
    uint64_t  hash;       /* SHA-256 hash of source file (if known) */
    /* DGT2 PP surfaces retain their source-owned 256-entry BGR555 CLUT as
     * RGBA. Other formats leave this unavailable rather than borrowing a
     * host or generated palette. */
    uint32_t  dgt2_palette_rgba[256];
    uint32_t  dgt2_palette_fnv1a32;
    int       dgt2_palette_loaded;
} Nexus_UI_Surface;

/* Named surfaces */
typedef enum {
    NEXUS_SURFACE_TITLE = 0,
    NEXUS_SURFACE_WARNING,
    NEXUS_SURFACE_GAMEOVER,
    NEXUS_SURFACE_FACE0,    /* portrait 0-23 */
    NEXUS_SURFACE_FACE23 = NEXUS_SURFACE_FACE0 + 23,
    NEXUS_SURFACE_STABG,    /* status area background */
    NEXUS_SURFACE_COUNT
} Nexus_UISurfaceType;

typedef struct {
    Nexus_UI_Surface surfaces[NEXUS_SURFACE_COUNT];
} Nexus_UI_Manager;

typedef enum {
    NEXUS_UI_BPK_IMPORT_OK = 0,
    NEXUS_UI_BPK_IMPORT_ERR_NULL = -1,
    NEXUS_UI_BPK_IMPORT_ERR_NOT_READY = -2,
    NEXUS_UI_BPK_IMPORT_ERR_EXTRACT = -3,
    NEXUS_UI_BPK_IMPORT_ERR_LOAD = -4
} Nexus_UI_BpkImportStatus;

typedef struct {
    int loaded;
    int blocked_prs3;
    int blocked_truncated;
    int blocked_not_ready;
    uint32_t entry_index;
    uint32_t payload_offset;
    uint32_t bytes_loaded;
    int width;
    int height;
    Nexus_V1_BpkSurfaceClass surface_class;
} Nexus_UI_BpkImportReceipt;

/* Sega Saturn DGT2 packed-pixel image view. The pixel and CLUT pointers
 * borrow the source container; callers keep it alive while using the view. */
typedef struct {
    const uint8_t *pixels;
    const uint8_t *clut_bgr555_be;
    size_t pixel_bytes;
    int width;
    int height;
} Nexus_UI_Dgt2PpView;

/* STABG.BIN "STMP" container framing, proven against the retail file
 * (SHA-256 7b8e44ff…, 53744 bytes): "STMP" magic, a big-endian u32
 * declaring the exact file size, and three (offset, length) region pairs
 * tiling [0x20, EOF): a tile-map directory, a 256-entry big-endian u16
 * CLUT, and 4bpp pixel data. The directory starts with a zero-terminated
 * u32 table of offsets (relative to the directory base) followed by a
 * contiguous run of (u16 w, u16 h, w*h BE u16 cells) maps whose cells
 * are word offsets (×2 bytes) into the pixel region. The first map is
 * the 40x21-cell status-area background (320x168 at 8px cells).
 * Pixel-unit decode semantics are NOT proven; this receipt proves the
 * container framing and bounded cell references only. */
#define NEXUS_UI_STABG_STMP_MAX_MAPS 64
typedef struct {
    int valid;
    uint32_t declared_size;
    uint32_t directory_offset;
    uint32_t directory_size;
    uint32_t palette_offset;
    uint32_t palette_size;
    uint32_t pixels_offset;
    uint32_t pixels_size;
    int map_count;
    int background_cell_w;
    int background_cell_h;
    uint32_t max_cell_word_offset;
    int cell_offsets_bounded;
} Nexus_UI_StabgStmpFraming;

/* Parse and bound-check the STABG.BIN STMP framing. Returns 0 and fills
 * out_framing (valid=1) only when every structural invariant holds. */
int nexus_ui_stabg_stmp_framing_receipt(const uint8_t *data,
                                        int data_size,
                                        Nexus_UI_StabgStmpFraming *out_framing);

/* TITLE.CG on the verified Saturn disc is a 32-byte zero prefix followed by
 * a 4bpp, high-nibble-first 328x1024 atlas. */
#define NEXUS_UI_TITLE_CG_HEADER_BYTES 32U
#define NEXUS_UI_TITLE_CG_WIDTH 328
#define NEXUS_UI_TITLE_CG_HEIGHT 1024
#define NEXUS_UI_TITLE_CG_PACKED_BYTES \
    ((size_t)NEXUS_UI_TITLE_CG_WIDTH * (size_t)NEXUS_UI_TITLE_CG_HEIGHT / 2U)
#define NEXUS_UI_TITLE_CG_BYTES \
    (NEXUS_UI_TITLE_CG_HEADER_BYTES + NEXUS_UI_TITLE_CG_PACKED_BYTES)

typedef struct {
    int valid;
    int header_size;
    int entry_count;
    int entry_size;
    int portrait_w;
    int portrait_h;
} Nexus_UI_FaceLayout;

typedef enum {
    NEXUS_UI_FACE_RECORD_NONE = 0,
    NEXUS_UI_FACE_RECORD_RAW_48X48,
    NEXUS_UI_FACE_RECORD_COMPACT_PADDED,
    NEXUS_UI_FACE_RECORD_PRS3_UNPROVEN,
    NEXUS_UI_FACE_RECORD_PRS3_DECODED
} Nexus_UI_FaceRecordDecodeKind;

typedef struct {
    Nexus_UI_FaceRecordDecodeKind kind;
    int valid;
    int face_index;
    int record_offset;
    int record_size;
    size_t prefix_offset;
    int prefix_size;
    size_t prs3_offset;
    size_t prs3_size;
    size_t stream_offset;
    size_t stream_size;
    uint32_t prs3_version;
    uint32_t pixel_count;
    uint32_t declared_pixel_count;
    int payload_offset;
    int payload_size;
    int width;
    int height;
    int blocks_real_portrait_decode;
} Nexus_UI_FaceCompactRecordDescriptor;

/* Whole-file PRS3 framing evidence for retail FACE.BIN. It records only
 * authenticated stream boundaries and byte witnesses, never token grammar,
 * decoded pixels, prefix-palette semantics, or a drawable portrait. */
typedef struct {
    int valid;
    int frame_count;
    size_t source_byte_count;
    uint64_t source_bytes_fnv1a64;
    uint64_t prs3_headers_fnv1a64;
    uint64_t stream_bytes_fnv1a64;
    size_t total_stream_byte_count;
    uint64_t declared_total_pixel_count;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_UI_FacePrs3CorpusReceipt;

/* One capture-producer request for a canonical FACE.BIN PRS3 frame. The
 * caller must separately attest the original FACE.BIN source and any Saturn
 * execution trace. Prefix, PRS3 header, and stream hashes deliberately stay
 * separate: none establishes a palette, token grammar, decoded pixels, or a
 * drawable portrait. */
typedef struct {
    int valid;
    int face_index;
    size_t source_byte_count;
    uint64_t source_bytes_fnv1a64;
    Nexus_UI_FaceCompactRecordDescriptor descriptor;
    uint64_t prefix_bytes_fnv1a64;
    uint64_t prs3_header_fnv1a64;
    uint64_t stream_bytes_fnv1a64;
    int capture_producer_required;
    int original_saturn_capture_required;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_UI_FacePrs3CaptureTarget;

/* Ordered source-only campaign for every canonical FACE.BIN PRS3 frame. The
 * ledger is for acquiring a complete external trace set, not for admitting a
 * decoder: it records byte identity and producer order only. */
typedef struct {
    int valid;
    int frame_count;
    size_t source_byte_count;
    uint64_t source_bytes_fnv1a64;
    uint64_t ordered_target_fnv1a64;
    uint64_t source_lanes_fnv1a64;
    size_t total_stream_byte_count;
    int capture_producer_required;
    int original_saturn_capture_required;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_UI_FacePrs3CaptureCampaignReceipt;

typedef struct {
    Nexus_UI_FaceRecordDecodeKind kind;
    int source_size;
    int copied_pixels;
    int zero_padded_pixels;
    int portrait_w;
    int portrait_h;
} Nexus_UI_FaceRecordDecodeInfo;

/* ── Manager lifecycle ─────────────────────────────────────────── */
void nexus_ui_manager_init(Nexus_UI_Manager *mgr);
void nexus_ui_manager_free(Nexus_UI_Manager *mgr);

/* ── Surface loaders ───────────────────────────────────────────── */

/* Load a generic indexed surface (320×200 CBUF or any w×h bitmap).
 * data may be NULL → fills with deterministic dark color + logs diag. */
int nexus_ui_surface_load(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which,
    const uint8_t *data, int data_size,
    int w, int h,
    uint8_t pal_start, uint8_t pal_count,
    const char *source);

int nexus_ui_load_bpk_runtime_surface(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which,
    const uint8_t *archive_data, size_t archive_size,
    const Nexus_V1_BpkRuntimeSurfaceHandoff *handoff,
    uint8_t pal_start, uint8_t pal_count,
    const char *source,
    Nexus_UI_BpkImportReceipt *out_receipt);

const char *nexus_ui_bpk_import_status_name(int status);

/* Decode one documented Sega DGT2 PP image. This accepts an image payload
 * beginning with "PP", not its eight-byte RES* directory record. */
int nexus_ui_dgt2_pp_view(const uint8_t *data,
                          size_t data_size,
                          Nexus_UI_Dgt2PpView *out_view);

/* Find and decode an id-addressed DGT2 PP resource from a Saturn RES*
 * container. The local WARNING.BIN uses this directory form. */
int nexus_ui_res_dgt2_pp_view(const uint8_t *data,
                              size_t data_size,
                              uint32_t resource_id,
                              Nexus_UI_Dgt2PpView *out_view);

/* Convert the documented big-endian BGR555 DGT2 CLUT to the host's RGBA
 * framebuffer representation. The caller must provide all 256 entries. */
int nexus_ui_dgt2_pp_palette_rgba(const Nexus_UI_Dgt2PpView *view,
                                  uint32_t out_palette[256]);

/* Decode the verified Saturn TITLE.CG atlas. The loader rejects any shape
 * other than the observed 32-byte zero prefix plus packed 4bpp payload. */
int nexus_ui_load_title(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load WARNING.BIN's documented RES* container DGT2 resource 0 as disclaimer art. */
int nexus_ui_load_warning(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load GAMEOVER.BIN (101 KB) as game over screen */
int nexus_ui_load_gameover(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load STABG.BIN (52 KB) as status-area background (200×52 or 320×200) */
int nexus_ui_load_stabg(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load FACE.BIN (44 KB) as champion portraits.
 * Layout: 24 portraits laid out in a horizontal strip.
 * Each portrait: 48×48 pixels (or closest power-of-2).
 * face_index: 0..23 → portrait number. */
int nexus_ui_load_faces(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_of_face,
    int data_size, int face_index,
    int portrait_w, int portrait_h,
    const uint32_t *palette);

int nexus_ui_face_full_entry_count(int data_size, int portrait_w, int portrait_h);
int nexus_ui_face_layout_detect(const uint8_t *data,
    int data_size,
    Nexus_UI_FaceLayout *out_layout);
int nexus_ui_face_compact_record_descriptor(
    const uint8_t *data,
    int data_size,
    int face_index,
    Nexus_UI_FaceCompactRecordDescriptor *out_descriptor);
int nexus_ui_face_prs3_corpus_receipt(const uint8_t *data,
                                      int data_size,
                                      Nexus_UI_FacePrs3CorpusReceipt *out_receipt);

/* Build a no-draw capture request for one exact canonical FACE.BIN frame.
 * `source_hash_verified` must be owned by the retail asset scanner; a
 * structurally valid arbitrary FACE container is not sufficient. */
int nexus_ui_face_prs3_capture_target(const uint8_t *data,
                                      int data_size,
                                      int face_index,
                                      int source_hash_verified,
                                      Nexus_UI_FacePrs3CaptureTarget *out_target);

/* Construct a complete ordered capture campaign for all canonical FACE.BIN
 * PRS3 frames. It requires scanner-owned source identity and retains no
 * loader, token, palette, or rendering semantics. */
int nexus_ui_face_prs3_capture_campaign(
    const uint8_t *data,
    int data_size,
    int source_hash_verified,
    Nexus_UI_FacePrs3CaptureCampaignReceipt *out_receipt);
int nexus_ui_expand_face_record_48x48(const uint8_t *record_data,
    int record_size,
    uint8_t *out_pixels,
    int out_size,
    Nexus_UI_FaceRecordDecodeInfo *out_info);
int nexus_ui_load_face_record(Nexus_UI_Manager *mgr,
    const uint8_t *record_data,
    int record_size,
    int face_index,
    int portrait_w,
    int portrait_h,
    const uint32_t *palette);
int nexus_ui_load_face_placeholder(Nexus_UI_Manager *mgr,
    int face_index, int portrait_w, int portrait_h);

/* Free a specific surface */
void nexus_ui_surface_free(Nexus_UI_Manager *mgr, Nexus_UISurfaceType which);

/* ── Rendering entry points ────────────────────────────────────── */

/* Simple 1:1 blit from surface → indexed framebuffer (320×200).
 * dx, dy = destination top-left in framebuffer.
 * Clips to framebuffer bounds.                               */
void nexus_ui_blit_surface(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h, int dx, int dy);

/* Blit with optional horizontal flip (for mirror-portrait champions) */
void nexus_ui_blit_surface_flip(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h, int dx, int dy, int flip_h);

/* Convenience wrappers using the global manager */
void nexus_ui_render_title(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h);
void nexus_ui_render_warning(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h);
void nexus_ui_render_gameover(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h);
void nexus_ui_render_stabg(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h,
    int dest_x, int dest_y);
/* portrait_index 0..23 → blit FACE.BIN entry at that index */
void nexus_ui_render_portrait(const Nexus_UI_Manager *mgr,
    int portrait_index,
    uint8_t *fb, int fb_w, int fb_h,
    int dest_x, int dest_y, int flip_h);

/* ── Blit helpers (palette index remap) ─────────────────────────── */

/* Remap surface palette indices to a different base.
 * e.g. portrait at pal_start=64 → remap to fb palette offset 192.   */
void nexus_ui_surface_remap_pal(Nexus_UI_Surface *surf,
    uint8_t new_pal_start);

/* Darken a surface in-place for focus/blur states (e.g. paused overlay) */
void nexus_ui_surface_darken(Nexus_UI_Surface *surf, float factor);

#endif /* NEXUS_V1_UI_SURFACES_H */
