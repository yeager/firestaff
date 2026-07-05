#include "csb_v1_boot.h"

#include "asset_find_by_hash.h"
#include "csb_v1_cmp_import_pc34_compat.h"
#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_m11_runtime_plan.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_engine_version_display_pc34_compat.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ReDMCSB source-lock for this boot/profile boundary:
 * ENTRANCE.C F0806 lines 409-441 builds the entrance micro-dungeon and
 * selects C28_ENTRANCE_CSB for CSB media.
 * ENTRANCE.C F0806 lines 857-883 waits on the entrance state machine and
 * switches G0298_B_NewGame to C001_MODE_LOAD_DUNGEON.
 * LOADSAVE.C F0435 lines 1940-1944 loads the initial party location from
 * DUNGEON.DAT and sets G0309_i_PartyMapIndex to map 0 for new games.
 */

static const char *const g_csb_boot_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611",
    "ebf6a57af3f27782e358c0490bfd2f2e",
    "291e1bc6803e3dc4b974c60117ca5d68",
    "cefaddfdf5651df2c91f61b5611a8362",
    NULL
};

static const CSB_V1_VariantId g_csb_boot_graphics_variants[] = {
    CSB_V1_VARIANT_PC34_EN,
    CSB_V1_VARIANT_ST21_EN,
    CSB_V1_VARIANT_AMIGA35_EN,
    CSB_V1_VARIANT_AMIGA35_MULTI
};

static const char *const g_csb_boot_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",
    NULL
};

/* ── DM1-assumption rejection strings ────────────────────────────────────
 *
 * Each csb_v1_boot_assume_no_dm1_runtime() failure has a stable reason
 * string.  The probe and any future diagnostics depend on these exact
 * phrases, so do not rename them without updating
 * probes/firestaff_csb_v1_no_dm1_runtime_assumption_gate_probe.c.
 *
 *   "csb_boot/assume_no_dm1_runtime: game_id is not 'csb'"
 *       Rejects profiles routed from DM1/DM2 launchers.
 *   "csb_boot/assume_no_dm1_runtime: variant_id outside CSB range"
 *       Catches raw enum values that escape the CSB_V1_VARIANT_* enum.
 *   "csb_boot/assume_no_dm1_runtime: party default matches DM1 HoC (11,29)"
 *       Catches (11,29) DM1 Hall of Champions defaults leaking into CSB.
 *   "csb_boot/assume_no_dm1_runtime: tick_ms is not CSB nominal (55)"
 *       Catches non-CSB tick quantum (DM1 is also 55; this makes CSB origin explicit).
 *   "csb_boot/assume_no_dm1_runtime: entrance_map_index != 255"
 *       ReDMCSB ENTRANCE.C F0806 selects C255_MAP_INDEX_ENTRANCE for CSB.
 *   "csb_boot/assume_no_dm1_runtime: start_map_index != 0"
 *       ReDMCSB LOADSAVE.C F0435 line 1940-1944 sets map 0 for new games. */
static const char *g_csb_assume_last_reason = "csb_boot/assume_no_dm1_runtime: ok";

#define CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX 564u
#define CSB_V1_GRAPHICS_OBJECT_NAMES_MAX_BYTES 65535u
#define CSB_V1_GRAPHICS_LZW_MAX_CODE 4096
#define CSB_V1_GRAPHICS_LZW_CLEAR_CODE 256
#define CSB_V1_GRAPHICS_LZW_END_CODE 257
#define CSB_V1_GRAPHICS_LZW_FIRST_CODE 258

typedef struct {
    const uint8_t *bytes;
    size_t size;
    size_t byte_pos;
    uint8_t chunk[12];
    int chunk_bit_idx;
    int chunk_bit_count;
    int needs_refill;
} CSB_V1_GraphicsBitReader;

static uint16_t csb_v1_graphics_read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static int csb_v1_graphics_read_bits(CSB_V1_GraphicsBitReader *br,
                                     int bit_count,
                                     uint16_t *out_code)
{
    static const uint8_t lsb_masks[9] = {
        0x00u, 0x01u, 0x03u, 0x07u, 0x0fu, 0x1fu, 0x3fu, 0x7fu, 0xffu
    };
    uint32_t value = 0u;
    int bit_index;
    int required;
    const uint8_t *p;
    if (!br || !out_code || bit_count <= 0 || bit_count > 12) {
        return -1;
    }
    /* ReDMCSB LZW.C F0495 reads codeBitCount bytes into a chunk and
     * extracts up to eight variable-width codes from that chunk. Width
     * changes force a refill instead of continuing through a flat stream. */
    if (br->needs_refill || br->chunk_bit_idx >= br->chunk_bit_count) {
        int chunk_bytes = bit_count;
        if (br->byte_pos + (size_t)chunk_bytes > br->size) {
            chunk_bytes = (int)(br->size - br->byte_pos);
        }
        if (chunk_bytes <= 0) {
            return -1;
        }
        memset(br->chunk, 0, sizeof(br->chunk));
        memcpy(br->chunk, br->bytes + br->byte_pos, (size_t)chunk_bytes);
        br->byte_pos += (size_t)chunk_bytes;
        br->chunk_bit_idx = 0;
        br->chunk_bit_count = (chunk_bytes << 3) - (bit_count - 1);
        br->needs_refill = 0;
    }
    bit_index = br->chunk_bit_idx;
    required = bit_count;
    p = br->chunk + (bit_index >> 3);
    bit_index &= 7;

    value = (uint32_t)(*p++ >> bit_index);
    required -= (8 - bit_index);
    bit_index = 8 - bit_index;
    if (required >= 8) {
        value |= (uint32_t)(*p++) << bit_index;
        bit_index += 8;
        required -= 8;
    }
    if (required > 0) {
        value |= (uint32_t)(*p & lsb_masks[required]) << bit_index;
    }
    br->chunk_bit_idx += bit_count;
    *out_code = (uint16_t)value;
    return 0;
}

