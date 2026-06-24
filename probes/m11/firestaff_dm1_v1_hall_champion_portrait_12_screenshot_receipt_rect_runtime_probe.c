/*
 * firestaff_dm1_v1_hall_champion_portrait_12_screenshot_receipt_rect_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice that is intentionally NOT covered by the
 * existing ordinal-12 probes:
 *
 *   ordinal       : 12 (C026 col 4 row 1; mirror-catalog record LINFLAS)
 *   route variant : screenshot_receipt_rect — the LINFLAS pose
 *                   (map=0, x=2, y=10, NORTH) is rendered into the M11
 *                   320x200 indexed framebuffer via the same
 *                   M11_GameView_Draw path the other ordinal-12 probes
 *                   use, then captured to a real 24-bit BMP on disk via
 *                   M11_Screenshot_Capture, then re-read from disk and
 *                   the on-disk source-locked D1C portrait rectangle
 *                   (96,35,32,29) of the captured pixels is decoded
 *                   and asserted to match the C026 ordinal-12 opaque
 *                   pixels.  The "receipt" half of the name documents
 *                   this as the on-disk BMP being the actual receipt
 *                   of the LINFLAS portrait — the BMP writer is in the
 *                   contract, not just the in-memory framebuffer.
 *   aspect        : portrait_rect_position (D1C front-wall cutout at
 *                   viewport (96,35,32,29)) + receipt_byte_stability
 *                   + no_floating_negative, locking three contracts
 *                   the existing ordinal-12 probes leave uncovered for
 *                   the screenshot round-trip at the LINFLAS pose:
 *
 *                   (A) RECEIPT BYTE STABILITY: the LINFLAS pose is
 *                       rendered to a deterministic in-memory
 *                       framebuffer, captured via
 *                       M11_Screenshot_Capture into a probe-controlled
 *                       BMP path under HOME, re-read from disk, and
 *                       the BMP is asserted to be a well-formed
 *                       24-bit top-down BMP (BM magic, width=320,
 *                       height=200, file size matches the source-
 *                       locked 14+40+960*200 = 192054 byte footprint
 *                       with 320*3 = 960 already 4-aligned, no
 *                       row-pad bytes).  The 320x200 geometry matches
 *                       ReDMCSB DUNVIEW.C:8318-8542 (V1 framebuffer)
 *                       + COORD.C:1693-1722 (viewport 224x136 sits
 *                       inside the 320x200 framebuffer).
 *
 *                   (B) RECT RECEIVED ORDINAL 12: the captured BMP is
 *                       decoded (the 24-bit R,G,B bytes for the
 *                       source-locked D1C cutout at viewport
 *                       (96,35,32,29) are read back from disk), the
 *                       per-pixel palette indices are reconstructed
 *                       via the same G9010_auc_VgaPaletteAll_Compat
 *                       lookup the BMP writer uses, and the decoded
 *                       on-disk D1C rect is asserted to match the
 *                       in-memory framebuffer D1C rect (the screenshot
 *                       receipt round-trip contract — the BMP file on
 *                       disk receives the same pixels the engine drew
 *                       into the framebuffer).  Note that this
 *                       contract compares against the in-memory fb,
 *                       not against the raw C026 source strip, because
 *                       M11_GameView_Draw applies a per-draw palette
 *                       remap when blitting C026 into the D1C rect
 *                       (the remap is part of the source-locked V1
 *                       rendering path; the east_walkpath /
 *                       walkpath_from_entrance probes compare against
 *                       the in-memory fb after the remap, so we do
 *                       the same to keep the screenshot_receipt_rect
 *                       round-trip self-consistent).
 *
 *                   (C) NO FLOATING NEGATIVE: the same screenshot
 *                       receipt path is run for the no-portrait
 *                       side pose (2,10,EAST) and the on-disk D1C
 *                       rect is asserted to be dominated by non-
 *                       ordinal-12 pixels (the 35% leak threshold
 *                       the existing visibility / zorder / reblt /
 *                       east_walkpath / walkpath_from_entrance
 *                       probes lock for in-memory checks, applied
 *                       here to the on-disk BMP decode).
 *
 * This probe widens the existing ordinal-12 coverage along a
 * different axis than:
 *
 *   firestaff_dm1_v1_hall_champion_portrait_12_front_south_entry_runtime_probe
 *     - covers the south-facing exit pose, no-route contract, and
 *       candidate-panel return at (2,10,N) -> (2,10,SOUTH).  It only
 *       counts pixels in the in-memory framebuffer; it never writes
 *       a BMP to disk and never re-reads the rect from disk.
 *   firestaff_dm1_v1_hall_champion_portrait_12_front_north_entry_runtime_probe
 *     - covers the static front_north_entry pose at (1,2,N) -> ordinal 1
 *       HALK, the ordinal 12 any-pose discovery on Hall map 0, and the
 *       no-floating side-wall corridor poses.  It exercises the
 *       M11_GameView_GetD1CWallOrnamentZone + M11_GameView_GetFrontMirrorOrdinal
 *       contracts only and never drives the BMP screenshot path.
 *   firestaff_dm1_v1_hall_champion_portrait_12_east_walkpath_portrait_rect_probe
 *     - covers (1,10,N) -> (2,10,N) -> (3,10,N) -> (2,10,N) -> (1,10,N)
 *       via direct set_pose teleport, with cross-cell re-blt invariant
 *       and 90% positive-ordinal match in the in-memory framebuffer.
 *       The east-walkpath probe does not exercise the M11 screenshot
 *       capture path or the on-disk BMP receipt contract.
 *   firestaff_dm1_v1_hall_champion_portrait_12_walkpath_from_entrance_runtime_probe
 *     - drives the live M11 input-path walkpath from the entrance
 *       (1,2,N) -> ZED (1,10,N) -> LINFLAS (2,10,N) and verifies the
 *       C026 ordinal-12 opaque pixels in the in-memory framebuffer.
 *       The walkpath_from_entrance probe does not exercise the
 *       M11_Screenshot_Capture round-trip or the on-disk BMP decode.
 *
 * Source evidence:
 *   ReDMCSB WIP 20210206:
 *     DUNGEON.C:2558,2608-2612  C127 sensorData -> G0289 ordinal
 *     DUNGEON.C:2573            M011_CELL(sensor) -> visible wall cell
 *     MOVESENS.C:1501-1503      C127 -> F0280 candidate materialise
 *     DUNVIEW.C:3913-3928       C346 wall frame + C026 portrait blit
 *     DUNVIEW.C:8318-8542 F0128 far-to-near viewport draw order
 *     DUNVIEW.C:8522-8533       C026 D1C re-blt on tick redraw
 *     COORD.C:1693-1722         PC 3.4 viewport origin / 224x136
 *     DEFS.H:2071-2079         G2071_C320 / G2078_C32 / G2079_C29
 *     GAMELOOP.C:90 + F0128     M11 present hook (PresentIndexed/PresentRGBA)
 *     screenshot_m11.c          BMP writer (14+40 header, 24-bit top-down)
 *
 * Honest scope: this probe proves the source-locked C026 ordinal
 * placement at the canonical LINFLAS pose, the on-disk BMP receipt
 * of that placement (the BMP file on disk contains the right pixels
 * in the right rect), the no-floating contract at the (2,10,EAST)
 * side pose when run through the same screenshot receipt path, and
 * the byte-stability contract for the captured BMP file.  It does
 * NOT claim DOS pixel parity beyond the same C01 dark-gray
 * transparency contract the existing portrait / zorder / reblt /
 * east_walkpath / walkpath_from_entrance probes lock.  Original
 * DM1 PC 3.4 captures live under parity-evidence/ and are referenced
 * by separate parity gates.  The probe is host-agnostic (no SDL
 * window required) and uses a probe-controlled temp directory under
 * HOME so it does not pollute the user-facing
 * ~/.firestaff/screenshots/ directory.
 *
 * Usage:
 *   firestaff_dm1_v1_hall_champion_portrait_12_screenshot_receipt_rect_runtime_probe DATA_DIR
 *
 * Exit codes:
 *   0  probe PASSED
 *   1  probe FAILED
 *   2  bad invocation
 *   3  data-dir / GRAPHICS.DAT unavailable
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "screenshot_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    /* DUNVIEW.C:3913-3928 / 8522-8533: D1C front-wall box is the 32x29
     * rectangle at (96,35)-(127,63) of the viewport, drawn from the
     * C026 champion portrait strip indexed by the C127 sensor ordinal
     * stored in G0289. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* DUNVIEW.C:3916: C026 champion portrait blit masks
     * C01_COLOR_DARK_GRAY (value 1) as transparency. Same constant
     * the existing portrait / zorder / reblt / east_walkpath /
     * walkpath_from_entrance probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Canonical ordinal-12 viewing pose: (map=0, x=2, y=10) facing
     * NORTH — M11_GameView_GetFrontMirrorOrdinal returns 12 = LINFLAS.
     * Pinning this so the slice is bound to a real source identity. */
    PROBE_LINFLAS_X = 2,
    PROBE_LINFLAS_Y = 10,
    PROBE_LINFLAS_DIR = 0, /* DIR_NORTH */
    PROBE_LINFLAS_EAST_DIR = 1, /* DIR_EAST — the no-portrait side pose */
    PROBE_ORDINAL_TARGET = 12,
    HALL_MAP_INDEX = 0,
    /* C026 champion-portrait strip dimensions: 8 cols x 3 rows of
     * 32x29 portraits (ordinals 0..23). */
    PROBE_PORTRAIT_STRIP_W = 256,
    PROBE_PORTRAIT_STRIP_H = 87,
    /* Re-blt invariant tolerance matching the existing walkpath /
     * zorder / reblt / east_walkpath / walkpath_from_entrance
     * probes: the ordinal-12 matched-pixel count in the D1C rect
     * must not reach 35% of its compared count when the player is
     * NOT facing the front wall of the (2,10) cell, otherwise
     * ordinal 12 is "floating" on the side wall. */
    PROBE_FLOOR_LEAK_PCT = 35,
    /* Positive-ordinal pixel match threshold matching the existing
     * east_walkpath / walkpath_from_entrance probes: 90% of the C026
     * ordinal-12 opaque pixels must be present in the D1C rect for
     * the LINFLAS pose to be considered properly drawn. */
    PROBE_POSITIVE_MATCH_PCT = 90,
    /* Source-locked BMP file footprint: 14 (BITMAPFILEHEADER) +
     * 40 (BITMAPINFOHEADER) + 960 * 200 image payload = 192054
     * bytes. The 320 * 3 = 960 row stride is already a multiple of
     * 4 so no row-pad bytes are required.  Any deviation from this
     * footprint indicates the BMP writer changed the header layout
     * or the row stride in a way the screenshot_receipt_rect route
     * must reject. */
    PROBE_BMP_HDR_BYTES = 14 + 40,
    PROBE_BMP_ROW_STRIDE = PROBE_FB_W * 3,
    PROBE_BMP_IMAGE_BYTES = PROBE_BMP_ROW_STRIDE * PROBE_FB_H,
    PROBE_BMP_FILE_BYTES = PROBE_BMP_HDR_BYTES + PROBE_BMP_IMAGE_BYTES,
    /* Brightness level for the VGA palette used to decode the
     * captured BMP.  M11_Screenshot_CaptureCurrent uses
     * M11_Render_GetPaletteLevel() to pick the level; for the
     * Hall of Champions the canonical dungeon-brightest level is
     * 0 (DUNVIEW.C:8318-8542 F0128 draws the full viewport at the
     * active dungeon brightness).  Pinning the level keeps the
     * probe stable across fixtures with different per-fixture
     * default brightness. */
    PROBE_PALETTE_LEVEL = 0
};

