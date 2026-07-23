#ifndef FIRESTAFF_DM1_V1_P0601_P0650_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0601_P0650_PARAMETER_OWNER_AUDIT_H

#include <stddef.h>
#include <stdint.h>

typedef struct DM1_V1_P0601P0650ParameterOwnerAudit {
    uint16_t parameter_number;
    uint16_t owner_function;
    const char *source_anchor;
    int standalone_port_forbidden;
} DM1_V1_P0601P0650ParameterOwnerAudit;

const DM1_V1_P0601P0650ParameterOwnerAudit *
dm1_v1_p0601_p0650_parameter_owner_audit_pc34(size_t *out_count);
const DM1_V1_P0601P0650ParameterOwnerAudit *
dm1_v1_p0601_p0650_parameter_owner_find_pc34(uint16_t parameter_number,
                                               size_t occurrence);
const char *dm1_v1_p0601_p0650_parameter_owner_evidence_pc34(void);

#endif /* FIRESTAFF_DM1_V1_P0601_P0650_PARAMETER_OWNER_AUDIT_H */
