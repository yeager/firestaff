/* Source: pinned SKProject SKWINSPX/v5/skmap.cpp:715-1255. */
#include "dm2_v1_arrange_dungeon_owner.h"
#include <stdlib.h>
#include <string.h>

#define END 0xfffeu
#define NULL_LINK 0xffffu
#define WALK_LIMIT 65536u

static uint16_t read_word(const DM2_V1_ArrangeDungeonOwner *o,
                          const uint8_t *p)
{
    return o->pools.words_big_endian
        ? (uint16_t)((p[0] << 8) | p[1])
        : (uint16_t)(p[0] | (p[1] << 8));
}
static void write_word(DM2_V1_ArrangeDungeonOwner *o, uint8_t *p, uint16_t w)
{
    p[o->pools.words_big_endian ? 1 : 0] = (uint8_t)w;
    p[o->pools.words_big_endian ? 0 : 1] = (uint8_t)(w >> 8);
}
static uint8_t *record(DM2_V1_ArrangeDungeonOwner *o, uint16_t r, int size)
{
    int db = (r >> 10) & 15;
    if (r == END || r == NULL_LINK || o->pools.pools[db].record_size < size)
        return NULL;
    return dm2_v1_record_pool_address_mut(&o->pools, (int16_t)r);
}
static uint8_t *tile(DM2_V1_ArrangeDungeonOwner *o,
                      const DM2_V1_DungeonData *d, int m, int x, int y)
{
    if (m < 0 || m >= d->level_count || x < 0 || y < 0 ||
        x >= d->level_widths[m] || y >= d->level_heights[m]) return NULL;
    return o->tiles + o->map_offsets[m] + x * d->level_heights[m] + y;
}
static int mark_tile(DM2_V1_ArrangeDungeonOwner *o,
                     const DM2_V1_DungeonData *d, int m, int x, int y)
{
    uint8_t *p = tile(o, d, m, x, y);
    if (!p) return 0;
    *p |= 2;
    return 1;
}
/* 2066_1f37 advances before inspecting the first actuator. */
static int mark_teleporter_actuators(DM2_V1_ArrangeDungeonOwner *o,
    uint16_t r, int map, unsigned *count, int *seen)
{
    for (unsigned n = 0; n < WALK_LIMIT; ++n) {
        uint8_t *p = record(o, r, 2);
        uint16_t w;
        if (!p) return 0;
        r = read_word(o, p);
        if (r == END) return 1;
        p = record(o, r, 2);
        if (!p) return 0;
        if (((r >> 10) & 15) != 3) continue;
        p = record(o, r, 4);
        if (!p) return 0;
        w = read_word(o, p + 2);
        if ((w & 127) != 0x27) continue;
        *seen = 1;
        if ((w & 0xff80) == 0) {
            write_word(o, p + 2, (uint16_t)((w & 127) |
                (((map + 1) & 511) << 7)));
            ++*count;
        }
    }
    return 0;
}
/* 2066_1ec9 reverses the DB0..DB3 prefix of b, then appends only the
 * first remaining record. APPEND_RECORD_TO resets that record's next word. */
