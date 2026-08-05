/*
 * dm1_v22_asset_pipeline.c — DM1 V2.2 Modern Asset Pipeline Implementation
 *
 * Generated/modern art path for Dungeon Master I — V2.2 presentation.
 *
 * Implements:
 *   - Asset provenance metadata and naming
 *   - Fallback chain definitions and lookup
 *   - Modern asset manifest discovery and validation
 *   - Asset loading with full fallback chain traversal
 *   - Asset validation (dimensions, format, pixel buffer consistency)
 *   - Best-available provenance selection
 *
 * Pipeline contract (V2.2 only — behind DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION):
 *   Modern RGBA texture (PNG/TGA, 1920×1080)
 *     → direct present surface blit
 *     → no EPX, no palette expansion, no indexed conversion
 *
 * Fallback chain:
 *   MODERN (V2.2) → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)
 *
 * Source-lock anchors:
 *   ReDMCSB DUNGEON.C:2238-2246   — square type decode for shape dispatch
 *   ReDMCSB DEFS.H:922-941         — M034_SQUARE_TYPE enumeration
 *   ReDMCSB DUNVIEW.C:6697-6816    — wall/floor draw composition order
 *   ReDMCSB PANEL.C:418-428        — G0304_i_DungeonViewPaletteIndex (6 levels)
 *   ReDMCSB DRAWVIEW.C:1-200       — creature sprite framing (G0011_i_CreaturePosture)
 *   ReDMCSB PROJEXPL.C:43-165      — projectile/explosion routing
 *   Firestaff dm1_v22_shapes.c     — shape type → modern renderer bridge
 */

#include "dm1/v2/modern/dm1_v22_asset_pipeline.h"
#include "dm1_v2_asset_pipeline_pc34.h"  /* For DM1_V2_ASSET_MODE_MODERN / _UPSCALED */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#if defined(FIRESTAFF_HAS_ZLIB) && FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

/* ══════════════════════════════════════════════════════════════════════
 * Module State
 * ══════════════════════════════════════════════════════════════════════ */

/* Path to the modern asset manifest JSON (set by dm1_v22_set_manifest_path). */
static char g_manifest_path[DM1_V22_ASSET_PATH_MAX] = {0};

/* Whether the manifest has been loaded (and validated as parseable). */
static int g_manifest_loaded = 0;

/* Whether the modern asset pack is installed and has critical categories.
 * Set by dm1_v22_load_manifest() after successful manifest parse. */
static int g_modern_assets_installed = 0;

/* V2.2 assets installed flag (mirrors the m11_v22_get_installed() API
 * but scoped here so the asset pipeline is self-contained). */
static int g_v22_installed = 0;

/* ══════════════════════════════════════════════════════════════════════
 * Provenance Naming
 * ══════════════════════════════════════════════════════════════════════ */

