/*
 * dm2_v1_object_model.c — DM2 V1 Object/Tile Model Implementation
 *
 * DM2 Phase 2: Object types, tile states, world-state objects.
 * Parses DM2 thing records from DUNGEON.DAT.
 *
 * Thing data byte counts (source-locked to ReDMCSB DUNGEON.C:35):
 *   Door(0)=4, Teleporter(1)=6, TextString(2)=4, Sensor(3)=8,
 *   Group(4)=16, Weapon(5)=4, Armour(6)=4, Scroll(7)=4,
 *   Potion(8)=4, Container(9)=8, Junk(10)=4, Unused(11-13)=0,
 *   Projectile(14)=8, Explosion(15)=4
 *
 * Source: ReDMCSB DUNGEON.C:35-60 — G0235/G0236 thing tables
 * Source: ReDMCSB DEFS.H:985-1116 — MAP/TILE/THING types
 * Source: docs/dm2_actuators.md — actuator/sensor types
 * Source: docs/dm2_special_squares.md — door/teleporter/pit
 */

#include "dm2_v1_object_model.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_world_model.h"
#include <stdlib.h>
#include <string.h>

/* ── Thing data byte counts (ReDMCSB DUNGEON.C:35) ──────────────── */
const uint8_t dm2_thing_data_byte_count[16] = {
    4,   /* Door */
    6,   /* Teleporter */
    4,   /* Text String */
    8,   /* Sensor */
    16,  /* Group */
    4,   /* Weapon */
    4,   /* Armour */
    4,   /* Scroll */
    4,   /* Potion */
    8,   /* Container */
    4,   /* Junk */
    0,   /* Unused 11 */
    0,   /* Unused 12 */
    0,   /* Unused 13 */
    8,   /* Projectile */
    4    /* Explosion */
};

/* ── LE read helper ─────────────────────────────────────────────── */
static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

/* ── Tile state model ───────────────────────────────────────────── */

/*
 * dm2_v1_tile_get_type — normalize tile type (lower 5 bits).
 * Same as dm2_v1_dungeon_get_square_type but via world model.
 * Source: ReDMCSB HASHBUCKET.C, DEFS.H:385-390
 */
int dm2_v1_tile_get_type(const uint8_t *tile_raw) {
    if (!tile_raw) return -1;
    return (int)(rd16le(tile_raw) & 0x001F);
}

/*
 * dm2_v1_tile_is_walkable — check if tile type is passable.
 * Source: SKULL.ASM T520 movement/collision
 */
int dm2_v1_tile_is_walkable(int tile_type) {
    switch (tile_type) {
        case 0: /* FLOOR */
        case 3: /* FLOOR_ORNATE */
        case 4: /* SECRET_DOOR */
        case 8: /* TELEPORTER */
        case 10: /* WATER */
            return 1;
        case 1: /* WALL */
        case 5: /* PIT */
        case 11: /* LAVA */
        case 13: /* INACCESSIBLE */
            return 0;
        default:
            return (tile_type <= 7) ? 1 : 0;
    }
}

/*
 * dm2_v1_tile_get_door_state — extract door state from tile square.
 * Source: ReDMCSB DEFS.H:1040-1045, M036/M037 macros
 */
int dm2_v1_tile_get_door_state(uint16_t tile_raw) {
    return (int)(tile_raw & 0x0007);
}

/* ── Object record parsing ──────────────────────────────────────── */

/*
 * dm2_v1_parse_door — parse a door thing record (4 bytes).
 * Layout: [flags:1][type:1][set:1][state:1] + optional target coords
 * Source: ReDMCSB DUNGEON.C:35, docs/dm2_special_squares.md §8
 */
static DM2_Object_Door dm2_v1_parse_door(const uint8_t *data) {
    DM2_Object_Door d;
    memset(&d, 0, sizeof(d));
    if (!data) return d;
    d.type  = data[1];
    d.state = data[3];
    d.flags = data[0];
    d.door_set = 0;
    d.target_x = -1; d.target_y = -1; d.target_level = -1;
    /* Target encoded in adjacent thing record or actuator data */
    return d;
}

/*
 * dm2_v1_parse_teleporter — parse a teleporter thing record (6 bytes).
 * Layout: [target_x:2_LE][target_y:2_LE][target_level:2_LE]
 * Source: ReDMCSB DUNGEON.C:35, docs/dm2_special_squares.md
 */
static DM2_Object_Teleporter dm2_v1_parse_teleporter(const uint8_t *data) {
    DM2_Object_Teleporter t;
    memset(&t, 0, sizeof(t));
    if (!data) return t;
    t.target_x     = (int16_t)rd16le(data + 0);
    t.target_y     = (int16_t)rd16le(data + 2);
    t.target_level = (int16_t)rd16le(data + 4);
    t.flags        = 0;
    t.teleporter_type = 0;
    return t;
}

/*
 * dm2_v1_parse_sensor — parse a sensor/actuator thing record (8 bytes).
 * Layout: [sensor_type:1][actuator_type:1][target_id:2][tx:2][ty:2]
 * Source: docs/dm2_sensors.md, docs/dm2_actuators.md
 */