static int merge_teleporter_chains(DM2_V1_ArrangeDungeonOwner *o,
    uint16_t a, uint16_t b, uint16_t *head)
{
    unsigned n = 0;
    if (a == END) { *head = b; return 1; }
    if (b == END) { *head = a; return 1; }
    while (b != END && ((b >> 10) & 15) < 4) {
        uint8_t *p = record(o, b, 2);
        uint16_t next;
        if (!p || ++n > WALK_LIMIT) return 0;
        next = read_word(o, p);
        write_word(o, p, a);
        a = b;
        b = next;
    }
    if (b != END) {
        uint8_t *p = record(o, b, 2);
        uint8_t *root = record(o, a, 2);
        uint16_t cursor;
        if (!p || !root) return 0;
        /* Exact source first write, before walking the destination. */
        write_word(o, p, END);
        cursor = read_word(o, root);
        while (cursor != END) {
            root = record(o, cursor, 2);
            if (!root || ++n > WALK_LIMIT) return 0;
            cursor = read_word(o, root);
        }
        write_word(o, root, b);
    }
    *head = a;
    return 1;
}
static int teleporter(DM2_V1_ArrangeDungeonOwner *o,
    const DM2_V1_DungeonData *d, uint16_t r, int m, int x, int y,
    int *stop)
{
    uint8_t *p = record(o, r, 6), *q;
    uint16_t w, other, head;
    int dm, dx, dy, seen = 0;
    unsigned count = 0;
    if (!p) return 0;
    w = read_word(o, p + 2);
    dx = w & 31; dy = (w >> 5) & 31;
    dm = read_word(o, p + 4) >> 8;
    if (!tile(o, d, dm, dx, dy)) return 0;
    if ((*tile(o, d, dm, dx, dy) & 0x10) == 0) return 1;
    other = (uint16_t)dm2_v1_dungeon_get_first_thing(d, dm, dx, dy);
    if (other == END) return 1;
    if (((other >> 10) & 15) != 1) return 1;
    q = record(o, other, 6);
    if (!q) return 0;
    w = read_word(o, q + 2);
    if ((w & 31) != x || ((w >> 5) & 31) != y ||
        (read_word(o, q + 4) >> 8) != m) return 1;
    if (!mark_teleporter_actuators(o, other, dm, &count, &seen) ||
        !mark_teleporter_actuators(o, r, m, &count, &seen)) return 0;
    if (count == 2) {
        if (!merge_teleporter_chains(o, read_word(o, p), read_word(o, q),
                                     &head)) return 0;
        write_word(o, q, head);
        write_word(o, p, head);
        ++o->teleporter_merges;
    } else if (count == 0 && seen) *stop = 1;
    return 1;
}
static int creature(DM2_V1_ArrangeDungeonOwner *o,
    const DM2_V1_CaiiSourceOwner *caii, uint16_t r, int m, int x, int y)
{
    uint8_t *p = record(o, r, 16);
    const DM2_AIDefinition *ai = NULL;
    uint8_t source_type;
    if (!p || !dm2_v1_caii_source_owner_db4_type(
            caii, r & 0x03ffu, &source_type) || source_type != p[4] ||
        !dm2_v1_caii_source_owner_ai_spec_def(caii, p[4], &ai) || !ai)
        return 0;
    write_word(o, p + 6, ai->BaseHP);
    write_word(o, p + 10, 0);
    if (ai->w0AIFlags & 1) {
        uint16_t possession;
        write_word(o, p + 8, 0);
        possession = read_word(o, p + 2);
        if ((read_word(o, p + 14) & 0x80) == 0 && possession != END) {
            write_word(o, p + 2, (uint16_t)((possession & 0x3fff) |
                (dm2_v1_drops_randdir(&o->rng) << 14)));
            ++o->rng_draws;
            for (unsigned n = 0;; ++n) {
                uint8_t *item;
                uint16_t next;
                if (possession == END) break;
                if (n >= WALK_LIMIT || !(item = record(o, possession, 2)))
                    return 0;
                next = read_word(o, item);
                if (next != END) {
                    next = (uint16_t)((next & 0x3fff) |
                        (dm2_v1_drops_randdir(&o->rng) << 14));
                    write_word(o, item, next);
                    ++o->rng_draws;
                }
                possession = next;
            }
        }
    } else {
        write_word(o, p + 12, (uint16_t)((x & 31) | ((y & 31) << 5) |
                                           ((m & 63) << 10)));
    }
    return 1;
}
void dm2_v1_arrange_dungeon_owner_free(DM2_V1_ArrangeDungeonOwner *o)
{
    if (!o) return;
    dm2_v1_record_pool_set_free(&o->pools);
    free(o->tiles);
    memset(o, 0, sizeof(*o));
}
int dm2_v1_arrange_dungeon_owner_build(const DM2_V1_DungeonData *d,
    const DM2_V1_RecordPoolSet *pools, const DM2_V1_CaiiSourceOwner *caii,
    const DM2_V1_DropRng *rng, const DM2_V1_ArrangeDungeonQueries *queries,
    DM2_V1_ArrangeDungeonOwner *out)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    DM2_V1_ArrangeDungeonOwner o;
    memset(&o, 0, sizeof(o));
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!d || !pools || !pools->valid || !pools->record_graph_complete ||
        !caii || !caii->valid || !rng || !queries ||
        !queries->pit_gdat_6a || !queries->locate_other_level ||
        d->square_bytes != 1 || !d->record_graph_complete ||
        d->g1_w0_chains_disabled || d->level_count <= 0 ||
        d->level_count > DM2_V1_MAX_LEVELS ||
        pools->words_big_endian != d->words_big_endian ||
        pools->source_words_big_endian != d->source_words_big_endian ||
        !dm2_v1_dungeon_validate_record_graph(d) ||
        !dm2_v1_record_pool_set_clone(&o.pools, pools)) return 0;
    o.rng = *rng;
    for (int m = 0; m < d->level_count; ++m) {
        int w = d->level_widths[m], h = d->level_heights[m];
        if (w <= 0 || w > 32 || h <= 0 || h > 32) goto fail;
        o.map_offsets[m] = o.tile_count;
        o.tile_count += (size_t)w * h;
    }
    o.tiles = malloc(o.tile_count);
    if (!o.tiles) goto fail;
    for (int m = 0; m < d->level_count; ++m)
        for (int x = 0; x < d->level_widths[m]; ++x)
            for (int y = 0; y < d->level_heights[m]; ++y) {
                int value = dm2_v1_dungeon_get_tile_raw(d, m, x, y);
                if (value < 0 || value > 255) goto fail;
                *tile(&o, d, m, x, y) = (uint8_t)value;
            }
    for (int m = 0; m < d->level_count; ++m)
        for (int x = 0; x < d->level_widths[m]; ++x)
            for (int y = 0; y < d->level_heights[m]; ++y) {
                /* Source snapshots this byte before its record mutations. */
                uint8_t value = *tile(&o, d, m, x, y);
                if (value & 0x10) {
                    int root = dm2_v1_dungeon_get_first_thing(d,m,x,y);
                    uint16_t r;
                    if (root < 0) goto fail;
                    /* The admitted File_header represents an empty tile list
                     * with NULL as well as END. Normalize only this root;
                     * NULL encountered as a subsequent record link still
                     * fails record() below. Keep the pit post-pass running. */
                    r = root == NULL_LINK ? END : (uint16_t)root;
                    for (unsigned n = 0; r != END; ++n) {
                        uint8_t *p = record(&o, r, 2);
                        int db = (r >> 10) & 15, stop = 0, mark = 0;
                        uint16_t w;
                        if (!p || n >= WALK_LIMIT) goto fail;
                        ++o.record_visits;
                        if (db == 1) {
                            if (!teleporter(&o,d,r,m,x,y,&stop)) goto fail;
                        } else if (db == 2) {
                            p = record(&o,r,4); if (!p) goto fail;
                            w = read_word(&o,p+2);
                            if ((w & 6) == 2) {
                                int usage = (w >> 11) & 31;
                                if (usage == 6 || usage == 7) mark = 1;
                                else if (usage == 8) write_word(&o,p+2,w|1);
                            }
                        } else if (db == 3) {
                            p = record(&o,r,8); if (!p) goto fail;
                            w = read_word(&o,p+2) & 127;
                            if (w == 0x1c) mark = 1;
                            else if (w == 0x27 || w == 0x2c) {
                                uint16_t flags = read_word(&o,p+4);
                                if (flags & 4) write_word(&o,p+4,flags|1);
                            } else if (w == 0x3f) {
                                uint16_t flags = read_word(&o,p+4);
                                write_word(&o,p+4,(flags&0xfffa)|4);
                            }
                            else if (w == 0x46) {
                                uint16_t target = read_word(&o,p+6);
                                int tx = (target >> 6)&31, ty = target >> 11;
                                uint8_t *t = tile(&o,d,m,tx,ty);
                                if (!t) goto fail;
                                if ((*t >> 5) == 4) {
                                    uint16_t door = (uint16_t)
                                        dm2_v1_dungeon_get_first_thing(d,m,tx,ty);
                                    uint8_t *dp = record(&o,door,4);
                                    uint16_t dw;
                                    if (!dp) goto fail;
                                    dw = read_word(&o,dp+2);
                                    write_word(&o,dp+2,(uint16_t)((dw&~0x2000u)|
                                        ((read_word(&o,p+4)&4)<<11)));
                                }
                            }
                        } else if (db == 4) {
                            if (!creature(&o,caii,r,m,x,y)) goto fail;
                        }
                        if (stop) break;
                        if (mark && !mark_tile(&o,d,m,x+dx[r>>14],y+dy[r>>14]))
                            goto fail;
                        /* Re-read from mutated pools, including alias merges. */
                        r = read_word(&o,p);
                    }
                }
                if ((value >> 5) == 2) {
                    uint16_t flag;
                    if (!queries->pit_gdat_6a(queries->context,m,&flag)) goto fail;
                    if (!flag) {
                        int tx=x, ty=y, tm;
                        if (!queries->locate_other_level(queries->context,m,
                                &tx,&ty,&tm) || !mark_tile(&o,d,tm,tx,ty))
                            goto fail;
                    }
                }
            }
    o.valid = 1;
    *out = o;
    return 1;
