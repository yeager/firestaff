#include "dm2_v1_hud_panel_routing.h"

#include <string.h>

static void dm2_query_cmdstr_text_begin(
    DM2_V1_QueryCmdstrTextReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_query_cmdstr_text_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static void dm2_transmit_ui_event_begin(
    DM2_V1_TransmitUiEventReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_transmit_ui_event_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static void dm2_update_right_panel_begin(
    DM2_V1_UpdateRightPanelReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_update_right_panel_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

void dm2_v1_query_cmdstr_text_receipt_clear(
    DM2_V1_QueryCmdstrTextReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

void dm2_v1_transmit_ui_event_receipt_clear(
    DM2_V1_TransmitUiEventReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

void dm2_v1_update_right_panel_receipt_clear(
    DM2_V1_UpdateRightPanelReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

static int dm2_query_cmdstr_text_impl(
    int category,
    int index,
    int field,
    const uint8_t *text,
    size_t text_size,
    DM2_V1_QueryCmdstrTextReceipt *out_receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_query_cmdstr_text_begin(out_receipt, symbol, source_path);
    /* QUERY_CMDSTR_TEXT reads GDAT itself in the original engine
     * (SKULLWIN/c_querydb.cpp:274; SKWIN/SkWinCore.cpp:8136).  This old
     * adapter accepted arbitrary host bytes and falsely recorded them as a
     * GDAT payload.  A category/index/field tuple alone cannot authenticate
     * such a buffer, so preserve the source boundary: block until a mounted
     * loader can supply the raw GDAT record and its provenance receipt. */
    (void)text;
    (void)text_size;
    if (out_receipt) {
        out_receipt->category = (uint8_t)category;
        out_receipt->index = (uint8_t)index;
        out_receipt->field = (uint8_t)field;
        out_receipt->blocked = 1;
    }
    return 0;
}

int dm2_v1_QUERY_CMDSTR_TEXT(
    int category,
    int index,
    int field,
    const uint8_t *text,
    size_t text_size,
    DM2_V1_QueryCmdstrTextReceipt *out_receipt)
{
    return dm2_query_cmdstr_text_impl(category,
                                      index,
                                      field,
                                      text,
                                      text_size,
                                      out_receipt,
                                      "QUERY_CMDSTR_TEXT",
                                      "SKWIN/SkWinCore.cpp:8136");
}

int dm2_v1_DM2_QUERY_CMDSTR_TEXT(
    int category,
    int index,
    int field,
    const uint8_t *text,
    size_t text_size,
    DM2_V1_QueryCmdstrTextReceipt *out_receipt)
{
    return dm2_query_cmdstr_text_impl(category,
                                      index,
                                      field,
                                      text,
                                      text_size,
                                      out_receipt,
                                      "DM2_QUERY_CMDSTR_TEXT",
                                      "SKULLWIN/c_querydb.cpp:274");
}

static int dm2_transmit_ui_event_impl(
    DM2_V1_HudUiEventKind event_kind,
    uint16_t payload_ref,
    const DM2_V1_QueryCmdstrTextReceipt *cmdstr_text,
    DM2_V1_TransmitUiEventReceipt *out_receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_transmit_ui_event_begin(out_receipt, symbol, source_path);
    if (event_kind == DM2_V1_HUD_UI_EVENT_NONE ||
        event_kind > DM2_V1_HUD_UI_EVENT_CHAMPION) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->event_kind = (uint8_t)event_kind;
            out_receipt->payload_ref = payload_ref;
        }
        return 0;
    }
    if (event_kind == DM2_V1_HUD_UI_EVENT_COMMAND_TEXT &&
        (!cmdstr_text || !cmdstr_text->valid)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->event_kind = (uint8_t)event_kind;
            out_receipt->payload_ref = payload_ref;
        }
        return 0;
    }
    if (out_receipt) {
        out_receipt->event_kind = (uint8_t)event_kind;
        out_receipt->payload_ref = payload_ref;
        out_receipt->requested_panel_refresh = 1u;
        if (cmdstr_text && cmdstr_text->valid) {
            out_receipt->consumed_real_cmdstr_text = 1u;
            out_receipt->category = cmdstr_text->category;
            out_receipt->index = cmdstr_text->index;
            out_receipt->field = cmdstr_text->field;
        }
        out_receipt->valid = 1;
    }
    return 1;
}

int dm2_v1_TRANSMIT_UI_EVENT(
    DM2_V1_HudUiEventKind event_kind,
    uint16_t payload_ref,
    const DM2_V1_QueryCmdstrTextReceipt *cmdstr_text,
    DM2_V1_TransmitUiEventReceipt *out_receipt)
{
    return dm2_transmit_ui_event_impl(event_kind,
                                      payload_ref,
                                      cmdstr_text,
                                      out_receipt,
                                      "TRANSMIT_UI_EVENT",
                                      "SKWIN/SkWinCore.cpp:12442");
}

int dm2_v1_DM2_TRANSMIT_UI_EVENT(
    DM2_V1_HudUiEventKind event_kind,
    uint16_t payload_ref,
    const DM2_V1_QueryCmdstrTextReceipt *cmdstr_text,
    DM2_V1_TransmitUiEventReceipt *out_receipt)
{
    return dm2_transmit_ui_event_impl(event_kind,
                                      payload_ref,
                                      cmdstr_text,
                                      out_receipt,
                                      "DM2_TRANSMIT_UI_EVENT",
                                      "SKULLWIN/c_input.cpp:180");
}

static uint8_t dm2_panel_mode_for_event(uint8_t event_kind)
{
    if (event_kind == DM2_V1_HUD_UI_EVENT_COMMAND_TEXT) {
        return DM2_V1_HUD_RIGHT_PANEL_COMMANDS;
    }
    if (event_kind == DM2_V1_HUD_UI_EVENT_CONTAINER) {
        return DM2_V1_HUD_RIGHT_PANEL_CONTAINER;
    }
    if (event_kind == DM2_V1_HUD_UI_EVENT_CHAMPION) {
        return DM2_V1_HUD_RIGHT_PANEL_CHAMPION;
    }
    return DM2_V1_HUD_RIGHT_PANEL_UNCHANGED;
}

static int dm2_update_right_panel_impl(
    DM2_V1_HudRightPanelMode previous_mode,
    const DM2_V1_TransmitUiEventReceipt *event,
    DM2_V1_UpdateRightPanelReceipt *out_receipt,
    const char *symbol,
    const char *source_path)
{
    uint8_t next_mode;

    dm2_update_right_panel_begin(out_receipt, symbol, source_path);
    if (previous_mode > DM2_V1_HUD_RIGHT_PANEL_CHAMPION ||
        !event || !event->valid || !event->requested_panel_refresh) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->previous_mode = (uint8_t)previous_mode;
        }
        return 0;
    }
    next_mode = dm2_panel_mode_for_event(event->event_kind);
    if (next_mode == DM2_V1_HUD_RIGHT_PANEL_UNCHANGED) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->previous_mode = (uint8_t)previous_mode;
        }
        return 0;
    }
    if (out_receipt) {
        out_receipt->previous_mode = (uint8_t)previous_mode;
        out_receipt->next_mode = next_mode;
        out_receipt->redraw_requested =
            next_mode != (uint8_t)previous_mode ? 1u : 0u;
        out_receipt->consumed_transmitted_event = 1u;
        out_receipt->consumed_real_cmdstr_text =
            event->consumed_real_cmdstr_text ? 1u : 0u;
        out_receipt->payload_ref = event->payload_ref;
        out_receipt->valid = 1;
    }
    return 1;
}