static DM2_Object_Sensor dm2_v1_parse_sensor(const uint8_t *data) {
    DM2_Object_Sensor s;
    memset(&s, 0, sizeof(s));
    if (!data) return s;
    s.sensor_type  = data[0];
    s.target_type   = data[1];
    s.target_id     = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
    s.target_map_x  = (int16_t)rd16le(data + 4);
    s.target_map_y  = (int16_t)rd16le(data + 6);
    s.target_level  = -1;
    return s;
}

/*
 * dm2_v1_parse_projectile — parse a projectile thing record (8 bytes).
 * Layout: [x:2_LE][y:2_LE][dx:2_LE][dy:2_LE]
 * Source: ReDMCSB DUNGEON.C:35
 */
static DM2_Object_Projectile dm2_v1_parse_projectile(const uint8_t *data) {
    DM2_Object_Projectile p;
    memset(&p, 0, sizeof(p));
    if (!data) return p;
    p.x = (int16_t)rd16le(data + 0);
    p.y = (int16_t)rd16le(data + 2);
    p.dx = (int16_t)rd16le(data + 4);
    p.dy = (int16_t)rd16le(data + 6);
    p.projectile_type = 0;
    p.flags = 0;
    return p;
}

static void dm2_v1_object_model_fill_record(DM2_ObjectRecord *r,
                                            int type,
                                            int level,
                                            int x,
                                            int y,
                                            const uint8_t *record,
                                            int record_size) {
    if (!r) return;
    memset(r, 0, sizeof(*r));
    r->type = (DM2_ThingType)type;
    r->level = level;
    r->x = x;
    r->y = y;
    if (record && record_size > 0) {
        size_t copy_size = (size_t)record_size;
        if (copy_size > sizeof(r->data)) copy_size = sizeof(r->data);
        memcpy(r->data, record, copy_size);
    }

    switch (type) {
        case DM2_THING_DOOR:
            r->u.door = dm2_v1_parse_door(record);
            break;
        case DM2_THING_TELEPORTER:
            r->u.teleporter = dm2_v1_parse_teleporter(record);
            break;
        case DM2_THING_SENSOR:
            r->u.sensor = dm2_v1_parse_sensor(record);
            break;
        case DM2_THING_PROJECTILE:
            r->u.projectile = dm2_v1_parse_projectile(record);
            break;
        default:
            break;
    }
}

static int dm2_v1_object_model_append_loader_record(DM2_ObjectModel *model,
                                                    int *rec_cap,
                                                    int type,
                                                    int level,
                                                    int x,
                                                    int y,
                                                    const uint8_t *record,
                                                    int record_size) {
    DM2_ObjectRecord *new_recs;

    if (!model || !rec_cap || !record || type < 0) return 0;
    if (model->object_count >= *rec_cap) {
        int new_cap = (*rec_cap <= 0) ? 32 : (*rec_cap * 2);
        new_recs = (DM2_ObjectRecord *)realloc(
            model->objects, (size_t)new_cap * sizeof(DM2_ObjectRecord));
        if (!new_recs) return -1;
        model->objects = new_recs;
        *rec_cap = new_cap;
    }
    dm2_v1_object_model_fill_record(
        &model->objects[model->object_count],
        type,
        level,
        x,
        y,
        record,
        record_size);
    model->object_count++;
    return 1;
}

typedef struct {
    DM2_ObjectModel *model;
    int *rec_cap;
    uint8_t *visited;
} DM2_ObjectModelLoaderWalk;

static int dm2_v1_object_model_loader_visit(void *user,
                                            uint16_t thing,
                                            int type,
                                            int index,
                                            const uint8_t *record,
                                            int record_size,
                                            int level,
                                            int x,
                                            int y) {
    DM2_ObjectModelLoaderWalk *ctx = (DM2_ObjectModelLoaderWalk *)user;
    int append_result;

    (void)thing;
    if (!ctx || !ctx->model || !ctx->rec_cap || !ctx->visited)
        return -1;
    if (type < 0 || type >= DM2_THING_COUNT ||
        index < 0 || index >= 1024 || record_size < 2) {
        return -1;
    }
    if (ctx->visited[type * 1024 + index])
        return 0;
    ctx->visited[type * 1024 + index] = 1;
    append_result = dm2_v1_object_model_append_loader_record(
        ctx->model, ctx->rec_cap, type, level, x, y, record, record_size);
    return append_result < 0 ? -1 : 0;
}

