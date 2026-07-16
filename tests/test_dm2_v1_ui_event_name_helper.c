#include "dm2_v1_ui_event_name_helper.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void expect_event_name(uint16_t event_code, const char *event_name)
{
    DM2_V1_UiEventNameReceipt receipt;

    expect_true(dm2_v1_getUIEventName(event_code, &receipt) == 1,
                "getUIEventName accepts source-locked event code");
    expect_true(receipt.handled && receipt.source_locked && receipt.valid &&
                    !receipt.blocked &&
                    receipt.event_code == event_code &&
                    strcmp(receipt.event_name, event_name) == 0 &&
                    strcmp(receipt.symbol, "getUIEventName") == 0 &&
                    strcmp(receipt.source_path,
                           "SKWIN/SkWinCore.cpp:558") == 0,
                "getUIEventName records event name and source provenance");
}

int main(void)
{
    DM2_V1_UiEventNameReceipt receipt;

    expect_event_name(16u, "UI_EVENT_SPELL");
    expect_event_name(95u, "UI_EVENT_LEADER_HAND");
    expect_event_name(116u, "UI_EVENT_CHAMPION_HAND_0");
    expect_event_name(117u, "UI_EVENT_CHAMPION_HAND_1");
    expect_event_name(120u, "UI_EVENT_PLAYER2_HAND_0");
    expect_event_name(0x00d7u, "UI_EVENTCODE_START_NEW_GAME");
    expect_event_name(0x00d9u, "UI_EVENTCODE_RESUME_GAME");

    expect_true(dm2_v1_getUIEventName(0x1234u, &receipt) == 0 &&
                    receipt.handled && receipt.source_locked &&
                    receipt.blocked && !receipt.valid &&
                    receipt.event_name == NULL,
                "getUIEventName rejects unknown event code without fallback name");
    expect_true(dm2_v1_getUIEventName(16u, NULL) == 0,
                "getUIEventName rejects missing receipt");
    dm2_v1_ui_event_name_receipt_clear(&receipt);
    expect_true(!receipt.valid && receipt.event_name == NULL,
                "UI event-name receipt clear resets output");
    expect_true(strstr(dm2_v1_getUIEventName_source_evidence(),
                       "getUIEventName:558") != NULL,
                "source evidence names skproject symbol line");

    if (failures) {
        return 1;
    }
    puts("DM2 UI event-name helper: ok");
    return 0;
}
