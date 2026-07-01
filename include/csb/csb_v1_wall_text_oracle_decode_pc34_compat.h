#ifndef FIRESTAFF_CSB_V1_WALL_TEXT_ORACLE_DECODE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_WALL_TEXT_ORACLE_DECODE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CSB V1 wall text / oracle text decode gate (ReDMCSB F0168_DUNGEON_DecodeText).
 *
 * This gate implements the 5-bit code-stream decoder used by
 * F0168_DUNGEON_DecodeText for CSB PC 3.4 (I34E/I34M branch) plus
 * the wall-text drawing bounds assertion from DUNVIEW.C F0107
 * lines 3590-3717 (MEDIA562 path) so the decoded text-bounds stay
 * inside the viewport and the indexed palette stays in the 16-color
 * CSB PC palette. Synthetic verified dungeon data is the only input
 * the gate accepts; no real-asset or original-DOS pixel parity is
 * claimed. */

/* F0168 text-type selectors, source-locked against
 * ReDMCSB DEFS.H:2838-2840. */
#define CSB_V1_WALL_TEXT_ORACLE_TYPE_INSCRIPTION_PC34 0 /* C0_TEXT_TYPE_INSCRIPTION */
#define CSB_V1_WALL_TEXT_ORACLE_TYPE_MESSAGE_PC34    1 /* C1_TEXT_TYPE_MESSAGE */
#define CSB_V1_WALL_TEXT_ORACLE_TYPE_SCROLL_PC34     2 /* C2_TEXT_TYPE_SCROLL */

/* F0168 terminator / separator bytes, source-locked against
 * ReDMCSB DUNGEON.C:2319 / 2337. */
#define CSB_V1_WALL_TEXT_ORACLE_CODE_TERMINATOR_PC34 0x81u
#define CSB_V1_WALL_TEXT_ORACLE_CODE_SEPARATOR_PC34 0x80u

/* F0168 5-bit codes (top-level), source-locked against
 * ReDMCSB DUNGEON.C:2280-2334. */
#define CSB_V1_WALL_TEXT_ORACLE_CODE_MAX_LETTER_PC34 27 /* 0..27 -> 'A'..'Z', 26=' ', 27='.' */
#define CSB_V1_WALL_TEXT_ORACLE_CODE_SEPARATOR_CODE_PC34 28
#define CSB_V1_WALL_TEXT_ORACLE_CODE_ESCAPE1_PC34        29 /* escape level 1 */
#define CSB_V1_WALL_TEXT_ORACLE_CODE_ESCAPE2_PC34        30 /* escape level 2 (full) */
#define CSB_V1_WALL_TEXT_ORACLE_CODE_END_PC34            31

/* F0168 escape replacement string banks. Each row holds up to 8
 * ASCII characters used to render the escape code 30 substitution
 * (and the secondary bank for codes < 30 escape 1). Source-locked
 * against ReDMCSB G0255 / G0256 / G0257 declared at DEFS.H:5628-5630. */
#define CSB_V1_WALL_TEXT_ORACLE_ESCAPE_BANK_ROWS_PC34 32
#define CSB_V1_WALL_TEXT_ORACLE_ESCAPE_BANK_COLS_PC34 8
#define CSB_V1_WALL_TEXT_ORACLE_ESCAPE_BANK_LEN_PC34 \
    (CSB_V1_WALL_TEXT_ORACLE_ESCAPE_BANK_ROWS_PC34 * \
     CSB_V1_WALL_TEXT_ORACLE_ESCAPE_BANK_COLS_PC34)

/* Wall text / oracle draw bounds (text-bounds). The Y line
 * table is the 4-entry G0203_auc_Graphic558_InscriptionLineY
 * array (DUNVIEW.C:1049 / MEDIA353 block), the C8 character
 * width / line height come from G2089 / G2090 (DEFS.H:6402-6403),
 * and the D2C/D3C viewport pixel width is C224 (G2073). The
 * text glyphs come from the M648_GRAPHIC_INSCRIPTION_FONT graphic
 * (DEFS.H:2376 for I34E). C10_COLOR_FLESH is the I34E transparency
 * (DEFS.H:2088). */
