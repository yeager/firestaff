#include "csb_v1_atari_save_decode_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *fp;
    long length;
    uint8_t *bytes;
    if (!path || !out || !out_size) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0 || (length = ftell(fp)) < 1 ||
        fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0; }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1u, (size_t)length, fp) != (size_t)length) {
        free(bytes); fclose(fp); return 0;
    }
    fclose(fp); *out = bytes; *out_size = (size_t)length; return 1;
}

int main(void)
{
    uint8_t tiny[640] = { 0 };
    const char *path = getenv("FIRESTAFF_CSB_ATARI_MINI");
    uint8_t *bytes = NULL;
    size_t size = 0u;
    CSB_V1_AtariSaveInfo info;
    CSB_V1_DungeonData dungeon;
    CSB_V1_PartyState party;

    if (csb_v1_atari_save_decode_pc34_compat(NULL, 0u, &info) != CSB_V1_ATARI_SAVE_ERR_NULL ||
        csb_v1_atari_save_decode_pc34_compat(tiny, sizeof(tiny), &info) != CSB_V1_ATARI_SAVE_ERR_BLOCK2_CHECKSUM) {
        return 1;
    }
    if (!path || !path[0]) {
        puts("SKIP: FIRESTAFF_CSB_ATARI_MINI is not set");
        return 0;
    }
    if (!read_file(path, &bytes, &size) ||
        csb_v1_atari_save_load_dungeon_pc34_compat(bytes, size, &dungeon, &info) != CSB_V1_ATARI_SAVE_OK ||
        info.champion_count != 1 || info.party_x != 22 || info.party_y != 18 ||
        info.party_direction != 2 || info.party_map_index != 4 ||
        info.dungeon_offset != 10160u || info.dungeon_size != 32655u ||
        dungeon.level_count != 11 || !dungeon.raw_data) {
        free(bytes); return 1;
    }
    if (csb_v1_atari_save_decode_party_pc34_compat(bytes, size, &party, NULL) !=
            CSB_V1_ATARI_SAVE_OK || party.ChampionCount != 1 ||
        strcmp(party.Champions[0].Name, "HALK") != 0 ||
        strcmp(party.Champions[0].Title, "GONZO BARBARIAN") != 0 ||
        party.Champions[0].CurrentHealth != 602 ||
        party.Champions[0].MaximumStamina != 1262 ||
        party.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] != 82u ||
        party.Champions[0].Slots[0] != 0xffffu) {
        csb_v1_dungeon_free(&dungeon); free(bytes); return 1;
    }
    csb_v1_dungeon_free(&dungeon);
    free(bytes);
    puts("PASS: original Atari CSB MINI.DAT decrypts to its source dungeon");
    return 0;
}
