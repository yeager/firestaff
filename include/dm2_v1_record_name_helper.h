#ifndef FIRESTAFF_DM2_V1_RECORD_NAME_HELPER_H
#define FIRESTAFF_DM2_V1_RECORD_NAME_HELPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint16_t object_id;
    uint8_t record_type;
    const char *record_name;
    const char *symbol;
    const char *source_path;
} DM2_V1_RecordNameReceipt;

void dm2_v1_record_name_receipt_clear(DM2_V1_RecordNameReceipt *receipt);

int dm2_v1_getRecordNameOf(uint16_t object_id,
                           DM2_V1_RecordNameReceipt *out_receipt);

const char *dm2_v1_getRecordNameOf_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RECORD_NAME_HELPER_H */
