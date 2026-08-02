#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_text_decode.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 2352
#define UD_PER_SECTOR 2048
#define SYNC_OFFSET 16

static uint8_t *load_track02_ud(const char *path, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsize <= 0) { fclose(fp); return NULL; }
    size_t raw_size = (size_t)fsize;
    uint8_t *raw = malloc(raw_size);
    if (!raw) { fclose(fp); return NULL; }
    fread(raw, 1, raw_size, fp);
    fclose(fp);
    size_t sectors = raw_size / SECTOR_SIZE;
    size_t ud_size = sectors * UD_PER_SECTOR;
    uint8_t *ud = calloc(1, ud_size);
    if (!ud) { free(raw); return NULL; }
    for (size_t s = 0; s < sectors; s++)
        memcpy(ud + s * UD_PER_SECTOR, raw + s * SECTOR_SIZE + SYNC_OFFSET, UD_PER_SECTOR);
    free(raw);
    *out_size = ud_size;
    return ud;
}

static const char *find_track02(void) {
    const char *home = getenv("HOME");
    if (!home) return NULL;
    static char path[512];
    snprintf(path, sizeof(path),
        "%s/.firestaff/data/theron/raw-us/"
        "Dungeon Master - Theron's Quest (USA) (Track 02).bin", home);
    FILE *fp = fopen(path, "rb");
    if (fp) { fclose(fp); return path; }
    return NULL;
}

static void test_codon_basic(void) {
    uint16_t codons[2];
    codons[0] = (0 << 10) | (1 << 5) | 2;
    codons[1] = (THERON_TEXT_END_MARKER << 10) | 0 | 0;

    Theron_TextBlock tb;
    assert(theron_v1_track02_text_decode(codons, 2, &tb) == 0);
    assert(tb.count == 1);
    assert(strcmp(tb.strings[0], "abc") == 0);
    printf("  Basic codon decode OK\n");
}

static void test_all_dungeons(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA", "DRATOR", "FORMICIA", "SARMON",
        "SHADODAN", "THIEVES", "DEMON"
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        assert(theron_v1_track02_dungeon_map_load(ud, ud_size, d, &dd));

        unsigned int total_tiles = 0;
        uint8_t flat_tiles[8192];
        unsigned int flat_pos = 0;
        for (unsigned int m = 0; m < dd.map_count; m++) {
            unsigned int w = dd.maps[m].header.x_dim + 1u;
            unsigned int h = dd.maps[m].header.y_dim + 1u;
            total_tiles += w * h;
            for (unsigned int x = 0; x < w; x++)
                for (unsigned int y = 0; y < h; y++)
                    flat_tiles[flat_pos++] = dd.maps[m].tiles[x][y];
        }

        unsigned int gref_count =
            theron_v1_track02_compute_ground_ref_count(flat_tiles, total_tiles);

        Theron_ThingData *td = calloc(1, sizeof(Theron_ThingData));
        assert(td);
        int ok = theron_v1_track02_thing_data_load(
            ud, ud_size, d, dd.object_counts, gref_count, td);
        assert(ok);

        Theron_TextBlock *tb = calloc(1, sizeof(Theron_TextBlock));
        assert(tb);
        assert(theron_v1_track02_text_decode(
            td->text_data, td->text_data_count, tb) == 0);

        printf("  %s: %u text strings decoded\n", names[d], tb->count);
        for (unsigned int s = 0; s < tb->count && s < 5; s++)
            printf("    [%u]: \"%s\"\n", s, tb->strings[s]);

        if (d == 0) {
            assert(tb->count > 0);
        }

        free(tb);
        free(td);
    }
}

int main(void) {
    printf("test_theron_v1_track02_text_decode\n");

    test_codon_basic();

    const char *path = find_track02();
    if (!path) {
        printf("  SKIP: Track 02 BIN not found\n");
        return 0;
    }

    size_t ud_size = 0;
    uint8_t *ud = load_track02_ud(path, &ud_size);
    if (!ud) {
        printf("  SKIP: could not load Track 02\n");
        return 0;
    }

    test_all_dungeons(ud, ud_size);
    free(ud);
    printf("PASS\n");
    return 0;
}