#define CSB_V1_WALL_TEXT_ORACLE_INSCRIPTION_LINE_COUNT_PC34 4
#define CSB_V1_WALL_TEXT_ORACLE_INSCRIPTION_LINE_Y_PC34 \
    { 48u, 59u, 75u, 86u }  /* G0203_auc_Graphic558_InscriptionLineY[0..3] */
#define CSB_V1_WALL_TEXT_ORACLE_INSCRIPTION_CHAR_WIDTH_PC34 8 /* G2089_C8_InscriptionCharacterWidth */
#define CSB_V1_WALL_TEXT_ORACLE_INSCRIPTION_LINE_HEIGHT_PC34 8 /* G2090_C8_InscriptionLineHeight */
#define CSB_V1_WALL_TEXT_ORACLE_VIEWPORT_WIDTH_PC34 224 /* C224 viewport pixel width */
#define CSB_V1_WALL_TEXT_ORACLE_VIEWPORT_HEIGHT_PC34 136 /* C136 viewport pixel height */
#define CSB_V1_WALL_TEXT_ORACLE_TRANSPARENT_PALETTE_INDEX_PC34 10 /* C10_COLOR_FLESH */
#define CSB_V1_WALL_TEXT_ORACLE_PALETTE_SIZE_PC34 16

/* Decode buffer limits. The ReDMCSB L0099_auc_InscriptionString
 * buffer is 70 bytes (DUNGEON.C:3568). The oracle / message /
 * scroll decode in F0168 routes through the same buffer. We keep
 * a hard upper bound so the CTest can refuse oversized inputs. */
#define CSB_V1_WALL_TEXT_ORACLE_MAX_TEXT_BYTES_PC34 70

/* Code-word stream limits. The F0168 source path reads 16-bit
 * little-endian words out of G0260_pui_DungeonTextData until it
 * sees code 31. Each word yields three 5-bit codes. The fixture
 * ships a fixed-size synthetic code-word block; an over-sized
 * stream is rejected by the gate rather than silently truncated. */
#define CSB_V1_WALL_TEXT_ORACLE_MAX_CODE_WORDS_PC34 64

typedef struct {
    int source_locked_contract_only;        /* contract-only gate (no parity claim) */
    int no_original_dos_pixel_parity_claim;  /* explicit non-claim */
    int no_game_data_load;                  /* synthetic fixture only */
    int viewport_width;                     /* C224 */
    int viewport_height;                    /* C136 */
    int inscription_char_width;             /* G2089 = 8 */
    int inscription_line_height;            /* G2090 = 8 */
    int transparent_palette_index;          /* C10 = 10 */
    int palette_size;                       /* 16 */
    int max_text_bytes;                     /* 70 (L0099_auc_InscriptionString) */
    int max_code_words;                     /* 64 */
    int inscription_y_count;                /* 4 */
    uint8_t inscription_line_y[CSB_V1_WALL_TEXT_ORACLE_INSCRIPTION_LINE_COUNT_PC34];
    const char *redmcsb_f0168_anchor;       /* "ReDMCSB DUNGEON.C:F0168_DUNGEON_DecodeText:2206-2372" */
    const char *redmcsb_f0107_anchor;       /* "ReDMCSB DUNVIEW.C:F0107:3502-3938" */
    const char *redmcsb_defs_anchor;        /* "ReDMCSB DEFS.H:M648:2376; C10:2088; G0203:5571; G2089/G2090:6402-6403" */
    const char *redmcsb_csb_branch_anchor;  /* "ReDMCSB MEDIA562_F20E_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E" */
    const char *csb_lineage_anchor;         /* CSBWin/Viewport.cpp inscription pass */
    const char *source_evidence;
} CSB_V1_WallTextOracleDecodePc34Spec;

