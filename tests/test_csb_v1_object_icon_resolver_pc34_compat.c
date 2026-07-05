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

static void check_str(const char *label, const char *got, const char *expected)
{
    if (!got || strcmp(got, expected) != 0) {
        fprintf(stderr, "FAIL: %s got=%s expected=%s\n",
                label, got ? got : "(null)", expected);
        ++g_failures;
    }
}

static void write_u16(unsigned char *p, unsigned int value)
{
    p[0] = (unsigned char)(value & 0xffu);
    p[1] = (unsigned char)((value >> 8) & 0xffu);
}

static size_t append_m564_name(unsigned char *dst,
                               size_t offset,
                               size_t cap,
                               const char *name)
{
    size_t i;

    if (!dst || !name || cap == 0U) return offset;
    for (i = 0U; name[i] != '\0'; ++i) {
        if (offset >= cap) return offset;
        dst[offset++] = (unsigned char)name[i];
    }
    if (offset > 0U) {
        dst[offset - 1U] = (unsigned char)(dst[offset - 1U] | 0x80U);
    } else if (offset < cap) {
        dst[offset++] = 0x80U;
    }
    return offset;
}

static size_t make_m564_names(unsigned char *dst,
                              size_t cap,
                              const char *icon7_name,
                              const char *icon32_name)
{
    size_t offset = 0U;
    int i;

    for (i = 0; i < CSB_V1_OBJECT_NAME_COUNT; ++i) {
        const char *name = "X";
        if (i == 7) {
            name = icon7_name;
        } else if (i == 32) {
            name = icon32_name;
        }
        offset = append_m564_name(dst, offset, cap, name);
    }
    return offset;
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

    {
        char name[32];
        memset(name, 0, sizeof(name));
        check_int("dagger has CSB-owned name",
                  csb_v1_runtime_object_name(&profile, dagger, name, sizeof(name)),
                  1);
        check_str("dagger name", name, "DAGGER");
        memset(name, 0, sizeof(name));
        check_int("lit torch has CSB-owned name",
                  csb_v1_runtime_object_name(&profile, lit_torch, name, sizeof(name)),
                  1);
        check_str("lit torch name", name, "TORCH");
        memset(name, 0, sizeof(name));
        check_int("closed scroll has CSB-owned name",
                  csb_v1_runtime_object_name(&profile, scroll_closed, name, sizeof(name)),
                  1);
        check_str("closed scroll name", name, "SCROLL");
        memset(name, 0, sizeof(name));
        check_int("invalid thing has no CSB name",
                  csb_v1_runtime_object_name(&profile, THING_NONE, name, sizeof(name)),
                  0);
        check_str("invalid thing leaves name blank", name, "");
    }

    {
        unsigned char object_names[4096];
        size_t object_names_size;
        char name[32];

        memset(object_names, 0, sizeof(object_names));
        object_names_size = make_m564_names(object_names,
                                            sizeof(object_names),
                                            "SOURCE TORCH",
                                            "SOURCE DAGGER");
        check_int("M564 object names load",
                  csb_v1_runtime_load_object_names_m564(
                      &profile,
                      object_names,
                      object_names_size),
                  1);

        memset(name, 0, sizeof(name));
        check_int("M564 dagger has icon-indexed name",
                  csb_v1_runtime_object_name(&profile, dagger, name, sizeof(name)),
                  1);
        check_str("M564 dagger name", name, "SOURCE DAGGER");

        memset(name, 0, sizeof(name));
        check_int("M564 lit torch uses resolved icon name",
                  csb_v1_runtime_object_name(&profile, lit_torch, name, sizeof(name)),
                  1);
        check_str("M564 lit torch name", name, "SOURCE TORCH");

        check_int("truncated M564 object names reject",
                  csb_v1_runtime_load_object_names_m564(
                      &profile,
                      object_names,
                      object_names_size - 1U),
                  0);
        memset(name, 0, sizeof(name));
        check_int("fallback survives rejected M564 table",
                  csb_v1_runtime_object_name(&profile, dagger, name, sizeof(name)),
                  1);
        check_str("fallback after rejected M564 table", name, "DAGGER");
    }

    if (g_failures != 0) return 1;
    printf("PASS: csb_v1_object_icon_resolver_pc34_compat\n");
    return 0;
}
