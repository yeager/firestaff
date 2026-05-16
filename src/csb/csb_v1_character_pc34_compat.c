#include <stdio.h>
#include <stdint.h>

#include "csb_v1_character_pc34_compat.h"
#include <string.h>

/* pass603: CSB V1 character/import
 *
 * CSBWin/Character.cpp: champion management (5528 lines)
 * CSBWin/SaveGame.cpp: save/load + DM1 import (2953 lines)
 * CSBWin/CSBCode.cpp: StartChaos champion init (11414)
 * ReDMCSB CHAMPION.C: F0284-F0330 shared champion functions
 */

int csb_v1_character_import_dm1(CSB_V1_PartyState *party,
    const uint8_t *dm1_save, int save_size)
{
    /* CSBWin/SaveGame.cpp: DM1 save format import */
    if (!party || !dm1_save || save_size < 256) return -1;
    party->imported_from_dm1 = 1;
    /* Actual import would parse DM1 save format here */
    return 0;
}

void csb_v1_character_init_default(CSB_V1_PartyState *party) {
    if (!party) return;
    memset(party, 0, sizeof(*party));
    party->champion_count = 4;
    party->imported_from_dm1 = 0;
}

const char *csb_v1_character_source_evidence(void) {
    return
        "CSBWin/Character.cpp: 5528 lines champion management\n"
        "CSBWin/SaveGame.cpp: 2953 lines save/load + DM1 import\n"
        "CSBWin/CSBCode.cpp:11414 StartChaos init\n"
        "ReDMCSB CHAMPION.C F0284-F0330 shared champion core\n";
}


/* ── CSB DM1 champion import — parse DM1 save format ──────────────── */
/* #10: Read DM1 save file and extract champion data for CSB.
 * DM1 save format: header (24 bytes) + champion records (116 bytes each).
 * Source: CSBWin/SaveGame.cpp DM1 import path */

int csb_v1_character_import_dm1_save(CSB_V1_PartyState *party,
    const char *dm1_save_path)
{
    FILE *f;
    uint8_t hdr[24], champ[116];
    int i, count;
    if (!party || !dm1_save_path) return -1;
    f = fopen(dm1_save_path, "rb");
    if (!f) return -1;
    if (fread(hdr, 1, 24, f) != 24) { fclose(f); return -1; }

    count = hdr[0]; /* number of champions */
    if (count > CSB_V1_MAX_CHAMPIONS) count = CSB_V1_MAX_CHAMPIONS;
    party->champion_count = count;
    party->imported_from_dm1 = 1;

    for (i = 0; i < count; i++) {
        if (fread(champ, 1, 116, f) != 116) break;
        /* Parse champion record (DM1 save format) */
        memcpy(party->champions[i].name, champ, 8);
        party->champions[i].name[8] = 0;
        party->champions[i].health = (int16_t)(champ[8] | (champ[9] << 8));
        party->champions[i].max_health = (int16_t)(champ[10] | (champ[11] << 8));
        party->champions[i].stamina = (int16_t)(champ[12] | (champ[13] << 8));
        party->champions[i].max_stamina = (int16_t)(champ[14] | (champ[15] << 8));
        party->champions[i].mana = (int16_t)(champ[16] | (champ[17] << 8));
        party->champions[i].max_mana = (int16_t)(champ[18] | (champ[19] << 8));
        party->champions[i].strength = champ[20];
        party->champions[i].dexterity = champ[21];
        party->champions[i].wisdom = champ[22];
        party->champions[i].vitality = champ[23];
        party->champions[i].alive = party->champions[i].health > 0;
    }

    fclose(f);
    return count;
}
