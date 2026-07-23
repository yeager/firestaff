#include "dm1_v1_champion_panel_food_water_status_box_pc34_compat.h"

#include "vga_palette_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_CPFW_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_CPFW_STATUS_BOX_FIRST_ZONE_PC34 = 151,
    DM1_V1_CPFW_STATUS_BOX_LAST_ZONE_PC34 = 154,
    DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34 = 69,
    DM1_V1_CPFW_COLOR_BLACK_PC34 = 0,
    DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34 = 5,
    DM1_V1_CPFW_COLOR_RED_PC34 = 8,
    DM1_V1_CPFW_COLOR_FLESH_PC34 = 10,
    DM1_V1_CPFW_COLOR_YELLOW_PC34 = 11,
    DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34 = 12,
    DM1_V1_CPFW_COLOR_BLUE_PC34 = 14,
    DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34 = 20,
    DM1_V1_CPFW_GFX_FOOD_LABEL_PC34 = 30,
    DM1_V1_CPFW_GFX_WATER_LABEL_PC34 = 31,
    DM1_V1_CPFW_ZONE_PANEL_PC34 = 101,
    DM1_V1_CPFW_ZONE_FOOD_BAR_PC34 = 103,
    DM1_V1_CPFW_ZONE_WATER_BAR_PC34 = 104,
    DM1_V1_CPFW_ZONE_FOOD_LABEL_PC34 = 500,
    DM1_V1_CPFW_ZONE_WATER_LABEL_PC34 = 501,
    DM1_V1_CPFW_THING_NONE_PC34 = 0xFFFFu,
    DM1_V1_CPFW_THING_END_OF_LIST_PC34 = 0xFFFEu
};

/*
 * ReDMCSB source-lock anchors for this contract-only slice:
 * - CHEST.C F0334:113-132 closes G0426, clears non-empty G0425 slots, and
 *   relinks the visible chest cell list before the next panel/status draw.
 * - PANEL.C F0354:2299-2322 brackets an inventory close by clearing G0423,
 *   calling F0334, dirtying MASK0x1000_STATUS_BOX, then calling F0292.
 * - CHAMDRAW.C F0292:771-789 owns the C151..C154 live 67x29 status-box fill;
 *   F0292:804/807 uses C10_COLOR_FLESH transparency for status-box borders.
 * - CHAMPION.C F0284:93-130 owns the G0305 party champion count traversal
 *   and the M516 champion-state addressing used with G0423 ordinals.
 * - PANEL.C F0345:1579-1615 reads M516_CHAMPIONS[G0423-1], blits C030/C031
 *   food/water labels, and draws C103/C104 food/water bars via F0344.
 * - PANEL.C F0344:1493-1561 maps food/water counters to red/yellow/base
 *   colors and proportional fill.
 * - MENU.C F0409/F0410/F0411:1666-1721 are spell/flask helpers in this
 *   ReDMCSB snapshot, so the food/water draw is explicitly pinned to PANEL.C
 *   while this contract records the requested MENU symbols as disambiguated.
 * - DEFS.H:2076-2092,2157,2190-2191,3776-3778,3783-3786,3869-3870 define
 *   C10/C12, C69, C030/C031, C101/C103/C104, C151..C154, and C500/C501.
 */

static const char s_source_evidence[] =
    "contract_only=1; CHEST.C F0334:113-132 closes G0426 and relinks "
    "non-empty G0425 chest cells before panel/status drawing. PANEL.C "
    "F0354:2299-2322 clears G0423 on close, calls F0334, marks "
    "MASK0x1000_STATUS_BOX, then calls F0292. CHAMDRAW.C F0292:771-789 "
    "fills the live C151..C154 status box as 67x29 at stride C69; "
    "CHAMDRAW.C F0292:804/807 uses DEFS.H:2088 C10_COLOR_FLESH transparency "
    "for status-box border overlays. CHAMPION.C F0284:93-130 anchors the "
    "G0305 party-count/M516 champion-state traversal used with G0423. "
    "PANEL.C F0345:1579-1615 reads M516_CHAMPIONS[G0423-1], blits "
    "C030/C031 to C500/C501, and draws C103/C104 food/water bars through "
    "PANEL.C F0344:1493-1561. MENU.C F0409/F0410/F0411:1666-1721 are "
    "spell/flask helpers in this ReDMCSB snapshot, not food/water panel "
    "draw functions; this gate disambiguates them and pins the actual "
    "food/water draw to PANEL.C. DEFS.H:2076-2092,2157,2190-2191,"
    "3776-3778,3783-3786,3869-3870 provide the constants.";

