/*
 * firestaff_dm1_v2_deterministic_screenshot_receipt_route_probe.c
 *
 * Data-free DM1 V2 deterministic screenshot receipt route.
 *
 * This probe builds the canonical DM1 PC 3.4 entry composition, selects the
 * V2.0 filtered presentation mode, writes a deterministic paletted BMP
 * receipt under a scratch directory, then writes a small JSON manifest whose
 * hashes are pinned by this test. It is deliberately a Firestaff receipt for
 * the V2 presentation route, not a DOSBox/original-pixel parity claim.
 *
 * Source locks:
 *   ReDMCSB DUNVIEW.C:2999-3000  viewport bitmap 224x136 dimensions.
 *   ReDMCSB DUNVIEW.C:8337-8338  floor/ceiling before walking squares.
 *   ReDMCSB DUNVIEW.C:8490-8542  D3 -> D0 draw order.
 *   ReDMCSB GAMELOOP.C:90        F0128_DUNGEONVIEW_Draw_CPSF snapshot draw.
 *   Firestaff dm1_v2_presentation_mode_pc34.c V2.0 presentation selection.
 *
 * Exit codes: 0 = PASS, 1 = FAIL.
 */

#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_screenshot_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"

#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#endif

#define EXPECTED_FRAMEBUFFER_HASH 0xd01a97faba345095ULL
#define EXPECTED_BMP_HASH         0x21068415c275b400ULL
#define EXPECTED_PALETTE_HASH     0x25e0f82aef1ebc06ULL
#define EXPECTED_MANIFEST_HASH    0xd24488d1f4264385ULL

static int g_failures = 0;

#define CHECK(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        g_failures++; \
    } else { \
        printf("PASS: " fmt "\n", ##__VA_ARGS__); \
    } \
} while (0)

static uint16_t read_u16_le(const unsigned char* p) {
    return (uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8);
}

static uint32_t read_u32_le(const unsigned char* p) {
    return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

static uint64_t fnv1a_begin(void) {
    return 14695981039346656037ULL;
}

static uint64_t fnv1a_byte(uint64_t h, unsigned char b) {
    h ^= (uint64_t)b;
    h *= 1099511628211ULL;
    return h;
}

static uint64_t fnv1a_bytes(uint64_t h, const void* data, size_t size) {
    const unsigned char* p = (const unsigned char*)data;
    size_t i;
    for (i = 0; i < size; ++i) h = fnv1a_byte(h, p[i]);
    return h;
}

static uint64_t fnv1a_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    unsigned char buf[4096];
    size_t n;
    uint64_t h = fnv1a_begin();
    if (!fp) return 0ULL;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0U) {
        h = fnv1a_bytes(h, buf, n);
    }
    fclose(fp);
    return h;
}

static int mkdir_one(const char* path) {
#if defined(_WIN32)
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif
}

static int make_scratch_dir(char* out, size_t outSize) {
    const char* root = getenv("TMPDIR");
    char parent[768];
    if (!root || !root[0]) root = getenv("TEMP");
    if (!root || !root[0]) root = "/tmp";
    snprintf(parent, sizeof(parent), "%s/firestaff_dm1_v2_receipts", root);
    if (!mkdir_one(parent)) return 0;
    snprintf(out, outSize, "%s/deterministic_v20", parent);
    return mkdir_one(out);
}

static void build_entry_viewport(DM1_V2_ViewportState* outVp) {
    const DM1_V2_DungeonStateFixture* fixture =
        dm1_v2_vp_dm1_pc34_entry_state_fixture();
    DM1_V2_ViewportCompositionInput input;
    CHECK(fixture != NULL, "entry fixture available");
    if (!fixture || !outVp) return;

    CHECK(fixture->startMapX == 1, "fixture startMapX=1");
    CHECK(fixture->startMapY == 3, "fixture startMapY=3");
    CHECK(fixture->startDirection == 2, "fixture startDirection=2");
    CHECK(dm1_v2_vp_build_composition_from_fixture(fixture,
                                                   fixture->startMapX,
                                                   fixture->startMapY,
                                                   fixture->startDirection,
                                                   &input) == 1,
          "build entry composition");
    CHECK(input.squares[1][1].element == DM1_V2_ELEMENT_WALL,
          "D1C front square is wall");
    CHECK(input.squares[0][1].element == DM1_V2_ELEMENT_CORRIDOR,
          "D0C current square is corridor");

    dm1_v2_vp_init(outVp);
    CHECK(dm1_v2_vp_render_composition_flat(outVp, &input) == 1,
          "render flat entry viewport");
}

