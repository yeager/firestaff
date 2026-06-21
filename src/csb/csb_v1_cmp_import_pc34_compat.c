/*
 * csb_v1_cmp_import_pc34_compat.c
 *
 * CSB V1 Utility Disk Champion Portrait (.CMP) import glue.
 * See csb_v1_cmp_import_pc34_compat.h for the contract.
 */

#include "csb_v1_cmp_import_pc34_compat.h"

#include <string.h>

/* Number of portrait bytes we copy from a CMP file into the
 * champion's portrait slot. The CMP file holds a 464-byte
 * portrait; the CSB_V1_Champion.Portrait slot is 3712 bytes
 * (CSB_V1_PORTRAIT_BYTE_COUNT). The CMP-encoded 464 bytes
 * are placed at the start of the slot; the remaining bytes
 * are left untouched (typically zero, after a prior
 * csb_v1_champion_init() call). */
#define CMP_PORTRAIT_BYTES   464u

int csb_v1_cmp_import_champion(CSB_V1_Champion* champion,
                                const uint8_t*   cmp_data,
                                size_t           cmp_size)
{
    if (!champion || !cmp_data) return -1;

    FirestaffCmp cmp;
    int rc = FirestaffCmp_Decode(cmp_data, cmp_size, &cmp);
    if (rc != 0) {
        /* Map FirestaffCmp_Decode's negative return values to
         * our import error codes:
         *   -1 (invalid args)  -> -1
         *   -2 (bad magic)     -> -2
         *   -3 (bad name/title)-> -3
         * FirestaffCmp_Decode already validated cmp_size >= 496
         * before returning, so a -1 from it on this side means
         * our data was malformed in some other way. */
        return rc;
    }

    /* Copy Name (8 bytes, null-padded) and Title (20 bytes,
     * null-padded) into the champion's fixed-width slots. */
    memset(champion->Name, 0, sizeof(champion->Name));
    memcpy(champion->Name, cmp.name, FIRESTAFF_CMP_NAME_SIZE);
    /* Ensure trailing NUL for string ops. The Name slot is
     * CSB_V1_MAX_NAME_LEN+1 = 16 bytes; cmp.name is 8 bytes.
     * The remaining bytes stay zero. */
    champion->Name[sizeof(champion->Name) - 1] = '\0';

    memset(champion->Title, 0, sizeof(champion->Title));
    memcpy(champion->Title, cmp.title, FIRESTAFF_CMP_TITLE_SIZE);
    champion->Title[sizeof(champion->Title) - 1] = '\0';

    /* Copy the 464-byte portrait into the start of the
     * champion's portrait slot. The slot is 3712 bytes; the
     * trailing bytes are intentionally left untouched. */
    if (cmp.portrait && cmp.portrait_size >= CMP_PORTRAIT_BYTES) {
        memcpy(champion->Portrait, cmp.portrait, CMP_PORTRAIT_BYTES);
    }

    return 0;
}

int csb_v1_cmp_import_to_party(CSB_V1_PartyState* party,
                                const uint8_t*     cmp_data,
                                size_t             cmp_size)
{
    if (!party) return -1;

    if (party->ChampionCount < 0 ||
        party->ChampionCount >= CSB_V1_MAX_CHAMPIONS) {
        return -4;
    }

    int slot = party->ChampionCount;
    CSB_V1_Champion* champion = &party->Champions[slot];

    /* Initialise the slot first so vitals/stats/skills are
     * at defaults and the portrait buffer is zeroed. This
     * makes the CMP overlay deterministic. */
    csb_v1_champion_init(champion);

    int rc = csb_v1_cmp_import_champion(champion, cmp_data, cmp_size);
    if (rc != 0) return rc;

    party->ChampionCount = slot + 1;
    return slot;
}

