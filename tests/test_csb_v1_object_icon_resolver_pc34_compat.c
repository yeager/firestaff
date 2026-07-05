/*
 * CSB V1 object-icon resolver contract.
 *
 * Pins the first CSB-owned object icon binding used by M11 leader-hand
 * rendering.  The resolver must read CSB dungeon thing records and follow
 * ReDMCSB DUNGEON.C F0141 + OBJECT.C F0033 instead of falling back to the
 * DM1 M11 world thing arrays.
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check_int(const char *label, int got, int expected)
{
    if (got != expected) {
        fprintf(stderr, "FAIL: %s got=%d expected=%d\n",
                label, got, expected);
        ++g_failures;
    }
}

static void write_u16(unsigned char *p, unsigned int value)
{
    p[0] = (unsigned char)(value & 0xffu);
    p[1] = (unsigned char)((value >> 8) & 0xffu);
}

static void init_dungeon(CSB_V1_DungeonData *dungeon,
                         unsigned char *raw,
                         size_t raw_size)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->thing_type_counts[THING_TYPE_WEAPON] = 2;
    dungeon->thing_type_counts[THING_TYPE_SCROLL] = 2;
    dungeon->thing_type_counts[THING_TYPE_JUNK] = 1;
    dungeon->thing_data_bases[THING_TYPE_WEAPON] = 0;
    dungeon->thing_data_bases[THING_TYPE_SCROLL] = 16;
    dungeon->thing_data_bases[THING_TYPE_JUNK] = 32;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[64];
    uint16_t dagger = (uint16_t)((THING_TYPE_WEAPON << 10) | 0u);
    uint16_t lit_torch = (uint16_t)((THING_TYPE_WEAPON << 10) | 1u);
    uint16_t scroll_open = (uint16_t)((THING_TYPE_SCROLL << 10) | 0u);
    uint16_t scroll_closed = (uint16_t)((THING_TYPE_SCROLL << 10) | 1u);
    uint16_t compass = (uint16_t)((THING_TYPE_JUNK << 10) | 0u);

    memset(&profile, 0, sizeof(profile));
    init_dungeon(&dungeon, raw, sizeof(raw));
    profile.dungeon_handle = &dungeon;
    profile.party_dir = 2;

    write_u16(raw + 0, THING_ENDOFLIST);
    write_u16(raw + 2, 8u);                         /* C08 dagger */
    write_u16(raw + 4, THING_ENDOFLIST);
    write_u16(raw + 6, 2u | (8u << 10) | 0x8000u);  /* lit torch, charge 8 */
    write_u16(raw + 16, THING_ENDOFLIST);
    write_u16(raw + 18, 0u);                        /* open scroll */
    write_u16(raw + 20, THING_ENDOFLIST);
    write_u16(raw + 22, 1u << 10);                  /* closed scroll */
    write_u16(raw + 32, THING_ENDOFLIST);
    write_u16(raw + 34, 0u);                        /* compass */

    check_int("dagger icon", csb_v1_runtime_object_icon_index(&profile, dagger), 32);
    check_int("lit torch charge icon", csb_v1_runtime_object_icon_index(&profile, lit_torch), 7);
    check_int("open scroll icon", csb_v1_runtime_object_icon_index(&profile, scroll_open), 30);
    check_int("closed scroll icon", csb_v1_runtime_object_icon_index(&profile, scroll_closed), 31);
    check_int("compass follows party direction", csb_v1_runtime_object_icon_index(&profile, compass), 2);
    check_int("empty thing has no icon", csb_v1_runtime_object_icon_index(&profile, THING_NONE), -1);
    check_int("out-of-range thing has no icon",
              csb_v1_runtime_object_icon_index(
                  &profile,
                  (uint16_t)((THING_TYPE_WEAPON << 10) | 31u)),
              -1);

    if (g_failures != 0) return 1;
    printf("PASS: csb_v1_object_icon_resolver_pc34_compat\n");
    return 0;
}