fail:
    dm2_v1_arrange_dungeon_owner_free(&o);
    return 0;
}

static int same_layout(const DM2_V1_DungeonData *a,
                       const DM2_V1_DungeonData *b)
{
#define SAME(field) \
    do { if (memcmp(&a->field, &b->field, sizeof(a->field)) != 0) return 0; } while (0)
    SAME(raw_size); SAME(level_count); SAME(level_types);
    SAME(level_widths); SAME(level_heights);
    SAME(level_offsets); SAME(map_offset_x); SAME(map_offset_y);
    SAME(map_level_number); SAME(map_graphics_flip_seed);
    SAME(map_door_set0); SAME(map_door_set1); SAME(map_use_door0);
    SAME(map_use_door1); SAME(map_door_ornate_count);
    SAME(square_bytes); SAME(raw_map_data_base); SAME(column_index_base);
    SAME(square_first_thing_base); SAME(square_first_thing_count);
    SAME(text_data_base); SAME(text_word_count);
    SAME(g1_extension_base); SAME(g1_extension_size);
    SAME(g1_extension_record_bases); SAME(g1_extension_record_counts);
    SAME(thing_data_bases); SAME(thing_type_counts);
    SAME(record_graph_complete); SAME(g1_w0_chains_disabled);
    SAME(words_big_endian); SAME(source_words_big_endian);
    SAME(initial_party_pose_valid); SAME(initial_party_x);
    SAME(initial_party_y); SAME(initial_party_dir);
    SAME(sky_texture_index); SAME(weather_zone_count);
#undef SAME
    return 1;
}

