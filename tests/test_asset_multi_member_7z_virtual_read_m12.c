#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_CSB_ATARI_MULTI_7Z");
    static const char *const member_names[] = {
        "HardDisk/2009-02-22 PP/HCSB.DAT",
        "HardDisk/2009-02-22 PP/HCSB.HTC",
        "HardDisk/2009-02-22 PP/MINI.DAT"
    };
    static const char *const member_md5[] = {
        "708e113c869ab922633e885aa72a3c77",
        "8ce69b54cf255a15e98e909bb45b9742",
        "531ea104a2fbc2011ea73d11f274c57d"
    };
    static const size_t member_sizes[] = {30793u, 66172u, 42815u};
    char member_paths[3][ASSET_PATH_MAX];
    char found_paths[3][ASSET_PATH_MAX];
    int found[3] = {0, 0, 0};
    int i;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic CSB Atari multi-member 7z is not configured");
        return 77;
    }

    for (i = 0; i < 3; ++i) {
        char actual_md5[33];
        uint8_t *bytes = NULL;
        size_t byte_count = 0u;
        if (snprintf(member_paths[i], sizeof(member_paths[i]), "%s::%s",
                     archive, member_names[i]) < 0 ||
            !asset_read_path_alloc(member_paths[i], &bytes, &byte_count) ||
            !bytes || byte_count != member_sizes[i] ||
            !asset_file_md5_hex(member_paths[i], actual_md5) ||
            strcmp(actual_md5, member_md5[i]) != 0) {
            free(bytes);
            fprintf(stderr, "FAIL: native multi-member 7z read for %s\n",
                    member_names[i]);
            return 1;
        }
        free(bytes);
    }

    if (asset_find_all_by_md5_list(archive, member_md5, found_paths, found,
                                   3, 0) != 3) {
        fputs("FAIL: asset scanner did not find all authenticated CSB R1 files\n",
              stderr);
        return 1;
    }
    for (i = 0; i < 3; ++i) {
        char actual_md5[33];
        size_t archive_length = strlen(archive);
        if (!found[i] || strncmp(found_paths[i], archive, archive_length) != 0 ||
            found_paths[i][archive_length] != ':' ||
            !asset_file_md5_hex(found_paths[i], actual_md5) ||
            strcmp(actual_md5, member_md5[i]) != 0) {
            fprintf(stderr, "FAIL: archive scanner returned no authenticated "
                            "member for %s\n", member_names[i]);
            return 1;
        }
    }

    {
        char virtual_path[ASSET_PATH_MAX * 2];
        uint8_t *bytes = NULL;
        size_t byte_count = 0u;
        if (snprintf(virtual_path, sizeof(virtual_path),
                     "%s::Floppy Disks STX/Chaos Strikes Back for Atari ST "
                     "Game Disk v2.1 (English).stx::START.PRG", archive) < 0 ||
            !asset_read_virtual_path_alloc(virtual_path, &bytes, &byte_count) ||
            !bytes || byte_count < 2u || bytes[0] != 0x60u ||
            bytes[1] != 0x1au) {
            free(bytes);
            fputs("FAIL: nested STX member from the solid 7z was not readable\n",
                  stderr);
            return 1;
        }
        free(bytes);
    }

    puts("M12 authentic CSB Atari multi-member 7z read and scan: PASS");
    return 0;
}
