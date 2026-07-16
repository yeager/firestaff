#ifndef FIRESTAFF_DM2_V1_UI_EVENT_NAME_HELPER_H
#define FIRESTAFF_DM2_V1_UI_EVENT_NAME_HELPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint16_t event_code;
    const char *event_name;
    const char *symbol;
    const char *source_path;
} DM2_V1_UiEventNameReceipt;

void dm2_v1_ui_event_name_receipt_clear(
    DM2_V1_UiEventNameReceipt *receipt);

int dm2_v1_getUIEventName(uint16_t event_code,
                          DM2_V1_UiEventNameReceipt *out_receipt);

const char *dm2_v1_getUIEventName_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_UI_EVENT_NAME_HELPER_H */
