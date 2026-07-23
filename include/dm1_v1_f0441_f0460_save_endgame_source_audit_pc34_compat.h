#ifndef FIRESTAFF_DM1_V1_F0441_F0460_SAVE_ENDGAME_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0441_F0460_SAVE_ENDGAME_SOURCE_AUDIT_PC34_COMPAT_H
#include <stdint.h>
typedef struct { uint16_t functionNumber; int hasEstablishedOwner; int hostFallbackForbidden; const char *source; const char *owner; } DM1_V1_F0441F0460SourceAuditPc34Compat;
const DM1_V1_F0441F0460SourceAuditPc34Compat *dm1_v1_f0441_f0460_save_endgame_source_audit_pc34(uint16_t function_number);
#endif