static void csb_v1_graphics_lzw_reset(uint16_t *prefix,
                                      uint8_t *append,
                                      int *next_code,
                                      int *code_bits)
{
    int i;
    for (i = 0; i < 256; i++) {
        prefix[i] = 0xffffu;
        append[i] = (uint8_t)i;
    }
    *next_code = CSB_V1_GRAPHICS_LZW_FIRST_CODE;
    *code_bits = 9;
}

static int csb_v1_graphics_lzw_emit(uint16_t code,
                                    const uint16_t *prefix,
                                    const uint8_t *append,
                                    uint8_t *stack,
                                    uint8_t *out,
                                    size_t out_capacity,
                                    size_t *out_pos,
                                    uint8_t *out_first)
{
    int stack_len = 0;
    uint16_t cursor = code;

    while (cursor >= 256u) {
        if (cursor >= CSB_V1_GRAPHICS_LZW_MAX_CODE ||
            prefix[cursor] == 0xffffu ||
            stack_len >= CSB_V1_GRAPHICS_LZW_MAX_CODE) {
            return -1;
        }
        stack[stack_len++] = append[cursor];
        cursor = prefix[cursor];
    }
    if (cursor >= 256u) {
        return -1;
    }
    if (out_first) {
        *out_first = (uint8_t)cursor;
    }
    if (*out_pos >= out_capacity) {
        return -1;
    }
    out[(*out_pos)++] = (uint8_t)cursor;
    while (stack_len > 0) {
        if (*out_pos >= out_capacity) {
            return -1;
        }
        out[(*out_pos)++] = stack[--stack_len];
    }
    return 0;
}

