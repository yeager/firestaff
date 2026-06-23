/*
 * firestaff_v1_ftl_swoosh_ascii_probe.c
 *
 * Pass-pass841 — FTL swoosh ASCII sanity probe.
 *
 * Companion to firestaff_v1_ftl_swoosh_palette_regression_probe. After
 * the unpack fix lands, this probe prints the decoded FTL logo as ASCII
 * (subsampled 4x4 per cell) so a regression can be eyeballed in the
 * test log without launching SDL. The pre-fix behavior was a half-blank
 * vertically-striped image; post-fix this should show a recognizable
 * logo silhouette.
 */

#include "swsh_frontend_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FB_W 320u
#define FB_H 200u
#define FB_BYTES (FB_W * FB_H)

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

/* Static buffers: 64000 bytes per buffer × 2 buffers would blow the
 * default 8 MB stack on macOS, so allocate in BSS instead. */
static unsigned char s_packed[FB_BYTES / 2u];
static unsigned char s_indexed[FB_BYTES];

int main(int argc, char** argv) {
    FILE* f;
    long fsize;
    unsigned char* buf;
    SWSH_CompatLogoPayload payload;
    unsigned char* packed = s_packed;
    unsigned char* indexed = s_indexed;
    unsigned int i;
    unsigned int r, c;
    int nonzeroRows = 0;
    int totalNonzero = 0;
    int lastNonzeroRow = -1;
    int firstNonzeroRow = -1;
    int firstNonzeroCol = -1;
    int lastNonzeroCol = -1;
    int fullResNonzero = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <SWOOSH-path>\n", argv[0]);
        return 1;
    }
    f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "FAIL cannot open %s\n", argv[1]); return 1; }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (unsigned char*)malloc((size_t)fsize);
    if (fread(buf, 1, (size_t)fsize, f) != (size_t)fsize) {
        fprintf(stderr, "FAIL short read\n");
        return 1;
    }
    fclose(f);

    memset(&payload, 0, sizeof(payload));
    if (!SWSH_Compat_FindLogoImagePayloadEx(buf, (unsigned int)fsize, &payload)) {
        fprintf(stderr, "FAIL no source-shaped FTL logo payload\n");
        free(buf);
        return 1;
    }

    memset(packed, 0, FB_BYTES / 2u);
    SWSH_Compat_ExpandLogoToBitmap(payload.payload, packed);

    memset(indexed, 0, FB_BYTES);
    for (i = 0u; i < FB_BYTES; i += 2u) {
        unsigned char b = packed[i >> 1];
        indexed[i]      = (unsigned char)((b >> 4) & 0x0Fu);
        indexed[i + 1u] = (unsigned char)(b & 0x0Fu);
    }

    /* Print ASCII: downsample 2x2 per cell, pick a representative index. */
    printf("FTL swoosh decoded logo (subsampled 2x):\n");
    for (r = 0u; r < FB_H; r += 2u) {
        int hasContent = 0;
        for (c = 0u; c < FB_W; c += 2u) {
            unsigned char v = indexed[r * FB_W + c];
            totalNonzero += (v != 0u) ? 1 : 0;
            hasContent |= (v != 0u);
            char ch;
            switch (v) {
                case 0:  ch = ' '; break;
                case 12: ch = '#'; break; /* background colour */
                case 15: ch = '@'; break; /* foreground */
                default: ch = '.'; break;
            }
            putchar(ch);
        }
        putchar('\n');
        if (hasContent) {
            if (firstNonzeroRow < 0) firstNonzeroRow = (int)r;
            lastNonzeroRow = (int)r;
            nonzeroRows++;
        }
    }

    for (i = 0u; i < FB_BYTES; ++i) {
        if (indexed[i] != 0u) {
            int col = (int)(i % FB_W);
            int row = (int)(i / FB_W);
            fullResNonzero++;
            if (firstNonzeroCol < 0 || col < firstNonzeroCol) firstNonzeroCol = col;
            if (col > lastNonzeroCol) lastNonzeroCol = col;
            if (firstNonzeroRow < 0 || row < firstNonzeroRow) firstNonzeroRow = row;
            if (row > lastNonzeroRow) lastNonzeroRow = row;
        }
    }

    printf("\n# logoStats: sampledNonzero=%d fullNonzero=%d nonzeroRows=%d firstRow=%d lastRow=%d firstCol=%d lastCol=%d\n",
           totalNonzero, fullResNonzero, nonzeroRows, firstNonzeroRow, lastNonzeroRow,
           firstNonzeroCol, lastNonzeroCol);

    SWSH_Compat_ReleaseLogoImagePayload(&payload);
    free(buf);
    if (fullResNonzero < 8000 || fullResNonzero > 12000 ||
        firstNonzeroRow < 45 || firstNonzeroRow > 60 ||
        lastNonzeroRow < 160 || lastNonzeroRow > 180 ||
        firstNonzeroCol < 15 || firstNonzeroCol > 35 ||
        lastNonzeroCol < 250 || lastNonzeroCol > 280) {
        fprintf(stderr, "FAIL decoded FTL logo silhouette is not source-shaped\n");
        return 1;
    }
    return 0;
}
