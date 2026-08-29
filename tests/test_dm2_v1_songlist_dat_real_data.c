/* Original-media SONGLIST.DAT regression.
 *
 * The selected corpus is explicit: FIRESTAFF_DM2_SONGLIST_ZIP names the
 * supplied DOS archive.  The source member is read directly to RAM; no HOME
 * lookup, extracted data directory, or generated map table is permitted.
 *
 * SKProject: SKWINSPX/src/v5/dm2data.cpp:217-225 (`tblMusicsMap[64]`).
 */

#include "dm2_v1_songlist_dat.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_SONGLIST_ZIP");
    uint8_t *bytes = NULL;
    size_t size = 0u;
    DM2_V1_SonglistDat songlist;
    size_t index;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_SONGLIST_ZIP is not set");
        return 77;
    }
    if (firestaff_zip_extract_by_suffix(archive, "data/songlist.dat",
                                        &bytes, &size) != 0 || !bytes) {
        fprintf(stderr, "FAIL: cannot read source data/songlist.dat from %s\n",
                archive);
        return 1;
    }
    if (size != DM2_SONGLIST_FILE_SIZE ||
        !dm2_v1_songlist_dat_parse(&songlist, bytes, size)) {
        fputs("FAIL: selected original SONGLIST.DAT has no valid 63-byte layout\n",
              stderr);
        free(bytes);
        return 1;
    }
    if (dm2_v1_songlist_dat_track_for_map(&songlist, 0) != 0x02 ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 3) != 0x1b ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 29) != 0x00 ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 44) != 0x11 ||
        dm2_v1_songlist_dat_track_for_map(&songlist, 45) != 0x02) {
        fputs("FAIL: selected original SONGLIST.DAT map selectors disagree with the source table\n",
              stderr);
        free(bytes);
        return 1;
    }
    for (index = 46u; index < DM2_SONGLIST_FILE_SIZE; ++index) {
        if (dm2_v1_songlist_dat_track_for_map(&songlist, (int)index) != -1) {
            fputs("FAIL: selected original SONGLIST.DAT no-music tail changed\n",
                  stderr);
            free(bytes);
            return 1;
        }
    }
    free(bytes);
    puts("PASS: selected original SONGLIST.DAT retains all 63 source selectors");
    return 0;
}
