#include "dm2_v1_ui_event_name_helper.h"

#include <string.h>

typedef struct {
    uint16_t event_code;
    const char *event_name;
} DM2_V1_UiEventName;

static const DM2_V1_UiEventName dm2_v1_ui_event_names[] = {
    { 16u, "UI_EVENT_SPELL" },
    { 50u, "UI_EVENT_UNADJUSTED" },
    { 95u, "UI_EVENT_LEADER_HAND" },
    { 116u, "UI_EVENT_CHAMPION_HAND_0" },
    { 117u, "UI_EVENT_CHAMPION_HAND_1" },
    { 120u, "UI_EVENT_PLAYER2_HAND_0" },
    { 0x00d7u, "UI_EVENTCODE_START_NEW_GAME" },
    { 0x00d9u, "UI_EVENTCODE_RESUME_GAME" }
};

void dm2_v1_ui_event_name_receipt_clear(
    DM2_V1_UiEventNameReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int dm2_v1_getUIEventName(uint16_t event_code,
                          DM2_V1_UiEventNameReceipt *out_receipt)
{
    size_t i;

    dm2_v1_ui_event_name_receipt_clear(out_receipt);
    if (!out_receipt) {
        return 0;
    }

    out_receipt->handled = 1;
    out_receipt->source_locked = 1;
    out_receipt->event_code = event_code;
    out_receipt->symbol = "getUIEventName";
    out_receipt->source_path = "SKWIN/SkWinCore.cpp:558";

    for (i = 0u; i < sizeof(dm2_v1_ui_event_names) /
                       sizeof(dm2_v1_ui_event_names[0]); ++i) {
        if (dm2_v1_ui_event_names[i].event_code == event_code) {
            out_receipt->valid = 1;
            out_receipt->event_name = dm2_v1_ui_event_names[i].event_name;
            return 1;
        }
    }

    out_receipt->blocked = 1;
    return 0;
}

const char *dm2_v1_getUIEventName_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp getUIEventName:558; "
           "bounded HUD/UI event-name receipt for event codes already "
           "source-locked by ADJUST_UI_EVENT and HANDLE_UI_EVENT. Unknown "
           "codes fail closed instead of receiving fallback labels.";
}