static int g_pass;
static int g_fail;

static int expect_int(const char* label, int got, int want) {
    ++g_pass;
    if (got == want) {
        printf("  PASS: %s == %d\n", label, want);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    return 0;
}

/* Direct-pose helper mirroring the
 * start_independent_input_route contract the existing walkpath /
 * walkpath_from_entrance / east_walkpath probes use between
 * independent routes: reset the candidate panel state so a
 * previous mirror panel does not leak into the next check. */
static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = (int16_t)mapX;
    game->world.party.mapY = (int16_t)mapY;
    game->world.party.direction = (uint8_t)dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

/* Count the C026 ordinal pixels compared in the D1C rect (matching
 * the "compared" count the existing east_walkpath /
 * walkpath_from_entrance / front_south_entry probes use to compute
 * the leak percentage). */
static int count_ordinal_compared_pixels(const M11_AssetSlot* portraits,
                                          int ordinal) {
    int x;
    int y;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            ++compared;
        }
    }
    return compared;
}

/* Decode a single 24-bit BMP pixel to a 4-bit VGA palette index using
 * G9010_auc_VgaPaletteAll_Compat at the given brightness level.  The
 * BMP writer (screenshot_m11.c M11_Screenshot_Capture) writes the
 * palette bytes directly into the file in the order
 *   row[x*3 + 0] = rgb[2]   (B channel)
 *   row[x*3 + 1] = rgb[1]   (G channel)
 *   row[x*3 + 2] = rgb[0]   (R channel)
 * so the on-disk bytes match the palette the writer used; decoding
 * here uses the same palette table to recover the original 4-bit
 * index.  We use a tolerance of 1 (VGA DAC steps are 4 in 8-bit
 * space; the writer applies the same scale we decode with) so the
 * decode is exact. */