int csb_v1_cmp_import_self_test(void)
{
    /* Build a synthetic CMP file. Mirrors the format the
     * CSB Utility Disk Champion Editor writes. */
    uint8_t cmp_buf[FIRESTAFF_CMP_FILE_SIZE];
    memset(cmp_buf, 0, sizeof(cmp_buf));
    /* cmp_i_C and cmp_i_E are already 0. */
    const char* name  = "HECTOR";
    const char* title = "WARRIOR";
    memcpy(cmp_buf + 4, name, 6);
    memcpy(cmp_buf + 4 + FIRESTAFF_CMP_NAME_SIZE, title, 7);
    /* Fill portrait with a recognisable pattern so we can
     * verify the copy in the test. */
    memset(cmp_buf + 4 + FIRESTAFF_CMP_NAME_SIZE + FIRESTAFF_CMP_TITLE_SIZE,
           0xA5, CMP_PORTRAIT_BYTES);

    /* 1. Valid import into a single champion. */
    {
        CSB_V1_Champion champ;
        csb_v1_champion_init(&champ);
        int rc = csb_v1_cmp_import_champion(&champ, cmp_buf, sizeof(cmp_buf));
        if (rc != 0) return -1;
        if (strcmp(champ.Name, "HECTOR") != 0) return -1;
        if (strcmp(champ.Title, "WARRIOR") != 0) return -1;
        if (champ.Portrait[0] != 0xA5) return -1;
        if (champ.Portrait[CMP_PORTRAIT_BYTES - 1] != 0xA5) return -1;
    }

    /* 2. Bad magic (cmp_i_C != 0). */
    {
        CSB_V1_Champion champ;
        csb_v1_champion_init(&champ);
        uint8_t bad_buf[FIRESTAFF_CMP_FILE_SIZE];
        memcpy(bad_buf, cmp_buf, sizeof(bad_buf));
        bad_buf[0] = 0x42;  /* cmp_i_C big-endian high byte */
        bad_buf[1] = 0x42;  /* cmp_i_C big-endian low byte */
        int rc = csb_v1_cmp_import_champion(&champ, bad_buf, sizeof(bad_buf));
        if (rc != -2) return -1;
    }

    /* 3. Bad name (lowercase). */
    {
        CSB_V1_Champion champ;
        csb_v1_champion_init(&champ);
        uint8_t bad_buf[FIRESTAFF_CMP_FILE_SIZE];
        memcpy(bad_buf, cmp_buf, sizeof(bad_buf));
        bad_buf[4] = 'h';  /* lowercase 'h' */
        int rc = csb_v1_cmp_import_champion(&champ, bad_buf, sizeof(bad_buf));
        if (rc != -3) return -1;
    }

    /* 4. NULL inputs. */
    {
        int rc = csb_v1_cmp_import_champion(NULL, cmp_buf, sizeof(cmp_buf));
        if (rc != -1) return -1;
        CSB_V1_Champion champ;
        rc = csb_v1_cmp_import_champion(&champ, NULL, sizeof(cmp_buf));
        if (rc != -1) return -1;
    }

    /* 5. Party import: insert into a fresh party. */
    {
        CSB_V1_PartyState party;
        memset(&party, 0, sizeof(party));
        int slot = csb_v1_cmp_import_to_party(&party, cmp_buf, sizeof(cmp_buf));
        if (slot != 0) return -1;
        if (party.ChampionCount != 1) return -1;
        if (strcmp(party.Champions[0].Name, "HECTOR") != 0) return -1;
        if (party.Champions[0].Portrait[0] != 0xA5) return -1;
    }

    /* 6. Party import: fill all 4 slots. */
    {
        CSB_V1_PartyState party;
        memset(&party, 0, sizeof(party));
        for (int i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
            int slot = csb_v1_cmp_import_to_party(&party, cmp_buf,
                                                   sizeof(cmp_buf));
            if (slot != i) return -1;
        }
        if (party.ChampionCount != CSB_V1_MAX_CHAMPIONS) return -1;
    }

    /* 7. Party import: party-full failure. */
    {
        CSB_V1_PartyState party;
        memset(&party, 0, sizeof(party));
        party.ChampionCount = CSB_V1_MAX_CHAMPIONS;
        int rc = csb_v1_cmp_import_to_party(&party, cmp_buf, sizeof(cmp_buf));
        if (rc != -4) return -1;
    }

    return 0;
}