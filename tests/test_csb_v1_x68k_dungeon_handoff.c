#include "csb_v1_x68k_dungeon_handoff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, unsigned char **out, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    long size;
    if (!fp || fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) <= 0 ||
        fseek(fp, 0, SEEK_SET) != 0 || !(*out = malloc((size_t)size)) ||
        fread(*out, 1u, (size_t)size, fp) != (size_t)size) {
        if (fp) fclose(fp);
        free(*out); return 0;
    }
    fclose(fp); *out_size = (size_t)size; return 1;
}

int main(int argc, char **argv) {
    unsigned char invalid[64] = {0};
    CSB_V1_DungeonData dungeon;
    if (csb_v1_x68k_hdm_load_dungeon(&dungeon, invalid, sizeof(invalid), NULL) !=
        CSB_V1_X68K_DUNGEON_HANDOFF_ERR_MEDIA) return 1;
    if (argc == 2) {
        unsigned char *hdm = NULL;
        size_t hdm_size = 0u;
        int level, x, y, direction;
        if (!read_file(argv[1], &hdm, &hdm_size) ||
            csb_v1_x68k_hdm_load_dungeon(&dungeon, hdm, hdm_size, NULL) !=
                CSB_V1_X68K_DUNGEON_HANDOFF_OK ||
            dungeon.square_bytes != 1 || dungeon.level_count != 2 ||
            !csb_v1_dungeon_initial_party_pose_pc34(&dungeon, &level, &x, &y,
                                                     &direction) ||
            level != 0 || x != 9 || y != 0 || direction != 2) {
            free(hdm); csb_v1_dungeon_free(&dungeon); return 1;
        }
        printf("PASS: original CSB X68000 DUNGEON.DAT: levels=%d party=%d,%d,%d,%d\n",
               dungeon.level_count, level, x, y, direction);
        free(hdm); csb_v1_dungeon_free(&dungeon);
    }
    puts("test_csb_v1_x68k_dungeon_handoff: PASS");
    return 0;
}