typedef struct {
    int ok;                              /* 1 if all assertions held, 0 otherwise */
    int text_type;                       /* CSB_V1_WALL_TEXT_ORACLE_TYPE_* */
    int codes_total;                     /* total 5-bit codes consumed */
    int letters_decoded;                 /* codes 0..27 rendered as 'A'..'Z', ' ', '.' */
    int separators_decoded;              /* code 28 emitted (incl. 0x80 for inscription) */
    int escapes_decoded;                 /* codes 29 / 30 emitted */
    int terminator_present;              /* terminator byte (0x81 or '\0') observed */
    int text_byte_count;                 /* bytes written into the decode buffer */
    int text_line_count;                 /* number of 0x80-separated lines (inscription) */
    int text_first_line_byte_count;      /* bytes in line 0 (for bounds check) */
    int text_longest_line_byte_count;    /* bytes in the longest line */
    int max_line_y_inscription;          /* index into inscription_line_y */
    int max_line_pixel_width;            /* char_count * char_width (px) */
    int palette_index_in_range;          /* all palette indices in [0, palette_size) */
    int text_width_within_viewport;      /* max_line_pixel_width <= viewport_width */
    int text_height_within_viewport;     /* max_line_y_inscription + line_height <= viewport_height */
    int transparent_index_matches_c10;   /* the indexed transparency is exactly C10 */
    uint32_t text_byte_fnv1a;            /* FNV-1a 32-bit hash of the decoded text buffer */
    uint32_t code_word_fnv1a;            /* FNV-1a 32-bit hash of the input code-word stream */
    const char *source_evidence;
} CSB_V1_WallTextOracleDecodePc34Trace;

const CSB_V1_WallTextOracleDecodePc34Spec *
csb_v1_wall_text_oracle_decode_pc34_spec(void);

const char *
csb_v1_wall_text_oracle_decode_pc34_source_evidence(void);

/* Decode a code-word stream and produce the inscription-style
 * byte buffer that DUNGEON.C F0168 would emit. Returns the
 * number of bytes written into out_text on success (always
 * <= CSB_V1_WALL_TEXT_ORACLE_MAX_TEXT_BYTES_PC34 + 1 for the
 * terminator), or -1 on bad arguments. The terminator byte
 * (0x81 for inscription, '\0' for message/scroll) is always
 * written at the end. */
int csb_v1_wall_text_oracle_decode_pc34(
    const uint16_t *code_words,
    size_t code_word_count,
    int text_type,
    uint8_t *out_text,
    size_t out_text_capacity,
    CSB_V1_WallTextOracleDecodePc34Trace *out_trace);

/* Render the decoded inscription byte buffer into an indexed
 * 320x200 framebuffer at the D1C wall-text box, source-locked
 * against DUNVIEW.C F0107 lines 3590-3717 MEDIA562 path. The
 * synthetic framebuffer is filled with the wall sentinel color
 * outside the text box and the transparent C10 index inside
 * the text-box background. Returns 0 on success, -1 on bad
 * arguments. */
int csb_v1_wall_text_oracle_render_pc34(
    const uint8_t *text,
    size_t text_byte_count,
    int text_type,
    uint8_t *framebuffer,
    size_t framebuffer_size,
    CSB_V1_WallTextOracleDecodePc34Trace *out_trace);

/* Verify the decoded text respects the G0203 / G2089 / G2090
 * draw bounds. Returns 1 if all bounds hold, 0 otherwise. The
 * trace fields are populated regardless. */
int csb_v1_wall_text_oracle_bounds_ok_pc34(
    const uint8_t *text,
    size_t text_byte_count,
    int text_type,
    int palette_index,
    CSB_V1_WallTextOracleDecodePc34Trace *out_trace);

/* Render the text glyphs into a 16x8 indexed bitmap slot.
 * The slot follows the M648 I34E inscription font: each glyph
 * is CSB_V1_WALL_TEXT_ORACLE_INSCRIPTION_CHAR_WIDTH_PC34 pixels
 * wide and CSB_V1_WALL_TEXT_ORACLE_INSCRIPTION_LINE_HEIGHT_PC34
 * pixels tall, with the C10 transparency index marking empty
 * pixels. The text glyph is the lower-case index (>= 'a') which
 * matches F0168 5-bit codes 0..25 mapping to 'A'..'Z'. The
 * test fixture expects `palette_index` to be the foreground
 * colour (any value in [0, palette_size-1] other than C10). */
int csb_v1_wall_text_oracle_glyph_index_pc34(
    int glyph,
    int *out_palette_index);

uint32_t csb_v1_wall_text_oracle_hash_pc34(
    const uint8_t *data,
    size_t size);

#ifdef __cplusplus
}
#endif

#endif