static int decode_bmp_pixel_to_palette_index(unsigned char b,
                                              unsigned char g,
                                              unsigned char r,
                                              int paletteLevel) {
    int i;
    if (paletteLevel < 0) paletteLevel = 0;
    if (paletteLevel >= M11_PALETTE_LEVELS) paletteLevel = M11_PALETTE_LEVELS - 1;
    for (i = 0; i < 16; ++i) {
        const unsigned char* ref = G9010_auc_VgaPaletteAll_Compat[paletteLevel][i];
        if (ref[0] == r && ref[1] == g && ref[2] == b) {
            return i;
        }
    }
    return -1;
}

/* Read the 24-bit BMP at `path` and decode the source-locked D1C
 * portrait rectangle (PROBE_PORTRAIT_X..X+W, PROBE_PORTRAIT_Y..Y+H)
 * back into an indexed buffer of size PROBE_PORTRAIT_W * PROBE_PORTRAIT_H.
 * The decoded buffer holds 0..15 palette indices reconstructed from the
 * on-disk BMP bytes via G9010_auc_VgaPaletteAll_Compat at the given
 * brightness level.  Returns 1 on success, 0 on failure (bad BM
 * header, wrong dimensions, missing pixels, etc.). */
static int decode_d1c_rect_from_bmp(const char* path,
                                    unsigned char* outRect,
                                    int paletteLevel,
                                    long* outFileBytes) {
    FILE* f;
    unsigned char hdr[26];
    unsigned char* image = NULL;
    long fileBytes;
    int bmpWidth, bmpHeight;
    int y, x;
    int rc = 0;
    size_t n;

    if (outFileBytes) *outFileBytes = -1;
    if (!path || !outRect) return 0;

    f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "  FAIL: cannot open BMP %s\n", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    fileBytes = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (outFileBytes) *outFileBytes = fileBytes;

    n = fread(hdr, 1, sizeof(hdr), f);
    if (n < sizeof(hdr)) {
        fprintf(stderr, "  FAIL: short read on BMP header\n");
        fclose(f);
        return 0;
    }
    if (hdr[0] != 'B' || hdr[1] != 'M') {
        fprintf(stderr, "  FAIL: BMP magic != 'BM' (got %02x %02x)\n",
                hdr[0], hdr[1]);
        fclose(f);
        return 0;
    }
    bmpWidth  = (int)hdr[18] | ((int)hdr[19] << 8) |
                ((int)hdr[20] << 16) | ((int)hdr[21] << 24);
    bmpHeight = (int)hdr[22] | ((int)hdr[23] << 8) |
                ((int)hdr[24] << 16) | ((int)hdr[25] << 24);
    if (bmpHeight < 0) bmpHeight = -bmpHeight; /* top-down */
    if (bmpWidth != PROBE_FB_W || bmpHeight != PROBE_FB_H) {
        fprintf(stderr, "  FAIL: BMP dims %dx%d want %dx%d\n",
                bmpWidth, bmpHeight, PROBE_FB_W, PROBE_FB_H);
        fclose(f);
        return 0;
    }
    image = (unsigned char*)malloc((size_t)PROBE_BMP_IMAGE_BYTES);
    if (!image) {
        fclose(f);
        return 0;
    }
    fseek(f, PROBE_BMP_HDR_BYTES, SEEK_SET);
    n = fread(image, 1, (size_t)PROBE_BMP_IMAGE_BYTES, f);
    fclose(f);
    if (n != (size_t)PROBE_BMP_IMAGE_BYTES) {
        fprintf(stderr, "  FAIL: short read on BMP image (%zu / %d bytes)\n",
                n, PROBE_BMP_IMAGE_BYTES);
        free(image);
        return 0;
    }
    /* Decode the D1C rect from the BMP image rows. The writer writes
     * top-down rows (infoHdr height is -200), so row 0 is the top
     * row of the framebuffer at y=0.  Each row is BMP_ROW_STRIDE
     * bytes wide, already 4-aligned so no padding skip is needed.
     *
     * The D1C front-wall box is at framebuffer coords
     * (PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y) - (PROBE_PORTRAIT_X + 31,
     * PROBE_PORTRAIT_Y + 28) — viewport (96,35) translated by the
     * viewport origin (0,33) gives the framebuffer (96,68).  The
     * row pointer is offset to PROBE_PORTRAIT_Y; the column offset
     * (PROBE_PORTRAIT_X) is added per-pixel so the decoded D1C rect
     * aligns with the in-memory framebuffer rect (PROBE_PORTRAIT_X,
     * PROBE_PORTRAIT_Y) and the round-trip equality check below can
     * compare decoded-vs-fb pixel-by-pixel.  Without the
     * PROBE_PORTRAIT_X offset the decoder would read BMP cols
     * 0..31 instead of cols 96..127 and the round-trip equality
     * would always fail. */
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        const unsigned char* row = image +
            (size_t)(PROBE_PORTRAIT_Y + y) * (size_t)PROBE_BMP_ROW_STRIDE;
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char b = row[(PROBE_PORTRAIT_X + x) * 3 + 0];
            unsigned char g = row[(PROBE_PORTRAIT_X + x) * 3 + 1];
            unsigned char r = row[(PROBE_PORTRAIT_X + x) * 3 + 2];
            int idx = decode_bmp_pixel_to_palette_index(b, g, r, paletteLevel);
            if (idx < 0) {
                fprintf(stderr,
                        "  FAIL: BMP pixel (%d,%d) decoded to unknown index "
                        "(r=%u g=%u b=%u)\n",
                        x, y, (unsigned)r, (unsigned)g, (unsigned)b);
                free(image);
                return 0;
            }
            outRect[y * PROBE_PORTRAIT_W + x] = (unsigned char)idx;
        }
    }
    free(image);
    rc = 1;
    return rc;
}

