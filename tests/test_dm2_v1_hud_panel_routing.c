#include "dm2_v1_hud_panel_routing.h"

#include <stdio.h>
#include <string.h>

enum {
    DM2_TEST_GDAT_CATEGORY_WEAPONS = 0x10,
    DM2_TEST_GDAT_CATEGORY_MESSAGES = 0x03
};

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void test_query_cmdstr_text(void)
{
    const uint8_t weapon_text[] = "SWING:SK=48 DM=-5 HN=12";
    const uint8_t bad_text[] = {'N', 'O', '_', 'N', 'U', 'L'};
    const uint8_t message_text[] = "OPEN:LV=2";
    DM2_V1_QueryCmdstrTextReceipt text;

    expect_true(dm2_v1_QUERY_CMDSTR_TEXT(DM2_TEST_GDAT_CATEGORY_WEAPONS,
                                         2,
                                         8,
                                         weapon_text,
                                         sizeof(weapon_text),
                                         &text) == 1,
                "QUERY_CMDSTR_TEXT accepts a real GDAT dtText payload");
    expect_true(text.valid && text.source_locked &&
                    strcmp(text.symbol, "QUERY_CMDSTR_TEXT") == 0 &&
                    text.category == DM2_TEST_GDAT_CATEGORY_WEAPONS &&
                    text.index == 2u &&
                    text.field == 8u &&
                    text.byte_count == 23u &&
                    strcmp(text.text, "SWING:SK=48 DM=-5 HN=12") == 0 &&
                    text.text_hash != 0u,
                "QUERY_CMDSTR_TEXT receipt preserves source tuple and bytes");

    expect_true(dm2_v1_DM2_QUERY_CMDSTR_TEXT(
                    DM2_TEST_GDAT_CATEGORY_MESSAGES,
                    4,
                    7,
                    message_text,
                    sizeof(message_text),
                    &text) == 1,
                "DM2_QUERY_CMDSTR_TEXT exposes the SKULLWIN querydb alias");
    expect_true(text.valid &&
                    strcmp(text.symbol, "DM2_QUERY_CMDSTR_TEXT") == 0 &&
                    strcmp(text.text, "OPEN:LV=2") == 0,
                "DM2_QUERY_CMDSTR_TEXT copies only original text bytes");

    expect_true(dm2_v1_QUERY_CMDSTR_TEXT(DM2_TEST_GDAT_CATEGORY_WEAPONS,
                                         3,
                                         8,
                                         bad_text,
                                         sizeof(bad_text),
                                         &text) == 0,
                "QUERY_CMDSTR_TEXT blocks dtText without a NUL terminator");
    expect_true(text.blocked && !text.valid,
                "unterminated command text is fail-closed");
}

static void test_event_and_panel_routing(void)
{
    const uint8_t weapon_text[] = "SWING:SK=48 DM=-5 HN=12";
    DM2_V1_QueryCmdstrTextReceipt text;
    DM2_V1_TransmitUiEventReceipt event;
    DM2_V1_UpdateRightPanelReceipt panel;

    expect_true(dm2_v1_QUERY_CMDSTR_TEXT(DM2_TEST_GDAT_CATEGORY_WEAPONS,
                                         2,
                                         8,
                                         weapon_text,
                                         sizeof(weapon_text),
                                         &text) == 1,
                "test fixture command text loads before event routing");

    expect_true(dm2_v1_TRANSMIT_UI_EVENT(
                    DM2_V1_HUD_UI_EVENT_COMMAND_TEXT,
                    0x2222u,
                    &text,
                    &event) == 1,
                "TRANSMIT_UI_EVENT accepts command-text event with real text");
    expect_true(event.valid && event.requested_panel_refresh &&
                    event.consumed_real_cmdstr_text &&
                    !event.used_synthetic_text &&
                    event.category == DM2_TEST_GDAT_CATEGORY_WEAPONS &&
                    event.index == 2u &&
                    event.field == 8u &&
                    strcmp(event.symbol, "TRANSMIT_UI_EVENT") == 0,
                "TRANSMIT_UI_EVENT records real GDAT command text provenance");

    expect_true(dm2_v1_UPDATE_RIGHT_PANEL(
                    DM2_V1_HUD_RIGHT_PANEL_CONTAINER,
                    &event,
                    &panel) == 1,
                "UPDATE_RIGHT_PANEL consumes transmitted HUD event");
    expect_true(panel.valid &&
                    panel.consumed_transmitted_event &&
                    panel.consumed_real_cmdstr_text &&
                    panel.previous_mode == DM2_V1_HUD_RIGHT_PANEL_CONTAINER &&
                    panel.next_mode == DM2_V1_HUD_RIGHT_PANEL_COMMANDS &&
                    panel.redraw_requested &&
                    panel.payload_ref == 0x2222u &&
                    strcmp(panel.symbol, "UPDATE_RIGHT_PANEL") == 0,
                "UPDATE_RIGHT_PANEL selects command panel from command event");

    expect_true(dm2_v1_DM2_TRANSMIT_UI_EVENT(
                    DM2_V1_HUD_UI_EVENT_CONTAINER,
                    0x3300u,
                    0,
                    &event) == 1,
                "DM2_TRANSMIT_UI_EVENT accepts non-text container event");
    expect_true(dm2_v1_DM2_UPDATE_RIGHT_PANEL(
                    DM2_V1_HUD_RIGHT_PANEL_COMMANDS,
                    &event,
                    &panel) == 1,
                "DM2_UPDATE_RIGHT_PANEL exposes SKULLWIN alias");
    expect_true(panel.next_mode == DM2_V1_HUD_RIGHT_PANEL_CONTAINER &&
                    !panel.consumed_real_cmdstr_text &&
                    strcmp(panel.symbol, "DM2_UPDATE_RIGHT_PANEL") == 0,
                "DM2_UPDATE_RIGHT_PANEL records container panel routing");

    expect_true(dm2_v1_TRANSMIT_UI_EVENT(
                    DM2_V1_HUD_UI_EVENT_COMMAND_TEXT,
                    0x2222u,
                    0,
                    &event) == 0,
                "TRANSMIT_UI_EVENT blocks command-text event without real text");
    expect_true(event.blocked && !event.valid,
                "missing real command text does not create synthetic event text");
    expect_true(dm2_v1_UPDATE_RIGHT_PANEL(
                    DM2_V1_HUD_RIGHT_PANEL_COMMANDS,
                    &event,
                    &panel) == 0,
                "UPDATE_RIGHT_PANEL blocks invalid transmitted event");
}

int main(void)
{
    test_query_cmdstr_text();
    test_event_and_panel_routing();
    expect_true(strstr(dm2_v1_hud_panel_routing_source_evidence(),
                       "QUERY_CMDSTR_TEXT:8136") != 0,
                "source evidence includes SKWIN command text symbol");
    expect_true(strstr(dm2_v1_hud_panel_routing_source_evidence(),
                       "DM2_UPDATE_RIGHT_PANEL:5182") != 0,
                "source evidence includes SKULLWIN right-panel alias");
    if (failures) {
        return 1;
    }
    puts("DM2 HUD panel routing: ok");
    return 0;
}