static int same_pool_layout(const DM2_V1_RecordPoolSet *a,
                            const DM2_V1_RecordPoolSet *b)
{
    if (!a->valid || !b->valid || !a->record_graph_complete ||
        !b->record_graph_complete || a->words_big_endian != b->words_big_endian ||
        a->source_words_big_endian != b->source_words_big_endian) return 0;
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        const DM2_V1_RecordPool *p = &a->pools[i], *q = &b->pools[i];
        if (p->record_count != q->record_count ||
            p->record_size != q->record_size || p->source_base != q->source_base ||
            p->extension_count != q->extension_count ||
            p->extension_base != q->extension_base) return 0;
    }
    return 1;
}

/* Authenticate each writable span against a freshly parsed original before
 * copying. Bounds use subtraction so hostile offsets cannot wrap an end. */
static int copy_span(uint8_t *raw, size_t size, int base,
                     const uint8_t *bytes, size_t count)
{
    if (!count) return 1;
    if (!bytes || base < 0 || (size_t)base > size ||
        count > size - (size_t)base) return 0;
    memcpy(raw + base, bytes, count);
    return 1;
}

/* The loader's LE admission validates roots without walking every w0 link.
 * Publication of this transformed graph additionally requires bounded full
 * chains. Teleporter aliases are legitimate; only each individual walk is
 * bounded, rather than rejecting records shared by separate ground roots. */
static int complete_tile_chains(const DM2_V1_DungeonData *d,
                                const DM2_V1_RecordPoolSet *pools)
{
    size_t limit = 0;
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i)
        limit += (size_t)pools->pools[i].record_count +
                 (size_t)pools->pools[i].extension_count;
    if (!limit) return 0;
    for (int m = 0; m < d->level_count; ++m)
        for (int x = 0; x < d->level_widths[m]; ++x)
            for (int y = 0; y < d->level_heights[m]; ++y) {
                int value = dm2_v1_dungeon_get_tile_raw(d, m, x, y);
                int r;
                size_t count = 0;
                if (value < 0) return 0;
                if (!(value & 0x10)) continue;
                r = dm2_v1_dungeon_get_first_thing(d, m, x, y);
                if (r == NULL_LINK) continue;
                while (r != END) {
                    if (r < 0 || ++count > limit ||
                        !dm2_v1_dungeon_get_thing_record(d, (uint16_t)r,
                                                         NULL, NULL, NULL))
                        return 0;
                    r = dm2_v1_dungeon_get_next_thing(d, (uint16_t)r);
                }
            }
    return 1;
}

