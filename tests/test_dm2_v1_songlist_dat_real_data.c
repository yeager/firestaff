/* Original-media SONGLIST.DAT regression.
 *
 * The selected corpus is explicit: FIRESTAFF_DM2_DATA_DIR must name the
 * directory containing the hash-admitted DOS files.  No HOME lookup or
 * generated map table is permitted here.
 *
 * SKProject: SKWINSPX/src/v5/dm2data.cpp:217-225 (`tblMusicsMap[64]`).
 */

#include "dm2_v1_songlist_dat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t *out, size_t capacity,
                     size_t *out_size)
{
    FILE *file;
    size_t size;

    if (out_size) *out_size = 0u;
    if (!path || !out || !out_size) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    size = fread(out, 1u, capacity + 1u, file);
    if (ferror(file) || fclose(file) != 0 || size > capacity) return 0;
    *out_size = size;
    return 1;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[1024];
    uint8_t bytes[DM2_SONGLIST_FILE_SIZE + 1u];
    size_t size = 0u;
    DM2_V1_SonglistDat songlist;
    size_t index;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR is not set");
        return 0;
    }
    if (snprintf(path, sizeof(path), "%s/songlist.dat", root) < 0 ||
        strlen(path) >= sizeof(path)) {
        fputs("FAIL: selected SONGLIST.DAT path is too long\n", stderr);
        return 1;
    }
    if (!read_file(path, bytes, DM2_SONGLIST_FILE_SIZE, &size)) {
        fprintf(stderr, "FAIL: cannot read selected original %s\n", path);
        return 1;
    }
    if (size != DM2_SONGLIST_FILE_SIZE ||
        !dm2_v1_songlist_dat_parse(&songlist, bytes, size)) {
        fputs("FAIL: selected original SONGLIST.DAT has no valid 63-byte layout\n",
              stderr);
        return 1;
    }
    if (dm2_v1_songlist_dat_track_for_map(&songlist, 0) != 0x02 ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 3) != 0x1b ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 29) != 0x00 ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 44) != 0x11 ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 45) != 0x02) {
        fputs("FAIL: selected original SONGLIST.DAT map selectors disagree with the source table\n",
              stderr);
        return 1;
    }
    for (index = 46u; index < DM2_SONGLIST_FILE_SIZE; ++index) {
        if (dm2_v1_songlist_dat_track_for_map(&songlist, (int)index) != -1) {
            fputs("FAIL: selected original SONGLIST.DAT no-music tail changed\n",
                  stderr);
            return 1;
        }
    }
    puts("PASS: selected original SONGLIST.DAT retains all 63 source selectors");
    return 0;
}
