#ifndef FIRESTAFF_DM1_V1_P0701_P0750_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0701_P0750_PARAMETER_OWNER_AUDIT_H

#include <stddef.h>
#include <stdint.h>

typedef struct DM1_V1_P0701P0750ParameterOwnerAudit {
    uint16_t parameter_number;
    uint16_t owner_function;
    const char *source_anchor;
    int standalone_port_forbidden;
} DM1_V1_P0701P0750ParameterOwnerAudit;

const DM1_V1_P0701P0750ParameterOwnerAudit *
dm1_v1_p0701_p0750_parameter_owner_audit_pc34(size_t *out_count);
const DM1_V1_P0701P0750ParameterOwnerAudit *
dm1_v1_p0701_p0750_parameter_owner_find_pc34(uint16_t parameter_number,
                                               size_t occurrence);
const char *dm1_v1_p0701_p0750_parameter_owner_evidence_pc34(void);

#endif /* FIRESTAFF_DM1_V1_P0701_P0750_PARAMETER_OWNER_AUDIT_H */