static int csb_v1_graphics_lzw_decode(const uint8_t *input,
                                      size_t input_size,
                                      uint8_t *out,
                                      size_t out_capacity,
                                      size_t *out_size)
{
    uint16_t prefix[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    uint8_t append[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    uint8_t stack[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    CSB_V1_GraphicsBitReader br;
    int next_code;
    int code_bits;
    int old_code = -1;
    uint8_t old_first = 0u;
    size_t out_pos = 0u;

    if (!input || !out || !out_size || input_size == 0u) {
        return -1;
    }
    br.bytes = input;
    br.size = input_size;
    br.byte_pos = 0u;
    br.chunk_bit_idx = 0;
    br.chunk_bit_count = 0;
    br.needs_refill = 1;
    csb_v1_graphics_lzw_reset(prefix, append, &next_code, &code_bits);

    for (;;) {
        uint16_t code;
        uint8_t first = 0u;
        if (csb_v1_graphics_read_bits(&br, code_bits, &code) != 0) {
            return -1;
        }
        if (code == CSB_V1_GRAPHICS_LZW_CLEAR_CODE) {
            csb_v1_graphics_lzw_reset(prefix, append, &next_code, &code_bits);
            old_code = -1;
            continue;
        }
        if (code == CSB_V1_GRAPHICS_LZW_END_CODE) {
            *out_size = out_pos;
            return 0;
        }
        if (code < (uint16_t)next_code) {
            if (csb_v1_graphics_lzw_emit(code, prefix, append, stack,
                                         out, out_capacity, &out_pos,
                                         &first) != 0) {
                return -1;
            }
        } else if (code == (uint16_t)next_code && old_code >= 0) {
            first = old_first;
            if (csb_v1_graphics_lzw_emit((uint16_t)old_code, prefix, append,
                                         stack, out, out_capacity, &out_pos,
                                         NULL) != 0 ||
                out_pos >= out_capacity) {
                return -1;
            }
            out[out_pos++] = first;
        } else {
            return -1;
        }

        if (old_code >= 0 && next_code < CSB_V1_GRAPHICS_LZW_MAX_CODE) {
            prefix[next_code] = (uint16_t)old_code;
            append[next_code] = first;
            next_code++;
            if (next_code > ((1 << code_bits) - 1) && code_bits < 12) {
                code_bits++;
                br.needs_refill = 1;
            }
        }
        old_code = (int)code;
        old_first = first;
    }
}

static int csb_v1_graphics_decode_entry_m564(const uint8_t *file_bytes,
                                             size_t file_size,
                                             uint8_t *out,
                                             size_t out_capacity,
                                             size_t *out_size)
{
    uint16_t signature;
    uint16_t count;
    size_t compressed_table;
    size_t decompressed_table;
    size_t dimensions_table;
    size_t payload_offset;
    size_t entry_offset;
    size_t i;
    uint16_t compressed_size;
    uint16_t decompressed_size;
    size_t decoded_size = 0u;

    if (!file_bytes || !out || !out_size || file_size < 4u) {
        return -1;
    }
    *out_size = 0u;
    signature = csb_v1_graphics_read_le16(file_bytes);
    count = csb_v1_graphics_read_le16(file_bytes + 2u);
    if ((signature & 0x8000u) == 0u ||
        count <= CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX) {
        return -1;
    }
    compressed_table = 4u;
    decompressed_table = compressed_table + (size_t)count * 2u;
    dimensions_table = decompressed_table + (size_t)count * 2u;
    payload_offset = dimensions_table + (size_t)count * 4u;
    if (payload_offset > file_size) {
        return -1;
    }

    entry_offset = payload_offset;
    for (i = 0u; i < CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX; i++) {
        entry_offset +=
            (size_t)csb_v1_graphics_read_le16(file_bytes + compressed_table + i * 2u);
        if (entry_offset > file_size) {
            return -1;
        }
    }
    compressed_size = csb_v1_graphics_read_le16(
        file_bytes + compressed_table + CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX * 2u);
    decompressed_size = csb_v1_graphics_read_le16(
        file_bytes + decompressed_table + CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX * 2u);
    if (compressed_size == 0u || decompressed_size == 0u ||
        decompressed_size > out_capacity ||
        entry_offset + (size_t)compressed_size > file_size) {
        return -1;
    }

    if (csb_v1_graphics_lzw_decode(file_bytes + entry_offset,
                                   (size_t)compressed_size,
                                   out,
                                   (size_t)decompressed_size,
                                   &decoded_size) != 0 ||
        decoded_size != (size_t)decompressed_size) {
        return -1;
    }
    *out_size = decoded_size;
    return 0;
}

static int csb_v1_boot_load_object_names_m564(CSB_V1_BootProfile *profile)
{
    FILE *file;
    long file_len;
    uint8_t *file_bytes = NULL;
    uint8_t *decoded = NULL;
    size_t read_count;
    size_t decoded_size = 0u;
    int ok = 0;

    if (!profile || profile->graphics_path[0] == '\0') {
        return 0;
    }
    file = fopen(profile->graphics_path, "rb");
    if (!file) {
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    file_len = ftell(file);
    if (file_len <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    file_bytes = (uint8_t *)malloc((size_t)file_len);
    decoded = (uint8_t *)malloc(CSB_V1_GRAPHICS_OBJECT_NAMES_MAX_BYTES);
    if (!file_bytes || !decoded) {
        free(decoded);
        free(file_bytes);
        fclose(file);
        return 0;
    }
    read_count = fread(file_bytes, 1u, (size_t)file_len, file);
    fclose(file);
    if (read_count == (size_t)file_len &&
        csb_v1_graphics_decode_entry_m564(
            file_bytes,
            (size_t)file_len,
            decoded,
            CSB_V1_GRAPHICS_OBJECT_NAMES_MAX_BYTES,
            &decoded_size) == 0) {
        /* ReDMCSB OBJECT.C F0031 loads M564_GRAPHIC_OBJECT_NAMES from
         * GRAPHICS.DAT and decodes C199 high-bit-terminated object names
         * before object UI asks F0033 for icon-indexed names. */
        ok = csb_v1_runtime_load_object_names_m564(&profile->runtime,
                                                   decoded,
                                                   decoded_size);
    }
    free(decoded);
    free(file_bytes);
    return ok == 1 ? 1 : 0;
}

const char *csb_v1_boot_last_assumption_reason(void)
{
    return g_csb_assume_last_reason;
}

static void csb_v1_boot_assume_fail(const char *reason)
{
    g_csb_assume_last_reason = reason ? reason : "csb_boot/assume_no_dm1_runtime: (null)";
}

int csb_v1_boot_assume_no_dm1_runtime(const CSB_V1_BootProfile *profile)
{
    if (!profile) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: NULL profile");
        return -1;
    }
    /* game_id must be the CSB literal.  ReDMCSB ENTRANCE.C F0806 selects
     * C28_ENTRANCE_CSB and the loader branches on game_id; a DM1 or DM2
     * profile routed here would otherwise inherit the CSB runtime
     * structure but keep DM1 start defaults. */
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: game_id is not 'csb'");
        return -1;
    }
    /* variant_id must stay inside the CSB enum.  A raw integer from a
     * DM1 or DM2 catalog that happens to land in this range would
     * otherwise resolve to a CSB variant silently.  We use a strict
     * inclusive check: variant_id >= 0 && variant_id < CSB_V1_VARIANT_COUNT. */
    if (profile->variant_id < 0 ||
        (unsigned)profile->variant_id >= (unsigned)CSB_V1_VARIANT_COUNT) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: variant_id outside CSB range");
        return -1;
    }
    /* DM1 Hall of Champions default is (11,29) facing North.  CSB's
     * Hall of Champions is (5,5) facing North.  These are disjoint
     * source-locked defaults — a profile carrying (11,29) almost
     * certainly came from a DM1 launcher side-effect.
     * Source: ReDMCSB ENTRANCE.C (DM1) line ~430 vs (CSB) line ~430
     * Source: csb_v1_runtime_pc34_compat.h CSB_V1_START_PARTY_{X,Y} */
    if (profile->default_party_x == 11U &&
        profile->default_party_y == 29U) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: party default matches DM1 HoC (11,29)");
        return -1;
    }
    /* CSB V1 tick is 55ms nominal (shared with DM1, but the assertion
     * makes CSB origin explicit; the runtime must use CSB_V1_TICK_MS_NOMINAL
     * exactly, not a DM2 or Nexus tick quantum). */
    if (profile->tick_ms != CSB_V1_TICK_MS_NOMINAL) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: tick_ms is not CSB nominal (55)");
        return -1;
    }
    /* entrance_map_index is 255 (C255_MAP_INDEX_ENTRANCE) for CSB.
     * Rejecting any other value catches DM1 maps 0..14 or future variants
     * that escape the source-locked entrance selection. */
    if (profile->entrance_map_index != 255U) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: entrance_map_index != 255");
        return -1;
    }
    /* start_map_index is 0 for new games.  Anything else implies DM1-style
     * dungeon-of-doom selection or a corrupted scan. */
    if (profile->start_map_index != 0U) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: start_map_index != 0");
        return -1;
    }
    csb_v1_boot_assume_fail("csb_boot/assume_no_dm1_runtime: ok");
    return 0;
}

