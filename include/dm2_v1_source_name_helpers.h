#ifndef FIRESTAFF_DM2_V1_SOURCE_NAME_HELPERS_H
#define FIRESTAFF_DM2_V1_SOURCE_NAME_HELPERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint16_t value;
    const char *name;
    const char *symbol;
    const char *source_path;
} DM2_V1_SourceNameReceipt;

void dm2_v1_source_name_receipt_clear(DM2_V1_SourceNameReceipt *receipt);

int dm2_v1_getSpellTypeName(uint16_t spell_type,
                            DM2_V1_SourceNameReceipt *out_receipt);
int dm2_v1_getSkillName(uint16_t skill,
                        DM2_V1_SourceNameReceipt *out_receipt);
int dm2_v1_getStatBonusName(uint16_t stat_bonus,
                            DM2_V1_SourceNameReceipt *out_receipt);

const char *dm2_v1_source_name_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SOURCE_NAME_HELPERS_H */
