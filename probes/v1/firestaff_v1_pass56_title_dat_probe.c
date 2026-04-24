#include "title_dat_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ProbeTally {
    int total;
    int passed;
} ProbeTally;

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

static unsigned long fnv1a_update(unsigned long h, const char* s) {
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h;
}

static int check_sequence_signature(const V1_TitleManifest* m,
                                    unsigned long* outHash) {
    unsigned long h = 2166136261u;
    unsigned int i;
    char tmp[64];
    for (i = 0; i < m->itemCount; ++i) {
        const V1_TitleRecord* r = &m->records[i];
        snprintf(tmp, sizeof(tmp), "%04u:%s:%u:%u:%u;",
                 r->index, r->tag, r->payloadBytes, r->width, r->height);
        h = fnv1a_update(h, tmp);
    }
    if (outHash) *outHash = h;
    return (h & 0xfffffffful) == 0x6ce154a7ul;
}

int main(void) {
    ProbeTally tally = {0, 0};
    V1_TitleManifest manifest;
    V1_TitlePlayer player;
    char pathBuf[1024];
    char err[256];
    const char* path = find_title_dat(pathBuf, sizeof(pathBuf));
    const V1_TitleRecord* frame;
    unsigned int frames = 0;
    unsigned int enFrames = 0;
    unsigned int dlFrames = 0;
    unsigned int firstSegmentFrames = 0;
    unsigned int secondSegmentFrames = 0;
    unsigned long seqHash = 0;
    unsigned int i;

    if (!path) {
        printf("FAIL P56_TITLE_00 no original DM PC 3.4 TITLE file found\n");
        return 1;
    }
    printf("# TITLE path: %s\n", path);

    if (!V1_Title_ParseManifest(path, &manifest, err, sizeof(err))) {
        printf("FAIL P56_TITLE_00 parse TITLE manifest: %s\n", err);
        return 1;
    }

    probe_record(&tally, "P56_TITLE_01",
                 manifest.fileBytes == 12002u && manifest.itemCount == 59u,
                 "local TITLE mapfile is 12002 bytes and parses to Greatstone's 59 items");
    probe_record(&tally, "P56_TITLE_02",
                 manifest.animationCount == 1u && manifest.breakCount == 1u &&
                     manifest.egaPaletteCount == 1u && manifest.paletteCount == 2u &&
                     manifest.doneCount == 1u,
                 "control/palette records match Greatstone order: AN, BR, P8, PL, PL, DO");
    probe_record(&tally, "P56_TITLE_03",
                 manifest.encodedImageCount == 2u && manifest.deltaLayerCount == 51u &&
                     manifest.frameCount == 53u,
                 "frame records match Greatstone: 2 ENcoded images plus 51 Delta Layers");
    probe_record(&tally, "P56_TITLE_04",
                 manifest.records[0].type == V1_TITLE_ITEM_AN &&
                     manifest.records[1].type == V1_TITLE_ITEM_BR &&
                     manifest.records[2].type == V1_TITLE_ITEM_P8 &&
                     manifest.records[3].type == V1_TITLE_ITEM_PL &&
                     manifest.records[4].type == V1_TITLE_ITEM_EN &&
                     manifest.records[41].type == V1_TITLE_ITEM_PL &&
                     manifest.records[42].type == V1_TITLE_ITEM_EN &&
                     manifest.records[58].type == V1_TITLE_ITEM_DO,
                 "record order aligns with Greatstone item numbers 0000..0058");

    for (i = 0; i < manifest.itemCount; ++i) {
        const V1_TitleRecord* r = &manifest.records[i];
        if (r->type == V1_TITLE_ITEM_EN || r->type == V1_TITLE_ITEM_DL) {
            if (r->width != 320u || r->height != 200u) break;
        }
    }
    probe_record(&tally, "P56_TITLE_05", i == manifest.itemCount,
                 "every EN/DL frame advertises the original 320x200 V1 canvas");

    V1_TitlePlayer_Init(&player, &manifest);
    while ((frame = V1_TitlePlayer_NextFrame(&player)) != NULL) {
        frames++;
        if (frame->type == V1_TITLE_ITEM_EN) enFrames++;
        if (frame->type == V1_TITLE_ITEM_DL) dlFrames++;
        if (frame->paletteOrdinal == 1u) firstSegmentFrames++;
        if (frame->paletteOrdinal == 2u) secondSegmentFrames++;
        if (frame->frameOrdinal != frames) break;
    }
    probe_record(&tally, "P56_TITLE_06",
                 frames == 53u && enFrames == 2u && dlFrames == 51u &&
                     firstSegmentFrames == 37u && secondSegmentFrames == 16u,
                 "player yields 53 frames in item order with palette break: 37 + 16 frames");
    probe_record(&tally, "P56_TITLE_07", player.done == 1,
                 "player consumes DOne and stops instead of looping or inventing cadence");
    probe_record(&tally, "P56_TITLE_08",
                 check_sequence_signature(&manifest, &seqHash),
                 "record offsets/types/sizes/dimensions match the pass-56 TITLE fingerprint");
    printf("# pass56 title sequence fnv1a32: 0x%08lx\n", seqHash & 0xfffffffful);

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
