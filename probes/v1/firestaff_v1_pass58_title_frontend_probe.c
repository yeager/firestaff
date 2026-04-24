#include "title_frontend_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ProbeTally {
    int total;
    int passed;
} ProbeTally;

typedef struct ExpectedFrame {
    unsigned int wanted;
    unsigned int found;
    unsigned long packedHash;
} ExpectedFrame;

static void probe_record(ProbeTally* tally,
                         const char* id,
                         int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

static int file_exists(const char* path) {
    FILE* f;
    if (!path || !*path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static const char* find_title_dat(char* buf, size_t cap) {
    const char* envPath = getenv("FIRESTAFF_TITLE_DAT");
    const char* home;
    if (file_exists(envPath)) return envPath;
    if (file_exists("TITLE")) return "TITLE";
    home = getenv("HOME");
    if (home && buf && cap > 0) {
        int n = snprintf(buf, cap, "%s/.firestaff/data/TITLE", home);
        if (n > 0 && (size_t)n < cap && file_exists(buf)) return buf;
    }
    if (file_exists("/Users/bosse/.openclaw/data/redmcsb-original/TITLE")) {
        return "/Users/bosse/.openclaw/data/redmcsb-original/TITLE";
    }
    return NULL;
}

static unsigned long fnv1a_byte(unsigned long h, unsigned int b) {
    h ^= (unsigned char)b;
    h *= 16777619u;
    return h;
}

static unsigned long hash_packed_screen(const unsigned char* screenBitmap) {
    unsigned long h = 2166136261u;
    unsigned int i;
    for (i = 0; i < 320u * 200u / 2u; ++i) {
        h = fnv1a_byte(h, screenBitmap[i]);
    }
    return h & 0xfffffffful;
}

static void pack_expected(const V1_TitleRenderFrame* frame, unsigned char* dst) {
    unsigned int x, y;
    memset(dst, 0, 320u * 200u / 2u);
    for (y = 0; y < frame->height; ++y) {
        unsigned char* row = dst + y * 160u;
        const unsigned char* src = frame->colorIndices + y * frame->width;
        for (x = 0; x < frame->width; x += 2u) {
            row[x >> 1] = (unsigned char)(((src[x] & 0x0fu) << 4) | (src[x + 1u] & 0x0fu));
        }
    }
}

static int on_expected_frame(const V1_TitleRenderFrame* frame,
                             void* userData,
                             char* errMsg,
                             size_t errMsgBytes) {
    ExpectedFrame* expected = (ExpectedFrame*)userData;
    unsigned char packed[320u * 200u / 2u];
    (void)errMsg;
    (void)errMsgBytes;
    if (frame && expected && frame->frameOrdinal == expected->wanted) {
        pack_expected(frame, packed);
        expected->packedHash = hash_packed_screen(packed);
        expected->found = 1u;
    }
    return 1;
}

static unsigned long expected_hash_for_frame(const char* titlePath, unsigned int ordinal) {
    ExpectedFrame expected;
    char err[256];
    memset(&expected, 0, sizeof(expected));
    expected.wanted = ordinal;
    if (!V1_Title_RenderFrames(titlePath, on_expected_frame, &expected, err, sizeof(err))) {
        return 0;
    }
    return expected.found ? expected.packedHash : 0;
}

int main(void) {
    ProbeTally tally;
    char pathBuf[1024];
    char err[256];
    const char* path = find_title_dat(pathBuf, sizeof(pathBuf));
    unsigned char storage[4u + 320u * 200u / 2u];
    unsigned char* screen = storage + 4u;
    unsigned int ordinals[] = {1u, 37u, 38u, 53u, 54u};
    unsigned long frontendHashes[5];
    unsigned long expectedHashes[5];
    V1_TitleFrontendRenderResult results[5];
    unsigned int i;

    memset(&tally, 0, sizeof(tally));
    memset(frontendHashes, 0, sizeof(frontendHashes));
    memset(expectedHashes, 0, sizeof(expectedHashes));
    memset(results, 0, sizeof(results));

    if (!path) {
        printf("FAIL P58_TITLE_FRONTEND_00 no original DM PC 3.4 TITLE file found\n");
        return 1;
    }
    printf("# TITLE path: %s\n", path);

    for (i = 0; i < 5u; ++i) {
        memset(storage, 0, sizeof(storage));
        if (!V1_TitleFrontend_RenderFrameToScreen(path, ordinals[i], screen, &results[i], err, sizeof(err))) {
            printf("FAIL P58_TITLE_FRONTEND_00 render frontend frame %u: %s\n", ordinals[i], err);
            return 1;
        }
        frontendHashes[i] = hash_packed_screen(screen);
        expectedHashes[i] = expected_hash_for_frame(path, results[i].renderedFrameOrdinal);
    }

    probe_record(&tally, "P58_TITLE_FRONTEND_01",
                 results[0].usedOriginalTitleData == 1 && results[0].totalFrames == 53u &&
                     results[0].width == 320u && results[0].height == 200u,
                 "frontend renders from original TITLE data into a 320x200 V1 screen bitmap");
    probe_record(&tally, "P58_TITLE_FRONTEND_02",
                 results[0].renderedFrameOrdinal == 1u && results[1].renderedFrameOrdinal == 37u &&
                     results[2].renderedFrameOrdinal == 38u && results[3].renderedFrameOrdinal == 53u,
                 "frontend can reach first frame, palette-break boundary frames, and last frame");
    probe_record(&tally, "P58_TITLE_FRONTEND_03",
                 results[0].paletteOrdinal == 1u && results[1].paletteOrdinal == 1u &&
                     results[2].paletteOrdinal == 2u && results[3].paletteOrdinal == 2u,
                 "frontend preserves pass-57 TITLE segment palettes as 37 + 16 frames");
    probe_record(&tally, "P58_TITLE_FRONTEND_04",
                 results[4].renderedFrameOrdinal == 1u,
                 "deterministic implementation advance wraps requested frame 54 to original frame 1; original timing remains unclaimed");
    probe_record(&tally, "P58_TITLE_FRONTEND_05",
                 frontendHashes[0] == expectedHashes[0] && frontendHashes[1] == expectedHashes[1] &&
                     frontendHashes[2] == expectedHashes[2] && frontendHashes[3] == expectedHashes[3] &&
                     frontendHashes[4] == expectedHashes[4],
                 "frontend-packed frames match pass-57 renderer palette-index output for sampled frames");
    printf("# pass58 frontend packed hashes: f1=0x%08lx f37=0x%08lx f38=0x%08lx f53=0x%08lx f54wrap=0x%08lx\n",
           frontendHashes[0], frontendHashes[1], frontendHashes[2], frontendHashes[3], frontendHashes[4]);
    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