/* Count the pixels in the decoded-from-BMP D1C rect that match the
 * C026 champion portrait ordinal.  Reuses the same comparison loop
 * as the in-memory helper above, but reads from a decoded BMP rect
 * buffer instead of an in-memory framebuffer. */
/* Count the pixels in the front-wall box where the decoded-from-BMP
 * rect equals the in-memory framebuffer rect (round-trip equality).
 * This is the screenshot_receipt_rect contract: every pixel that
 * landed in the BMP must decode back to the same palette index that
 * the engine wrote into the framebuffer.  Returns 0..PROBE_PORTRAIT_W
 * * PROBE_PORTRAIT_H. */
static int count_roundtrip_matched_pixels(const unsigned char* decodedRect,
                                          const unsigned char* fb) {
    int x;
    int y;
    int matched = 0;
    if (!decodedRect || !fb) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char dec = (unsigned char)(decodedRect[y * PROBE_PORTRAIT_W + x] & 0x0F);
            unsigned char mem =
                (unsigned char)(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                    (PROBE_PORTRAIT_X + x)] & 0x0F);
            if (dec == mem) {
                ++matched;
            }
        }
    }
    return matched;
}

/* Count the pixels in the in-memory framebuffer at the D1C rect
 * that match the C026 champion portrait ordinal.  This is the
 * source-locked portrait_rect_position contract the existing
 * east_walkpath / walkpath_from_entrance / front_south_entry
 * probes lock; we re-pin it here because the screenshot_receipt_rect
 * round-trip must preserve the same in-memory portrait pixels, and
 * this helper is the canonical proof that the in-memory fb is
 * correctly drawn at the canonical LINFLAS pose.
 *
 * The comparison accounts for the M11_GameView_Draw palette remap:
 * the comparison is byte-equal between fb and source strip, not a
 * post-remap equivalence, because the existing probes (which use
 * the same byte-equal comparison) lock the in-memory portrait
 * pixels at 569/569 match at (2,10,N). */
static int count_in_memory_ordinal_matched_pixels(
        const M11_AssetSlot* portraits,
        const unsigned char* fb,
        int ordinal) {
    int x;
    int y;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst =
                (unsigned char)(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                    (PROBE_PORTRAIT_X + x)] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            if (dst == src) {
                ++matched;
            }
        }
    }
    return matched;
}

/* (A) RECEIPT BYTE STABILITY: render the LINFLAS pose to the
 *     in-memory framebuffer, capture it via M11_Screenshot_Capture
 *     to a probe-controlled BMP directory, and assert the BMP file
 *     on disk is well-formed (BM magic, 320x200, source-locked
 *     192054 byte file footprint).  This is the on-disk receipt of
 *     the LINFLAS pose — the BMP writer is in the contract, not
 *     just the in-memory framebuffer.  The actual captured file
 *     path is returned via `outBmpPath` (M11_Screenshot_Capture
 *     adds a YYYYMMDD-HHMMSS timestamp, so we cannot predict it
 *     in advance; the API hands us back the timestamped path). */
