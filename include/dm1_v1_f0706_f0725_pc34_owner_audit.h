#ifndef FIRESTAFF_DM1_V1_F0706_F0725_PC34_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F0706_F0725_PC34_OWNER_AUDIT_H
#include <stdint.h>
typedef struct { uint16_t n; int proven; int failClosed; } DM1V1F0706F0725;
const DM1V1F0706F0725 *dm1_v1_f0706_f0725_pc34_owner_audit(uint16_t n);
#endif