int dm2_v1_UPDATE_RIGHT_PANEL(
    DM2_V1_HudRightPanelMode previous_mode,
    const DM2_V1_TransmitUiEventReceipt *event,
    DM2_V1_UpdateRightPanelReceipt *out_receipt)
{
    return dm2_update_right_panel_impl(previous_mode,
                                      event,
                                      out_receipt,
                                      "UPDATE_RIGHT_PANEL",
                                      "SKWIN/SkWinCore.cpp:11283");
}

int dm2_v1_DM2_UPDATE_RIGHT_PANEL(
    DM2_V1_HudRightPanelMode previous_mode,
    const DM2_V1_TransmitUiEventReceipt *event,
    DM2_V1_UpdateRightPanelReceipt *out_receipt)
{
    return dm2_update_right_panel_impl(previous_mode,
                                      event,
                                      out_receipt,
                                      "DM2_UPDATE_RIGHT_PANEL",
                                      "SKULLWIN/c_gui_draw.cpp:5182");
}

const char *dm2_v1_hud_panel_routing_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp QUERY_CMDSTR_TEXT:8136 "
           "UPDATE_RIGHT_PANEL:11283 TRANSMIT_UI_EVENT:12442; "
           "SKULLWIN/c_querydb.cpp DM2_QUERY_CMDSTR_TEXT:274 "
           "SKULLWIN/c_gui_draw.cpp DM2_UPDATE_RIGHT_PANEL:5182 "
           "SKULLWIN/c_input.cpp DM2_TRANSMIT_UI_EVENT:180; "
           "bounded HUD panel routing over real GDAT dtText.";
}
