/*
 * test_dm2_v1_dungeon_loader_fmtowns.c
 *
 * Validates DM2 dungeon loader against real FM Towns DUNGEON.DAT.
 * Source: ~/.firestaff/data/dm2/fmtowns_iso/DATA/DUNGEON.DAT
 *
 * The FM Towns DUNGEON.DAT uses magic 0x3094 at offset 2 instead of
 * PC's 0x3147 ('G1'), but has the same header layout, map definition
 * format, and byte-square tile data.
 */

#include "dm2_v1_dungeon_loader.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    data = malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)sz, f) != (size_t)sz) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return data;
}

static void test_fmtowns_load(const char *path) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read FM Towns DUNGEON.DAT at %s\n", path);
        return;
    }

    printf("  FM Towns DUNGEON.DAT: %zu bytes\n", data_size);
    assert(data_size == 37954);

    /* Magic should be 0x3094 */
    assert(data[2] == 0x94 && data[3] == 0x30);

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    printf("  PASS: FM Towns DUNGEON.DAT loaded successfully\n");
    {
        int mirrors = 0;
        DM2_V1_G1ChampionMirrorReceipt receipt;
        for (int i = 0; i < dungeon.thing_type_counts[3]; ++i) {
            int type = -1;
            int size = 0;
            const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                &dungeon, (uint16_t)((3u << 10) | (unsigned)i),
                &type, NULL, &size);
            if (record && type == 3 && size >= 8 &&
                ((record[2] | ((uint16_t)record[3] << 8)) & 0x7fu) == 0x7eu) {
                ++mirrors;
            }
        }
        assert(mirrors == 16);
        assert(dm2_v1_dungeon_collect_g1_champion_mirrors(&dungeon, &receipt));
        assert(receipt.committed && receipt.mirror_count == 16);
        printf("  PASS: 16 FM Towns champion mirror records\n");
    }

    /* File_header.nMaps is the byte at offset 4 (SKWIN/DME.h:93-101,
     * mirrored by docs/dm2_save_format.md's section order). Both the PC and
     * FM Towns releases store 0x2c = 44 there. The old expectation of 28 was
     * reading offset 6, which is cwTextData -- verified against the real
     * files: map descriptors 0..43 are all well-formed with monotonically
     * increasing map-data offsets (0x0000..0x2ff0 inside the 12615-byte
     * cbMapData region), and the 45th 16-byte slot is already the column
     * index table. */
    assert(dungeon.level_count == 44);
    printf("  PASS: 44 maps\n");

    /* Byte-sized squares */
    assert(dungeon.square_bytes == 1);
    printf("  PASS: byte squares\n");

    /* File_header.nRecords[16] starts at offset 12 (SKWIN/DME.h:101), so
     * thing_type_counts[0] is the first DB pool. The old expectations began
     * at 209, which is nRecords[1]: they were written against the shifted
     * header offsets in dm2_v1_try_load_pc_g1_byte_layout, which reads the
     * pool table from offset 14. The real FM Towns header holds
     * 53, 209, 448, 1020, 280, 169 at offsets 12, 14, 16, 18, 20, 22. */
    assert(dungeon.thing_type_counts[0] == 53);
    assert(dungeon.thing_type_counts[1] == 209);
    assert(dungeon.thing_type_counts[2] == 448);
    assert(dungeon.thing_type_counts[3] == 1020);
    assert(dungeon.thing_type_counts[4] == 280);
    assert(dungeon.thing_type_counts[5] == 169);
    printf("  PASS: thing type counts match header\n");

    /* Verify some map data was parsed — raw_map_data_base should be set */
    assert(dungeon.raw_map_data_base >= 0);
    printf("  PASS: raw map data base set (%d)\n", dungeon.raw_map_data_base);

    /* Column index should be set */
    assert(dungeon.column_index_base >= 0);
    printf("  PASS: column index base set (%d)\n", dungeon.column_index_base);

    {
        const char *home = getenv("HOME");
        char graphics_path[1024];
        size_t graphics_size = 0u;
        uint8_t *graphics;
        DM2_V1_AssetLoader loader;
        DM2_V1_CLightStoneRoomReceipt room;
        assert(home);
        assert(snprintf(graphics_path, sizeof(graphics_path),
                        "%s/.firestaff/data/dm2/fmtowns_iso/DATA/GRAPHICS.DAT",
                        home) < (int)sizeof(graphics_path));
        graphics = read_file(graphics_path, &graphics_size);
        assert(graphics);
        memset(&loader, 0, sizeof(loader));
        assert(dm2_v1_asset_loader_init(&loader, graphics,
                                        graphics_size) == 0);
        /* Original map 38 DB1 destination (6,6) is a class-2, no-record
         * tile. SKProject SUMMARIZE_STONE_ROOM promotes its effective tile
         * type to one and resolves the GRAPHICSSET/0 animated ceiling
         * ornament from GDAT 0x6b through FLOOR_GFX/0x28 frame data. */
        assert(dm2_v1_dungeon_c_light_stone_room_receipt(
            &dungeon, &loader, 38, 6, 6, 0u, &room));
        assert(room.valid && room.raw_tile == 0x40u &&
               room.source_tile_type == 1u &&
               room.first_record_link == DM2_THING_NULL_MARKER &&
               room.ceiling_ornament_index == 0x28u &&
               room.ceiling_animation_frame == 1u &&
               room.ceiling_ornament_word == 0x0a28u &&
               room.ornament_source_hash != 0u);
        {
            DM2_V1_CLightTileOrnamentReceipt light;
            uint16_t original_word = 0;
            int16_t base;
            assert(dm2_v1_query_gdat_entry_data_index(
                &loader, 10, 0x28, 11, 0xf8, &original_word));
            assert(dm2_v1_dungeon_c_light_tile_ornament_receipt(
                &room, &loader, 0, 1u, &light));
            assert(light.valid && light.gdat_light_word == original_word &&
                   light.source_ornament_index == 0x28u);
            printf("  FM Towns map 38 ceiling light: GDAT=%04x delta=%d\n",
                   original_word, light.v1e0974_delta);
            base = (original_word != 0u &&
                    ((original_word & 0x8000u) == 0u ||
                     room.ceiling_ornament_word >> 8))
                ? (int16_t)(original_word & 0x7fffu) : 0;
            assert(light.v1e0974_delta == base &&
                   light.v1e0978_delta == 0);
            assert(dm2_v1_dungeon_c_light_tile_ornament_receipt(
                &room, &loader, 5, 1u, &light));
            assert(light.v1e0974_delta ==
                   (base ? (base - 90 > 2 ? base - 90 : 2) : 0));
            assert(dm2_v1_dungeon_c_light_tile_ornament_receipt(
                &room, &loader, 0, 0u, &light));
            assert(light.v1e0974_delta == 0);
        }
        assert(dm2_v1_dungeon_c_light_stone_room_receipt(
            &dungeon, &loader, 38, 6, 6, 1u, &room));
        assert(room.ceiling_animation_frame == 2u &&
               room.ceiling_ornament_word == 0x1428u);
        assert(dm2_v1_dungeon_c_light_stone_room_receipt(
            &dungeon, &loader, 38, 5, 6, 0u, &room));
        assert(room.raw_tile == 0x48u && room.source_tile_type == 2u &&
               room.first_record_link == DM2_THING_NULL_MARKER &&
               room.ceiling_ornament_word == 0x00ffu &&
               room.ceiling_ornament_index == 0xffu &&
               room.ornament_source_hash == 0u);
        /* The immediate northern cell is an original DB1 teleporter. Its
         * first record is skipped by SUMMARIZE_STONE_ROOM's DB0..DB3 loop,
         * leaving the same GRAPHICSSET ceiling ornament as the destination. */
        assert(dm2_v1_dungeon_c_light_stone_room_receipt(
            &dungeon, &loader, 38, 6, 5, 0u, &room));
        assert(room.valid && room.raw_tile == 0xb0u &&
               room.source_tile_type == 1u &&
               room.first_record_link == 0x0442u &&
               room.ceiling_ornament_index == 0x28u);
        {
            int type = -1;
            int first;
            const uint8_t *db1 = dm2_v1_dungeon_get_thing_record(
                &dungeon, room.first_record_link, &type, NULL, NULL);
            const uint8_t *sensor = dm2_v1_dungeon_get_thing_record(
                &dungeon, 0x0c8cu, NULL, NULL, NULL);
            assert(db1 && type == 1 && sensor &&
                   dm2_v1_dungeon_read_record_u16(&dungeon, db1) ==
                       0x0c8cu &&
                   dm2_v1_dungeon_read_record_u16(&dungeon, db1 + 4) ==
                       0x0306u &&
                   (dm2_v1_dungeon_read_record_u16(&dungeon, sensor + 2)
                       & 0x7fu) == 0x27u &&
                   dm2_v1_dungeon_read_record_u16(&dungeon, sensor) ==
                       0xfffeu);
            first = dm2_v1_dungeon_get_first_thing(&dungeon, 3, 13, 10);
            db1 = dm2_v1_dungeon_get_thing_record(
                &dungeon, (uint16_t)first, &type, NULL, NULL);
            assert(db1 && type == 1 &&
                   dm2_v1_dungeon_read_record_u16(&dungeon, db1) ==
                       0x0e46u);
            sensor = dm2_v1_dungeon_get_thing_record(
                &dungeon, 0x0e46u, &type, NULL, NULL);
            assert(sensor && type == 3 &&
                   (dm2_v1_dungeon_read_record_u16(&dungeon, sensor + 2)
                       & 0x7fu) == 0x27u &&
                   dm2_v1_dungeon_read_record_u16(&dungeon, sensor) ==
                       0x10a1u);
            {
                const uint8_t *creature =
                    dm2_v1_dungeon_get_thing_record(
                        &dungeon, 0x10a1u, &type, NULL, NULL);
                assert(creature && type == 4 && creature[4] == 0x46u &&
                       dm2_v1_dungeon_read_record_u16(
                           &dungeon, creature) == 0xfffeu);
            }
        }
        {
            int first = dm2_v1_dungeon_get_first_thing(&dungeon, 3, 13, 11);
            int type = -1;
            const uint8_t *db1 = first >= 0 ?
                dm2_v1_dungeon_get_thing_record(
                    &dungeon, (uint16_t)first, &type, NULL, NULL) : NULL;
            uint16_t w2, w4;
            assert(db1 && type == 1);
            w2 = dm2_v1_dungeon_read_record_u16(&dungeon, db1 + 2);
            w4 = dm2_v1_dungeon_read_record_u16(&dungeon, db1 + 4);
            assert((w2 & 0x1fu) == 6u && ((w2 >> 5) & 0x1fu) == 6u &&
                   ((w4 >> 8) & 0xffu) == 38u &&
                   dm2_v1_dungeon_get_tile_raw(&dungeon, 3, 13, 11) ==
                       0xb8 &&
                   dm2_v1_dungeon_read_record_u16(&dungeon, db1) ==
                       0xfffeu);
            first = dm2_v1_dungeon_get_first_thing(&dungeon, 38, 6, 4);
            db1 = first >= 0 ? dm2_v1_dungeon_get_thing_record(
                &dungeon, (uint16_t)first, &type, NULL, NULL) : NULL;
            assert(db1 && type == 1);
            w2 = dm2_v1_dungeon_read_record_u16(&dungeon, db1 + 2);
            w4 = dm2_v1_dungeon_read_record_u16(&dungeon, db1 + 4);
            assert((w2 & 0x1fu) == 13u && ((w2 >> 5) & 0x1fu) == 9u &&
                   ((w4 >> 8) & 0xffu) == 3u &&
                   dm2_v1_dungeon_get_tile_raw(&dungeon, 38, 6, 4) ==
                       0xb8 &&
                   dm2_v1_dungeon_read_record_u16(&dungeon, db1) ==
                       0xfffeu);
        }
        {
            DM2_V1_CLightTileOrnamentReceipt normal;
            DM2_V1_CLightTileOrnamentReceipt via_teleporter;
            DM2_V1_CLightTileOrnamentReceipt dark_weather;
            DM2_V1_CLightTileOrnamentReceipt mode7;
            assert(dm2_v1_dungeon_c_light_teleporter_ornament_receipt(
                &room, &loader, 0, 1u, 0, 0, &normal));
            assert(dm2_v1_dungeon_c_light_teleporter_ornament_receipt(
                &room, &loader, 0, 1u, 1, 0, &via_teleporter));
            assert(dm2_v1_dungeon_c_light_teleporter_ornament_receipt(
                &room, &loader, 0, 1u, 1, 5, &dark_weather));
            assert(dm2_v1_dungeon_c_light_teleporter_ornament_receipt(
                &room, &loader, 0, 4u, 1, 0, &mode7));
            assert(normal.valid && via_teleporter.valid &&
                   dark_weather.valid && mode7.valid &&
                   mode7.v1e0974_delta == 0 &&
                   mode7.v1e0978_delta == 0 &&
                   normal.gdat_light_word == via_teleporter.gdat_light_word &&
                   via_teleporter.v1e0974_delta ==
                     (int16_t)((normal.v1e0974_delta * 99) / 100) &&
                   dark_weather.v1e0974_delta == 0);
        }
        {
            DM2_V1_CLightFlags4FloorReceipt floor;
            {
                uint8_t floor_list[16];
                uint8_t door[16];
                /* SKProject GET_FLOOR_DECORATION indexes beyond the map's
                 * FloorGraphics count without a bounds check. On this
                 * original file map 3's third floor byte is the last byte
                 * before map 4. Ordinals 4/5/6 would read map 4 tile data,
                 * not another declared map-3 graphics entry. */
                assert(dm2_v1_dungeon_get_map_floor_gfx_list(
                    &dungeon, 3, floor_list, 16) == 3);
                assert(floor_list[0] == 0x1cu &&
                       floor_list[1] == 0x22u &&
                       floor_list[2] == 0x23u);
                assert(dm2_v1_dungeon_get_map_door_ornate_list(
                    &dungeon, 3, door, 16) == 0);
                assert(dungeon.level_offsets[4] - dungeon.level_offsets[3]
                       == 25 * 19 + 9 + 6 + 3);
                assert(dungeon.raw_map_data_base + dungeon.level_offsets[4]
                       == 26868);
            }
            int no_record_count = 0;
            int record_count = 0;
            int admitted_record_count = 0;
            int ornament_record_count = 0;
            for (int level = 3; level <= 38; level += 35) {
                for (int y = 0; y < dungeon.level_heights[level]; ++y) {
                    for (int x = 0; x < dungeon.level_widths[level]; ++x) {
                        int raw = dm2_v1_dungeon_get_tile_raw(
                            &dungeon, level, x, y);
                        int first = dm2_v1_dungeon_get_first_thing(
                            &dungeon, level, x, y);
                        if (raw < 0 || ((unsigned)raw >> 5) != 0u) continue;
                        if (first == -1) {
                            assert(dm2_v1_dungeon_c_light_flags4_no_record_floor_receipt(
                                &dungeon, level, x, y, &floor));
                            assert(floor.valid && floor.source_flags == 4u &&
                                   floor.floor_ornament_word == 0x00ffu &&
                                   floor.contributes_light == 0u);
                            ++no_record_count;
                        } else {
                            assert(!dm2_v1_dungeon_c_light_flags4_no_record_floor_receipt(
                                &dungeon, level, x, y, &floor));
                            if (dm2_v1_dungeon_c_light_flags4_record_floor_receipt(
                                    &dungeon, &loader, level, x, y, 0u,
                                    &floor)) {
                                assert(floor.valid && floor.source_flags == 4u &&
                                       floor.first_record_link == (uint16_t)first);
                                ++admitted_record_count;
                                if ((floor.floor_ornament_word & 0xffu) != 0xffu) {
                                    assert((floor.floor_ornament_word & 0xffu) != 0xffu &&
                                           floor.ornament_source_hash != 0u);
                                    ++ornament_record_count;
                                }
                                assert(floor.floor_light_word == 0u &&
                                       floor.contributes_light == 0u);
                            }
                            ++record_count;
                        }
                    }
                }
            }
            printf("  PASS: FM Towns source flags-4 floor gates (%d no-record, %d record-bearing, %d admitted, %d ornament)\n",
                   no_record_count, record_count, admitted_record_count,
                   ornament_record_count);
            assert(no_record_count == 127 && record_count == 34 &&
                   admitted_record_count == 30 && ornament_record_count == 12);
            /* Source DB2 text and DB3 actuator cases on the real map. */
            assert(dm2_v1_dungeon_c_light_flags4_record_floor_receipt(
                &dungeon, &loader, 3, 12, 0, 0u, &floor));
            assert(floor.floor_ornament_word == 0x0a56u &&
                   floor.floor_light_word == 0u &&
                   floor.contributes_light == 0u);
            assert(dm2_v1_dungeon_c_light_flags4_record_floor_receipt(
                &dungeon, &loader, 3, 15, 0, 0u, &floor));
            assert(floor.floor_ornament_word == 0x0a1cu &&
                   floor.floor_light_word == 0u &&
                   floor.contributes_light == 0u);
            assert(!dm2_v1_dungeon_c_light_flags4_record_floor_receipt(
                &dungeon, &loader, 3, 17, 4, 0u, &floor));
            assert(!dm2_v1_dungeon_c_light_flags4_record_floor_receipt(
                &dungeon, &loader, 3, 7, 5, 0u, &floor));
            assert(!dm2_v1_dungeon_c_light_flags4_record_floor_receipt(
                &dungeon, &loader, 3, 11, 8, 0u, &floor));
            assert(!dm2_v1_dungeon_c_light_flags4_record_floor_receipt(
                &dungeon, &loader, 3, 15, 8, 0u, &floor));
        }
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        printf("  PASS: FM Towns map 38 source stone-room light inputs\n");
    }

    printf("  PASS: FM Towns dungeon loader\n");
    free(data);
}

static void test_pc_still_works(const char *path) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read PC DUNGEON.DAT at %s\n", path);
        return;
    }

    /* Magic should be 0x3147 = 'G1' */
    assert(data[2] == 0x47 && data[3] == 0x31);

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    assert(dungeon.level_count == 28);
    printf("  PASS: PC DUNGEON.DAT still loads\n");

    free(data);
}

int main(void) {
    const char *home;
    char fm_path[512], pc_path[512];

    printf("DM2 FM Towns dungeon loader tests:\n");

    home = getenv("HOME");
    if (!home) {
        printf("  SKIP: HOME not set\n");
        return 0;
    }

    snprintf(fm_path, sizeof(fm_path),
             "%s/.firestaff/data/dm2/fmtowns_iso/DATA/DUNGEON.DAT", home);
    test_fmtowns_load(fm_path);

    snprintf(pc_path, sizeof(pc_path),
             "%s/.firestaff/data/dm2/DUNGEON.DAT", home);
    test_pc_still_works(pc_path);

    printf("\nAll FM Towns dungeon loader tests passed.\n");
    return 0;
}
