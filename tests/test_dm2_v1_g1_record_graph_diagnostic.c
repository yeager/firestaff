/*
 * test_dm2_v1_g1_record_graph_diagnostic.c
 *
 * Validates DM2 PC G1 DUNGEON.DAT record graph completion.
 * Loads real game data and verifies record_graph_complete == 1.
 *
 * Key finding: G1 byte-square format stores game data in w0,
 * NOT next-links. The validator checks ground-stack entries
 * resolve to valid records (including G1 extension pools).
 */

#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 256 * 1024) { fclose(f); return NULL; }
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

typedef struct {
    int count;
} FileHeaderWalkTrace;

static int count_file_header_record(void *user, uint16_t thing, int type,
                                    int index, const uint8_t *record,
                                    int record_size, int level, int x, int y)
{
    FileHeaderWalkTrace *trace = (FileHeaderWalkTrace *)user;
    (void)thing;
    (void)index;
    (void)record;
    (void)record_size;
    (void)level;
    (void)x;
    (void)y;
    if (!trace || type < 0 || type >= 16) return -1;
    ++trace->count;
    return 0;
}

int main(void) {
    const char *paths[] = {
        NULL,
        "/Users/bosse/.firestaff/data/dm2/data/dungeon.dat",
        NULL
    };
    const char *env = getenv("DM2_DUNGEON_DAT");
    uint8_t *dat = NULL;
    int size = 0;
    int failures = 0;
    DM2_V1_DungeonData d;
    DM2_V1_FileHeaderRuntimeMapReceipt map0;
    DM2_V1_G1RuntimeMapDoorReceipt doors;
    DM2_V1_G1RuntimeMapActuatorReceipt actuators;
    DM2_V1_FileHeaderRuntimeTeleporterReceipt teleporters;
    int text_record_total = 0;
    int creature_record_total = 0;
    FileHeaderWalkTrace map0_walk;

    paths[0] = env;

    for (int i = 0; paths[i]; ++i) {
        dat = load_file(paths[i], &size);
        if (dat) break;
    }
    if (!dat) {
        printf("SKIP: no DM2 DUNGEON.DAT available\n");
        return 0;
    }

    memset(&d, 0, sizeof(d));
    if (dm2_v1_dungeon_load(&d, dat, size) != 0) {
        printf("FAIL: dm2_v1_dungeon_load rejected data\n");
        free(dat);
        return 1;
    }

    printf("G1 record graph: levels=%d, records=%d, graph_complete=%d\n",
           d.level_count, d.square_first_thing_count,
           d.record_graph_complete);

    if (!d.record_graph_complete) {
        printf("FAIL: record_graph_complete is 0\n");
        ++failures;
    }
    if (d.partial_map_boot.incomplete) {
        printf("FAIL: partial_map_boot.incomplete is 1\n");
        ++failures;
    }
    if (d.square_bytes != 1) {
        printf("FAIL: expected G1 byte-square format (square_bytes=1)\n");
        ++failures;
    }
    if (dm2_v1_dungeon_get_next_thing(&d, 0x04A5) !=
        (int)DM2_THING_END_MARKER) {
        printf("FAIL: G1 get_next_thing should return END_MARKER\n");
        ++failures;
    }
    memset(&map0, 0, sizeof(map0));
    if (!dm2_v1_dungeon_validate_file_header_runtime_map(&d, 0, &map0) ||
        !map0.committed || map0.incomplete_world != 1 ||
        map0.root_count <= 0 || map0.record_count < map0.root_count ||
        map0.link_word_reads != map0.record_count) {
        printf("FAIL: File_header map-0 record owner was not retained\n");
        ++failures;
    }
    memset(&map0_walk, 0, sizeof(map0_walk));
    if (dm2_v1_dungeon_walk_file_header_runtime_map(
            &d, 0, count_file_header_record, &map0_walk) !=
            map0.record_count ||
        map0_walk.count != map0.record_count) {
        printf("FAIL: File_header map-0 complete record walk was not retained\n");
        ++failures;
    }
    for (int map = 0; map < d.level_count; ++map) {
        DM2_V1_FileHeaderRuntimeMapReceipt map_receipt;
        FileHeaderWalkTrace map_walk;
        DM2_V1_FileHeaderRuntimeTextReceipt texts;
        DM2_V1_FileHeaderRuntimeCreatureReceipt creatures;

        memset(&map_receipt, 0, sizeof(map_receipt));
        memset(&map_walk, 0, sizeof(map_walk));
        if (!dm2_v1_dungeon_validate_file_header_runtime_map(
                &d, map, &map_receipt) || !map_receipt.committed ||
            dm2_v1_dungeon_walk_file_header_runtime_map(
                &d, map, count_file_header_record, &map_walk) !=
                map_receipt.record_count ||
            map_walk.count != map_receipt.record_count) {
            printf("FAIL: File_header map-%d complete record walk was not retained\n",
                   map);
            ++failures;
        }
        memset(&texts, 0, sizeof(texts));
        if (!dm2_v1_dungeon_materialize_file_header_runtime_map_texts(
                &d, map, &texts) || !texts.committed ||
            texts.text_record_reads != texts.text_record_count) {
            printf("FAIL: File_header map-%d text records were not retained\n",
                   map);
            ++failures;
        } else {
            text_record_total += texts.text_record_count;
        }
        memset(&creatures, 0, sizeof(creatures));
        if (!dm2_v1_dungeon_materialize_file_header_runtime_map_creatures(
                &d, map, &creatures) || !creatures.committed ||
            creatures.creature_record_reads != creatures.creature_record_count) {
            printf("FAIL: File_header map-%d creature records were not retained\n",
                   map);
            ++failures;
        } else {
            creature_record_total += creatures.creature_record_count;
            for (int creature_index = 0;
                 creature_index < creatures.creature_record_count;
                 ++creature_index) {
                DM2_V1_FileHeaderCreaturePossessionReceipt possessions;
                memset(&possessions, 0, sizeof(possessions));
                if (!dm2_v1_dungeon_collect_file_header_creature_possession_chain(
                        &d, &creatures, creature_index, &possessions) ||
                    !possessions.committed ||
                    possessions.creature_object_id !=
                        creatures.creatures[creature_index].object_id ||
                    possessions.link_word_reads != possessions.node_count) {
                    printf("FAIL: File_header map-%d creature possessions were not retained\n",
                           map);
                    ++failures;
                    break;
                }
            }
        }
    }
    if (text_record_total <= 0) {
        printf("FAIL: canonical File_header contains no retained DB2 texts\n");
        ++failures;
    }
    if (creature_record_total <= 0) {
        printf("FAIL: canonical File_header contains no retained DB4 creatures\n");
        ++failures;
    }
    memset(&doors, 0, sizeof(doors));
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_doors(
            &d, 0, &doors) || !doors.committed ||
        doors.door_record_reads != doors.door_root_count) {
        printf("FAIL: File_header map-0 door roots were not retained\n");
        ++failures;
    }
    memset(&actuators, 0, sizeof(actuators));
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_actuators(
            &d, 0, &actuators) || !actuators.committed ||
        actuators.actuator_record_reads != actuators.actuator_root_count) {
        printf("FAIL: File_header map-0 actuator roots were not retained\n");
        ++failures;
    }
    memset(&teleporters, 0, sizeof(teleporters));
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_teleporters(
            &d, 0, &teleporters) || !teleporters.committed ||
        teleporters.teleporter_record_reads != teleporters.teleporter_root_count) {
        printf("FAIL: File_header map-0 teleporter roots were not retained\n");
        ++failures;
    }

    dm2_v1_dungeon_free(&d);
    free(dat);

    printf("%s: DM2 V1 G1 record graph diagnostic\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
