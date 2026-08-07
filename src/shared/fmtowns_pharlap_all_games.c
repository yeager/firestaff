#include "fmtowns_pharlap_all_games.h"
#include <string.h>

/* Byte-verified from Track 01 extraction 2026-08-07. */
const fmtowns_pharlap_binary_profile_t
fmtowns_pharlap_binary_profiles[FMTOWNS_PHARLAP_BINARY_COUNT] = {
    /* DM1 Japanese Rev 1 (HMA-240 1992) */
    { "EDM.EXP",   "DM1", 310518, 0x42a48, 0x46b41, 0x51b5,
      70, 20, 1, 2, 93 },
    { "JDM.EXP",   "DM1", 290221, 0x42cb4,      0,      0,
      70, 20, 1, 2, 93 },
    { "TMENU.EXP", "DM1", 235476,  0x9408,      0,      0,
      70, 20, 1, 1, 92 },

    /* CSB Victor 1990 */
    { "CHTWE.EXP",     "CSB", 283936, 0x2619c,      0,     0,
      17, 11, 0, 2, 30 },
    { "CHTWJ.EXP",     "CSB", 284416, 0x26348,      0,     0,
      17, 11, 0, 2, 30 },
    { "SWITCHTW.EXP",  "CSB",  90456, 0x14a88,      0,     0,
      13, 11, 0, 2, 26 },
    { "ANIMTW.EXP",    "CSB", 124161, 0x1cfbc,      0,     0,
      13,  0, 0, 2, 15 },
    { "TMENU.EXP",     "CSB", 243212, 0x18c2c, 0x33a92, 31610,
      26, 14, 1, 8, 49 },
    { "UTILE.EXP",     "CSB", 152387,  0xfe00,      0,     0,
      17, 11, 0, 2, 30 },

    /* DM2 Skull Keep Victor 1993 */
    { "SKULL.EXP",  "DM2", 374416, 0x5741c,      0,     0,
      17, 10, 0, 2, 29 },
    { "TWANIM.EXP", "DM2",  72184, 0x10470,      0,     0,
      13,  0, 0, 2, 15 },
};

const fmtowns_pharlap_binary_profile_t *
fmtowns_pharlap_binary_profile_for_name_pc34(const char *name) {
    unsigned int i;
    if (!name) return NULL;
    for (i = 0; i < FMTOWNS_PHARLAP_BINARY_COUNT; ++i) {
        if (strcmp(fmtowns_pharlap_binary_profiles[i].name, name) == 0) {
            return &fmtowns_pharlap_binary_profiles[i];
        }
    }
    return NULL;
}

int fmtowns_pharlap_all_binaries_use_canonical_slots_only_pc34(void) {
    unsigned int i;
    for (i = 0; i < FMTOWNS_PHARLAP_BINARY_COUNT; ++i) {
        const fmtowns_pharlap_binary_profile_t *p =
            &fmtowns_pharlap_binary_profiles[i];
        uint32_t sum = p->slot_tbios + p->slot_secondary +
                       p->slot_timing + p->slot_hardware_init;
        if (sum != p->slot_total) return 0;
    }
    return 1;
}