static void stamp_v20_receipt_overlay(DM1_V2_ViewportState* vp) {
    int x;
    int y;
    static const uint8_t barcode[16] = {
        1, 0, 1, 1, 0, 0, 1, 0,
        0, 1, 1, 1, 0, 1, 0, 1
    };
    if (!vp) return;

    /* This is a receipt stamp, not gameplay art: it makes the scratch BMP
     * self-identifying for the V2.0 route while leaving the manifest honest
     * that no original-pixel parity is claimed. */
    for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
        dm1_v2_vp_set_pixel(vp, x, 0, 60, 92, 128, 255);
        dm1_v2_vp_set_pixel(vp, x, DM1_V2_VIEWPORT_H - 1, 60, 92, 128, 255);
    }
    for (y = 0; y < DM1_V2_VIEWPORT_H; ++y) {
        dm1_v2_vp_set_pixel(vp, 0, y, 60, 92, 128, 255);
        dm1_v2_vp_set_pixel(vp, DM1_V2_VIEWPORT_W - 1, y, 60, 92, 128, 255);
    }
    for (x = 0; x < 16; ++x) {
        uint8_t r = barcode[x] ? 188 : 42;
        uint8_t g = barcode[x] ? 221 : 64;
        uint8_t b = barcode[x] ? 79 : 91;
        int px = 8 + x * 3;
        for (y = 8; y < 40; ++y) {
            dm1_v2_vp_set_pixel(vp, px + 0, y, r, g, b, 255);
            dm1_v2_vp_set_pixel(vp, px + 1, y, r, g, b, 255);
        }
    }
    for (y = 48; y < 56; ++y) {
        for (x = 8; x < 216; ++x) {
            uint8_t v = (uint8_t)(32 + ((x + y) % 64));
            dm1_v2_vp_set_pixel(vp, x, y, v, (uint8_t)(v + 16), 112, 255);
        }
    }
}

static int palette_find_or_add(uint32_t* palette,
                               int* paletteCount,
                               uint32_t color) {
    int i;
    for (i = 0; i < *paletteCount; ++i) {
        if (palette[i] == color) return i;
    }
    if (*paletteCount >= 256) return -1;
    palette[*paletteCount] = color;
    *paletteCount += 1;
    return *paletteCount - 1;
}

static int viewport_to_indexed(const DM1_V2_ViewportState* vp,
                               uint8_t* outIndexed,
                               uint32_t* outPalette,
                               int* outPaletteCount,
                               uint64_t* outFramebufferHash,
                               uint64_t* outPaletteHash) {
    int x, y;
    int paletteCount = 0;
    uint64_t fbHash = fnv1a_begin();
    if (!vp || !outIndexed || !outPalette || !outPaletteCount) return 0;
    memset(outPalette, 0, sizeof(uint32_t) * 256U);

    for (y = 0; y < DM1_V2_VIEWPORT_H; ++y) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
            const DM1_V2_Color* c = &vp->framebuffer[y][x];
            uint32_t color = ((uint32_t)c->b)
                           | ((uint32_t)c->g << 8)
                           | ((uint32_t)c->r << 16);
            int idx = palette_find_or_add(outPalette, &paletteCount, color);
            if (idx < 0) return 0;
            outIndexed[y * DM1_V2_VIEWPORT_W + x] = (uint8_t)idx;
            fbHash = fnv1a_bytes(fbHash, c, sizeof(*c));
        }
    }

    *outPaletteCount = paletteCount;
    if (outFramebufferHash) *outFramebufferHash = fbHash;
    if (outPaletteHash) {
        *outPaletteHash = fnv1a_bytes(fnv1a_begin(),
                                      outPalette,
                                      sizeof(uint32_t) * (size_t)paletteCount);
    }
    return 1;
}

