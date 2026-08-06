#ifndef DM2_V1_DIALOGUE_GDAT_H
#define DM2_V1_DIALOGUE_GDAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm2_v1_asset_loader.h"

/* skproject/SKULLWIN/c_gui_vp.cpp dialogue branch uses GRAPHICSSET images
 * -4/-3/-2 for the dialogue shell and field 3 for its glyph source.  This
 * receipt binds only those original IMG3 records and their local palettes;
 * it deliberately does not infer dialogue layout, text, or a draw call. */
#define DM2_V1_DIALOGUE_SHELL_FIELD_MIN 0xfcu
#define DM2_V1_DIALOGUE_SHELL_FIELD_MAX 0xfeu
#define DM2_V1_DIALOGUE_GLYPH_FIELD     0x03u

/* skproject/SKWINSPX/src/v5/uidialog.cpp DM2_dialog_2066_3820 and
 * SKWINSPX/src/v4/sktext.cpp DIALOG_2066_3820 both draw this exact save/load
 * dialogue panel through QUERY_GDAT_IMAGE_ENTRY_BUFF/LOCALPAL(0x1a, 0x81, 0).
 * Keep it separate from the map GRAPHICSSET dialogue shell above: the two
 * paths have different original GDAT owners. */
#define DM2_V1_DIALOGUE_BOX_INDEX 0x81u
#define DM2_V1_DIALOGUE_BOX_FIELD 0x00u
#define DM2_V1_DIALOGUE_BOX_RECT_INDEX 453u
#define DM2_V1_DIALOGUE_BOX_TEXT_Y_OFFSET 4u
#define DM2_V1_DIALOGUE_BOX_TEXT_PALETTE_SLOT 15u
#define DM2_V1_DIALOGUE_BOX_HIGHLIGHT_PALETTE_SLOT 11u

/* skproject/SKULLWIN/c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL opens the
 * save/load shell before DM2_dialog_2066_3820 redraws the selected save name.
 * These fields and rectangle IDs are source data, not a Firestaff dialogue
 * layout. */
#define DM2_V1_DIALOGUE_OPEN_PANEL_RECT_INDEX       4u
#define DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_RECT     450u
#define DM2_V1_DIALOGUE_OPEN_PANEL_PRIMARY_RECT     466u
#define DM2_V1_DIALOGUE_OPEN_PANEL_SECONDARY_RECT   467u
#define DM2_V1_DIALOGUE_OPEN_PANEL_SAVE_LIST_RECT   451u
#define DM2_V1_DIALOGUE_OPEN_PANEL_PRIMARY_TEXT_RECT 0x1d2u
#define DM2_V1_DIALOGUE_OPEN_PANEL_SECONDARY_TEXT_RECT 0x1d3u
#define DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_PALETTE  12u
#define DM2_V1_DIALOGUE_OPEN_PANEL_BUTTON_PALETTE   11u
#define DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT       2u
#define DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_CAPACITY    80u
#define DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT     "V1.0"
#define DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT_SIZE 5u

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint8_t shell_field;
    DM2_V1_GdatImageMetadata shell_metadata;
    DM2_V1_GdatImageMetadata glyph_metadata;
    uint8_t shell_palette[16];
    uint8_t glyph_palette[16];
    uint32_t shell_palette_hash;
    uint32_t glyph_palette_hash;
    uint32_t receipt_hash;
} DM2_V1_DialogueGdatReceipt;

typedef struct {
    int valid;
    DM2_V1_GdatImageMetadata metadata;
    uint8_t palette[16];
    uint32_t palette_hash;
    uint32_t receipt_hash;
} DM2_V1_DialogueBoxGdatReceipt;

/* This is the original save/load dialogue draw plan, not a replacement UI.
 * skproject expands rectangle 453, blits DIALOG_BOXES/0x81/0 with its local
 * palette, writes yellow text at y + 4, and optionally clears the remaining
 * text cell with orange. M11 still owns expansion of the original rectangle
 * table and the actual draw; callers must not invent coordinates or panels. */
typedef struct {
    int valid;
    int gdat_category;
    int gdat_index;
    int gdat_field;
    uint16_t expanded_rect_index;
    uint8_t text_y_offset;
    uint8_t text_palette_slot;
    uint8_t highlight_palette_slot;
    int optional_highlight_clear;
    DM2_V1_DialogueBoxGdatReceipt material;
    uint32_t plan_hash;
} DM2_V1_DialogueBoxDrawPlan;