static void csb_v1_boot_copy(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0U) return;
    if (!src) src = "";
    snprintf(dst, dst_size, "%s", src);
}

static CSB_V1_AssetGfxArchiveType csb_v1_boot_graphics_kind(const char *path)
{
    const char *name;
    if (!path) return CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    name = strrchr(path, '/');
#if defined(_WIN32)
    {
        const char *slash = strrchr(path, '\\');
        if (slash && (!name || slash > name)) name = slash;
    }
#endif
    name = name ? name + 1 : path;
    if (strcmp(name, "CSB.DAT") == 0 || strcmp(name, "csb.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSB;
    }
    if (strcmp(name, "CSBGRAPH.DAT") == 0 || strcmp(name, "csbgraph.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    }
    return CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
}

void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));
    csb_v1_boot_copy(profile->game_id, sizeof(profile->game_id), CSB_V1_BOOT_GAME_ID);
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id), "unknown");
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label), "Unknown");
    profile->tick_ms = CSB_V1_TICK_MS_NOMINAL;
    profile->entrance_map_index = 255U;
    profile->start_map_index = 0U;
    profile->default_party_x = CSB_V1_START_PARTY_X;
    profile->default_party_y = CSB_V1_START_PARTY_Y;
    profile->default_party_dir = CSB_V1_START_PARTY_DIR;
    profile->imported_party_ready = 0;
    profile->cmp_import_attempted = 0;
    profile->cmp_import_succeeded = 0;
    profile->cmp_imported_slot = -1;
    profile->cmp_imported_champion_count = 0;
    profile->engine_version_displayed = 0;
    profile->csbgraphics_scan_attempted = 0;
    profile->csbgraphics_scan_result =
        CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND;
    profile->csbgraphics_plan_result =
        CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_CACHE;
    profile->csbgraphics_skin_def_loaded = 0;
    profile->csbgraphics_skin_def_word_count = 0u;
    memset(profile->csbgraphics_skin_def_words, 0,
           sizeof(profile->csbgraphics_skin_def_words));
    csb_v1_csbgraphics_dat_real_cache_init(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_m11_runtime_plan_init(&profile->csbgraphics_m11_plan);
    csb_v1_character_init_default(&profile->imported_party);
    csb_v1_runtime_init(&profile->runtime, NULL);
    csb_v1_engine_version_display_set_csb(0);
}

static void csb_v1_boot_reset_csbgraphics(CSB_V1_BootProfile *profile)
{
    if (!profile) {
        return;
    }
    csb_v1_csbgraphics_dat_real_cache_free(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_dat_real_cache_init(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_m11_runtime_plan_init(&profile->csbgraphics_m11_plan);
    profile->csbgraphics_scan_attempted = 0;
    profile->csbgraphics_scan_result =
        CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND;
    profile->csbgraphics_plan_result =
        CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_CACHE;
    profile->csbgraphics_skin_def_loaded = 0;
    profile->csbgraphics_skin_def_word_count = 0u;
    memset(profile->csbgraphics_skin_def_words, 0,
           sizeof(profile->csbgraphics_skin_def_words));
}

int csb_v1_boot_scan_csbgraphics(CSB_V1_BootProfile *profile,
                                 const char *cache_dir)
{
    const char *root;

    if (!profile) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT;
    }
    root = profile->asset_root[0] ? profile->asset_root : NULL;
    csb_v1_boot_reset_csbgraphics(profile);
    profile->csbgraphics_scan_attempted = 1;
    profile->csbgraphics_scan_result =
        csb_v1_csbgraphics_dat_real_scan_and_load(
            root, cache_dir, 4, &profile->csbgraphics_cache);
    if (profile->csbgraphics_scan_result !=
        CSB_V1_CSBGRAPHICS_DAT_REAL_OK) {
        return profile->csbgraphics_scan_result;
    }
    profile->csbgraphics_plan_result =
        csb_v1_csbgraphics_m11_runtime_plan_build_from_cache(
            &profile->csbgraphics_cache,
            &profile->csbgraphics_m11_plan);
    {
        size_t skin_def_word_count = 0u;
        int skin_rc =
            csb_v1_csbgraphics_m11_runtime_plan_decode_custom_background_skin_def(
                &profile->csbgraphics_cache,
                profile->csbgraphics_skin_def_words,
                CSB_V1_CSBGRAPHICS_M11_SKIN_DEF_MAX_WORDS,
                &skin_def_word_count);
        if (skin_rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
            int add_rc;
            profile->csbgraphics_skin_def_loaded = 1;
            profile->csbgraphics_skin_def_word_count = skin_def_word_count;
            add_rc =
                csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                    &profile->csbgraphics_cache,
                    profile->csbgraphics_skin_def_words,
                    profile->csbgraphics_skin_def_word_count,
                    &profile->csbgraphics_m11_plan);
            if (add_rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
                profile->csbgraphics_plan_result =
                    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
            }
        }
    }
    return profile->csbgraphics_plan_result;
}

