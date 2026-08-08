/*
 * test_dm2_v1_g1_record_graph_diagnostic.c
 *
 * Validates DM2 PC G1 DUNGEON.DAT record graph completion.
 * Loads real game data and verifies record_graph_complete == 1.
 *
 * The canonical File_header route follows record w0 links from every marked
 * square. The validator checks that each complete chain resolves to declared
 * original record pools.
 */

#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"

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

/* SKProject SKWIN/DME.h::Door, Teleporter, Text, Actuator and Creature.
 * The receipts are deliberately read-only, but must still retain the exact
 * payload fields of every original File_header-chain record.  Do not use
 * fixture values here: this diagnostic is meaningful only against the
 * mounted retail DUNGEON.DAT. */
static int receipt_record(const DM2_V1_DungeonData *d, uint16_t object_id,
                          int expected_type, const uint8_t **out_record)
{
    int type = -1;
    int index = -1;
    const uint8_t *record = dm2_v1_dungeon_get_thing_record(
        d, object_id, &type, &index, NULL);
    if (out_record) *out_record = record;
    return record && type == expected_type && index == (object_id & 0x03ffu);
}

/* Exercise the one source-complete sensor atom only with an actuator and
 * target door that both come from the mounted retail File_header image.
 * A PushButtonSwitch uses GET_ADDRESS_OF_TILE_RECORD on its target, so the
 * DB0 door must be the first record on that particular map square. */
