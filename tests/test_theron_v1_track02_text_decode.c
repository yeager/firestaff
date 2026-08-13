#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_text_decode.h"
#include "theron_v1_world.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 2352
#define UD_PER_SECTOR 2048
#define SYNC_OFFSET 16

/* This target is also built with NDEBUG by some release configurations, so
 * do not use assert() for source-admission checks. */
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        return 0; \
    } \
} while (0)

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
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *home = getenv("HOME");
    static char path[512];
    const char *candidates[3] = { explicit_path, NULL, NULL };
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin", home);
        candidates[1] = path;
        snprintf(path + 256, sizeof(path) - 256,
                 "%s/.firestaff/data/theron/raw-us/"
                 "Dungeon Master - Theron's Quest (USA) (Track 02).bin", home);
        candidates[2] = path + 256;
    }
    for (unsigned int i = 0; i < 3u; ++i) {
        FILE *fp;
        if (!candidates[i] || !candidates[i][0]) continue;
        fp = fopen(candidates[i], "rb");
        if (fp) { fclose(fp); return candidates[i]; }
    }
    return NULL;
}

static int test_codon_basic(void) {
    uint16_t codons[2];
    codons[0] = (0 << 10) | (1 << 5) | 2;
    codons[1] = (THERON_TEXT_END_MARKER << 10) | 0 | 0;

    Theron_TextBlock tb;
    CHECK(theron_v1_track02_text_decode(codons, 2, &tb) == 0);
    CHECK(tb.count == 1);
    CHECK(strcmp(tb.strings[0], "abc") == 0);
    CHECK(tb.raw_glyph_count == 6);
    CHECK(tb.raw_glyphs[0] == 0 && tb.raw_glyphs[1] == 1 &&
          tb.raw_glyphs[2] == 2 && tb.raw_glyphs[3] == THERON_TEXT_END_MARKER);
    CHECK(tb.token_count == 6);
    CHECK(tb.tokens[0].word_index == 0 && tb.tokens[0].packed_slot == 0 &&
          tb.tokens[0].kind == THERON_TEXT_TOKEN_RAW);
    CHECK(tb.tokens[3].word_index == 1 && tb.tokens[3].packed_slot == 0 &&
          tb.tokens[3].kind == THERON_TEXT_TOKEN_END);
    printf("  Basic codon decode OK\n");
    return 1;
}

static int test_all_dungeons(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA", "DRATOR", "FORMICIA", "SARMON",
        "SHADODAN", "THIEVES", "DEMON"
    };
    static const unsigned int expected_word_counts[] = {
        0x013c, 0x00d0, 0x00e0, 0x00e8, 0x00e0, 0x00d9, 0x00e8
    };
    static const unsigned int expected_string_counts[] = {
        17, 11, 11, 15, 13, 13, 13
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        CHECK(theron_v1_track02_dungeon_map_load(ud, ud_size, d, &dd));

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
        CHECK(td != NULL);
        int ok = theron_v1_track02_thing_data_load(
            ud, ud_size, d, dd.object_counts, gref_count, td);
        CHECK(ok);
        CHECK(td->text_data_count == expected_word_counts[d]);

        Theron_TextBlock *tb = calloc(1, sizeof(Theron_TextBlock));
        CHECK(tb != NULL);
        CHECK(theron_v1_track02_text_decode(
            td->text_data, td->text_data_count, tb) == 0);

        printf("  %s: %u text strings decoded\n", names[d], tb->count);
        for (unsigned int s = 0; s < tb->count && s < 5; s++)
            printf("    [%u]: \"%s\"\n", s, tb->strings[s]);

        /* Every source block contains unresolved control glyphs.  This
         * verifies the real US stream and, critically, that none of its
         * candidate text is accidentally exposed as gameplay/UI text. */
        CHECK(tb->count == expected_string_counts[d]);
        CHECK(tb->diagnostic_only);
        CHECK(tb->unresolved_control_codes > 0);
        CHECK(tb->raw_glyph_count == td->text_data_count * 3u);
        CHECK(tb->token_count == tb->raw_glyph_count);
        CHECK(tb->tokens[0].word_index == 0 &&
              tb->tokens[0].packed_slot == 0);
        CHECK(tb->tokens[tb->token_count - 1u].word_index ==
              td->text_data_count - 1u);
        for (unsigned int ti = 0; ti < tb->token_count; ++ti) {
            CHECK(tb->tokens[ti].word_index == ti / 3u);
            CHECK(tb->tokens[ti].packed_slot == ti % 3u);
            CHECK(tb->tokens[ti].value == tb->raw_glyphs[ti]);
        }

        Theron_V1_World *world = calloc(1, sizeof(*world));
        CHECK(world != NULL);
        theron_v1_world_init(world);
        CHECK(theron_v1_world_load_dungeon_text(
            world, td->text_data, td->text_data_count) == 0);
        CHECK(world->dungeon_text_count == 0);
        CHECK(theron_v1_world_dungeon_text(world, 0) == NULL);
        CHECK(theron_v1_world_source_dungeon_text_count(world) ==
              td->text_data_count);
        CHECK(theron_v1_world_source_dungeon_text_token_count(world) ==
              tb->token_count);
        {
            Theron_TextToken token;
            CHECK(theron_v1_world_source_dungeon_text_token(
                world, 0u, &token));
            CHECK(token.value == tb->tokens[0].value &&
                  token.word_index == tb->tokens[0].word_index &&
                  token.packed_slot == tb->tokens[0].packed_slot &&
                  token.kind == tb->tokens[0].kind);
        }

        free(world);
        free(tb);
        free(td);
    }
    return 1;
}

int main(void) {
    printf("test_theron_v1_track02_text_decode\n");

    if (!test_codon_basic()) return 1;

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

    if (!test_all_dungeons(ud, ud_size)) {
        free(ud);
        return 1;
    }

    free(ud);
    printf("PASS\n");
    return 0;
}
