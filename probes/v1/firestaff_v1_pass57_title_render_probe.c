#include "title_dat_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ProbeTally {
    int total;
    int passed;
} ProbeTally;

typedef struct RenderStats {
    ProbeTally tally;
    unsigned int frames;
    unsigned int enFrames;
    unsigned int dlFrames;
    unsigned int firstSegmentFrames;
    unsigned int secondSegmentFrames;
    unsigned int badOrdinal;
    unsigned int badDimensions;
    unsigned int badDuration;
    unsigned int badPalette;
    unsigned long indexHash;
    unsigned long rgbHash;
    const char* dumpDir;
} RenderStats;

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

static void hash_u32(unsigned long* h, unsigned int v) {
    *h = fnv1a_byte(*h, (v >> 24) & 0xffu);
    *h = fnv1a_byte(*h, (v >> 16) & 0xffu);
    *h = fnv1a_byte(*h, (v >> 8) & 0xffu);
    *h = fnv1a_byte(*h, v & 0xffu);
}

static int dump_ppm(const V1_TitleRenderFrame* frame, const char* dir,
                    char* errMsg, size_t errMsgBytes) {
    char path[1024];
    FILE* f;
    unsigned int i;
    if (!dir || !*dir) return 1;
    if (!frame || !frame->palette || !frame->colorIndices) return 0;
    snprintf(path, sizeof(path), "%s/frame_%04u.ppm", dir, frame->frameOrdinal);
    f = fopen(path, "wb");
    if (!f) {
        snprintf(errMsg, errMsgBytes, "cannot write %s", path);
        return 0;
    }
    fprintf(f, "P6\n%u %u\n255\n", frame->width, frame->height);
    for (i = 0; i < frame->width * frame->height; ++i) {
        const unsigned char* rgb = frame->palette->rgba[frame->colorIndices[i] & 0x0fu];
        fputc(rgb[0], f);
        fputc(rgb[1], f);
        fputc(rgb[2], f);
    }
    fclose(f);
    return 1;
}

static int on_frame(const V1_TitleRenderFrame* frame,
                    void* userData,
                    char* errMsg,
                    size_t errMsgBytes) {
    RenderStats* s = (RenderStats*)userData;
    unsigned int i;
    if (!s || !frame || !frame->record || !frame->palette || !frame->colorIndices) return 0;
    s->frames += 1u;
    if (frame->record->type == V1_TITLE_ITEM_EN) s->enFrames += 1u;
    if (frame->record->type == V1_TITLE_ITEM_DL) s->dlFrames += 1u;
    if (frame->paletteOrdinal == 1u) s->firstSegmentFrames += 1u;
    if (frame->paletteOrdinal == 2u) s->secondSegmentFrames += 1u;
    if (frame->frameOrdinal != s->frames) s->badOrdinal += 1u;
    if (frame->width != 320u || frame->height != 200u) s->badDimensions += 1u;
    if (frame->durationFrames != 1u) s->badDuration += 1u;
    if (frame->paletteOrdinal != 1u && frame->paletteOrdinal != 2u) s->badPalette += 1u;

    hash_u32(&s->indexHash, frame->frameOrdinal);
    hash_u32(&s->indexHash, frame->record->index);
    hash_u32(&s->indexHash, frame->record->type);
    hash_u32(&s->indexHash, frame->paletteOrdinal);
    for (i = 0; i < frame->width * frame->height; ++i) {
        unsigned int c = frame->colorIndices[i] & 0x0fu;
        const unsigned char* rgb = frame->palette->rgba[c];
        s->indexHash = fnv1a_byte(s->indexHash, c);
        s->rgbHash = fnv1a_byte(s->rgbHash, rgb[0]);
        s->rgbHash = fnv1a_byte(s->rgbHash, rgb[1]);
        s->rgbHash = fnv1a_byte(s->rgbHash, rgb[2]);
    }
    return dump_ppm(frame, s->dumpDir, errMsg, errMsgBytes);
}

int main(void) {
    RenderStats stats;
    char pathBuf[1024];
    char err[256];
    const char* path = find_title_dat(pathBuf, sizeof(pathBuf));

    memset(&stats, 0, sizeof(stats));
    stats.indexHash = 2166136261u;
    stats.rgbHash = 2166136261u;
    stats.dumpDir = getenv("FIRESTAFF_PASS57_DUMP_DIR");

    if (!path) {
        printf("FAIL P57_TITLE_RENDER_00 no original DM PC 3.4 TITLE file found\n");
        return 1;
    }
    printf("# TITLE path: %s\n", path);

    if (!V1_Title_RenderFrames(path, on_frame, &stats, err, sizeof(err))) {
        printf("FAIL P57_TITLE_RENDER_00 render TITLE frames: %s\n", err);
        return 1;
    }

    probe_record(&stats.tally, "P57_TITLE_RENDER_01",
                 stats.frames == 53u && stats.enFrames == 2u && stats.dlFrames == 51u,
                 "renderer emits 53 original TITLE frames: 2 EN base frames plus 51 DL composites");
    probe_record(&stats.tally, "P57_TITLE_RENDER_02",
                 stats.firstSegmentFrames == 37u && stats.secondSegmentFrames == 16u && stats.badPalette == 0u,
                 "renderer applies source PL break segmentation as 37 + 16 frames");
    probe_record(&stats.tally, "P57_TITLE_RENDER_03",
                 stats.badOrdinal == 0u && stats.badDimensions == 0u,
                 "render callback order is frameOrdinal 1..53 with 320x200 canvas dimensions");
    probe_record(&stats.tally, "P57_TITLE_RENDER_04",
                 stats.badDuration == 0u,
                 "source EN/DL item attributes are one display frame each; no frontend cadence is inferred");
    probe_record(&stats.tally, "P57_TITLE_RENDER_05",
                 (stats.indexHash & 0xfffffffful) == 0xb4e5d330ul,
                 "decoded TITLE palette-index composites match pass-57 original-data fingerprint");
    probe_record(&stats.tally, "P57_TITLE_RENDER_06",
                 (stats.rgbHash & 0xfffffffful) == 0x143fa969ul,
                 "decoded TITLE RGB composites match pass-57 PL-applied fingerprint");
    printf("# pass57 title index fnv1a32: 0x%08lx\n", stats.indexHash & 0xfffffffful);
    printf("# pass57 title rgb fnv1a32: 0x%08lx\n", stats.rgbHash & 0xfffffffful);
    printf("# summary: %d/%d invariants passed\n", stats.tally.passed, stats.tally.total);
    return (stats.tally.passed == stats.tally.total) ? 0 : 1;
}