static int dm2_v1_object_model_load_via_loader(DM2_ObjectModel *model,
                                               const uint8_t *dungeon_raw,
                                               int dungeon_size,
                                               int level) {
    DM2_V1_DungeonData dungeon;
    int has_record_bases = 0;
    int rec_cap = 64;
    uint8_t visited[16 * 1024];
    DM2_ObjectModelLoaderWalk walk_ctx;

    if (dm2_v1_dungeon_load(&dungeon, dungeon_raw, dungeon_size) != 0)
        return -1;
    if (level < 0 || level >= dungeon.level_count) {
        dm2_v1_dungeon_free(&dungeon);
        return -1;
    }
    for (int i = 0; i < DM2_THING_COUNT; ++i) {
        if (dungeon.thing_data_bases[i] >= 0 &&
            dungeon.thing_type_counts[i] > 0) {
            has_record_bases = 1;
            break;
        }
    }
    if (!has_record_bases) {
        int is_unresolved_byte_layout = (dungeon.square_bytes == 1);
        dm2_v1_dungeon_free(&dungeon);
        if (is_unresolved_byte_layout)
            return 0;
        return 1;
    }
    if (dungeon.square_bytes == 1 && dungeon.g1_extension_size > 0 &&
        !dungeon.record_graph_complete) {
        dm2_v1_dungeon_free(&dungeon);
        return 0;
    }

    model->objects = (DM2_ObjectRecord *)malloc(
        (size_t)rec_cap * sizeof(DM2_ObjectRecord));
    if (!model->objects) {
        dm2_v1_dungeon_free(&dungeon);
        return -1;
    }
    memset(visited, 0, sizeof(visited));
    walk_ctx.model = model;
    walk_ctx.rec_cap = &rec_cap;
    walk_ctx.visited = visited;

    for (int x = 0; x < dungeon.level_widths[level]; ++x) {
        for (int y = 0; y < dungeon.level_heights[level]; ++y) {
            if (dm2_v1_dungeon_walk_square_things(
                    &dungeon, level, x, y, 1024,
                    dm2_v1_object_model_loader_visit, &walk_ctx) < 0) {
                free(model->objects);
                model->objects = NULL;
                model->object_count = 0;
                dm2_v1_dungeon_free(&dungeon);
                return -1;
            }
        }
    }

    dm2_v1_dungeon_free(&dungeon);
    return 0;
}

/* ── Public API ──────────────────────────────────────────────────── */

/*
 * dm2_v1_object_model_load — load all thing records for a given level.
 *
 * Thing data in DUNGEON.DAT:
 *   - 16 pools of variable-length thing records (one pool per thing type)
 *   - Pool counts stored in DUNGEON_HEADER.thing_count[16]
 *   - Square-first thing index at DUNGEON_HEADER.square_first_thing_count
 *   - Each record's position in the dungeon determines its (x,y,level)
 *
 * We use the simplified approach: walk the thing data pools for the
 * given level and construct object records for things that have state.
 *
 * Source: ReDMCSB DEFS.H:985-998 (header), DUNGEON.C:35-60 (thing tables)
 */
int dm2_v1_object_model_load(DM2_ObjectModel *model,
                             const uint8_t *dungeon_raw,
                             int dungeon_size,
                             int level) {
    int loader_result;

    if (!model || !dungeon_raw || dungeon_size <= 0) return -1;
    memset(model, 0, sizeof(*model));

    /* skproject SKWIN/SkWinCore.cpp GET_TILE_RECORD_LINK follows
     * column-object indexes into the ground-stack list, and ObjectID records
     * carry their next link in w0. Prefer the shared dungeon loader for that
     * source-shaped layout so object consumers do not duplicate offset math. */
    loader_result = dm2_v1_object_model_load_via_loader(
        model, dungeon_raw, dungeon_size, level);
    if (loader_result <= 0)
        return loader_result;

    /* The remaining code is an older inferred sequential-pool parser.  DM2
     * records are owned by SKProject's G1 c_record links, not by a guessed
     * post-tile byte range.  Do not promote legacy fixtures or arbitrary data
     * to object records when the source-shaped loader has no complete owner. */
    return 0;
}

/*
 * dm2_v1_object_model_free — free object model and all owned resources.
 */
void dm2_v1_object_model_free(DM2_ObjectModel *model) {
    if (!model) return;
    if (model->objects) {
        free(model->objects);
        model->objects = NULL;
    }
    model->object_count = 0;
}

const char *dm2_v1_object_model_source_evidence(void) {
    return
        "DM2 V1 Object/Tile Model — Phase 2 World/Data Ingestion\n"
        "Source: ReDMCSB DUNGEON.C:35-60 — G0235/G0236 thing data byte counts\n"
        "Source: ReDMCSB DEFS.H:985-1116 — MAP/TILE/THING types and door states\n"
        "Source: ReDMCSB DUNGEON.C:993-1015 — MAP/CurrentMap globals\n"
        "Source: docs/dm2_actuators.md — actuator/sensor types (40+ types)\n"
        "Source: docs/dm2_sensors.md — floor/wall sensor types\n"
        "Source: docs/dm2_special_squares.md — door/teleporter/pit/ladder\n"
        "Source: SKWin.GDAT2.InternalCodes.txt — door strength, color keys, mirror flag\n"
        "Source: SkWinCore.cpp:847-852 — item DB types (dbWeapon=5, dbCloth=6, dbScroll=7, dbMisc=10)\n"
        "Source: skproject SKWIN/SkWinCore.cpp GET_TILE_RECORD_LINK/GET_NEXT_RECORD_LINK — loader-owned square chains through ObjectID w0\n"
        "Asset: DM2 PC English DUNGEON.DAT 6caccd7875009e82fe2e28e7f6d6adc0\n";
}