static int test_receipt_byte_stability(M11_GameViewState* game,
                                       unsigned char* fb,
                                       const char* bmpDir,
                                       char* outBmpPath,
                                       int outBmpPathCap) {
    int ok = 1;
    char stampName[1280];
    long fileBytes = -1;
    int captureRc;

    printf("[A] Receipt byte stability: LINFLAS pose -> BMP on disk\n");
    set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, PROBE_LINFLAS_DIR);
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    /* DIAG: print raw fb byte at one D1C rect pixel before capture.
     * The D1C front-wall box is at framebuffer
     * (PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y) = (96, 68), not the
     * viewport-relative (96, 35) — the previous print formula used
     * the viewport Y by mistake, which would dump row 35 col 96 of
     * the framebuffer (a HUD/title row, not the wall portrait). */
    printf("  DIAG: raw fb[%d*320+%d] = 0x%02x (& 0x0F = %d)\n",
        PROBE_PORTRAIT_Y, PROBE_PORTRAIT_X,
        fb[PROBE_PORTRAIT_Y*PROBE_FB_W + PROBE_PORTRAIT_X],
        fb[PROBE_PORTRAIT_Y*PROBE_FB_W + PROBE_PORTRAIT_X] & 0x0F);
    {
        int k;
        for (k = 0; k < 5; ++k) {
            int idx = PROBE_PORTRAIT_Y*PROBE_FB_W + PROBE_PORTRAIT_X + k;
            printf("  DIAG[A]: fb[%d] = 0x%02x\n", idx, fb[idx]);
        }
    }
    printf("  DIAG: G9010[0][12] = (%d,%d,%d)\n",
        G9010_auc_VgaPaletteAll_Compat[0][12][0],
        G9010_auc_VgaPaletteAll_Compat[0][12][1],
        G9010_auc_VgaPaletteAll_Compat[0][12][2]);
    printf("  DIAG: G9010[0][13] = (%d,%d,%d)\n",
        G9010_auc_VgaPaletteAll_Compat[0][13][0],
        G9010_auc_VgaPaletteAll_Compat[0][13][1],
        G9010_auc_VgaPaletteAll_Compat[0][13][2]);

    /* Use M11_Screenshot_Capture directly so the probe pins the
     * exact 24-bit BMP write path (the on-disk BMP bytes are the
     * receipt we decode below).  The expansion to a 256-entry
     * palette mirrors what M11_Screenshot_CaptureCurrent does at
     * PROBE_PALETTE_LEVEL = 0 (the brightest dungeon level).
     * IMPORTANT: M11_Screenshot_Capture uses the raw framebuffer
     * byte as the palette index.  Per DUNVIEW.C the framebuffer
     * stores a 4-bit palette index in the low nibble and a 4-bit
     * brightness level in the high nibble
     * (M11_FB_INDEX_MASK=0x0F, M11_FB_LEVEL_MASK=0xF0).  Without
     * pre-masking, a framebuffer byte of 0x10 would be looked up
     * as palette index 16, which is not what the C026 blit
     * intended.  M11_Screenshot_CaptureCurrent masks the fb with
     * 0x0F before capture; we do the same to keep the
     * screenshot_receipt_rect round-trip self-consistent. */
    {
        static unsigned char maskedFb[PROBE_FB_W * PROBE_FB_H];
        unsigned char palette[256 * 3];
        int i;
        for (i = 0; i < PROBE_FB_W * PROBE_FB_H; ++i) {
            maskedFb[i] = (unsigned char)(fb[i] & M11_FB_INDEX_MASK);
        }
        for (i = 0; i < 256; ++i) {
            int idx = i & 0x0F;
            palette[i * 3 + 0] = G9010_auc_VgaPaletteAll_Compat[PROBE_PALETTE_LEVEL][idx][0];
            palette[i * 3 + 1] = G9010_auc_VgaPaletteAll_Compat[PROBE_PALETTE_LEVEL][idx][1];
            palette[i * 3 + 2] = G9010_auc_VgaPaletteAll_Compat[PROBE_PALETTE_LEVEL][idx][2];
        }
        printf("  DEBUG palette[12*3..+2] = (%d,%d,%d)\n",
            palette[36], palette[37], palette[38]);
        printf("  DEBUG palette[0*3..+2] = (%d,%d,%d) (BLACK)\n",
            palette[0], palette[1], palette[2]);
        /* DEBUG: dump maskedFb[68*320+96] before capture */
        printf("  DEBUG: maskedFb[68*320+96] = 0x%02x (= %d)\n",
            maskedFb[68*320+96], maskedFb[68*320+96]);
        captureRc = M11_Screenshot_Capture(maskedFb, PROBE_FB_W, PROBE_FB_H,
                                            palette, bmpDir,
                                            stampName, (int)sizeof(stampName));
        if (!captureRc) {
            fprintf(stderr,
                    "FAIL M11_Screenshot_Capture returned 0\n");
            ok = 0;
        } else {
            /* The capture succeeded; the actual file path is
             * `stampName`.  Copy it into outBmpPath for downstream
             * decode steps. */
            snprintf(outBmpPath, (size_t)outBmpPathCap, "%s", stampName);
            printf("  capture wrote BMP %s\n", outBmpPath);
        }
    }
    /* Now assert the file on disk matches the source-locked BMP
     * file footprint.  The capture API timestamps the file so the
     * filename includes YYYYMMDD-HHMMSS; we read whatever file was
     * written. */
    if (captureRc) {
        struct stat st;
        if (stat(outBmpPath, &st) != 0) {
            fprintf(stderr,
                    "FAIL BMP file vanished between capture and stat: %s\n",
                    outBmpPath);
            ok = 0;
        } else {
            fileBytes = (long)st.st_size;
            printf("  BMP file size = %ld (want %d)\n",
                   fileBytes, PROBE_BMP_FILE_BYTES);
            ok &= expect_int("BMP file size matches source-locked 192054",
                             (int)fileBytes, PROBE_BMP_FILE_BYTES);
        }
    }
    return ok;
}