const CSB_V1_CSBGraphicsM11RuntimePlan *
csb_v1_boot_csbgraphics_m11_plan(const CSB_V1_BootProfile *profile)
{
    return profile ? &profile->csbgraphics_m11_plan : NULL;
}

const CSB_V1_CSBGraphicsDatRealCache *
csb_v1_boot_csbgraphics_cache(const CSB_V1_BootProfile *profile)
{
    return profile ? &profile->csbgraphics_cache : NULL;
}

const uint16_t *
csb_v1_boot_csbgraphics_skin_def_words(const CSB_V1_BootProfile *profile,
                                       size_t *out_word_count)
{
    if (out_word_count) {
        *out_word_count = 0u;
    }
    if (!profile || !profile->csbgraphics_skin_def_loaded ||
        profile->csbgraphics_skin_def_word_count == 0u) {
        return NULL;
    }
    if (out_word_count) {
        *out_word_count = profile->csbgraphics_skin_def_word_count;
    }
    return profile->csbgraphics_skin_def_words;
}

int csb_v1_boot_set_imported_party(CSB_V1_BootProfile *profile,
                                   const CSB_V1_PartyState *party)
{
    if (!profile || !party) return -1;
    if (party->ChampionCount <= 0 ||
        party->ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    profile->imported_party = *party;
    profile->imported_party_ready = 1;
    return 0;
}

int csb_v1_boot_set_imported_party_from_cmp(CSB_V1_BootProfile *profile,
                                            const uint8_t *cmp_buf,
                                            size_t cmp_size)
{
    int slot;
    if (!profile || !cmp_buf) return -1;
    profile->cmp_import_attempted = 1;
    slot = csb_v1_cmp_import_to_party(&profile->imported_party,
                                      cmp_buf, cmp_size);
    if (slot < 0) {
        profile->cmp_import_succeeded = 0;
        profile->cmp_imported_slot = slot;
        return slot;
    }
    profile->cmp_import_succeeded = 1;
    profile->cmp_imported_slot = slot;
    profile->cmp_imported_champion_count =
        profile->imported_party.ChampionCount;
    profile->imported_party_ready = 1;
    return 0;
}

int csb_v1_boot_mark_imported_party_ready(CSB_V1_BootProfile *profile)
{
    if (!profile) return -1;
    profile->cmp_import_attempted = 1;
    if (profile->imported_party.ChampionCount <= 0 ||
        profile->imported_party.ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    profile->cmp_import_succeeded = 1;
    profile->cmp_imported_champion_count =
        profile->imported_party.ChampionCount;
    profile->imported_party_ready = 1;
    return 0;
}

void csb_v1_boot_reset_engine_version_to_dm1(void)
{
    csb_v1_engine_version_display_set_csb(0);
}

void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir)
{
    if (!profile) return;
    if (save_dir && save_dir[0] != '\0') {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root), save_dir);
    } else if (profile->asset_root[0] != '\0') {
        snprintf(profile->save_root, sizeof(profile->save_root),
                 "%s/../%s", profile->asset_root, CSB_V1_BOOT_SAVE_SUBDIR);
    } else {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root),
                         csb_v1_runtime_save_dir());
    }
}

