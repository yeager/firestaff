/*
 * firestaff_v1_snd3_event_map_probe — Pass 52 verification probe.
 *
 * Verifies the source-anchored DM PC v3.4 sound-event -> GRAPHICS.DAT SND3
 * item mapping without wiring runtime playback.
 */

#include "../../sound_event_snd3_map_v1.h"
#include "../../graphics_dat_snd3_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int soundIndex;
    unsigned int itemIndex;
    unsigned int ordinal;
    const char* macroNeedle;
    const char* labelNeedle;
} ExpectedMap;

static const ExpectedMap kExpected[V1_DM_SOUND_EVENT_COUNT] = {
    { 0, 671,  0, "METALLIC_THUD", "Falling item"},
    { 1, 672,  1, "SWITCH", "Switch"},
    { 2, 673,  2, "DOOR_RATTLE", "Door"},
    { 3, 673,  2, "DOOR_RATTLE_ENTRANCE", "Door"},
    { 4, 674,  3, "WOODEN_THUD", "Touching Wall"},
    { 5, 675,  4, "STRONG_EXPLOSION", "Exploding Fireball"},
    { 6, 675,  4, "WEAK_EXPLOSION", "Exploding Fireball"},
    { 7, 677,  5, "SCREAM", "Falling and Dying"},
    { 8, 678,  6, "SWALLOW", "Swallowing"},
    { 9, 679,  7, "CHAMPION_0", "Champion Wounded 1"},
    {10, 680,  8, "CHAMPION_1", "Champion Wounded 2"},
    {11, 681,  9, "CHAMPION_2", "Champion Wounded 3"},
    {12, 682, 10, "CHAMPION_3", "Champion Wounded 4"},
    {13, 684, 12, "SKELETON_ANIMATED_ARMOUR", "Skeleton - Animated Armour"},
    {14, 685, 13, "BUZZ", "Teleporting"},
    {15, 687, 14, "PARTY_DAMAGED", "Running Into A Wall"},
    {16, 683, 11, "SPELL", "Exploding Spell"},
    {17, 707, 27, "WAR_CRY", "War Cry"},
    {18, 704, 24, "BLOW_HORN", "Blowing Horn Of Fear"},
    {19, 690, 17, "SCREAMER_OITU", "Screamer - Oitu"},
    {20, 691, 18, "GIANT_SCORPION", "Giant Scorpion"},
    {21, 692, 19, "MAGENTA_WORM", "Magenta Worm"},
    {22, 693, 20, "GIGGLER", "Giggler"},
    {23, 688, 15, "PAIN_RAT", "Pain Rat - Red Dragon"},
    {24, 708, 28, "ROCK_ROCKPILE", "Rockpile"},
    {25, 689, 16, "MUMMY_GHOST", "Mummy - Ghost"},
    {26, 709, 29, "WATER_ELEMENTAL", "Water Elemental"},
    {27, 710, 30, "COUATL", "Couatl"},
    {28, 701, 21, "ANIMATED_ARMOUR", "Move (Animated Armour)"},
    {29, 702, 22, "COUATL_GIANT_WASP", "Giant Wasp - Couatl"},
    {30, 703, 23, "MUMMY_TROLIN", "Mummy - Trolin"},
    {31, 705, 25, "SCREAMER_ROCK", "Move (Screamer"},
    {32, 706, 26, "SWAMP_SLIME", "Swamp Slime - Water Elemental"},
    {33, 711, 31, "RED_DRAGON", "Red Dragon"},
    {34, 712, 32, "SKELETON", "Move (Skeleton)"}
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

static int contains_item(const unsigned int* items, unsigned int count, unsigned int item) {
    unsigned int i;
    for (i = 0; i < count; ++i) if (items[i] == item) return 1;
    return 0;
}

int main(void) {
    unsigned int count = 0;
    const V1_SoundEventSnd3MapEntry* map = V1_SoundEventSnd3_Map(&count);

    if (map && count == V1_DM_SOUND_EVENT_COUNT && count == 35u) {
        pass("INV_V1_SND3_EVENT_MAP_01", "35 DM PC v3.4 sound-event entries exposed");
    } else {
        fail("INV_V1_SND3_EVENT_MAP_01", "wrong map count");
    }

    int tableOk = 1;
    for (unsigned int i = 0; i < count && i < V1_DM_SOUND_EVENT_COUNT; ++i) {
        const V1_SoundEventSnd3MapEntry* got = &map[i];
        const ExpectedMap* exp = &kExpected[i];
        const char* label = V1_SoundEventSnd3_LabelForSoundIndex(exp->soundIndex);
        if (got->soundIndex != exp->soundIndex || got->snd3ItemIndex != exp->itemIndex ||
            got->snd3Ordinal != exp->ordinal || !got->macroName ||
            !strstr(got->macroName, exp->macroNeedle) ||
            !V1_GraphicsSnd3_IsItemIndex(got->snd3ItemIndex) ||
            !label || !strstr(label, exp->labelNeedle)) {
            char buf[256];
            snprintf(buf, sizeof(buf), "entry %u got sound=%d item=%u ordinal=%u macro=%s label=%s",
                     i, got->soundIndex, got->snd3ItemIndex, got->snd3Ordinal,
                     got->macroName ? got->macroName : "(null)", label ? label : "(null)");
            fail("INV_V1_SND3_EVENT_MAP_02", buf);
            tableOk = 0;
            break;
        }
    }
    if (tableOk) pass("INV_V1_SND3_EVENT_MAP_02", "source table order maps expected sound indices to SND3 items/labels");

    int findOk = 1;
    for (int s = -2; s <= 36; ++s) {
        const V1_SoundEventSnd3MapEntry* e = V1_SoundEventSnd3_Find(s);
        int item = V1_SoundEventSnd3_ItemForSoundIndex(s);
        if (s >= 0 && s < (int)V1_DM_SOUND_EVENT_COUNT) {
            if (!e || item != (int)kExpected[s].itemIndex) findOk = 0;
        } else {
            if (e || item != V1_DM_SOUND_EVENT_NONE) findOk = 0;
        }
    }
    if (findOk) pass("INV_V1_SND3_EVENT_MAP_03", "lookup API accepts 0..34 and rejects out-of-range/none values");
    else fail("INV_V1_SND3_EVENT_MAP_03", "lookup API mismatch");

    unsigned int unique[35];
    unsigned int uniqueCount = 0;
    for (unsigned int i = 0; i < count && i < V1_DM_SOUND_EVENT_COUNT; ++i) {
        if (!contains_item(unique, uniqueCount, map[i].snd3ItemIndex)) {
            unique[uniqueCount++] = map[i].snd3ItemIndex;
        }
    }
    unsigned int snd3Count = 0;
    const unsigned short* snd3Items = V1_GraphicsSnd3_ItemIndices(&snd3Count);
    int coverageOk = (uniqueCount == V1_GRAPHICS_DAT_SND3_COUNT && snd3Count == V1_GRAPHICS_DAT_SND3_COUNT);
    for (unsigned int i = 0; i < snd3Count; ++i) {
        if (!contains_item(unique, uniqueCount, snd3Items[i])) coverageOk = 0;
    }
    if (coverageOk && map[2].snd3ItemIndex == map[3].snd3ItemIndex &&
        map[5].snd3ItemIndex == map[6].snd3ItemIndex) {
        pass("INV_V1_SND3_EVENT_MAP_04", "35 events cover all 33 SND3 items; only door-rattle and explosion aliases duplicate items");
    } else {
        fail("INV_V1_SND3_EVENT_MAP_04", "unique SND3 coverage/alias invariant mismatch");
    }

    const char* path = resolve_graphics_path();
    FILE* probe = fopen(path, "rb");
    if (!probe) {
        printf("SKIP INV_V1_SND3_EVENT_MAP_05 original GRAPHICS.DAT not present at %s\n", path);
        printf("  set GRAPHICS_DAT_PATH=<path to original GRAPHICS.DAT> to validate manifest metadata\n");
    } else {
        fclose(probe);
        V1_GraphicsSnd3Manifest m;
        char err[256];
        if (!V1_GraphicsSnd3_ParseManifest(path, &m, err, sizeof(err))) {
            fail("INV_V1_SND3_EVENT_MAP_05", err);
        } else {
            int manifestOk = 1;
            for (unsigned int i = 0; i < count && i < V1_DM_SOUND_EVENT_COUNT; ++i) {
                const V1_SoundEventSnd3MapEntry* e = &map[i];
                if (m.items[e->snd3Ordinal].itemIndex != e->snd3ItemIndex ||
                    m.items[e->snd3Ordinal].declaredSampleCount == 0 ||
                    m.items[e->snd3Ordinal].trailingBytes > 3u) {
                    manifestOk = 0;
                    break;
                }
            }
            if (manifestOk) pass("INV_V1_SND3_EVENT_MAP_05", "mapping resolves to populated SND3 manifest entries with valid sample metadata");
            else fail("INV_V1_SND3_EVENT_MAP_05", "mapping references missing/invalid manifest metadata");
        }
    }

    printf("# summary: %d/%d invariants passed\n", g_passCount, g_passCount + g_failCount);
    return g_failCount == 0 ? 0 : 1;
}