static const dm1_v1_champion_panel_food_water_status_box_contract_pc34_t
    s_contract = {
        1,
        DM1_V1_CPFW_CHAMPION_COUNT_PC34,
        DM1_V1_CPFW_STATUS_BOX_FIRST_ZONE_PC34,
        DM1_V1_CPFW_STATUS_BOX_LAST_ZONE_PC34,
        DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34,
        DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34,
        DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34,
        DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
        DM1_V1_CPFW_COLOR_FLESH_PC34,
        DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34,
        DM1_V1_CPFW_GFX_FOOD_LABEL_PC34,
        DM1_V1_CPFW_GFX_WATER_LABEL_PC34,
        DM1_V1_CPFW_ZONE_FOOD_LABEL_PC34,
        DM1_V1_CPFW_ZONE_WATER_LABEL_PC34,
        DM1_V1_CPFW_ZONE_FOOD_BAR_PC34,
        DM1_V1_CPFW_ZONE_WATER_BAR_PC34,
        DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34,
        DM1_V1_CPFW_COLOR_BLUE_PC34,
        DM1_V1_CPFW_COLOR_YELLOW_PC34,
        DM1_V1_CPFW_COLOR_RED_PC34,
        "CHEST.C F0334:113-132",
        "MENU.C F0409/F0410/F0411:1666-1721 disambiguated",
        "PANEL.C F0344:1493-1561; F0345:1563-1616",
        "CHAMDRAW.C F0292:771-789,804,807",
        "CHAMPION.C F0284:93-130",
        "PANEL.C F0354:2299-2322",
        "DEFS.H C10/C12/C69/C030/C031/C101/C103/C104/C151-C154/C500-C501"
    };

const dm1_v1_champion_panel_food_water_status_box_contract_pc34_t *
dm1_v1_champion_panel_food_water_status_box_contract_pc34(void)
{
    return &s_contract;
}

const char *
dm1_v1_champion_panel_food_water_status_box_source_evidence_pc34(void)
{
    return s_source_evidence;
}

dm1_v1_champion_panel_food_water_status_box_input_pc34_t
dm1_v1_champion_panel_food_water_status_box_default_input_pc34(void)
{
    dm1_v1_champion_panel_food_water_status_box_input_pc34_t input;

    memset(&input, 0, sizeof(input));
    input.inventory_champion_ordinal = 2;
    input.party_champion_count = DM1_V1_CPFW_CHAMPION_COUNT_PC34;
    input.current_health = 99;
    input.food = 256;
    input.water = -700;
    input.poison_event_count = 0;
    input.open_chest_thing = 0x1234u;
    input.chest_slots[0] = 0x0101u;
    input.chest_slots[1] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[2] = 0x0102u;
    input.chest_slots[3] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[4] = 0x0103u;
    input.chest_slots[5] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[6] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[7] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    return input;
}

static int valid_input(
    const dm1_v1_champion_panel_food_water_status_box_input_pc34_t *input)
{
    return input->party_champion_count > 0 &&
           input->party_champion_count <= DM1_V1_CPFW_CHAMPION_COUNT_PC34 &&
           input->inventory_champion_ordinal > 0 &&
           input->inventory_champion_ordinal <= input->party_champion_count;
}

static int color_for_amount(int amount, int base_color)
{
    /*
     * ReDMCSB PANEL.C F0344:1519-1525: red below -512, yellow below zero,
     * otherwise the food/water base color passed by F0345:1614-1615.
     */
    if (amount < -512) {
        return DM1_V1_CPFW_COLOR_RED_PC34;
    }
    if (amount < 0) {
        return DM1_V1_CPFW_COLOR_YELLOW_PC34;
    }
    return base_color;
}

