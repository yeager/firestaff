/*
 * firestaff_v1_graphics_dat_snd3_probe — Pass 51 verification probe.
 *
 * Verifies the original DM PC v3.4 GRAPHICS.DAT SND3 in-game SFX bank:
 *   - DMCSB2 signature/count/header shape
 *   - dmweb SND3 index set (33 items)
 *   - Greatstone labels carried by the loader
 *   - real-file offsets/sizes/attributes/sample counts match the
 *     locally verified DM PC v3.4 English table
 *   - each SND3 item decodes to exactly its declared unsigned PCM count
 *
 * Without GRAPHICS.DAT, exits 0 with SKIP.  Original assets are not
 * vendored or implied by this probe.
 */

#include "../../graphics_dat_snd3_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int index;
    unsigned long offset;
    unsigned int bytes;
    unsigned int decomp;
    unsigned short attr0;
    unsigned short attr1;
    unsigned int samples;
    unsigned int tail;
    const char* labelNeedle;
} ExpectedSnd3;

static const ExpectedSnd3 kExpected[V1_GRAPHICS_DAT_SND3_COUNT] = {
    {671, 237375,  886,  886, 0x0372, 0x0009,  882, 2, "Falling item"},
    {672, 238261,  132,  132, 0x0080, 0x000B,  128, 2, "Switch"},
    {673, 238393, 2271, 2271, 0x08DB, 0x0003, 2267, 2, "Door"},
    {674, 240664, 1408, 1408, 0x057C, 0x0009, 1404, 2, "Touching Wall"},
    {675, 242072, 3973, 3973, 0x0F82, 0x005F, 3970, 1, "Exploding Fireball"},
    {677, 246045, 7707, 7707, 0x1E17, 0x000C, 7703, 2, "Falling and Dying"},
    {678, 253752, 1615, 1615, 0x064B, 0x0016, 1611, 2, "Swallowing"},
    {679, 255367, 1311, 1311, 0x051B, 0x0008, 1307, 2, "Champion Wounded 1"},
    {680, 256678, 1383, 1383, 0x0563, 0x000A, 1379, 2, "Champion Wounded 2"},
    {681, 258061, 1271, 1271, 0x04F3, 0x000B, 1267, 2, "Champion Wounded 3"},
    {682, 259332, 1251, 1251, 0x04DF, 0x0009, 1247, 2, "Champion Wounded 4"},
    {683, 260583, 3111, 3111, 0x0C23, 0x0009, 3107, 2, "Exploding Spell"},
    {684, 263694,  999,  999, 0x03E3, 0x000A,  995, 2, "Skeleton - Animated Armour"},
    {685, 264693, 1509, 1509, 0x05E2, 0x000B, 1506, 1, "Teleporting"},
    {687, 266202, 4578, 4578, 0x11E0, 0x0000, 4576, 0, "Running Into A Wall"},
    {688, 270780, 4007, 4007, 0x0FA3, 0x0006, 4003, 2, "Pain Rat - Red Dragon"},
    {689, 274787, 3223, 3223, 0x0C93, 0x0009, 3219, 2, "Mummy - Ghost"},
    {690, 278010, 1897, 1897, 0x0766, 0x0001, 1894, 1, "Screamer - Oitu"},
    {691, 279907, 4695, 4695, 0x1253, 0x0009, 4691, 2, "Giant Scorpion"},
    {692, 284602, 4087, 4087, 0x0FF3, 0x0007, 4083, 2, "Magenta Worm"},
    {693, 288689, 2183, 2183, 0x0883, 0x000D, 2179, 2, "Giggler"},
    {701, 302647, 3188, 3188, 0x0C71, 0x0000, 3185, 1, "Move (Animated Armour)"},
    {702, 305835, 3012, 3012, 0x0BC1, 0x0000, 3009, 1, "Giant Wasp - Couatl"},
    {703, 308847, 3394, 3394, 0x0D40, 0x0000, 3392, 0, "Mummy - Trolin"},
    {704, 312241, 5316, 5316, 0x14C1, 0x0000, 5313, 1, "Blowing Horn Of Fear"},
    {705, 317557, 3762, 3762, 0x0EB0, 0x0000, 3760, 0, "Move (Screamer"},
    {706, 321319, 8642, 8642, 0x21C0, 0x0000, 8640, 0, "Swamp Slime - Water Elemental"},
    {707, 329961, 6338, 6338, 0x18C0, 0x0001, 6336, 0, "War Cry"},
    {708, 336299, 4418, 4418, 0x1140, 0x0000, 4416, 0, "Attack (Rockpile)"},
    {709, 340717, 5956, 5956, 0x1741, 0x00F9, 5953, 1, "Water Elemental"},
    {710, 346673, 4390, 4390, 0x1122, 0x00FD, 4386, 2, "Couatl"},
    {711, 351063, 8912, 8912, 0x22CD, 0x0000, 8909, 1, "Red Dragon"},
    {712, 359975, 3442, 3442, 0x0D70, 0x0000, 3440, 0, "Move (Skeleton)"}
};