static int inspect_bmp(const char* path, int paletteCount) {
    FILE* fp = fopen(path, "rb");
    unsigned char hdr[54];
    size_t n;
    uint32_t fileSize, pixelOffset, dibSize, width, height, imageBytes;
    uint16_t planes, bpp;
    long actualSize;
    int rowSize = (DM1_V2_VIEWPORT_W + 3) & ~3;
    int expectedImageBytes = rowSize * DM1_V2_VIEWPORT_H;
    int expectedFileSize = 14 + 40 + paletteCount * 4 + expectedImageBytes;
    if (!fp) return 0;
    n = fread(hdr, 1, sizeof(hdr), fp);
    fclose(fp);
    if (n != sizeof(hdr)) return 0;
    if (hdr[0] != 'B' || hdr[1] != 'M') return 0;

    fileSize = read_u32_le(hdr + 2);
    pixelOffset = read_u32_le(hdr + 10);
    dibSize = read_u32_le(hdr + 14);
    width = read_u32_le(hdr + 18);
    height = read_u32_le(hdr + 22);
    planes = read_u16_le(hdr + 26);
    bpp = read_u16_le(hdr + 28);
    imageBytes = read_u32_le(hdr + 34);

    actualSize = -1;
    {
        struct stat st;
        if (stat(path, &st) == 0) actualSize = (long)st.st_size;
    }

    CHECK(fileSize == (uint32_t)expectedFileSize, "BMP header file size=%u", fileSize);
    CHECK(actualSize == expectedFileSize, "BMP actual file size=%ld", actualSize);
    CHECK(pixelOffset == (uint32_t)(14 + 40 + paletteCount * 4),
          "BMP pixel offset=%u", pixelOffset);
    CHECK(dibSize == 40U, "BMP DIB header size=40");
    CHECK(width == DM1_V2_VIEWPORT_W, "BMP width=%u", width);
    CHECK(height == DM1_V2_VIEWPORT_H, "BMP height=%u", height);
    CHECK(planes == 1U, "BMP planes=1");
    CHECK(bpp == 8U, "BMP bpp=8");
    CHECK(imageBytes == (uint32_t)expectedImageBytes, "BMP image bytes=%u", imageBytes);
    return 1;
}

static int write_manifest(const char* path,
                          uint64_t framebufferHash,
                          uint64_t bmpHash,
                          uint64_t paletteHash,
                          int paletteCount) {
    FILE* fp = fopen(path, "wb");
    int n;
    if (!fp) return 0;
    n = fprintf(fp,
        "{\n"
        "  \"schema\": \"firestaff.dm1_v2.deterministic_screenshot_receipt.v1\",\n"
        "  \"route\": \"dm1_v2_v20_filtered_scratch_bmp_receipt\",\n"
        "  \"presentation_mode\": \"V2.0\",\n"
        "  \"v2_active\": true,\n"
        "  \"receipt_overlay\": \"v20_border_barcode_no_parity_claim\",\n"
        "  \"screenshot\": \"dm1_v2_v20_receipt.bmp\",\n"
        "  \"width\": %d,\n"
        "  \"height\": %d,\n"
        "  \"palette_entries\": %d,\n"
        "  \"framebuffer_fnv1a64\": \"%016llx\",\n"
        "  \"bmp_fnv1a64\": \"%016llx\",\n"
        "  \"palette_fnv1a64\": \"%016llx\",\n"
        "  \"source_lock\": [\n"
        "    \"ReDMCSB DUNVIEW.C:2999-3000 viewport 224x136\",\n"
        "    \"ReDMCSB DUNVIEW.C:8337-8542 draw order\",\n"
        "    \"ReDMCSB GAMELOOP.C:90 F0128 snapshot draw\"\n"
        "  ],\n"
        "  \"parity_claim\": \"none; Firestaff V2 receipt only\"\n"
        "}\n",
        DM1_V2_VIEWPORT_W,
        DM1_V2_VIEWPORT_H,
        paletteCount,
        (unsigned long long)framebufferHash,
        (unsigned long long)bmpHash,
        (unsigned long long)paletteHash);
    fclose(fp);
    return n > 0;
}