int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir)
{
    char graphics_path[ASSET_PATH_MAX];
    char dungeon_path[ASSET_PATH_MAX];
    int graphics_match = -1;
    int dungeon_match = -1;
    const CSB_V1_VariantInfo *variant;
    const char *root;

    if (!profile) return -1;
    root = (data_dir && data_dir[0] != '\0') ? data_dir : ".";
    csb_v1_boot_copy(profile->asset_root, sizeof(profile->asset_root), root);
    /* A reused launcher profile must not carry stale CSB paths across scans.
     * ReDMCSB only enters the CSB load path after the current media probe has
     * selected CSB and found a dungeon to load.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    profile->assets_verified = 0;
    profile->graphics_verified = 0;
    profile->dungeon_verified = 0;
    profile->graphics_path[0] = '\0';
    profile->dungeon_path[0] = '\0';
    profile->graphics_md5[0] = '\0';
    profile->dungeon_md5[0] = '\0';
    profile->graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    csb_v1_boot_reset_csbgraphics(profile);

    /* A successful csb_v1_boot_enter_game() hands the verified DUNGEON.DAT
     * off to the runtime as profile->runtime.dungeon_handle and to the global
     * singleton via csb_v1_dungeon_set_current().  A follow-up rescan
     * (different data_dir, removed asset, launcher refresh) must not leave
     * that handoff alive: the runtime-owned handle would still point at the
     * previous heap allocation, the global singleton would still expose the
     * previous dungeon through csb_v1_dungeon_get_current(), and the next
     * enter_game() would either fail to replace the handle (when verification
     * fails) or silently keep serving the previous dungeon through the new
     * profile paths.  Release the handle and reset the singleton here, before
     * the rescan-driven profile fields are populated.  The full runtime
     * re-init still happens in csb_v1_boot_enter_game() on the next launch.
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    if (profile->runtime.dungeon_handle != NULL ||
        csb_v1_dungeon_get_current() != NULL) {
        csb_v1_dungeon_unload();
        free(profile->runtime.dungeon_handle);
        profile->runtime.dungeon_handle = NULL;
    }

    profile->graphics_verified =
        asset_find_by_md5_list(root, g_csb_boot_graphics_hashes,
                               graphics_path, sizeof(graphics_path),
                               &graphics_match, 4);
    profile->dungeon_verified =
        asset_find_by_md5_list(root, g_csb_boot_dungeon_hashes,
                               dungeon_path, sizeof(dungeon_path),
                               &dungeon_match, 4);
    if (profile->graphics_verified) {
        csb_v1_boot_copy(profile->graphics_path, sizeof(profile->graphics_path),
                         graphics_path);
        csb_v1_boot_copy(profile->graphics_md5, sizeof(profile->graphics_md5),
                         g_csb_boot_graphics_hashes[graphics_match]);
        profile->graphics_kind = csb_v1_boot_graphics_kind(graphics_path);
        profile->variant_id = g_csb_boot_graphics_variants[graphics_match];
    }
    if (profile->dungeon_verified) {
        csb_v1_boot_copy(profile->dungeon_path, sizeof(profile->dungeon_path),
                         dungeon_path);
        csb_v1_boot_copy(profile->dungeon_md5, sizeof(profile->dungeon_md5),
                         g_csb_boot_dungeon_hashes[dungeon_match]);
    }

    profile->assets_verified = profile->graphics_verified && profile->dungeon_verified;
    if (profile->variant_id == CSB_V1_VARIANT_UNKNOWN && profile->dungeon_verified) {
        profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    }
    variant = csb_v1_runtime_get_variant_info(profile->variant_id);
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label),
                     variant->name);
    csb_v1_boot_copy(profile->media_ref, sizeof(profile->media_ref),
                     variant->media_ref);
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id),
                     profile->graphics_md5[0] ? profile->graphics_md5 : "unknown");
    if (profile->save_root[0] == '\0') {
        csb_v1_boot_set_save_root(profile, NULL);
    }
    (void)csb_v1_boot_scan_csbgraphics(profile, NULL);
    if (profile->assets_verified) {
        profile->state = CSB_V1_BOOT_STATE_ASSETS_READY;
        return 0;
    }
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    return -1;
}

int csb_v1_boot_probe_available(const char *data_dir)
{
    CSB_V1_BootProfile profile;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, data_dir) == 0 ? 1 : 0;
}

int csb_v1_boot_enter_game(CSB_V1_BootProfile *profile)
{
    if (!profile || !profile->assets_verified) return -1;
    /* The launcher may carry several game profiles at once.  Do not let an
     * aggregate READY bit alone hand a non-CSB or partial profile to the CSB
     * runtime: ReDMCSB enters the CSB dungeon only after the CSB entrance/media
     * path has selected C28_ENTRANCE_CSB and the load path has a dungeon header
     * to consume.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0 ||
        !profile->graphics_verified ||
        !profile->dungeon_verified ||
        profile->graphics_path[0] == '\0' ||
        profile->dungeon_path[0] == '\0') {
        return -1;
    }
    /* Explicit launch-to-runtime assumption gate.  This is the source-locked
     * boundary that catches DM1-shape defaults leaking into the CSB profile
     * (e.g. (11,29) HoC start, raw DM1 variant ordinal, non-CSB tick quantum).
     * The reason string is exposed via csb_v1_boot_last_assumption_reason()
     * so a misrouted profile can be attributed to a specific CSB invariant.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944
     * Source: csb_v1_runtime_pc34_compat.h CSB_V1_TICK_MS_NOMINAL */
    if (csb_v1_boot_assume_no_dm1_runtime(profile) != 0) {
        return -1;
    }
    /* Re-entering the CSB profile replaces the live dungeon context just as
     * ReDMCSB's global dungeon/map state is replaced when a new game is
     * loaded.  Clear the previous heap-owned runtime before csb_v1_runtime_init
     * overwrites its handle fields.
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 */
    csb_v1_runtime_cleanup(&profile->runtime);
    csb_v1_runtime_init(&profile->runtime, profile->asset_root);
    profile->runtime.variant_id = profile->variant_id;
    profile->runtime.difficulty = CSB_V1_DIFFICULTY_HARD;
    profile->runtime.save_dir = profile->save_root;
    profile->runtime.dungeon_path = profile->dungeon_path;
    profile->runtime.graphics_path = profile->graphics_path;
    profile->runtime.dungeon_asset.path = profile->dungeon_path;
    profile->runtime.dungeon_asset.kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    profile->runtime.graphics_asset.path = profile->graphics_path;
    profile->runtime.graphics_asset.kind = profile->graphics_kind;
    /* Copy entrance/start map indices from the boot profile so the runtime
     * honours the source-locked new-game map selection.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (C255_MAP_INDEX_ENTRANCE)
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 (new-game map 0) */
    profile->runtime.entrance_map_index = profile->entrance_map_index;
    profile->runtime.start_map_index = profile->start_map_index;
    profile->runtime.state = CSB_STATE_TITLE;
    profile->runtime.chaos_magic.magic_initialized = 1;
    profile->runtime.chaos_magic.spell_grid_version = 0U;
    profile->runtime.chaos_magic.chaos_level = 0U;
    /* CSB shows engine version 2.1 on the title/dialog surface.
     * Flip the shared helper when a verified CSB boot profile
     * actually enters the CSB runtime, then reset it on cleanup.
     * Source: ReDMCSB CHANGE8_13; DIALOG.C:2014-2023. */
    csb_v1_engine_version_display_set_csb(1);
    profile->engine_version_displayed = 1;
    if (profile->imported_party_ready) {
        (void)csb_v1_runtime_set_party_state(&profile->runtime,
                                             &profile->imported_party);
    }
    (void)csb_v1_boot_load_object_names_m564(profile);
    /* Load the verified DUNGEON.DAT into the runtime so that the
     * dungeon-layer accessors (csb_v1_dungeon_get_current_level,
     * csb_v1_dungeon_get_square_type, ...) become live immediately
     * after launch — without a second hash search or a follow-up
     * csb_v1_runtime_boot() call from the game-view.
     *
     * The dungeon is heap-allocated and owned by the runtime profile
     * (dungeon_handle).  csb_v1_runtime_cleanup() / csb_v1_boot_cleanup()
     * are responsible for releasing it.
     *
     * Failure is non-fatal: if the verified path cannot be opened
     * (e.g. archive-backed path not yet materialized by M12), the
     * runtime continues with dungeon_handle == NULL and the dungeon
     * accessors return ENDOF — matching csb_v1_runtime_boot()'s
     * pre-existing tolerant behaviour.
     *
     * Source: CSBWin/CSBCode.cpp:6800-6950 LoadDungeon
     * Source: ReDMCSB DUNGEON.C F0237 dungeon load entry
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 entrance micro-dungeon */
    {
        CSB_V1_DungeonData *dungeon = (CSB_V1_DungeonData *)calloc(1, sizeof(CSB_V1_DungeonData));
        if (dungeon) {
            if (csb_v1_dungeon_load_from_file(dungeon, profile->dungeon_path) == 0) {
                profile->runtime.dungeon_handle = dungeon;
                csb_v1_dungeon_set_current(dungeon);
                csb_v1_dungeon_set_current_level(0);
            } else {
                free(dungeon);
                profile->runtime.dungeon_handle = NULL;
            }
        }
    }
    profile->state = CSB_V1_BOOT_STATE_RUNTIME_READY;
    return 0;
}