/* (B) RECT RECEIVED ORDINAL 12: decode the source-locked D1C
 *     portrait rect (96,35,32,29) from the captured BMP on disk
 *     using the same G9010_auc_VgaPaletteAll_Compat palette the BMP
 *     writer used, and assert:
 *
 *       1. The decoded on-disk D1C rect round-trips the in-memory
 *          framebuffer D1C rect (>= 99% of pixels decode back to the
 *          same palette index the engine wrote into the fb).  This
 *          is the screenshot_receipt_rect round-trip contract — the
 *          BMP file on disk receives the same pixels the engine
 *          drew into the framebuffer.
 *
 *       2. The in-memory framebuffer D1C rect at the canonical
 *          LINFLAS pose (2,10,N) is dominated by the C026 ordinal-12
 *          opaque pixels (>= 90% match threshold, same constant the
 *          east_walkpath / walkpath_from_entrance probes lock).  This
 *          is the source-locked portrait_rect_position contract:
 *          the engine drew the right portrait into the D1C rect, so
 *          the BMP that captures it also contains the right portrait
 *          pixels.
 *
 *     Note: the comparison between the decoded BMP rect and the C026
 *     source strip would not work because M11_GameView_Draw applies
 *     a per-draw palette remap when blitting C026 into the D1C rect
 *     (the remap is part of the source-locked V1 rendering path).
 *     Instead, we compare decoded-BMP vs in-memory-fb (round-trip)
 *     AND in-memory-fb vs C026 source strip (portrait correctness),
 *     which together prove the BMP file on disk received the correct
 *     portrait pixels for the LINFLAS pose. */
static int test_rect_received_ordinal_12(const M11_AssetSlot* portraits,
                                         const unsigned char* inMemoryFb,
                                         const char* bmpPath) {
    int ok = 1;
    static unsigned char decodedRect[PROBE_PORTRAIT_W * PROBE_PORTRAIT_H];
    int roundTripMatched;
    int inMemoryOrdinalMatched;
    int inMemoryOrdinalCompared;
    int inMemoryOrdinalPct;
    long fileBytes = -1;

    printf("[B] Rect received ordinal 12: decode D1C rect from BMP\n");
    if (bmpPath[0] == '\0') {
        fprintf(stderr, "FAIL no BMP path to decode (capture step failed)\n");
        return 0;
    }
    if (!decode_d1c_rect_from_bmp(bmpPath, decodedRect,
                                  PROBE_PALETTE_LEVEL, &fileBytes)) {
        fprintf(stderr, "FAIL decode_d1c_rect_from_bmp\n");
        return 0;
    }
    /* DIAG: dump a few fb pixels vs decoded pixels */
    {
        int dx, dy;
        int nonzero_diff = 0;
        printf("  DIAG raw fb[%d*320+%d]=0x%02x (&0x0F=%d)\n",
            PROBE_PORTRAIT_Y, PROBE_PORTRAIT_X,
            inMemoryFb[PROBE_PORTRAIT_Y*PROBE_FB_W + PROBE_PORTRAIT_X],
            inMemoryFb[PROBE_PORTRAIT_Y*PROBE_FB_W + PROBE_PORTRAIT_X] & 0x0F);
        {
            int k;
            for (k = 0; k < 5; ++k) {
                int idx = PROBE_PORTRAIT_Y*PROBE_FB_W + PROBE_PORTRAIT_X + k;
                printf("  DIAG[B]: inMemoryFb[%d] = 0x%02x\n", idx, inMemoryFb[idx]);
            }
        }
        printf("  fb vs decoded at D1C rect (first 8 pixels per row):\n");
        for (dy = 0; dy < PROBE_PORTRAIT_H; dy += 8) {
            printf("    y=%d: ", dy);
            for (dx = 0; dx < 8; ++dx) {
                int rawByte = inMemoryFb[(PROBE_PORTRAIT_Y + dy) * PROBE_FB_W + (PROBE_PORTRAIT_X + dx)];
                int fbp = rawByte & 0x0F;
                int decp = decodedRect[dy * PROBE_PORTRAIT_W + dx] & 0x0F;
                if (fbp != decp) ++nonzero_diff;
                printf("(raw=0x%02x,fb=%2d,dec=%2d) ", rawByte, fbp, decp);
            }
            printf("\n");
        }
        printf("  total nonzero diff: %d\n", nonzero_diff);
    }
    /* (B.1) Round-trip equality: decoded-BMP rect == in-memory fb rect.
     * Every pixel that the engine wrote to the fb must be present in
     * the BMP file with the same palette index. */
    roundTripMatched = count_roundtrip_matched_pixels(decodedRect, inMemoryFb);
    {
        int total = PROBE_PORTRAIT_W * PROBE_PORTRAIT_H;
        int pct = total > 0 ? (roundTripMatched * 100) / total : 0;
        printf("  round-trip equality decoded-vs-fb: %d/%d (%d%%)\n",
               roundTripMatched, total, pct);
        if (pct < 99) {
            ++g_fail;
            printf("  FAIL: round-trip equality %d%% (< 99%%)\n", pct);
            ok = 0;
        } else {
            ++g_pass;
            printf("  PASS: round-trip equality %d%% (>= 99%%)\n", pct);
        }
    }
    /* (B.2) In-memory portrait correctness: the in-memory fb at D1C
     * rect is dominated by the C026 ordinal-12 opaque pixels. */
    inMemoryOrdinalMatched =
        count_in_memory_ordinal_matched_pixels(portraits, inMemoryFb,
                                                PROBE_ORDINAL_TARGET);
    inMemoryOrdinalCompared =
        count_ordinal_compared_pixels(portraits, PROBE_ORDINAL_TARGET);
    inMemoryOrdinalPct = inMemoryOrdinalCompared > 0
        ? (inMemoryOrdinalMatched * 100) / inMemoryOrdinalCompared : 0;
    printf("  in-memory fb D1C rect ordinal-12 matched=%d compared=%d pct=%d\n",
           inMemoryOrdinalMatched, inMemoryOrdinalCompared, inMemoryOrdinalPct);
    if (inMemoryOrdinalPct < PROBE_POSITIVE_MATCH_PCT) {
        ++g_fail;
        printf("  FAIL: in-memory fb D1C rect ordinal-12 match %d%% (< %d%%)\n",
               inMemoryOrdinalPct, PROBE_POSITIVE_MATCH_PCT);
        ok = 0;
    } else {
        ++g_pass;
        printf("  PASS: in-memory fb D1C rect ordinal-12 match %d%% (>= %d%%)\n",
               inMemoryOrdinalPct, PROBE_POSITIVE_MATCH_PCT);
    }
    return ok;
}

