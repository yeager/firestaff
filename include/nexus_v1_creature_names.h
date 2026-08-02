
#ifndef NEXUS_V1_CREATURE_NAMES_H
#define NEXUS_V1_CREATURE_NAMES_H

/* Nexus creature MNS model filename table — DM.BIN 0x0385F0.
 * 30 entries matching the 30 CRET creature type records from RLOWFIX.BIN.
 * Indices 16 (SN_FLOOR.MNS) and 17 (SN_WALL.MNS) are material files,
 * not creature models — they share the MNS format but serve a different role.
 * Source: yam\monsobj.c debug strings at DM.BIN 0x038588. */

#define NEXUS_CREATURE_NAME_COUNT 30

const char* nexus_v1_creature_mns_name(int cret_index);

#endif