const char* dm1_v22_provenance_name(DM1_V22_AssetProvenance p) {
    switch (p) {
        case DM1_V22_PROVENANCE_UNKNOWN:    return "unknown";
        case DM1_V22_PROVENANCE_ORIGINAL:    return "original";     /* V1: 320×200 indexed */
        case DM1_V22_PROVENANCE_FILTERED:    return "filtered";     /* V2.0: scanline/palette */
        case DM1_V22_PROVENANCE_UPSCALED:    return "upscaled";    /* V2.1: EPX 2× → RGBA */
        case DM1_V22_PROVENANCE_MODERN:      return "modern";       /* V2.2: 1920×1080 RGBA */
        default:                             return "???";
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Fallback Chain Definitions
 * ══════════════════════════════════════════════════════════════════════ */

/* Predefined fallback chains, one per presentation mode.
 * Index 0 = preferred (highest quality). */
static const DM1_V22_FallbackChain k_fallback_chains[] = {
    /* DM1_V22_PROVENANCE_MODERN: modern → upscaled → filtered → original */
    [DM1_V22_PROVENANCE_MODERN] = {
        .levels = {
            DM1_V22_PROVENANCE_MODERN,
            DM1_V22_PROVENANCE_UPSCALED,
            DM1_V22_PROVENANCE_FILTERED,
            DM1_V22_PROVENANCE_ORIGINAL
        },
        .count = 4
    },
    /* DM1_V22_PROVENANCE_UPSCALED: upscaled → filtered → original */
    [DM1_V22_PROVENANCE_UPSCALED] = {
        .levels = {
            DM1_V22_PROVENANCE_UPSCALED,
            DM1_V22_PROVENANCE_FILTERED,
            DM1_V22_PROVENANCE_ORIGINAL,
            DM1_V22_PROVENANCE_UNKNOWN
        },
        .count = 3
    },
    /* DM1_V22_PROVENANCE_FILTERED: filtered → original */
    [DM1_V22_PROVENANCE_FILTERED] = {
        .levels = {
            DM1_V22_PROVENANCE_FILTERED,
            DM1_V22_PROVENANCE_ORIGINAL,
            DM1_V22_PROVENANCE_UNKNOWN,
            DM1_V22_PROVENANCE_UNKNOWN
        },
        .count = 2
    },
    /* DM1_V22_PROVENANCE_ORIGINAL: original only */
    [DM1_V22_PROVENANCE_ORIGINAL] = {
        .levels = {
            DM1_V22_PROVENANCE_ORIGINAL,
            DM1_V22_PROVENANCE_UNKNOWN,
            DM1_V22_PROVENANCE_UNKNOWN,
            DM1_V22_PROVENANCE_UNKNOWN
        },
        .count = 1
    }
};

const DM1_V22_FallbackChain* dm1_v22_fallback_for_mode(DM1_V22_AssetProvenance mode) {
    if (mode <= DM1_V22_PROVENANCE_UNKNOWN || mode > DM1_V22_PROVENANCE_MODERN) {
        return NULL;
    }
    return &k_fallback_chains[mode];
}

DM1_V22_AssetProvenance dm1_v22_fallback_next(DM1_V22_AssetProvenance current) {
    switch (current) {
        case DM1_V22_PROVENANCE_MODERN:   return DM1_V22_PROVENANCE_UPSCALED;
        case DM1_V22_PROVENANCE_UPSCALED:  return DM1_V22_PROVENANCE_FILTERED;
        case DM1_V22_PROVENANCE_FILTERED: return DM1_V22_PROVENANCE_ORIGINAL;
        case DM1_V22_PROVENANCE_ORIGINAL: return DM1_V22_PROVENANCE_UNKNOWN;
        default:                           return DM1_V22_PROVENANCE_UNKNOWN;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Category Naming
 * ══════════════════════════════════════════════════════════════════════ */

const char* dm1_v22_category_name(DM1_V22_AssetCategory cat) {
    switch (cat) {
        case DM1_V22_CATEGORY_UNKNOWN:     return "unknown";
        case DM1_V22_CATEGORY_WALL:       return "wall";
        case DM1_V22_CATEGORY_FLOOR:      return "floor";
        case DM1_V22_CATEGORY_CEILING:    return "ceiling";
        case DM1_V22_CATEGORY_DOOR:       return "door";
        case DM1_V22_CATEGORY_CREATURE:   return "creature";
        case DM1_V22_CATEGORY_OBJECT:     return "object";
        case DM1_V22_CATEGORY_PROJECTILE: return "projectile";
        case DM1_V22_CATEGORY_EXPLOSION:  return "explosion";
        case DM1_V22_CATEGORY_FLUXCAGE:   return "fluxcage";
        case DM1_V22_CATEGORY_FONT:        return "font";
        case DM1_V22_CATEGORY_UI_CHROME:  return "ui_chrome";
        case DM1_V22_CATEGORY_PANEL:      return "panel";
        case DM1_V22_CATEGORY_TITLE:       return "title";
        case DM1_V22_CATEGORY_ENTRANCE:    return "entrance";
        default:                            return "???";
    }
}

const DM1_V22_AssetDescriptor* dm1_v22_get_missing_descriptor(int* out_w, int* out_h) {
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    return NULL;
}

/* ══════════════════════════════════════════════════════════════════════
 * Modern Asset Manifest Discovery
 * ══════════════════════════════════════════════════════════════════════ */

void dm1_v22_set_manifest_path(const char* manifest_path) {
    if (!manifest_path) return;
    strncpy(g_manifest_path, manifest_path, sizeof(g_manifest_path) - 1);
    g_manifest_path[sizeof(g_manifest_path) - 1] = '\0';
    /* Invalidate entire manifest cache when path changes */
    g_manifest_loaded = 0;
    g_modern_assets_installed = 0;
    /* Also reset V2 installed flag since manifest changed */
    g_v22_installed = 0;
}

const char* dm1_v22_get_manifest_path(void) {
    return g_manifest_path[0] ? g_manifest_path : "";
}

/* Minimal JSON manifest parser.
 * Accepts the subset of JSON needed for modern_asset_manifest.json:
 *   { "categories": { "wall_shapes": [ {"id": "...", "source_file": "..."} ], ... } }
 * This avoids adding a JSON dependency while providing basic manifest support.
 *
 * Returns: -1 on error (malformed), 0 if file not found, 1 on success.
 * Sets g_modern_assets_installed=1 on success. */
static int parse_minimal_manifest(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) return 0;

    /* Read the file into a buffer (max 64 KB). */
    char buf[65536];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    if (n == 0) return -1;
    buf[n] = '\0';

    /* Very lightweight JSON sanity check:
     * 1. Must start with '{'
     * 2. Must contain "categories": {
     * 3. Must contain at least one top-level category key followed by ':'
     * 4. After ':', there must be '[' (array start)
     *
     * We don't implement a full JSON parser — we just verify the structure
     * is sufficient for the renderer to build asset paths. */

    if (n < 2 || buf[0] != '{') {
        return -1;
    }

    /* Find "categories" key */
    const char* cats = strstr(buf, "\"categories\"");
    if (!cats) return -1;

    /* Skip past "categories" and any whitespace, find the ':' */
    const char* p = cats + strlen("\"categories\"");
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != ':') return -1;
    p++;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != '{') return -1;

    /* Verify at least one category key exists: "category_name":
     * We look for a pattern like "wall_shapes" or "creature_shapes" in quotes. */
    const char* wall = strstr(buf, "\"wall_shapes\"");
    const char* creature = strstr(buf, "\"creature_shapes\"");
    const char* floor_s = strstr(buf, "\"floor_shapes\"");

    if (!wall && !creature && !floor_s) {
        return -1; /* No known critical categories found */
    }

    /* If we have wall_shapes, verify it has an array with at least one entry */
    if (wall) {
        const char* arr_start = wall + strlen("\"wall_shapes\"");
        while (*arr_start && (*arr_start == ' ' || *arr_start == '\t' || *arr_start == '\n' || *arr_start == '\r' || *arr_start == ':')) arr_start++;
        if (*arr_start != '[') return -1;
        /* Look for closing ']' — if none, malformed */
        const char* arr_end = strchr(arr_start, ']');
        if (!arr_end) return -1;
    }

    /* All basic structural checks passed */
    g_modern_assets_installed = 1;
    g_manifest_loaded = 1;
    return 1;
}

int dm1_v22_load_manifest(const char* manifest_path) {
    const char* path = manifest_path ? manifest_path : g_manifest_path;
    if (!path || !path[0]) return 0;
    return parse_minimal_manifest(path);
}

int dm1_v22_validate_manifest(const char* manifest_path) {
    const char* path = manifest_path ? manifest_path : g_manifest_path;
    if (!path || !path[0]) return 0;

    FILE* fp = fopen(path, "r");
    if (!fp) return 0;

    /* Quick structural checks: see parse_minimal_manifest() above.
     * We repeat the checks here so validate is self-contained. */
    char buf[65536];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    if (n == 0) return -1;
    buf[n] = '\0';

    if (buf[0] != '{') return -1;

    const char* cats = strstr(buf, "\"categories\"");
    if (!cats) return -1;

    const char* p = cats + strlen("\"categories\"");
    while (*p && ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != ':') return -1;
    p++;
    while (*p && ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != '{') return -1;

    /* Check critical categories */
    int found_critical = 0;
    const char* wall = strstr(buf, "\"wall_shapes\"");
    const char* creature = strstr(buf, "\"creature_shapes\"");
    const char* floor_s = strstr(buf, "\"floor_shapes\"");

    if (wall)   found_critical = 1;
    if (creature) found_critical = 1;
    if (floor_s)  found_critical = 1;

    if (!found_critical) return -1; /* No critical categories */

    /* If we have wall_shapes, verify array structure */
    if (wall) {
        const char* arr_start = wall + strlen("\"wall_shapes\"");
        while (*arr_start && ( *arr_start == ' ' || *arr_start == '\t' || *arr_start == '\n' || *arr_start == '\r' || *arr_start == ':')) arr_start++;
        if (*arr_start != '[') return -1;
        const char* arr_end = strchr(arr_start, ']');
        if (!arr_end) return -1;
        /* Verify the array is not empty (has at least one entry with "id") */
        const char* id_key = strstr(arr_start, "\"id\"");
        if (!id_key || id_key > arr_end) return 0; /* Missing id in first entry → partial */
    }

    return 1; /* Complete and valid */
}

int dm1_v22_modern_assets_available(void) {
    /* If manifest was already loaded successfully, return that state */
    if (g_manifest_loaded) return g_modern_assets_installed;
    /* If no manifest path is set, no assets are available.
     * Empty path: cannot open a file — mark cache as checked and return 0. */
    if (!g_manifest_path[0]) {
        g_modern_assets_installed = 0;
        g_manifest_loaded = 1;  /* mark as checked so we don't re-enter */
        return 0;
    }
    /* Otherwise try to load from the configured path */
    int r = parse_minimal_manifest(g_manifest_path);
    g_modern_assets_installed = (r == 1) ? 1 : 0;
    g_manifest_loaded = 1;
    return g_modern_assets_installed;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Installed State (mirrors m11_v22_set_installed/get_installed)
 * ══════════════════════════════════════════════════════════════════════ */

void dm1_v22_set_installed(int installed) {
    g_v22_installed = installed ? 1 : 0;
}

int dm1_v22_get_installed(void) {
    return g_v22_installed;
}

/* ══════════════════════════════════════════════════════════════════════
 * Asset Loading with Fallback Chain
 *
 * Given a category/asset_id and desired provenance, walks the fallback
 * chain and returns the first successfully opened asset.
 *
 * Since we don't ship actual asset files, the implementation validates
 * the pipeline state and descriptor semantics. A full file-loading
 * implementation would use the modern asset root + category + asset_id
 * to build the file path and load PNG/TGA/RGBA data.
 * ══════════════════════════════════════════════════════════════════════ */

/* Build the expected file path for a given provenance/category/asset_id.
 * Returns 1 if the path was built into out_path, 0 if the combination
 * is not representable as a file path (e.g. ORIGINAL has no file). */
static int build_asset_path(DM1_V22_AssetProvenance prov,
                            const char* category,
                            const char* asset_id,
                            char* out_path, size_t out_size) {
    if (!out_path || out_size == 0) return 0;

    /* ORIGINAL (V1) and FILTERED (V2.0) and UPSCALED (V2.1) assets
     * are not standalone files — they are generated at runtime from
     * GRAPHICS.DAT. Only MODERN (V2.2) assets have explicit files. */
    if (prov != DM1_V22_PROVENANCE_MODERN) {
        out_path[0] = '\0';
        return 0;
    }

    /* Modern assets: <root>/<category>/<asset_id>.png or .tga */
    if (!g_manifest_path[0]) {
        out_path[0] = '\0';
        return 0;
    }

    /* Strip the manifest filename to get the asset root */
    const char* root = g_manifest_path;
    const char* last_slash = NULL;
    for (const char* cp = root; *cp; cp++) {
        if (*cp == '/' || *cp == '\\') last_slash = cp;
    }
    size_t root_len = last_slash ? (size_t)(last_slash - root) : strlen(root);

    /* Build <root>/<category>/<asset_id>.png */
    int n = snprintf(out_path, out_size, "%.*s/%s/%s.png",
                     (int)root_len, root,
                     category ? category : "unknown",
                     asset_id  ? asset_id : "unknown");
    if (n < 0 || (size_t)n >= out_size) {
        out_path[0] = '\0';
        return 0;
    }
    return 1;
}

static uint32_t png_u32(const unsigned char* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int png_unfilter(unsigned char* rows, size_t stride, int height,
                        int bpp) {
    for (int y = 0; y < height; y++) {
        unsigned char* row = rows + (size_t)y * stride;
        const unsigned char* prev = y ? row - stride : NULL;
        unsigned int filter = row[0];
        for (size_t x = 1; x < stride; x++) {
            unsigned int left = x > (size_t)bpp ? row[x - bpp] : 0;
            unsigned int up = prev ? prev[x] : 0;
            unsigned int up_left = prev && x > (size_t)bpp ? prev[x - bpp] : 0;
            switch (filter) {
                case 0: break;
                case 1: row[x] = (unsigned char)(row[x] + left); break;
                case 2: row[x] = (unsigned char)(row[x] + up); break;
                case 3: row[x] = (unsigned char)(row[x] + ((left + up) / 2)); break;
                case 4: {
                    int p = (int)left + (int)up - (int)up_left;
                    int pa = abs(p - (int)left), pb = abs(p - (int)up);
                    int pc = abs(p - (int)up_left);
                    unsigned int predict = pa <= pb && pa <= pc ? left :
                        (pb <= pc ? up : up_left);
                    row[x] = (unsigned char)(row[x] + predict);
                    break;
                }
                default: return 0;
            }
        }
    }
    return 1;
}

/* Decode the real non-interlaced RGB/RGBA PNG assets. Unsupported variants
 * are rejected; no replacement pixels are generated. */
static int png_decode_rgba(const char* path, int* out_w, int* out_h,
                           void** out_pixels, size_t* out_size) {
#if !defined(FIRESTAFF_HAS_ZLIB) || !FIRESTAFF_HAS_ZLIB
    (void)path; (void)out_w; (void)out_h; (void)out_pixels; (void)out_size;
    return 0;
#else
    static const unsigned char sig[8] = {0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a};
    FILE* fp = fopen(path, "rb");
    if (!fp || fseek(fp, 0, SEEK_END) != 0) { if (fp) fclose(fp); return 0; }
    long length = ftell(fp);
    if (length < 33) { fclose(fp); return 0; }
    rewind(fp);
    size_t input_size = (size_t)length;
    unsigned char* input = (unsigned char*)malloc(input_size);
    if (!input || fread(input, 1, input_size, fp) != input_size) {
        free(input); fclose(fp); return 0;
    }
    fclose(fp);
    int ok = 0;
    uint32_t width = 0, height = 0;
    unsigned char depth = 0, type_value = 0, interlace = 0;
    unsigned char* compressed = NULL;
    size_t compressed_size = 0;
    if (memcmp(input, sig, sizeof(sig)) != 0) goto done;
    size_t pos = 8;
    while (pos + 12 <= input_size) {
        uint32_t chunk_len = png_u32(input + pos);
        if ((size_t)chunk_len > input_size - pos - 12) goto done;
        const unsigned char* chunk_type = input + pos + 4;
        const unsigned char* data = input + pos + 8;
        if (memcmp(chunk_type, "IHDR", 4) == 0) {
            if (chunk_len != 13 || width != 0) goto done;
            width = png_u32(data); height = png_u32(data + 4);
            depth = data[8]; type_value = data[9]; interlace = data[12];
            if (data[10] != 0 || data[11] != 0) goto done;
        } else if (memcmp(chunk_type, "IDAT", 4) == 0) {
            if ((size_t)chunk_len > SIZE_MAX - compressed_size) goto done;
            unsigned char* grown = (unsigned char*)realloc(
                compressed, compressed_size + (size_t)chunk_len);
            if (!grown) goto done;
            compressed = grown;
            memcpy(compressed + compressed_size, data, chunk_len);
            compressed_size += chunk_len;
        } else if (memcmp(chunk_type, "IEND", 4) == 0) break;
        pos += (size_t)chunk_len + 12;
    }
    if (!width || !height || depth != 8 || interlace != 0 ||
        (type_value != 2 && type_value != 6) || !compressed_size ||
        width > INT_MAX || height > INT_MAX || compressed_size > UINT_MAX) goto done;
    int channels = type_value == 6 ? 4 : 3;
    size_t row_bytes = (size_t)width * (size_t)channels;
    if (row_bytes > SIZE_MAX - 1 || (size_t)height > SIZE_MAX / (row_bytes + 1)) goto done;
    size_t raw_size = (row_bytes + 1) * (size_t)height;
    if (raw_size > UINT_MAX) goto done;
    unsigned char* rows = (unsigned char*)malloc(raw_size);
    if (!rows) goto done;
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    if (inflateInit(&stream) != Z_OK) { free(rows); goto done; }
    stream.next_in = compressed; stream.avail_in = (uInt)compressed_size;
    stream.next_out = rows; stream.avail_out = (uInt)raw_size;
    int result = inflate(&stream, Z_FINISH);
    size_t actual = stream.total_out;
    inflateEnd(&stream);
    if (result != Z_STREAM_END || actual != raw_size ||
        !png_unfilter(rows, row_bytes + 1, (int)height, channels)) {
        free(rows); goto done;
    }
    if ((size_t)width > SIZE_MAX / (size_t)height / 4) { free(rows); goto done; }
    size_t pixel_size = (size_t)width * (size_t)height * 4;
    unsigned char* pixels = (unsigned char*)malloc(pixel_size);
    if (!pixels) { free(rows); goto done; }
    for (uint32_t y = 0; y < height; y++) {
        const unsigned char* src = rows + (size_t)y * (row_bytes + 1) + 1;
        unsigned char* dst = pixels + (size_t)y * (size_t)width * 4;
        for (uint32_t x = 0; x < width; x++) {
            dst[x * 4 + 0] = src[x * channels + 0];
            dst[x * 4 + 1] = src[x * channels + 1];
            dst[x * 4 + 2] = src[x * channels + 2];
            dst[x * 4 + 3] = channels == 4 ? src[x * 4 + 3] : 255;
        }
    }
    free(rows);
    *out_w = (int)width; *out_h = (int)height;
    *out_pixels = pixels; *out_size = pixel_size;
    ok = 1;
done:
    free(compressed); free(input);
    return ok;
#endif
}

/* Attempt to load a single asset file (PNG or TGA) from disk.
 * This is the low-level file load used within the fallback chain.
 *
 * We implement a minimal PNG header read to get dimensions without
 * a full PNG library. TGA is not implemented (would require the
 * full asset pack to be present).
 *
 * Returns: 1 if file was found and header is valid, 0 otherwise.
 * On success, out_w/out_h and out_format are filled in.
 * NOTE: actual pixel loading is not implemented in this probe-friendly
 * version — a metadata-only result is not renderable and callers must keep
 * the route no-draw until a complete pixel decoder is bound. */
static int try_load_asset_file(const char* file_path,
                               int* out_w, int* out_h,
                               DM1_V22_AssetFormat* out_format,
                               void** out_pixels, size_t* out_pixels_size) {
    if (!file_path || !file_path[0]) return 0;
    if (out_pixels) *out_pixels = NULL;
    if (out_pixels_size) *out_pixels_size = 0;
    if (png_decode_rgba(file_path, out_w, out_h, out_pixels, out_pixels_size)) {
        if (out_format) *out_format = DM1_V22_FORMAT_PNG;
        return 1;
    }

    FILE* fp = fopen(file_path, "rb");
    if (!fp) return 0;

    unsigned char header[8];
    size_t hdr_n = fread(header, 1, sizeof(header), fp);
    fclose(fp);

    if (hdr_n < 8) return 0;

    /* PNG signature: 89 50 4E 47 0D 0A 1A 0A */
    if (header[0] == 0x89 && header[1] == 0x50 &&
        header[2] == 0x4E && header[3] == 0x47 &&
        header[4] == 0x0D && header[5] == 0x0A &&
        header[6] == 0x1A && header[7] == 0x0A) {
        /* Minimal PNG: read IHDR chunk to get width/height.
         * This requires scanning chunks, but we know IHDR is the first
         * chunk after the 8-byte signature. */
        FILE* fp2 = fopen(file_path, "rb");
        if (!fp2) return 0;
        unsigned char buf[256];
        size_t r = fread(buf, 1, sizeof(buf), fp2);
        fclose(fp2);
        if (r < 29) return 0; /* Not enough for IHDR */

        /* PNG chunk: length (4 bytes BE) + type (4 bytes) + data + CRC (4 bytes)
         * Chunk type for IHDR is "IHDR" = 0x49 0x48 0x44 0x52
         * Data is 13 bytes: width (4 BE), height (4 BE), bit depth, color type,
         * compression, filter, interlace */
        if (buf[12] != 0x49 || buf[13] != 0x48 || /* I */
            buf[14] != 0x44 || buf[15] != 0x52) { /* HDR */
            return 0;
        }

        /* Width: bytes 16-19 (big-endian uint32) */
        uint32_t w = ((uint32_t)buf[16] << 24) |
                     ((uint32_t)buf[17] << 16) |
                     ((uint32_t)buf[18] << 8)  |
                     ((uint32_t)buf[19]);
        uint32_t h = ((uint32_t)buf[20] << 24) |
                     ((uint32_t)buf[21] << 16) |
                     ((uint32_t)buf[22] << 8)  |
                     ((uint32_t)buf[23]);

        if (out_w)     *out_w     = (int)w;
        if (out_h)     *out_h     = (int)h;
        if (out_format) *out_format = DM1_V22_FORMAT_PNG;
        return 1;
    }

    /* TGA signature: no magic — a TGA without extension is identified
     * by having the extension area offset = 0 and a 0 vendor string.
     * We don't implement TGA loading without a full asset pack. */
    (void)out_format;
    return 0;
}

int dm1_v22_asset_load(const char* category, const char* asset_id,
                       DM1_V22_AssetProvenance desired_provenance,
                       DM1_V22_AssetDescriptor* out_desc) {
    if (!out_desc) return 0;

    /* Zero-initialise the output descriptor */
    memset(out_desc, 0, sizeof(*out_desc));

    if (!category || !asset_id) return 0;

    /* Get the fallback chain for the desired provenance */
    const DM1_V22_FallbackChain* chain = dm1_v22_fallback_for_mode(desired_provenance);
    if (!chain) return 0;

    /* Walk the fallback chain */
    for (int i = 0; i < chain->count; i++) {
        DM1_V22_AssetProvenance prov = chain->levels[i];

        /* Try to build the file path for this provenance level */
        char path[DM1_V22_ASSET_PATH_MAX];
        int path_valid = build_asset_path(prov, category, asset_id, path, sizeof(path));

        if (path_valid && path[0]) {
            /* Try to load the file */
            int w = 0, h = 0;
            DM1_V22_AssetFormat fmt = DM1_V22_FORMAT_UNKNOWN;
            void* pixels = NULL;
            size_t pixels_size = 0;
            if (try_load_asset_file(path, &w, &h, &fmt,
                                     &pixels, &pixels_size)) {
                /* File found and decoded — fill descriptor. */
                out_desc->provenance = prov;
                out_desc->category   = DM1_V22_CATEGORY_UNKNOWN; /* caller sets */
                out_desc->asset_id   = asset_id;
                out_desc->source_anchor = "dm1_v22_asset_pipeline.c:fallback_chain";
                strncpy(out_desc->file_path, path, sizeof(out_desc->file_path) - 1);
                out_desc->width   = w;
                out_desc->height  = h;
                out_desc->format  = fmt;
                out_desc->pixels  = pixels;
                out_desc->pixels_size = pixels_size;
                out_desc->is_valid = pixels != NULL ? 1 : 0;
                out_desc->load_attempted = 1;
                return 1; /* Found it */
            }
        }

        /* For non-file provenance levels (ORIGINAL/FILTERED/UPSCALED),
         * we don't have file paths but we record the provenance as
         * the best available by consulting the V2.1 pipeline state.
         *
         * Source-lock anchors for the runtime-resolved descriptors:
         *   UPSCALED  → dm1_v2_asset_pipeline_pc34.c:F0115
         *     (V2.1 EPX 2x pipeline; ReDMCSB DUNVIEW.C:4547-4602 F0115
         *      DrawObjectsCreaturesProjectiles — same call site that V1
         *      indexing feeds, only the EPX upscale is added on top)
         *   FILTERED  → dm1_v2_filter_palette_correct_pc34.c:F0337
         *     (V2.0 gamma/brightness LUT builder; ReDMCSB PANEL.C:418-428
         *      F0337_INVENTORY_SetDungeonViewPalette selects the 6-level
         *      palette; F0337 owns the index→palette routing)
         *   ORIGINAL  → dm1_v1_graphics_loader_pc34_compat.c:G0163
         *     (V1 wall set table; ReDMCSB DEFS.H G0163 holds the 30-wall
         *      parity bitmap selection)
         *
         * Each branch is gated on the corresponding V2.1 asset-mode
         * threshold (DM1_V2_ASSET_MODE_UPSCALED / _FILTERED / _ORIGINAL)
         * so the chain stays in sync with dm1_v22_best_available_provenance().
         * Without the FILTERED branch the V2.0 + V2.1 mismatch case
         * silently collapses to V1 ORIGINAL, bypassing the V2.0 filter
         * presentation. (Regressed by Phase 8 modern asset pipeline
         * initial seed; fixed by this gate.) */
        if (!path_valid || !path[0]) {
            DM1_V2_AssetMode mode = DM1_V2_GetAssetMode();
            if (prov == DM1_V22_PROVENANCE_UPSCALED) {
                /* V2.1 EPX 2x pipeline — available when V2.1 mode is
                 * UPSCALED (or MODERN, which is a superset). */
                if (mode >= DM1_V2_ASSET_MODE_UPSCALED) {
                    out_desc->provenance = prov;
                    out_desc->category   = DM1_V22_CATEGORY_UNKNOWN;
                    out_desc->asset_id   = asset_id;
                    out_desc->source_anchor = "dm1_v2_asset_pipeline_pc34.c:F0115_DrawObjectsCreaturesProjectiles";
                    out_desc->width   = 320;
                    out_desc->height = 200;
                    out_desc->format  = DM1_V22_FORMAT_RGBA;
                    out_desc->pixels  = NULL;
                    out_desc->pixels_size = 0;
                    out_desc->is_valid = 1; /* V2.1 upscaled pipeline is always valid */
                    out_desc->load_attempted = 1;
                    return 1;
                }
            } else if (prov == DM1_V22_PROVENANCE_FILTERED) {
                /* V2.0 filter pipeline — available when V2.1 mode is
                 * FILTERED (or UPSCALED / MODERN, which are supersets).
                 * The V2.0 path renders the V1 indexed framebuffer through
                 * the gamma/brightness LUT builder and is the lowest-cost
                 * "no-EPX" presentation mode. */
                if (mode >= DM1_V2_ASSET_MODE_FILTERED) {
                    out_desc->provenance = prov;
                    out_desc->category   = DM1_V22_CATEGORY_UNKNOWN;
                    out_desc->asset_id   = asset_id;
                    out_desc->source_anchor = "dm1_v2_filter_palette_correct_pc34.c:F0337_INVENTORY_SetDungeonViewPalette";
                    out_desc->width   = 320;
                    out_desc->height  = 200;
                    out_desc->format  = DM1_V22_FORMAT_PALETTED;
                    out_desc->pixels  = NULL;
                    out_desc->pixels_size = 0;
                    out_desc->is_valid = 1; /* V2.0 filter pipeline is always valid */
                    out_desc->load_attempted = 1;
                    return 1;
                }
            } else if (prov == DM1_V22_PROVENANCE_ORIGINAL) {
                /* V1 original is always available as the final fallback */
                out_desc->provenance = prov;
                out_desc->category   = DM1_V22_CATEGORY_UNKNOWN;
                out_desc->asset_id   = asset_id;
                out_desc->source_anchor = "dm1_v1_graphics_loader_pc34_compat.c:G0163_WallSetTable";
                out_desc->width   = 320;
                out_desc->height  = 200;
                out_desc->format  = DM1_V22_FORMAT_INDEXED;
                out_desc->pixels  = NULL;
                out_desc->pixels_size = 0;
                out_desc->is_valid = 1; /* V1 indexed is always available */
                out_desc->load_attempted = 1;
                return 1;
            }
        }
    }

    /* Exhausted fallback chain — return missing descriptor reference */
    out_desc->provenance = DM1_V22_PROVENANCE_UNKNOWN;
    out_desc->is_valid   = 0;
    out_desc->load_attempted = 1;
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Asset Lifecycle
 * ══════════════════════════════════════════════════════════════════════ */

void dm1_v22_asset_free(DM1_V22_AssetDescriptor* desc) {
    if (!desc) return;
    if (desc->pixels) {
        free(desc->pixels);
    }
    memset(desc, 0, sizeof(*desc));
}

/* ══════════════════════════════════════════════════════════════════════
 * Asset Validation
 * ══════════════════════════════════════════════════════════════════════ */

int dm1_v22_asset_validate(const DM1_V22_AssetDescriptor* desc) {
    if (!desc) return 0;

    /* Check provenance is valid */
    if (desc->provenance <= DM1_V22_PROVENANCE_UNKNOWN ||
        desc->provenance > DM1_V22_PROVENANCE_MODERN) {
        return 0;
    }

    /* Dimensions must be non-zero */
    if (desc->width <= 0 || desc->height <= 0) return 0;

    /* Format must be valid */
    if (desc->format <= DM1_V22_FORMAT_UNKNOWN ||
        desc->format > DM1_V22_FORMAT_TGA) {
        return 0;
    }

    /* Pixel-buffer requirement policy:
     *
     * Provenance-based split:
     *   MODERN      → file-backed (PNG/TGA). pixels MUST be non-NULL
     *                 and pixels_size MUST match the format minimum
     *                 when is_valid=1.
     *   UPSCALED    → pipeline-resolved V2.1 EPX 2x path. The V1
     *                 indexed framebuffer is the pipeline input; no
     *                 pre-decoded pixel buffer is needed. pixels=NULL
     *                 is allowed with is_valid=1.
     *   FILTERED    → pipeline-resolved V2.0 filter path. Same
     *                 rationale as UPSCALED: V1 framebuffer is the
     *                 pipeline input.
     *   ORIGINAL    → pipeline-resolved V1 path. The V1 indexed
     *                 framebuffer is rendered directly; no pre-decoded
     *                 buffer is needed.
     *
     * Before this policy was clarified, the validator unconditionally
     * required pixels whenever is_valid=1, which made every pipeline-
     * resolved descriptor (V1/V2.0/V2.1) fail validation immediately
     * after dm1_v22_asset_load(). That inconsistency was hidden by the
     * MODERN branch (which sets is_valid=0 because pixels weren't
     * loaded yet in the probe version of the loader).
     *
     * is_valid=0 always validates (descriptor is metadata-only / a
     * placeholder). The validator only enforces buffer consistency when
     * the caller claims the descriptor is rendering-ready. */
    if (!desc->is_valid) {
        return 1;
    }

    /* is_valid=1: enforce pixel buffer consistency only when the
     * descriptor actually carries pixel data.
     *
     * Policy:
     *   MODERN      → pixels MUST be non-NULL (file-backed); if present,
     *                 pixels_size MUST match the format minimum.
     *   UPSCALED    → pixels is optional (V2.1 EPX pipeline uses the V1
     *                 indexed framebuffer as input, no pre-decoded
     *                 buffer is needed). If pixels is non-NULL, the size
     *                 is still checked for data integrity.
     *   FILTERED    → same as UPSCALED (V2.0 filter pipeline).
     *   ORIGINAL    → same as UPSCALED (V1 indexed framebuffer is the
     *                 pipeline input).
     *
     * This split preserves the existing INDEXED / RGBA / PALETTED
     * "undersized buffer fails" contract from the original validator
     * while admitting the pipeline-resolved descriptors that
     * dm1_v22_asset_load() returns for V1 / V2.0 / V2.1. */
    if (desc->provenance == DM1_V22_PROVENANCE_MODERN && !desc->pixels) {
        /* MODERN is file-backed: pixels must be present when is_valid=1. */
        return 0;
    }
    if (desc->pixels) {
        /* When pixels are present (any provenance), the size must be
         * consistent with the format and dimensions. This protects the
         * V2.0 / V1 / V2.1 callers that pre-load a buffer themselves
         * and want the validator to catch undersized buffers. */
        size_t expected_min = 0;
        switch (desc->format) {
            case DM1_V22_FORMAT_PNG:
            case DM1_V22_FORMAT_TGA:
            case DM1_V22_FORMAT_RGBA:
                expected_min = (size_t)desc->width * desc->height * 4;
                break;
            case DM1_V22_FORMAT_PALETTED:
                expected_min = (size_t)desc->width * desc->height;
                break;
            case DM1_V22_FORMAT_INDEXED:
                /* 4-bit indexed: 1 byte per 2 pixels, rows padded to byte */
                expected_min = ((size_t)(desc->width + 1) / 2) * desc->height;
                break;
            default:
                return 0;
        }
        if (desc->pixels_size < expected_min) return 0;
    }

    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Best-Available Provenance Selection
 * ══════════════════════════════════════════════════════════════════════ */

DM1_V22_AssetProvenance dm1_v22_best_available_provenance(
    const char* category, const char* asset_id,
    DM1_V22_AssetProvenance desired) {

    (void)category;
    (void)asset_id;

    if (desired <= DM1_V22_PROVENANCE_UNKNOWN) {
        return DM1_V22_PROVENANCE_UNKNOWN;
    }

    /* Walk the fallback chain from the desired level downward.
     * Stop at the first level that is currently "available":
     *   MODERN: available if g_v22_installed and manifest loaded
     *   UPSCALED: available if DM1_V2_GetAssetMode() >= UPSCALED
     *   FILTERED: available if DM1_V2_GetAssetMode() >= FILTERED
     *   ORIGINAL: always available */
    DM1_V22_AssetProvenance prov = desired;
    while (prov != DM1_V22_PROVENANCE_UNKNOWN) {
        switch (prov) {
            case DM1_V22_PROVENANCE_MODERN:
                if (g_v22_installed && dm1_v22_modern_assets_available()) {
                    return prov;
                }
                break;
            case DM1_V22_PROVENANCE_UPSCALED:
                if (DM1_V2_GetAssetMode() >= DM1_V2_ASSET_MODE_UPSCALED) {
                    return prov;
                }
                break;
            case DM1_V22_PROVENANCE_FILTERED:
                if (DM1_V2_GetAssetMode() >= DM1_V2_ASSET_MODE_FILTERED) {
                    return prov;
                }
                break;
            case DM1_V22_PROVENANCE_ORIGINAL:
                /* Always available */
                return prov;
            default:
                break;
        }
        prov = dm1_v22_fallback_next(prov);
    }

    return DM1_V22_PROVENANCE_ORIGINAL; /* Absolute fallback */
}

/* ══════════════════════════════════════════════════════════════════════
 * Source Evidence
 * ══════════════════════════════════════════════════════════════════════ */

const char* dm1_v22_asset_pipeline_source_evidence(void) {
    return
        "dm1_v22_asset_pipeline.h/c — Phase 8 V2.2 modern asset pipeline\n"
        "Source-lock anchors:\n"
        "  ReDMCSB DUNGEON.C:2238-2246   — square type decode for shape dispatch\n"
        "  ReDMCSB DEFS.H:922-941        — M034_SQUARE_TYPE enumeration\n"
        "  ReDMCSB DUNVIEW.C:6697-6816   — wall/floor draw composition order\n"
        "  ReDMCSB PANEL.C:418-428       — G0304_i_DungeonViewPaletteIndex (6 levels)\n"
        "  ReDMCSB DRAWVIEW.C:1-200      — creature sprite framing (G0011_i_CreaturePosture)\n"
        "  ReDMCSB PROJEXPL.C:43-165     — projectile/explosion routing\n"
        "  Firestaff dm1_v22_shapes.c    — shape type → modern renderer bridge\n"
        "  Firestaff dm1_v2_asset_pipeline_pc34.c — V2.1 EPX pipeline (fallback source)\n"
        "Pipeline: MODERN (V2.2) → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)\n";
}