int dm2_v1_arrange_dungeon_owner_reload(
    const DM2_V1_DungeonData *original,
    const DM2_V1_ArrangeDungeonOwner *arranged,
    DM2_V1_DungeonData *out)
{
    DM2_V1_DungeonData baseline = {0}, candidate = {0};
    DM2_V1_RecordPoolSet baseline_pools = {0}, reloaded_pools = {0};
    uint8_t *raw = NULL;
    size_t tile_count = 0;
    int success = 0;
    if (!original || !arranged || !arranged->valid || !out ||
        out == original || out->raw_data || !original->raw_data ||
        original->raw_size <= 0 || !arranged->tiles ||
        dm2_v1_dungeon_load(&baseline, original->raw_data,
                              original->raw_size) != 0 ||
        !same_layout(original, &baseline) ||
        !dm2_v1_dungeon_validate_record_graph(&baseline) ||
        !dm2_v1_record_pool_set_init_from_dungeon(&baseline_pools, &baseline) ||
        !same_pool_layout(&baseline_pools, &arranged->pools)) goto done;
    raw = malloc((size_t)original->raw_size);
    if (!raw) goto done;
    memcpy(raw, original->raw_data, (size_t)original->raw_size);
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        const DM2_V1_RecordPool *p = &arranged->pools.pools[i];
        /* Counts and record sizes have just matched the loader-owned layout. */
        size_t size = (size_t)p->record_count * (size_t)p->record_size;
        size_t extension = (size_t)p->extension_count * (size_t)p->record_size;
        if (!copy_span(raw, (size_t)original->raw_size, p->source_base,
                       p->bytes, size) ||
            !copy_span(raw, (size_t)original->raw_size, p->extension_base,
                       p->extension_bytes, extension)) goto done;
    }
    for (int m = 0; m < baseline.level_count; ++m) {
        size_t size = (size_t)baseline.level_widths[m] *
                      (size_t)baseline.level_heights[m];
        size_t base;
        if (arranged->map_offsets[m] != tile_count ||
            tile_count > arranged->tile_count ||
            size > arranged->tile_count - tile_count ||
            baseline.raw_map_data_base < 0 || baseline.level_offsets[m] < 0)
            goto done;
        base = (size_t)baseline.raw_map_data_base +
               (size_t)baseline.level_offsets[m];
        if (base > (size_t)original->raw_size ||
            size > (size_t)original->raw_size - base) goto done;
        memcpy(raw + base, arranged->tiles + tile_count, size);
        tile_count += size;
    }
    if (tile_count != arranged->tile_count ||
        dm2_v1_dungeon_load(&candidate, raw, original->raw_size) != 0 ||
        !same_layout(&baseline, &candidate) ||
        !dm2_v1_dungeon_validate_record_graph(&candidate) ||
        !dm2_v1_record_pool_set_init_from_dungeon(&reloaded_pools, &candidate) ||
        !same_pool_layout(&arranged->pools, &reloaded_pools) ||
        !complete_tile_chains(&candidate, &reloaded_pools)) goto done;
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        const DM2_V1_RecordPool *p = &arranged->pools.pools[i];
        const DM2_V1_RecordPool *q = &reloaded_pools.pools[i];
        size_t size = (size_t)p->record_count * (size_t)p->record_size;
        size_t extension = (size_t)p->extension_count * (size_t)p->record_size;
        if ((size && memcmp(p->bytes, q->bytes, size)) ||
            (extension && memcmp(p->extension_bytes, q->extension_bytes,
                                 extension))) goto done;
    }
    /* Reload must retain every serialized tile, as well as pool bytes. */
    for (int m = 0; m < baseline.level_count; ++m)
        for (int x = 0; x < baseline.level_widths[m]; ++x)
            for (int y = 0; y < baseline.level_heights[m]; ++y)
                if (dm2_v1_dungeon_get_tile_raw(&candidate, m, x, y) !=
                    arranged->tiles[arranged->map_offsets[m] +
                                    x * baseline.level_heights[m] + y])
                    goto done;
    *out = candidate;
    memset(&candidate, 0, sizeof(candidate));
    success = 1;
done:
    free(raw);
    dm2_v1_record_pool_set_free(&reloaded_pools);
    dm2_v1_record_pool_set_free(&baseline_pools);
    dm2_v1_dungeon_free(&candidate);
    dm2_v1_dungeon_free(&baseline);
    return success;
}