int main(void) {
    DM1_V2_ViewportState vp;
    uint8_t indexed[DM1_V2_VIEWPORT_W * DM1_V2_VIEWPORT_H];
    uint32_t palette[256];
    int paletteCount = 0;
    uint64_t framebufferHash = 0ULL;
    uint64_t paletteHash = 0ULL;
    uint64_t bmpHash = 0ULL;
    uint64_t manifestHash = 0ULL;
    char scratch[1024];
    char bmpPath[1200];
    char manifestPath[1200];
    const DM1_V2_PresentationModeState* pm;

    printf("--- DM1 V2 deterministic screenshot receipt route ---\n");
    printf("schema=firestaff.dm1_v2.deterministic_screenshot_receipt.v1\n");
    printf("viewport=%dx%d\n", DM1_V2_VIEWPORT_W, DM1_V2_VIEWPORT_H);

    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    pm = dm1_v2_presentation_mode_state();
    CHECK(pm != NULL, "presentation mode state available");
    CHECK(pm && pm->kind == DM1_V2_PM_V20_FILTERED,
          "presentation mode resolved to V2.0 filtered");
    CHECK(pm && pm->v2Active == 1, "presentation mode v2Active=1");
    CHECK(pm && pm->v20FilterActive == 1, "presentation mode v20FilterActive=1");

    CHECK(make_scratch_dir(scratch, sizeof(scratch)) == 1, "scratch dir ready");
    snprintf(bmpPath, sizeof(bmpPath), "%s/dm1_v2_v20_receipt.bmp", scratch);
    snprintf(manifestPath, sizeof(manifestPath), "%s/dm1_v2_v20_receipt_manifest.json", scratch);

    build_entry_viewport(&vp);
    stamp_v20_receipt_overlay(&vp);
    CHECK(viewport_to_indexed(&vp,
                              indexed,
                              palette,
                              &paletteCount,
                              &framebufferHash,
                              &paletteHash) == 1,
          "viewport quantized to deterministic indexed receipt");
    CHECK(paletteCount > 1 && paletteCount <= 256,
          "palette count in range (%d)", paletteCount);

    CHECK(v2_screenshot_capture(indexed,
                                DM1_V2_VIEWPORT_W,
                                DM1_V2_VIEWPORT_H,
                                palette,
                                paletteCount,
                                bmpPath) == 0,
          "v2_screenshot_capture wrote BMP receipt");
    CHECK(inspect_bmp(bmpPath, paletteCount) == 1,
          "BMP receipt header inspected");
    bmpHash = fnv1a_file(bmpPath);
    CHECK(bmpHash != 0ULL, "BMP receipt hash nonzero");

    CHECK(write_manifest(manifestPath,
                         framebufferHash,
                         bmpHash,
                         paletteHash,
                         paletteCount) == 1,
          "receipt manifest written");
    manifestHash = fnv1a_file(manifestPath);
    CHECK(manifestHash != 0ULL, "manifest hash nonzero");

    printf("receipt_bmp=%s\n", bmpPath);
    printf("receipt_manifest=%s\n", manifestPath);
    printf("palette_entries=%d\n", paletteCount);
    printf("framebuffer_fnv1a64=%016llx\n", (unsigned long long)framebufferHash);
    printf("palette_fnv1a64=%016llx\n", (unsigned long long)paletteHash);
    printf("bmp_fnv1a64=%016llx\n", (unsigned long long)bmpHash);
    printf("manifest_fnv1a64=%016llx\n", (unsigned long long)manifestHash);

    CHECK(framebufferHash == EXPECTED_FRAMEBUFFER_HASH,
          "framebuffer hash matches pinned receipt");
    CHECK(paletteHash == EXPECTED_PALETTE_HASH,
          "palette hash matches pinned receipt");
    CHECK(bmpHash == EXPECTED_BMP_HASH,
          "BMP hash matches pinned receipt");
    CHECK(manifestHash == EXPECTED_MANIFEST_HASH,
          "manifest hash matches pinned receipt");

    if (g_failures) {
        fprintf(stderr, "result=FAIL failures=%d\n", g_failures);
        return 1;
    }
    printf("result=PASS\n");
    return 0;
}