static int verify_real_push_button_switch(DM2_V1_DungeonData *d)
{
    DM2_V1_RecordPoolSet pools;
    DM2_V1_ActuatorEventReceipt receipt;
    const uint8_t *actuator = NULL;
    int candidate_map = -1;
    int candidate_action = -1;
    int16_t target_link = DM2_V1_RECORD_HANDLE_NULL;

    if (!d || !dm2_v1_record_pool_set_init_from_dungeon(&pools, d)) {
        return 0;
    }
    for (int prefer_nonzero = 1; prefer_nonzero >= 0 && !actuator;
         --prefer_nonzero) {
        for (int map = 0; map < d->level_count && !actuator; ++map) {
            if ((prefer_nonzero != 0) != (map != 0)) continue;
            for (int x = 0; x < d->level_widths[map] && !actuator; ++x) {
                for (int y = 0; y < d->level_heights[map] && !actuator; ++y) {
                    int link = dm2_v1_dungeon_get_first_thing(d, map, x, y);
                    int budget = 0;
                    while (link >= 0 && link != (int)DM2_THING_END_MARKER &&
                           budget++ < 4096) {
                        int type = -1;
                        const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                            d, (uint16_t)link, &type, NULL, NULL);
                        if (!record) break;
                        if (type == 3 && (record[2] & 0x7fu) ==
                                             DM2_ACTU_PUSH_BUTTON_SWITCH) {
                            const int tx = (int)dm2_actu_xcoord(record);
                            const int ty = (int)dm2_actu_ycoord(record);
                            const int root = dm2_v1_dungeon_get_first_thing(
                                d, map, tx, ty);
                            int root_type = -1;
                            if (root >= 0 &&
                                dm2_v1_dungeon_get_thing_record(
                                    d, (uint16_t)root, &root_type, NULL,
                                    NULL) != NULL && root_type == 0) {
                                actuator = record;
                                candidate_map = map;
                                candidate_action = 2;
                                target_link = (int16_t)root;
                                break;
                            }
                        }
                        link = dm2_v1_dungeon_get_next_thing(d,
                                                              (uint16_t)link);
                    }
                }
            }
        }
    }
    if (!actuator) {
        /* The corpus may simply not contain this optional actuator type. */
        dm2_v1_record_pool_set_free(&pools);
        printf("INFO: retail corpus has no direct PUSH_BUTTON_SWITCH target\n");
        return 1;
    }
    {
        uint8_t *door = dm2_v1_record_pool_address_mut(&pools, target_link);
        uint16_t before;
        uint16_t after;
        memset(&receipt, 0, sizeof(receipt));
        if (!door) {
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
        before = (uint16_t)door[2] | ((uint16_t)door[3] << 8);
        if (!dm2_v1_push_button_switch(&pools, d, actuator, candidate_map,
                                       candidate_action, &receipt)) {
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
        after = (uint16_t)door[2] | ((uint16_t)door[3] << 8);
        if (((before ^ after) != 0x2000u) ||
            receipt.door_bit13_toggled != 1) {
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
    }
    printf("PASS: real PUSH_BUTTON_SWITCH uses source map %d and direct DB0\n",
           candidate_map);
    dm2_v1_record_pool_set_free(&pools);
    return 1;
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
    int door_record_total = 0;
    int teleporter_record_total = 0;
    int actuator_record_total = 0;
    int object_record_total = 0;
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

    if (!verify_real_push_button_switch(&d)) {
        printf("FAIL: real PUSH_BUTTON_SWITCH direct-door atom was rejected\n");
        ++failures;
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
        DM2_V1_FileHeaderRuntimeSceneCensus scene_census;
        DM2_V1_FileHeaderRuntimeTileCensus tile_census;
        DM2_V1_FileHeaderRuntimeObjectReceipt objects;
        FileHeaderWalkTrace map_walk;
        DM2_V1_FileHeaderRuntimeTextReceipt texts;
        DM2_V1_FileHeaderRuntimeCreatureReceipt creatures;
        DM2_V1_G1RuntimeMapDoorReceipt chain_doors;
        DM2_V1_FileHeaderRuntimeTeleporterReceipt chain_teleporters;
        DM2_V1_G1RuntimeMapActuatorReceipt chain_actuators;

        memset(&map_receipt, 0, sizeof(map_receipt));
        memset(&scene_census, 0, sizeof(scene_census));
        memset(&tile_census, 0, sizeof(tile_census));
        memset(&objects, 0, sizeof(objects));
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
        if (!dm2_v1_dungeon_collect_file_header_runtime_map_scene_census(
                &d, map, &scene_census) || !scene_census.committed ||
            scene_census.record_count != map_receipt.record_count) {
            printf("FAIL: File_header map-%d scene census was not retained\n", map);
            ++failures;
        }
        if (!dm2_v1_dungeon_collect_file_header_runtime_map_objects(
                &d, map, &objects) || !objects.committed ||
            objects.object_record_reads != objects.object_record_count) {
            printf("FAIL: File_header map-%d object records were not retained\n", map);
            ++failures;
        } else {
            object_record_total += objects.object_record_count;
            for (int object_index = 0;
                 object_index < objects.object_record_count; ++object_index) {
                const DM2_V1_FileHeaderObjectRecord *object =
                    &objects.objects[object_index];
                const uint8_t *record = NULL;
                if (!receipt_record(&d, object->object_id, object->type, &record) ||
                    object->type < 5 || object->index !=
                        (object->object_id & 0x03ffu) ||
                    object->direction != (uint8_t)(object->object_id >> 14) ||
                    object->record_offset != (int)(record - d.raw_data) ||
                    object->record_size <= 0 ||
                    object->record_offset + object->record_size > d.raw_size) {
                    printf("FAIL: File_header map-%d object address differs\n", map);
                    ++failures;
                    break;
                }
            }
        }
        if (!dm2_v1_dungeon_collect_file_header_runtime_map_tile_census(
                &d, map, &tile_census) || !tile_census.committed ||
            tile_census.tile_count != map_receipt.width * map_receipt.height) {
            printf("FAIL: File_header map-%d tile census was not retained\n", map);
            ++failures;
        }
        memset(&texts, 0, sizeof(texts));
        memset(&chain_doors, 0, sizeof(chain_doors));
        if (!dm2_v1_dungeon_collect_file_header_runtime_map_doors(
                &d, map, &chain_doors) || !chain_doors.committed ||
            chain_doors.door_record_reads != chain_doors.door_root_count) {
            printf("FAIL: File_header map-%d chained door records were not retained\n",
                   map);
            ++failures;
        } else {
            door_record_total += chain_doors.door_record_reads;
            for (int door_index = 0;
                 door_index < chain_doors.door_root_count; ++door_index) {
                const DM2_V1_G1DirectDoorRoot *door =
                    &chain_doors.doors[door_index];
                const uint8_t *record = NULL;
                uint16_t w2;
                if (!receipt_record(&d, door->object_id, 0, &record)) {
                    printf("FAIL: File_header map-%d DB0 source was lost\n", map);
                    ++failures;
                    break;
                }
                w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                if (door->index != (door->object_id & 0x03ffu) ||
                    door->direction != (uint8_t)(door->object_id >> 14) ||
                    door->attributes != w2 ||
                    door->button != (uint8_t)((w2 >> 6) & 1u) ||
                    door->door_type != (uint8_t)(w2 & 1u) ||
                    door->button_state != (uint8_t)((w2 >> 11) & 1u) ||
                    door->opening_dir != (uint8_t)((w2 >> 5) & 1u) ||
                    door->ornate_index != (uint8_t)((w2 >> 1) & 0x0fu) ||
                    door->destroyable_by_fireball != (uint8_t)((w2 >> 7) & 1u) ||
                    door->bashable_by_chopping != (uint8_t)((w2 >> 8) & 1u)) {
                    printf("FAIL: File_header map-%d DB0 fields disagree with source\n", map);
                    ++failures;
                    break;
                }
            }
        }
        memset(&chain_teleporters, 0, sizeof(chain_teleporters));
        if (!dm2_v1_dungeon_collect_file_header_runtime_map_teleporters(
                &d, map, &chain_teleporters) || !chain_teleporters.committed ||
            chain_teleporters.teleporter_record_reads !=
                chain_teleporters.teleporter_root_count) {
            printf("FAIL: File_header map-%d chained teleporter records were not retained\n",
                   map);
            ++failures;
        } else {
            teleporter_record_total += chain_teleporters.teleporter_record_reads;
            for (int teleporter_index = 0;
                 teleporter_index < chain_teleporters.teleporter_root_count;
                 ++teleporter_index) {
                const DM2_V1_G1DirectTeleporterRoot *teleporter =
                    &chain_teleporters.teleporters[teleporter_index];
                const uint8_t *record = NULL;
                uint16_t w2;
                uint16_t w4;
                if (!receipt_record(&d, teleporter->object_id, 1, &record)) {
                    printf("FAIL: File_header map-%d DB1 source was lost\n", map);
                    ++failures;
                    break;
                }
                w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                w4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
                if (teleporter->index != (teleporter->object_id & 0x03ffu) ||
                    teleporter->direction != (uint8_t)(teleporter->object_id >> 14) ||
                    teleporter->destination_word != w4 ||
                    teleporter->destination_x != (uint8_t)(w2 & 0x001fu) ||
                    teleporter->destination_y != (uint8_t)((w2 >> 5) & 0x001fu) ||
                    teleporter->destination_map != (uint8_t)(w4 >> 8) ||
                    teleporter->scope != (uint8_t)((w2 >> 13) & 3u) ||
                    teleporter->sound != (uint8_t)((w2 >> 15) & 1u) ||
                    teleporter->rotation != (uint8_t)((w2 >> 10) & 3u) ||
                    teleporter->rotation_type != (uint8_t)((w2 >> 12) & 1u)) {
                    printf("FAIL: File_header map-%d DB1 fields disagree with source\n", map);
                    ++failures;
                    break;
                }
            }
        }
        memset(&chain_actuators, 0, sizeof(chain_actuators));
        if (!dm2_v1_dungeon_collect_file_header_runtime_map_actuators(
                &d, map, &chain_actuators) || !chain_actuators.committed ||
            chain_actuators.actuator_record_reads !=
                chain_actuators.actuator_root_count) {
            printf("FAIL: File_header map-%d chained actuator records were not retained\n",
                   map);
            ++failures;
        } else {
            actuator_record_total += chain_actuators.actuator_record_reads;
            for (int actuator_index = 0;
                 actuator_index < chain_actuators.actuator_root_count;
                 ++actuator_index) {
                const DM2_V1_G1DirectActuatorRoot *actuator =
                    &chain_actuators.actuators[actuator_index];
                const uint8_t *record = NULL;
                uint16_t w2;
                uint16_t w4;
                uint16_t w6;
                if (!receipt_record(&d, actuator->object_id, 3, &record)) {
                    printf("FAIL: File_header map-%d DB3 source was lost\n", map);
                    ++failures;
                    break;
                }
                w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                w4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
                w6 = (uint16_t)record[6] | ((uint16_t)record[7] << 8);
                if (actuator->index != (actuator->object_id & 0x03ffu) ||
                    actuator->direction != (uint8_t)(actuator->object_id >> 14) ||
                    actuator->attributes != w2 ||
                    actuator->control_word != w4 ||
                    actuator->target_word != w6 ||
                    actuator->actuator_type != (uint8_t)(w2 & 0x007fu) ||
                    actuator->actuator_data != (uint16_t)((w2 >> 7) & 0x01ffu) ||
                    actuator->graphic_number != (uint8_t)((w4 >> 12) & 0x0fu) ||
                    actuator->disabled != (uint8_t)((w4 >> 11) & 1u) ||
                    actuator->delay != (uint8_t)((w4 >> 7) & 0x0fu) ||
                    actuator->sound_effect != (uint8_t)((w4 >> 6) & 1u) ||
                    actuator->revert_effect != (uint8_t)((w4 >> 5) & 1u) ||
                    actuator->action_type != (uint8_t)((w4 >> 3) & 3u) ||
                    actuator->once_only != (uint8_t)((w4 >> 2) & 1u) ||
                    actuator->active_status != (uint8_t)(w4 & 1u) ||
                    actuator->target_direction != (uint8_t)((w6 >> 4) & 3u) ||
                    actuator->target_x != (uint8_t)((w6 >> 6) & 0x001fu) ||
                    actuator->target_y != (uint8_t)((w6 >> 11) & 0x001fu)) {
                    printf("FAIL: File_header map-%d DB3 fields disagree with source\n", map);
                    ++failures;
                    break;
                }
            }
        }
        if (!dm2_v1_dungeon_materialize_file_header_runtime_map_texts(
                &d, map, &texts) || !texts.committed ||
            texts.text_record_reads != texts.text_record_count) {
            printf("FAIL: File_header map-%d text records were not retained\n",
                   map);
            ++failures;
        } else {
            text_record_total += texts.text_record_count;
            for (int text_index = 0; text_index < texts.text_record_count;
                 ++text_index) {
                const DM2_V1_FileHeaderTextRecord *text = &texts.texts[text_index];
                const uint8_t *record = NULL;
                uint16_t w2;
                if (!receipt_record(&d, text->object_id, 2, &record)) {
                    printf("FAIL: File_header map-%d DB2 source was lost\n", map);
                    ++failures;
                    break;
                }
                w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                if (text->index != (text->object_id & 0x03ffu) ||
                    text->direction != (uint8_t)(text->object_id >> 14) ||
                    text->visible != (uint8_t)(w2 & 1u) ||
                    text->mode != (uint8_t)((w2 >> 1) & 3u) ||
                    text->text_index != (uint16_t)((w2 >> 3) & 0x1fffu) ||
                    text->simple_extension_usage !=
                        (uint8_t)(((w2 >> 3) >> 8) & 0x1fu)) {
                    printf("FAIL: File_header map-%d DB2 fields disagree with source\n", map);
                    ++failures;
                    break;
                }
            }
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
                const DM2_V1_FileHeaderCreatureRecord *creature =
                    &creatures.creatures[creature_index];
                const uint8_t *record = NULL;
                memset(&possessions, 0, sizeof(possessions));
                if (!receipt_record(&d, creature->object_id, 4, &record) ||
                    creature->index != (creature->object_id & 0x03ffu) ||
                    creature->direction != (uint8_t)(record[15] & 3u) ||
                    creature->possession_object_id !=
                        ((uint16_t)record[2] | ((uint16_t)record[3] << 8)) ||
                    creature->creature_type != record[4] ||
                    creature->info_slot != record[5] ||
                    creature->hit_points_1 !=
                        ((uint16_t)record[6] | ((uint16_t)record[7] << 8)) ||
                    creature->hit_points_2 !=
                        ((uint16_t)record[8] | ((uint16_t)record[9] << 8)) ||
                    creature->hit_points_3 !=
                        ((uint16_t)record[10] | ((uint16_t)record[11] << 8)) ||
                    creature->hit_points_4 !=
                        ((uint16_t)record[12] | ((uint16_t)record[13] << 8)) ||
                    creature->state_byte_14 != record[14] ||
                    creature->state_byte_15 != record[15]) {
                    printf("FAIL: File_header map-%d DB4 fields disagree with source\n",
                           map);
                    ++failures;
                    break;
                }
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
    if (door_record_total <= 0 || teleporter_record_total <= 0 ||
        actuator_record_total <= 0 || object_record_total <= 0) {
        printf("FAIL: canonical File_header lacks chained DB0/DB1/DB3/object records\n");
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
    } else {
        for (int i = 0; i < teleporters.teleporter_root_count; ++i) {
            const DM2_V1_G1DirectTeleporterRoot *teleporter =
                &teleporters.teleporters[i];
            const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                &d, teleporter->object_id, NULL, NULL, NULL);
            uint16_t w2;
            uint16_t w4;

            if (!record) {
                printf("FAIL: File_header map-0 teleporter source was lost\n");
                ++failures;
                break;
            }
            w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
            w4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
            if (teleporter->destination_x != (uint8_t)(w2 & 0x001fu) ||
                teleporter->destination_y != (uint8_t)((w2 >> 5) & 0x001fu) ||
                teleporter->destination_map != (uint8_t)(w4 >> 8) ||
                teleporter->scope != (uint8_t)((w2 >> 13) & 3u) ||
                teleporter->sound != (uint8_t)((w2 >> 15) & 1u) ||
                teleporter->rotation != (uint8_t)((w2 >> 10) & 3u) ||
                teleporter->rotation_type != (uint8_t)((w2 >> 12) & 1u)) {
                printf("FAIL: File_header map-0 teleporter bitfields disagree with source\n");
                ++failures;
                break;
            }
        }
    }

    dm2_v1_dungeon_free(&d);
    free(dat);

    printf("%s: DM2 V1 G1 record graph diagnostic\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
