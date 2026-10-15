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
    if (!p || !dm2_v1_caii_source_owner_ai_spec_def(caii, p[4], &ai) || !ai)
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