void csb_v1_boot_cleanup(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    /* The boot profile owns the runtime handoff dungeon.  Release it through
     * the same runtime cleanup path used by csb_v1_runtime_boot() so the
     * singleton dungeon/map accessors do not retain a stale CSB context after
     * leaving the profile.
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 */
    csb_v1_runtime_cleanup(&profile->runtime);
    csb_v1_boot_reset_csbgraphics(profile);
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    memset(&profile->runtime, 0, sizeof(profile->runtime));
    csb_v1_engine_version_display_set_csb(0);
    profile->engine_version_displayed = 0;
}

size_t csb_v1_boot_diagnostic_report(const CSB_V1_BootProfile *profile,
                                     char *buf,
                                     size_t buf_size)
{
    int n;
    const char *engine_version_str;
    if (!profile || !buf || buf_size == 0U) return 0U;
    engine_version_str = csb_v1_engine_version_display_get();
    n = snprintf(buf, buf_size,
                 "=== CSB V1 Boot Profile ===\n"
                 "state=%d verified=%s variant=%s media=%s\n"
                 "asset_root=%s\n"
                 "graphics=%s md5=%s\n"
                 "dungeon=%s md5=%s\n"
                 "save_root=%s tick_ms=%u entrance_map=%u start_map=%u\n"
                 "engine_version=%s flipped=%s\n"
                 "csbgraphics_scan attempted=%s result=%s plan=%s ready=%s planned=%u\n"
                 "cmp_import attempted=%s succeeded=%s slot=%d champions=%d\n"
                 "imported_party_ready=%s\n",
                 (int)profile->state,
                 profile->assets_verified ? "YES" : "NO",
                 profile->variant_label,
                 profile->media_ref,
                 profile->asset_root[0] ? profile->asset_root : "(unset)",
                 profile->graphics_path[0] ? profile->graphics_path : "(missing)",
                 profile->graphics_md5[0] ? profile->graphics_md5 : "(missing)",
                 profile->dungeon_path[0] ? profile->dungeon_path : "(missing)",
                 profile->dungeon_md5[0] ? profile->dungeon_md5 : "(missing)",
                 profile->save_root[0] ? profile->save_root : "(unset)",
                 (unsigned)profile->tick_ms,
                 (unsigned)profile->entrance_map_index,
                 (unsigned)profile->start_map_index,
                 engine_version_str ? engine_version_str : "(unset)",
                 profile->engine_version_displayed ? "YES" : "NO",
                 profile->csbgraphics_scan_attempted ? "YES" : "NO",
                 csb_v1_csbgraphics_dat_real_result_name(
                     profile->csbgraphics_scan_result),
                 csb_v1_csbgraphics_m11_runtime_plan_result_name(
                     profile->csbgraphics_plan_result),
                 profile->csbgraphics_m11_plan.ready ? "YES" : "NO",
                 (unsigned)profile->csbgraphics_m11_plan.planned_count,
                 profile->cmp_import_attempted ? "YES" : "NO",
                 profile->cmp_import_succeeded ? "YES" : "NO",
                 profile->cmp_imported_slot,
                 profile->cmp_imported_champion_count,
                 profile->imported_party_ready ? "YES" : "NO");
    if (n < 0) return 0U;
    return (size_t)n < buf_size ? (size_t)n : buf_size - 1U;
}

void csb_v1_boot_print_summary(const CSB_V1_BootProfile *profile)
{
    if (!profile) {
        printf("CSB: no boot profile\n");
        return;
    }
    printf("CSB: %s assets=%s dungeon=%s graphics=%s\n",
           profile->variant_label,
           profile->assets_verified ? "READY" : "missing",
           profile->dungeon_verified ? "ok" : "missing",
           profile->graphics_verified ? "ok" : "missing");
}

