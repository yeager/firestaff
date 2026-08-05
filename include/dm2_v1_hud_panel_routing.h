#ifndef FIRESTAFF_DM2_V1_HUD_PANEL_ROUTING_H
#define FIRESTAFF_DM2_V1_HUD_PANEL_ROUTING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_HUD_PANEL_CMDSTR_TEXT_CAP 128u

typedef enum {
    DM2_V1_HUD_UI_EVENT_NONE = 0,
    DM2_V1_HUD_UI_EVENT_COMMAND_TEXT = 1,
    DM2_V1_HUD_UI_EVENT_CONTAINER = 2,
    DM2_V1_HUD_UI_EVENT_CHAMPION = 3
} DM2_V1_HudUiEventKind;

typedef enum {
    DM2_V1_HUD_RIGHT_PANEL_UNCHANGED = 0,
    DM2_V1_HUD_RIGHT_PANEL_COMMANDS = 1,
    DM2_V1_HUD_RIGHT_PANEL_CONTAINER = 2,
    DM2_V1_HUD_RIGHT_PANEL_CHAMPION = 3
} DM2_V1_HudRightPanelMode;

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t truncated;
    uint16_t byte_count;
    char text[DM2_V1_HUD_PANEL_CMDSTR_TEXT_CAP];
    uint32_t text_hash;
    const char *symbol;
    const char *source_path;
} DM2_V1_QueryCmdstrTextReceipt;

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint8_t event_kind;
    uint8_t requested_panel_refresh;
    uint8_t consumed_real_cmdstr_text;
    uint8_t used_synthetic_text;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint16_t payload_ref;
    const char *symbol;
    const char *source_path;
} DM2_V1_TransmitUiEventReceipt;

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint8_t previous_mode;
    uint8_t next_mode;
    uint8_t redraw_requested;
    uint8_t consumed_transmitted_event;
    uint8_t consumed_real_cmdstr_text;
    uint16_t payload_ref;
    const char *symbol;
    const char *source_path;
} DM2_V1_UpdateRightPanelReceipt;

void dm2_v1_query_cmdstr_text_receipt_clear(
    DM2_V1_QueryCmdstrTextReceipt *receipt);
void dm2_v1_transmit_ui_event_receipt_clear(
    DM2_V1_TransmitUiEventReceipt *receipt);
void dm2_v1_update_right_panel_receipt_clear(
    DM2_V1_UpdateRightPanelReceipt *receipt);

int dm2_v1_QUERY_CMDSTR_TEXT(
    int category,
    int index,
    int field,
    const uint8_t *text,
    size_t text_size,
    DM2_V1_QueryCmdstrTextReceipt *out_receipt);
/* The present signature cannot prove the caller-provided bytes belong to the
 * requested GDAT tuple, so both query entry points fail closed. A future
 * runtime binding must query the mounted original GDAT loader directly. */
int dm2_v1_DM2_QUERY_CMDSTR_TEXT(
    int category,
    int index,
    int field,
    const uint8_t *text,
    size_t text_size,
    DM2_V1_QueryCmdstrTextReceipt *out_receipt);

int dm2_v1_TRANSMIT_UI_EVENT(
    DM2_V1_HudUiEventKind event_kind,
    uint16_t payload_ref,
    const DM2_V1_QueryCmdstrTextReceipt *cmdstr_text,
    DM2_V1_TransmitUiEventReceipt *out_receipt);
int dm2_v1_DM2_TRANSMIT_UI_EVENT(
    DM2_V1_HudUiEventKind event_kind,
    uint16_t payload_ref,
    const DM2_V1_QueryCmdstrTextReceipt *cmdstr_text,
    DM2_V1_TransmitUiEventReceipt *out_receipt);

int dm2_v1_UPDATE_RIGHT_PANEL(
    DM2_V1_HudRightPanelMode previous_mode,
    const DM2_V1_TransmitUiEventReceipt *event,
    DM2_V1_UpdateRightPanelReceipt *out_receipt);
int dm2_v1_DM2_UPDATE_RIGHT_PANEL(
    DM2_V1_HudRightPanelMode previous_mode,
    const DM2_V1_TransmitUiEventReceipt *event,
    DM2_V1_UpdateRightPanelReceipt *out_receipt);

const char *dm2_v1_hud_panel_routing_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_HUD_PANEL_ROUTING_H */