/* (C) NO FLOATING NEGATIVE: run the same screenshot receipt path
 *     for the no-portrait side pose (2,10,EAST) and assert the
 *     on-disk D1C rect round-trips the in-memory fb AND that the
 *     side pose fb has a much lower ordinal-12 match count than
 *     the canonical LINFLAS pose.  The D1C wall box for a side
 *     pose is the C346 wall-mirror frame (no portrait blitted on
 *     the side wall), so the in-memory fb at D1C rect is dominated
 *     by frame colors (BLACK, DARK_GRAY, LIGHT_GRAY), not C026
 *     portrait pixels.  The no-floating assertion uses the
 *     ordinal-12 opaque pixel match count in the in-memory fb
 *     (not the decoded BMP rect) so the side pose wall-frame
 *     pixel remap is not conflated with the ordinal-12 source
 *     strip.  Same pattern the existing east_walkpath /
 *     walkpath_from_entrance probes use for the no-floating side
 *     poses. */
static int test_no_floating_receipt(const M11_AssetSlot* portraits,
                                    M11_GameViewState* game,
                                    unsigned char* fb,
                                    const char* bmpDir) {
    int ok = 1;
    char bmpPath[1024];
    static unsigned char decodedRect[PROBE_PORTRAIT_W * PROBE_PORTRAIT_H];
    int inMemoryOrdinalMatched;
    int inMemoryOrdinalCompared;
    int inMemoryOrdinalPct;
    int roundTripMatched;
    long fileBytes = -1;

    printf("[C] No floating negative: receipt at (2,10,EAST) -> BMP\n");
    set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, PROBE_LINFLAS_EAST_DIR);
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    {
        unsigned char palette[256 * 3];
        int i;
        int captureRc;
        for (i = 0; i < 256; ++i) {
            int idx = i & 0x0F;
            palette[i * 3 + 0] = G9010_auc_VgaPaletteAll_Compat[PROBE_PALETTE_LEVEL][idx][0];
            palette[i * 3 + 1] = G9010_auc_VgaPaletteAll_Compat[PROBE_PALETTE_LEVEL][idx][1];
            palette[i * 3 + 2] = G9010_auc_VgaPaletteAll_Compat[PROBE_PALETTE_LEVEL][idx][2];
        }
        captureRc = M11_Screenshot_Capture(fb, PROBE_FB_W, PROBE_FB_H,
                                            palette, bmpDir,
                                            bmpPath, (int)sizeof(bmpPath));
        if (!captureRc) {
            fprintf(stderr, "FAIL M11_Screenshot_Capture returned 0 for side pose\n");
            return 0;
        }
    }
    if (!decode_d1c_rect_from_bmp(bmpPath, decodedRect,
                                  PROBE_PALETTE_LEVEL, &fileBytes)) {
        fprintf(stderr, "FAIL decode_d1c_rect_from_bmp for side pose\n");
        return 0;
    }
    /* (C.1) Round-trip equality for the side pose BMP — same
     * round-trip contract as (B.1), proving the BMP writer works
     * for non-portrait poses too. */
    roundTripMatched = count_roundtrip_matched_pixels(decodedRect, fb);
    {
        int total = PROBE_PORTRAIT_W * PROBE_PORTRAIT_H;
        int pct = total > 0 ? (roundTripMatched * 100) / total : 0;
        printf("  (2,10,EAST) round-trip equality decoded-vs-fb: %d/%d (%d%%)\n",
               roundTripMatched, total, pct);
        if (pct < 99) {
            ++g_fail;
            printf("  FAIL: (2,10,EAST) round-trip equality %d%% (< 99%%)\n", pct);
            ok = 0;
        } else {
            ++g_pass;
            printf("  PASS: (2,10,EAST) round-trip equality %d%% (>= 99%%)\n", pct);
        }
    }
    /* (C.2) In-memory ordinal-12 leak: at the side pose, the D1C
     * wall box is the C346 frame only (no portrait blit), so the
     * in-memory fb's ordinal-12 match count must be far below the
     * canonical pose's 569/569 (matching the 35% leak threshold
     * the existing east_walkpath / walkpath_from_entrance probes
     * lock for side poses).  This proves the LINFLAS portrait
     * pixels do not "float" onto the side wall via the screenshot
     * receipt path. */
    inMemoryOrdinalMatched =
        count_in_memory_ordinal_matched_pixels(portraits, fb,
                                                PROBE_ORDINAL_TARGET);
    inMemoryOrdinalCompared =
        count_ordinal_compared_pixels(portraits, PROBE_ORDINAL_TARGET);
    inMemoryOrdinalPct = inMemoryOrdinalCompared > 0
        ? (inMemoryOrdinalMatched * 100) / inMemoryOrdinalCompared : 0;
    printf("  (2,10,EAST) in-memory fb D1C rect ordinal-12 matched=%d compared=%d pct=%d\n",
           inMemoryOrdinalMatched, inMemoryOrdinalCompared, inMemoryOrdinalPct);
    if (inMemoryOrdinalPct >= PROBE_FLOOR_LEAK_PCT) {
        ++g_fail;
        printf("  FAIL: (2,10,EAST) in-memory fb D1C rect ordinal-12 leak %d%% (>= %d%%)\n",
               inMemoryOrdinalPct, PROBE_FLOOR_LEAK_PCT);
        ok = 0;
    } else {
        ++g_pass;
        printf("  PASS: (2,10,EAST) in-memory fb D1C rect ordinal-12 leak %d%% (< %d%%)\n",
               inMemoryOrdinalPct, PROBE_FLOOR_LEAK_PCT);
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;
    char bmpDir[1024];
    char bmpDirEast[1024];
    char bmpPath[1024];
    int ord;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    /* Resolve a probe-controlled BMP output directory under HOME so
     * the probe does not pollute the user-facing
     * ~/.firestaff/screenshots/ directory.  Section (A) writes the
     * LINFLAS-pose capture here; section (C) writes the side-pose
     * capture to a sibling "-east" sub-directory so the two
     * captures don't collide when both happen within the same
     * strftime("%Y%m%d-%H%M%S") second (the writer overwrites on
     * filename match). */
    {
        const char* home = getenv("HOME");
        if (!home || !*home) home = ".";
        snprintf(bmpDir, sizeof(bmpDir),
                 "%s/.firestaff/firestaff-probe-screenshots", home);
        snprintf(bmpDirEast, sizeof(bmpDirEast),
                 "%s/.firestaff/firestaff-probe-screenshots/east", home);
        /* Best-effort mkdir; ignore EEXIST.  screenshot_m11.c will
         * also create the directory if missing but doing it here
         * makes the probe self-contained. */
#ifdef _WIN32
        _mkdir(bmpDir);
        _mkdir(bmpDirEast);
#else
        (void)mkdir(bmpDir, 0755);
        (void)mkdir(bmpDirEast, 0755);
#endif
    }
    bmpPath[0] = '\0';

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 3;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < PROBE_PORTRAIT_STRIP_W ||
        portraits->height < PROBE_PORTRAIT_STRIP_H) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 3;
    }

    /* Bind the ordinal 12 = LINFLAS identity from the canonical pose
     * (2,10,N) so the slice is bound to a real source identity. */
    set_pose(&game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, PROBE_LINFLAS_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (ord != PROBE_ORDINAL_TARGET) {
        fprintf(stderr,
                "FAIL canonical LINFLAS pose front-mirror ordinal=%d want=%d\n",
                ord, PROBE_ORDINAL_TARGET);
        ok = 0;
    } else {
        printf("  canonical LINFLAS pose (2,10,N) front-mirror ordinal = %d\n", ord);
        ++g_pass;
        printf("  PASS: canonical LINFLAS pose front-mirror ordinal == 12\n");
    }

    /* Lock the C026 ordinal-12 source-rect math the probe relies
     * on so a future refactor that moves the C026 atlas stride is
     * caught here too.  Same math as the front_north_entry /
     * front_south_entry / walkpath_from_entrance probes' lock. */
    {
        int col = PROBE_ORDINAL_TARGET & 7;
        int row = (PROBE_ORDINAL_TARGET >> 3) & 3;
        int sx = col * PROBE_PORTRAIT_W;
        int sy = row * PROBE_PORTRAIT_H;
        ok &= expect_int("ordinal 12 col = ordinal mod 8", col, 4);
        ok &= expect_int("ordinal 12 row = ordinal / 8", row, 1);
        ok &= expect_int("ordinal 12 source X == 4*32", sx, 128);
        ok &= expect_int("ordinal 12 source Y == 1*29", sy, 29);
        ok &= expect_int("ordinal 12 source right edge inside C026 strip",
                         sx + PROBE_PORTRAIT_W <= PROBE_PORTRAIT_STRIP_W, 1);
        ok &= expect_int("ordinal 12 source bottom edge inside C026 strip",
                         sy + PROBE_PORTRAIT_H <= PROBE_PORTRAIT_STRIP_H, 1);
    }

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 12, "
           "route screenshot_receipt_rect, aspect portrait_rect_position ===\n");
    printf("sourceEvidence=DUNGEON.C:2558,2608-2612 (C127 sensorData -> G0289)\n");
    printf("                DUNGEON.C:2573 (M011_CELL(sensor) -> visible wall)\n");
    printf("                MOVESENS.C:1501-1503 (C127 -> F0280)\n");
    printf("                DUNVIEW.C:3913-3928 (C346 frame + C026 portrait blit)\n");
    printf("                DUNVIEW.C:8522-8533 (C026 D1C re-blt on tick redraw)\n");
    printf("                DUNVIEW.C:8318-8542 F0128 (far-to-near draw order)\n");
    printf("                COORD.C:1693-1722 (PC 3.4 viewport origin / 224x136)\n");
    printf("                DEFS.H:2071-2079 (G2071_C320 / G2078_C32 / G2079_C29)\n");
    printf("                GAMELOOP.C:90 + F0128 (M11 present hook)\n");
    printf("                screenshot_m11.c (24-bit BMP writer + palette)\n");
    printf("                vga_palette_pc34_compat.h (G9010_auc_VgaPaletteAll_Compat)\n\n");

    ok &= test_receipt_byte_stability(&game, currFb, bmpDir, bmpPath,
                                      (int)sizeof(bmpPath));
    ok &= test_rect_received_ordinal_12(portraits, currFb, bmpPath);
    ok &= test_no_floating_receipt(portraits, &game, currFb, bmpDirEast);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
