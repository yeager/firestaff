#ifndef FIRESTAFF_DM1_V1_F0621_F0645_CHAMPION_LAYOUT_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0621_F0645_CHAMPION_LAYOUT_SOURCE_AUDIT_PC34_COMPAT_H
#include <stdint.h>
typedef struct { uint16_t n; int requirePc34; int failClosed; } DM1V1F0621F0645;
const DM1V1F0621F0645 *dm1_v1_f0621_f0645_champion_layout_source_audit_pc34(uint16_t n);
#endif