static int g_passCount = 0;
static int g_failCount = 0;

static void pass(const char* name, const char* detail) {
    printf("PASS %s%s%s\n", name, (detail && detail[0]) ? " " : "", detail ? detail : "");
    g_passCount++;
}

static void fail(const char* name, const char* detail) {
    printf("FAIL %s%s%s\n", name, (detail && detail[0]) ? " " : "", detail ? detail : "");
    g_failCount++;
}

static const char* resolve_graphics_path(void) {
    const char* env = getenv("GRAPHICS_DAT_PATH");
    if (env && env[0]) return env;
    return "/Users/bosse/.firestaff/data/GRAPHICS.DAT";
}

int main(void) {
    const char* path = resolve_graphics_path();
    FILE* probe = fopen(path, "rb");
    if (!probe) {
        printf("SKIP INV_V1_SND3_01 original GRAPHICS.DAT not present at %s\n", path);
        printf("  set GRAPHICS_DAT_PATH=<path to original GRAPHICS.DAT> to run this probe\n");
        printf("# summary: skipped (no original asset present) — not a failure\n");
        return 0;
    }
    fclose(probe);

    V1_GraphicsSnd3Manifest m;
    char err[256];
    if (!V1_GraphicsSnd3_ParseManifest(path, &m, err, sizeof(err))) {
        fail("INV_V1_SND3_01", err);
        printf("# summary: %d/%d invariants passed\n", g_passCount, g_passCount + g_failCount);
        return 1;
    }
    pass("INV_V1_SND3_01", "manifest parsed");

    if (m.signature == V1_GRAPHICS_DAT_SIGNATURE &&
        m.itemCount == V1_GRAPHICS_DAT_ITEM_COUNT &&
        m.headerBytes == V1_GRAPHICS_DAT_HEADER_BYTES) {
        pass("INV_V1_SND3_02", "DMCSB2 signature=0x8001, 713 items, 5708-byte header");
    } else {
        fail("INV_V1_SND3_02", "header invariants not met");
    }

    unsigned int count = 0;
    const unsigned short* indices = V1_GraphicsSnd3_ItemIndices(&count);
    int indexOk = (count == V1_GRAPHICS_DAT_SND3_COUNT);
    for (unsigned int i = 0; i < count && i < V1_GRAPHICS_DAT_SND3_COUNT; ++i) {
        if (indices[i] != kExpected[i].index || !V1_GraphicsSnd3_IsItemIndex(indices[i])) indexOk = 0;
    }
    if (!V1_GraphicsSnd3_IsItemIndex(676) && !V1_GraphicsSnd3_IsItemIndex(686) &&
        !V1_GraphicsSnd3_IsItemIndex(694) && !V1_GraphicsSnd3_IsItemIndex(700)) {
        /* expected holes/non-SND3 neighbors */
    } else {
        indexOk = 0;
    }
    if (indexOk) pass("INV_V1_SND3_03", "dmweb SND3 item index set matches 33-item bank");
    else fail("INV_V1_SND3_03", "SND3 item index set mismatch");

    int tableOk = 1;
    for (unsigned int i = 0; i < V1_GRAPHICS_DAT_SND3_COUNT; ++i) {
        const V1_GraphicsSnd3Item* got = &m.items[i];
        const ExpectedSnd3* exp = &kExpected[i];
        if (got->itemIndex != exp->index || got->fileOffset != exp->offset ||
            got->itemBytes != exp->bytes || got->decompressedBytes != exp->decomp ||
            got->attr0 != exp->attr0 || got->attr1 != exp->attr1 ||
            got->declaredSampleCount != exp->samples || got->trailingBytes != exp->tail) {
            char buf[256];
            snprintf(buf, sizeof(buf),
                     "item %u mismatch: off=%lu bytes=%u decomp=%u attr=%04x/%04x samples=%u tail=%u",
                     got->itemIndex, got->fileOffset, got->itemBytes, got->decompressedBytes,
                     got->attr0, got->attr1, got->declaredSampleCount, got->trailingBytes);
            fail("INV_V1_SND3_04", buf);
            tableOk = 0;
            break;
        }
    }
    if (tableOk) pass("INV_V1_SND3_04", "all 33 SND3 headers match verified DM PC v3.4 table");

    int decodeOk = 1;
    unsigned long totalSamples = 0;
    for (unsigned int i = 0; i < V1_GRAPHICS_DAT_SND3_COUNT; ++i) {
        V1_GraphicsSnd3Buffer b;
        if (!V1_GraphicsSnd3_DecodeItem(path, &m, kExpected[i].index, &b, err, sizeof(err))) {
            char buf[160];
            snprintf(buf, sizeof(buf), "item %u decode failed: %s", kExpected[i].index, err);
            fail("INV_V1_SND3_05", buf);
            decodeOk = 0;
            break;
        }
        if (b.sampleRateHz != V1_GRAPHICS_DAT_SND3_SAMPLE_RATE_HZ ||
            b.declaredSampleCount != kExpected[i].samples ||
            b.decodedSampleCount != kExpected[i].samples) {
            char buf[160];
            snprintf(buf, sizeof(buf), "item %u decoded=%u declared=%u rate=%u",
                     b.itemIndex, b.decodedSampleCount, b.declaredSampleCount, b.sampleRateHz);
            fail("INV_V1_SND3_05", buf);
            V1_GraphicsSnd3_FreeBuffer(&b);
            decodeOk = 0;
            break;
        }
        totalSamples += b.decodedSampleCount;
        V1_GraphicsSnd3_FreeBuffer(&b);
    }
    if (decodeOk) {
        char buf[128];
        snprintf(buf, sizeof(buf), "all 33 SND3 items decode to unsigned PCM at 6000 Hz (%lu samples)", totalSamples);
        pass("INV_V1_SND3_05", buf);
    }

    int labelsOk = 1;
    for (unsigned int i = 0; i < V1_GRAPHICS_DAT_SND3_COUNT; ++i) {
        const char* label = V1_GraphicsSnd3_ItemLabel(kExpected[i].index);
        if (!label || !strstr(label, kExpected[i].labelNeedle)) {
            char buf[160];
            snprintf(buf, sizeof(buf), "item %u label '%s' missing '%s'",
                     kExpected[i].index, label ? label : "(null)", kExpected[i].labelNeedle);
            fail("INV_V1_SND3_06", buf);
            labelsOk = 0;
            break;
        }
    }
    if (labelsOk) pass("INV_V1_SND3_06", "Greatstone per-sound labels present for Sound 00..32");

    int total = g_passCount + g_failCount;
    printf("# summary: %d/%d invariants passed\n", g_passCount, total);
    return g_failCount == 0 ? 0 : 1;
}