typedef struct {
    int valid;
    DM2_V1_DialogueBoxGdatReceipt material;
    uint8_t version_text[DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT_SIZE];
    size_t version_text_size;
    uint32_t version_text_hash;
    uint8_t decoded_text[DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT]
                        [DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_CAPACITY];
    const uint8_t *text[DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT];
    size_t text_size[DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT];
    uint32_t text_hash[DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT];
    uint16_t panel_rect_index;
    uint16_t version_rect_index;
    uint16_t primary_button_rect_index;
    uint16_t secondary_button_rect_index;
    uint16_t save_list_rect_index;
    uint8_t version_palette_slot;
    uint8_t button_palette_slot;
    uint8_t save_slot_count;
    int fade_when_dialog2;
    uint32_t receipt_hash;
} DM2_V1_DialogueOpenPanelReceipt;

/* An optional locale owner may provide already-decoded source text for the
 * two QUERY_GDAT_TEXT records that OPEN_DIALOG_PANEL consumes. The callback
 * remains keyed by the original GDAT address, so it cannot introduce a host
 * label, geometry, or a text record not selected by c_dialog.cpp. */
typedef const uint8_t *(*DM2_V1_DialogueTextOverride)(
    void *userdata, int category, int index, int field, size_t *out_size);

/* c_dialog.cpp::DM2_dialog_2066_33e7 receives these four eventqueue values
 * while the original save-name panel is open.  They are deliberately not
 * Firestaff menu actions: the host must first decode a source rectangle hit
 * into SELECT_SLOT before it may call this state machine. */
typedef enum {
    DM2_V1_DIALOGUE_SAVE_EVENT_CANCEL = 0,
    DM2_V1_DIALOGUE_SAVE_EVENT_ACCEPT = 1,
    DM2_V1_DIALOGUE_SAVE_EVENT_SELECT_SLOT = 2,
    DM2_V1_DIALOGUE_SAVE_EVENT_EDIT = 3
} DM2_V1_DialogueSaveEvent;

typedef struct {
    int valid;
    int selected_slot;       /* 0..9, or source sentinel 10 */
    int editing;
    uint8_t text_length;     /* c_dialog.cpp caps the editable name at 31 */
    uint8_t text[32];
    uint32_t state_hash;
} DM2_V1_DialogueSaveInputState;

typedef struct {
    int valid;
    int redraw;
    int close_panel;
    int cancelled;
    int accepted_slot;       /* -1 unless the source accepts a selection */
    uint32_t route_hash;
} DM2_V1_DialogueSaveInputReceipt;

/* Returns an exact material receipt only when both source images are IMG3
 * 4bpp images with their own QUERY_GDAT_IMAGE_LOCALPAL tail. */
int dm2_v1_dialogue_gdat_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t graphicsset,
                                 uint8_t shell_field,
                                 DM2_V1_DialogueGdatReceipt *out);

/* Returns the source-owned save/load dialogue-panel material only for the
 * skproject 0x1a/0x81/0 record.  Callers still need source-proven rectangle
 * 453 placement and text layout before they may issue a draw. */
int dm2_v1_dialogue_box_gdat_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DialogueBoxGdatReceipt *out);

/* Produces the source-owned runtime plan for the save/load panel. The plan
 * has no screen coordinates because only skproject's expanded RECT_453 table
 * authorizes those coordinates. */
int dm2_v1_dialogue_box_draw_plan(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DialogueBoxDrawPlan *out);

/* Captures the complete source-owned save/load panel setup from
 * c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL. The button text pointers refer
 * to the verified GDAT payload; the version heading is the exact compiled
 * skproject dm2data.cpp constant and must not be replaced by a host label. */
int dm2_v1_dialogue_open_panel_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DialogueOpenPanelReceipt *out);

/* As above, but lets an authenticated locale overlay replace only a decoded
 * source text payload. Passing NULL preserves the native GDAT result. */
int dm2_v1_dialogue_open_panel_receipt_with_text_override(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DialogueTextOverride text_override,
    void *text_override_userdata,
    DM2_V1_DialogueOpenPanelReceipt *out);

/* Initializes the source save-dialogue state after OPEN_DIALOG_PANEL has
 * authenticated the panel's two GDAT labels. `initial_name` is a save-header
 * value, never a replacement label. */
int dm2_v1_dialogue_save_input_init(
    const DM2_V1_DialogueOpenPanelReceipt *panel,
    int selected_slot,
    const uint8_t *initial_name,
    size_t initial_name_size,
    DM2_V1_DialogueSaveInputState *out_state);

/* Mirrors c_dialog.cpp lines 152-301 for its already-decoded eventqueue
 * values. The only accepted text key is the original uppercased ASCII entry;
 * keyboard scancode conversion and RECT_451 hit decoding stay with their
 * source-owning host routes. */
int dm2_v1_dialogue_save_input_apply(
    const DM2_V1_DialogueOpenPanelReceipt *panel,
    DM2_V1_DialogueSaveInputState *state,
    DM2_V1_DialogueSaveEvent event,
    int source_slot,
    uint8_t text_key,
    DM2_V1_DialogueSaveInputReceipt *out_receipt);

#endif