/* ── CSB V1 boot profile -> M11 entry guard ─────────────────────
 *
 * The canonical CSB V1 media hash registry below mirrors the four
 * graphics md5s and the one dungeon md5 the launcher scans for.
 * The gate hashes here MUST stay in sync with g_csb_boot_graphics_hashes
 * and g_csb_boot_dungeon_hashes above; the registry is the source of
 * truth that the M11 dispatch consults before activating a CSB launch.
 *
 * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB media class
 *   detection by hash, not by filename/path).
 * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 (new-game dungeon
 *   load is gated on a hash-known dungeon header). */
static const char *const g_csb_m11_entry_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611", /* PC DOS 3.4 English         MEDIA278 */
    "ebf6a57af3f27782e358c0490bfd2f2e", /* Atari ST 2.0/2.1 English   MEDIA332 */
    "291e1bc6803e3dc4b974c60117ca5d68", /* Amiga 3.5 English          MEDIA529 */
    "cefaddfdf5651df2c91f61b5611a8362", /* Amiga 3.5 Multilanguage    MEDIA529 */
    NULL
};

static const char *const g_csb_m11_entry_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de", /* shared CSB V1 DUNGEON.DAT  MEDIA278/332/529 */
    NULL
};

static int csb_v1_md5_is_canonical_graphics(const char *md5)
{
    size_t i;
    if (!md5 || md5[0] == '\0') return 0;
    for (i = 0U; g_csb_m11_entry_graphics_hashes[i] != NULL; ++i) {
        if (strcmp(md5, g_csb_m11_entry_graphics_hashes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int csb_v1_md5_is_canonical_dungeon(const char *md5)
{
    size_t i;
    if (!md5 || md5[0] == '\0') return 0;
    for (i = 0U; g_csb_m11_entry_dungeon_hashes[i] != NULL; ++i) {
        if (strcmp(md5, g_csb_m11_entry_dungeon_hashes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void csb_v1_boot_gate_set_reason(char *reason,
                                        size_t reason_size,
                                        const char *fmt,
                                        ...)
{
    if (!reason || reason_size == 0U) return;
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(reason, reason_size, fmt, args);
        va_end(args);
        reason[reason_size - 1U] = '\0';
    }
}

int csb_v1_boot_graphics_dungeon_m11_entry_gate(const char *graphics_md5,
                                                const char *dungeon_md5,
                                                char *reason,
                                                size_t reason_size)
{
    if (!graphics_md5 || graphics_md5[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: GRAPHICS md5 is empty "
            "(scanner did not record a matched graphics hash)");
        return 0;
    }
    if (!dungeon_md5 || dungeon_md5[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: DUNGEON md5 is empty "
            "(scanner did not record a matched dungeon hash)");
        return 0;
    }
    if (!csb_v1_md5_is_canonical_graphics(graphics_md5)) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: GRAPHICS md5 %s is not in the canonical "
            "CSB V1 media registry (PC3.4EN / Atari ST 2.x / Amiga 3.x)",
            graphics_md5);
        return 0;
    }
    if (!csb_v1_md5_is_canonical_dungeon(dungeon_md5)) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: DUNGEON md5 %s is not in the canonical "
            "CSB V1 media registry",
            dungeon_md5);
        return 0;
    }
    csb_v1_boot_gate_set_reason(reason, reason_size,
        "CSB M11 entry guard: GRAPHICS=%s DUNGEON=%s accepted",
        graphics_md5, dungeon_md5);
    return 1;
}

int csb_v1_boot_profile_m11_entry_gate(const CSB_V1_BootProfile *profile,
                                       char *reason,
                                       size_t reason_size)
{
    if (!profile) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: NULL boot profile "
            "(launcher did not initialize a CSB profile before dispatch)");
        return 0;
    }
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: profile game_id=%s != %s "
            "(foreign game reached the CSB entry path)",
            profile->game_id[0] ? profile->game_id : "(empty)",
            CSB_V1_BOOT_GAME_ID);
        return 0;
    }
    if (profile->state < CSB_V1_BOOT_STATE_ASSETS_READY) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: boot state=%d below ASSETS_READY "
            "(scanner did not record both required assets)",
            (int)profile->state);
        return 0;
    }
    if (!profile->assets_verified ||
        !profile->graphics_verified ||
        !profile->dungeon_verified) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: assets_verified=%d graphics_verified=%d "
            "dungeon_verified=%d (one or more required files missing)",
            profile->assets_verified,
            profile->graphics_verified,
            profile->dungeon_verified);
        return 0;
    }
    if (profile->graphics_path[0] == '\0' ||
        profile->dungeon_path[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: empty asset path "
            "(graphics_path=%s dungeon_path=%s)",
            profile->graphics_path[0] ? profile->graphics_path : "(empty)",
            profile->dungeon_path[0] ? profile->dungeon_path : "(empty)");
        return 0;
    }
    return csb_v1_boot_graphics_dungeon_m11_entry_gate(
        profile->graphics_md5, profile->dungeon_md5, reason, reason_size);
}

const char *csb_v1_boot_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C F0806 lines 409-441: CSB entrance setup and C28_ENTRANCE_CSB palette\n"
        "ReDMCSB ENTRANCE.C F0806 lines 857-883: entrance waits then switches G0298_B_NewGame\n"
        "ReDMCSB LOADSAVE.C F0435 lines 1940-1944: new-game party location and map 0\n"
        "ReDMCSB BASE.C lines 36-39: G0298_B_NewGame boot/load mode storage\n";
}