static int proportional_units_for_amount(int amount)
{
    long normalized;

    /*
     * ReDMCSB PANEL.C F0344:1537-1544: the PC34 route adds 1024 then
     * passes a 0..10000 proportional value to F0637_GetProportionalZone.
     */
    normalized = amount + 1024L;
    if (normalized < 0) {
        normalized = 0;
    }
    if (normalized > 3072L) {
        normalized = 3072L;
    }
    return (int)((normalized * 10000L) / 3072L);
}

static uint32_t material_fingerprint(const uint8_t *pixels, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    for (index = 0u; index < count; ++index) {
        hash ^= pixels[index];
        hash *= 16777619u;
    }
    return hash;
}

static int material_has_visible_pixel(const uint8_t *pixels, size_t count)
{
    size_t index;

    for (index = 0u; index < count; ++index) {
        if (pixels[index] != 0u) {
            return 1;
        }
    }
    return 0;
}

static int material_surface_matches(
    const dm1_v1_champion_panel_food_water_material_surface_pc34_t *surface,
    int graphic_id,
    int width,
    int height)
{
    return surface != NULL && surface->graphics_dat_backed &&
           surface->graphic_id == graphic_id && surface->width == width &&
           surface->height == height && surface->pixels != NULL;
}

static int material_slot_is_indexed_vga4(const M11_AssetSlot *slot,
                                         unsigned int graphic_id,
                                         unsigned short width,
                                         unsigned short height)
{
    size_t pixel_count;
    size_t index;

    if (!slot || !slot->loaded || slot->graphicIndex != graphic_id ||
        slot->width != width || slot->height != height || !slot->pixels) {
        return 0;
    }
    pixel_count = (size_t)slot->width * (size_t)slot->height;
    for (index = 0u; index < pixel_count; ++index) {
        if (slot->pixels[index] > 15u) {
            return 0;
        }
    }
    return 1;
}

int dm1_v1_champion_panel_food_water_material_admit_pc34(
    const dm1_v1_champion_panel_food_water_material_surface_pc34_t *panel,
    const dm1_v1_champion_panel_food_water_material_surface_pc34_t *food_label,
    const dm1_v1_champion_panel_food_water_material_surface_pc34_t *water_label,
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t *out_receipt)
{
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.f0134_status_fill_color = DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
    receipt.f0135_panel_graphic = DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34;
    receipt.f0135_food_label_graphic = DM1_V1_CPFW_GFX_FOOD_LABEL_PC34;
    receipt.f0135_water_label_graphic = DM1_V1_CPFW_GFX_WATER_LABEL_PC34;
    receipt.sourceEvidence =
        "VIDEO.C F0134/F0135; PANEL.C F0345:1597-1615; "
        "DEFS.H C020/C030/C031";

    if (!material_surface_matches(panel, DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34,
                                  144, 73)) {
        receipt.rejected_missing_panel = 1;
    }
    if (!material_surface_matches(food_label, DM1_V1_CPFW_GFX_FOOD_LABEL_PC34,
                                  34, 9)) {
        receipt.rejected_missing_food_label = 1;
    }
    if (!material_surface_matches(water_label,
                                  DM1_V1_CPFW_GFX_WATER_LABEL_PC34,
                                  46, 9)) {
        receipt.rejected_missing_water_label = 1;
    }
    if (!receipt.rejected_missing_panel &&
        !receipt.rejected_missing_food_label &&
        !receipt.rejected_missing_water_label) {
        receipt.panel_pixel_fingerprint =
            material_fingerprint(panel->pixels, 144u * 73u);
        receipt.food_label_pixel_fingerprint =
            material_fingerprint(food_label->pixels, 34u * 9u);
        receipt.water_label_pixel_fingerprint =
            material_fingerprint(water_label->pixels, 46u * 9u);
        /* An all-zero surface is transparent/empty, never usable source art. */
        receipt.admitted = material_has_visible_pixel(panel->pixels, 144u * 73u) &&
                           material_has_visible_pixel(food_label->pixels, 34u * 9u) &&
                           material_has_visible_pixel(water_label->pixels, 46u * 9u);
    }
    if (out_receipt) {
        *out_receipt = receipt;
    }
    return receipt.admitted;
}

int dm1_v1_champion_panel_food_water_material_admit_graphics_slots_pc34(
    const M11_AssetSlot *panel,
    const M11_AssetSlot *food_label,
    const M11_AssetSlot *water_label,
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t *out_receipt)
{
    dm1_v1_champion_panel_food_water_material_surface_pc34_t panel_surface;
    dm1_v1_champion_panel_food_water_material_surface_pc34_t food_surface;
    dm1_v1_champion_panel_food_water_material_surface_pc34_t water_surface;
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t receipt;
    int admitted;

    memset(&receipt, 0, sizeof(receipt));
    receipt.graphics_dat_loader_ready =
        panel != NULL && food_label != NULL && water_label != NULL;
    receipt.f0134_status_fill_color = DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
    receipt.f0135_panel_graphic = DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34;
    receipt.f0135_food_label_graphic = DM1_V1_CPFW_GFX_FOOD_LABEL_PC34;
    receipt.f0135_water_label_graphic = DM1_V1_CPFW_GFX_WATER_LABEL_PC34;
    receipt.sourceEvidence =
        "PANEL.C F0345 C020/C030/C031 via original GRAPHICS.DAT indexed VGA4";
    if (!receipt.graphics_dat_loader_ready) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.indexed_vga4_format_valid =
        material_slot_is_indexed_vga4(panel, DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34,
                                      144u, 73u) &&
        material_slot_is_indexed_vga4(food_label,
                                      DM1_V1_CPFW_GFX_FOOD_LABEL_PC34,
                                      34u, 9u) &&
        material_slot_is_indexed_vga4(water_label,
                                      DM1_V1_CPFW_GFX_WATER_LABEL_PC34,
                                      46u, 9u);
    if (!receipt.indexed_vga4_format_valid) {
        receipt.rejected_invalid_pixel_format = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    panel_surface = (dm1_v1_champion_panel_food_water_material_surface_pc34_t){
        1, DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34, (int)panel->width,
        (int)panel->height, panel->pixels};
    food_surface = (dm1_v1_champion_panel_food_water_material_surface_pc34_t){
        1, DM1_V1_CPFW_GFX_FOOD_LABEL_PC34, (int)food_label->width,
        (int)food_label->height, food_label->pixels};
    water_surface = (dm1_v1_champion_panel_food_water_material_surface_pc34_t){
        1, DM1_V1_CPFW_GFX_WATER_LABEL_PC34, (int)water_label->width,
        (int)water_label->height, water_label->pixels};
    admitted = dm1_v1_champion_panel_food_water_material_admit_pc34(
        &panel_surface, &food_surface, &water_surface, &receipt);
    receipt.graphics_dat_loader_ready = 1;
    receipt.indexed_vga4_format_valid = 1;
    receipt.sourceEvidence =
        "PANEL.C F0345 C020/C030/C031 via original GRAPHICS.DAT indexed VGA4";
    if (out_receipt) *out_receipt = receipt;
    return admitted;
}

int dm1_v1_champion_panel_food_water_material_admit_runtime_pc34(
    const M11_AssetSlot *panel,
    const M11_AssetSlot *food_label,
    const M11_AssetSlot *water_label,
    const uint8_t *palette,
    size_t paletteByteCount,
    int panelX,
    int panelY,
    int foodLabelX,
    int foodLabelY,
    int waterLabelX,
    int waterLabelY,
    int framebufferWidth,
    int framebufferHeight,
    dm1_v1_champion_panel_food_water_runtime_receipt_pc34_t *out_receipt)
{
    dm1_v1_champion_panel_food_water_runtime_receipt_pc34_t receipt;
    const size_t paletteSize = VGA_PALETTE_PC34_COLOR_COUNT * 3u;

    memset(&receipt, 0, sizeof(receipt));
    receipt.noDraw = 1;
    receipt.sourceEvidence =
        "ReDMCSB PANEL.C F0344/F0345/F0658 C020/C030/C031; "
        "VIDEODRV.C G8151 LIGHT0 palette; C101/C500/C501 zones";
    if (!dm1_v1_champion_panel_food_water_material_admit_graphics_slots_pc34(
            panel, food_label, water_label, &receipt.material) ||
        !palette || paletteByteCount != paletteSize ||
        memcmp(palette, G9010_auc_VgaPaletteBrightest_Compat, paletteSize) != 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.paletteSourceBound = 1;
    receipt.paletteFingerprint = material_fingerprint(palette, paletteSize);
    receipt.panelX = panelX;
    receipt.panelY = panelY;
    receipt.foodLabelX = foodLabelX;
    receipt.foodLabelY = foodLabelY;
    receipt.waterLabelX = waterLabelX;
    receipt.waterLabelY = waterLabelY;
    receipt.livePlacementValid = framebufferWidth > 0 && framebufferHeight > 0 &&
        panelX >= 0 && panelY >= 0 &&
        panelX + (int)panel->width <= framebufferWidth &&
        panelY + (int)panel->height <= framebufferHeight &&
        foodLabelX == panelX + 32 &&
        foodLabelY == panelY + 13 - (((int)food_label->height + 1) / 2) &&
        waterLabelX == panelX + 32 &&
        waterLabelY == panelY + 36 - (((int)water_label->height + 1) / 2) &&
        foodLabelX >= panelX && foodLabelY >= panelY &&
        waterLabelX >= panelX && waterLabelY >= panelY &&
        foodLabelX + (int)food_label->width <= panelX + (int)panel->width &&
        waterLabelX + (int)water_label->width <= panelX + (int)panel->width &&
        foodLabelY + (int)food_label->height <= panelY + (int)panel->height &&
        waterLabelY + (int)water_label->height <= panelY + (int)panel->height;
    if (!receipt.livePlacementValid || receipt.paletteFingerprint == 0u) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.admitted = 1;
    receipt.noDraw = 0;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

static void fill_status_box(
    dm1_v1_champion_panel_food_water_status_box_frame_pc34_t *frame)
{
    int i;

    for (i = 0; i < DM1_V1_CPFW_STATUS_BOX_BYTES_PC34; ++i) {
        frame->bytes[i] = (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
    }
    frame->fill_pixel_count = DM1_V1_CPFW_STATUS_BOX_BYTES_PC34;
}

static int pixel_index(int x, int y)
{
    return y * DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34 + x;
}

static void apply_transparent_border(
    dm1_v1_champion_panel_food_water_status_box_frame_pc34_t *frame)
{
    int x;
    int y;
    int preserved;

    /*
     * ReDMCSB CHAMDRAW.C F0292:804/807: status-box border graphics use
     * C10_COLOR_FLESH as transparent, so C10 source pixels preserve the
     * C12 fill already written by F0292:786/789.
     */
    preserved = 0;
    for (x = 0; x < DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34; ++x) {
        frame->bytes[pixel_index(x, 0)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        frame->bytes[pixel_index(x, DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34 - 1)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        preserved += 2;
    }
    for (y = 1; y < DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34 - 1; ++y) {
        frame->bytes[pixel_index(0, y)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        frame->bytes[pixel_index(DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34 - 1, y)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        preserved += 2;
    }
    frame->transparent_border_pixel_count = preserved;
}

static void append_operation(
    dm1_v1_champion_panel_food_water_status_box_result_pc34_t *result,
    dm1_v1_champion_panel_food_water_status_box_operation_kind_pc34_t kind,
    int graphic_id,
    int zone_id,
    int transparent_color,
    int fill_color,
    int amount,
    int proportional_units,
    const char *source_evidence)
{
    dm1_v1_champion_panel_food_water_status_box_operation_pc34_t *operation;

    if (result->operation_count >= DM1_V1_CPFW_MAX_OPERATIONS_PC34) {
        return;
    }
    operation = &result->operations[result->operation_count];
    memset(operation, 0, sizeof(*operation));
    operation->kind = kind;
    operation->sequence = result->operation_count;
    operation->graphic_id = graphic_id;
    operation->zone_id = zone_id;
    operation->transparent_color = transparent_color;
    operation->fill_color = fill_color;
    operation->amount = amount;
    operation->proportional_units = proportional_units;
    operation->sourceEvidence = source_evidence;
    result->operation_count++;
}

dm1_v1_champion_panel_food_water_status_box_result_pc34_t
dm1_v1_champion_panel_food_water_status_box_probe_pc34(
    const dm1_v1_champion_panel_food_water_status_box_input_pc34_t *input)
{
    dm1_v1_champion_panel_food_water_status_box_result_pc34_t result;
    dm1_v1_champion_panel_food_water_status_box_input_pc34_t local_input;
    int i;

    memset(&result, 0, sizeof(result));
    result.contract_only = 1;
    result.sourceEvidence = s_source_evidence;
    result.loads_graphics_dat = 0;
    result.loads_dungeon_dat = 0;
    result.relink_first_thing = (uint16_t)DM1_V1_CPFW_THING_END_OF_LIST_PC34;
    result.relink_last_thing = (uint16_t)DM1_V1_CPFW_THING_END_OF_LIST_PC34;

    if (!input) {
        local_input = dm1_v1_champion_panel_food_water_status_box_default_input_pc34();
        input = &local_input;
    }
    if (!valid_input(input)) {
        result.rejected_invalid_champion = 1;
        return result;
    }
    if (input->current_health <= 0) {
        result.rejected_dead_champion = 1;
        return result;
    }

    result.valid = 1;
    result.inventory_champion_index = input->inventory_champion_ordinal - 1;
    result.party_champion_count = input->party_champion_count;
    result.g0423_inventory_champion_ordinal = input->inventory_champion_ordinal;
    result.g0305_party_champion_count = input->party_champion_count;
    result.food_counter = input->food;
    result.water_counter = input->water;
    result.poison_event_count = input->poison_event_count;
    result.food_word_color = DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34;
    result.water_word_color = DM1_V1_CPFW_COLOR_BLUE_PC34;
    result.food_bar_color = color_for_amount(
        input->food, DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34);
    result.water_bar_color = color_for_amount(
        input->water, DM1_V1_CPFW_COLOR_BLUE_PC34);
    result.food_bar_units = proportional_units_for_amount(input->food);
    result.water_bar_units = proportional_units_for_amount(input->water);
    result.menu_f0409_f0410_f0411_disambiguated = 1;

    result.frame.champion_index = result.inventory_champion_index;
    result.frame.zone_id =
        DM1_V1_CPFW_STATUS_BOX_FIRST_ZONE_PC34 + result.inventory_champion_index;
    result.frame.screen_x =
        result.inventory_champion_index * DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34;
    result.frame.screen_y = 0;
    result.frame.width = DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34;
    result.frame.height = DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34;
    result.frame.fill_color = DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
    result.frame.border_transparent_color = DM1_V1_CPFW_COLOR_FLESH_PC34;

    result.chest_was_open =
        input->open_chest_thing != DM1_V1_CPFW_THING_NONE_PC34;
    if (result.chest_was_open) {
        result.open_chest_cleared = 1;
        for (i = 0; i < DM1_V1_CPFW_CHEST_SLOT_COUNT_PC34; ++i) {
            if (input->chest_slots[i] != DM1_V1_CPFW_THING_NONE_PC34) {
                if (!result.non_empty_chest_slots) {
                    result.relink_first_thing = input->chest_slots[i];
                }
                result.relink_last_thing = input->chest_slots[i];
                result.non_empty_chest_slots++;
            }
        }
    }

    append_operation(&result,
                     DM1_V1_CPFW_OP_CHEST_CLOSE_PC34,
                     -1,
                     -1,
                     -1,
                     -1,
                     result.non_empty_chest_slots,
                     result.open_chest_cleared,
                     "CHEST.C F0334:113-132");
    append_operation(&result,
                     DM1_V1_CPFW_OP_PANEL_CLOSE_BRACKET_PC34,
                     -1,
                     -1,
                     -1,
                     -1,
                     input->inventory_champion_ordinal,
                     input->party_champion_count,
                     "PANEL.C F0354:2299-2322");

    fill_status_box(&result.frame);
    append_operation(&result,
                     DM1_V1_CPFW_OP_STATUS_BOX_FILL_PC34,
                     -1,
                     result.frame.zone_id,
                     -1,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     DM1_V1_CPFW_STATUS_BOX_BYTES_PC34,
                     DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34,
                     "CHAMDRAW.C F0292:771-789");
    apply_transparent_border(&result.frame);
    append_operation(&result,
                     DM1_V1_CPFW_OP_STATUS_BOX_BORDER_PC34,
                     -1,
                     result.frame.zone_id,
                     DM1_V1_CPFW_COLOR_FLESH_PC34,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     result.frame.transparent_border_pixel_count,
                     DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34,
                     "CHAMDRAW.C F0292:804,807 and DEFS.H:2088");

    append_operation(&result,
                     DM1_V1_CPFW_OP_PANEL_EMPTY_PC34,
                     DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34,
                     DM1_V1_CPFW_ZONE_PANEL_PC34,
                     DM1_V1_CPFW_COLOR_RED_PC34,
                     -1,
                     0,
                     0,
                     "PANEL.C F0345:1597");
    append_operation(&result,
                     DM1_V1_CPFW_OP_FOOD_LABEL_PC34,
                     DM1_V1_CPFW_GFX_FOOD_LABEL_PC34,
                     DM1_V1_CPFW_ZONE_FOOD_LABEL_PC34,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     result.food_word_color,
                     input->food,
                     0,
                     "PANEL.C F0345:1598 and DEFS.H:2190,3869");
    append_operation(&result,
                     DM1_V1_CPFW_OP_WATER_LABEL_PC34,
                     DM1_V1_CPFW_GFX_WATER_LABEL_PC34,
                     DM1_V1_CPFW_ZONE_WATER_LABEL_PC34,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     result.water_word_color,
                     input->water,
                     0,
                     "PANEL.C F0345:1599 and DEFS.H:2191,3870");
    append_operation(&result,
                     DM1_V1_CPFW_OP_FOOD_BAR_PC34,
                     -1,
                     DM1_V1_CPFW_ZONE_FOOD_BAR_PC34,
                     -1,
                     result.food_bar_color,
                     input->food,
                     result.food_bar_units,
                     "PANEL.C F0344:1493-1561 and F0345:1614");
    append_operation(&result,
                     DM1_V1_CPFW_OP_WATER_BAR_PC34,
                     -1,
                     DM1_V1_CPFW_ZONE_WATER_BAR_PC34,
                     -1,
                     result.water_bar_color,
                     input->water,
                     result.water_bar_units,
                     "PANEL.C F0344:1493-1561 and F0345:1615");

    result.close_before_status_box =
        result.operations[0].kind == DM1_V1_CPFW_OP_CHEST_CLOSE_PC34 &&
        result.operations[2].kind == DM1_V1_CPFW_OP_STATUS_BOX_FILL_PC34;
    result.status_box_before_panel =
        result.operations[3].kind == DM1_V1_CPFW_OP_STATUS_BOX_BORDER_PC34 &&
        result.operations[4].kind == DM1_V1_CPFW_OP_PANEL_EMPTY_PC34;

    return result;
}

dm1_v1_champion_panel_food_water_bar_zone_pc34_t
dm1_v1_champion_panel_food_bar_zone_pc34(void) {
    dm1_v1_champion_panel_food_water_bar_zone_pc34_t zone;
    zone.x = 112;
    zone.y = 67;
    zone.w = 67;
    zone.h = 5;
    /* DEFS.H G2097_FoodOrWaterBarShadowOffset is 2 in the PC 3.4 data. */
    zone.shadow_offset = 2;
    return zone;
}

dm1_v1_champion_panel_food_water_bar_zone_pc34_t
dm1_v1_champion_panel_water_bar_zone_pc34(void) {
    dm1_v1_champion_panel_food_water_bar_zone_pc34_t zone;
    zone.x = 112;
    zone.y = 90;
    zone.w = 67;
    zone.h = 5;
    zone.shadow_offset = 2;
    return zone;
}
