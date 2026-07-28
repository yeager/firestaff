#ifndef FIRESTAFF_M11_GAME_VIEW_H
#define FIRESTAFF_M11_GAME_VIEW_H

#include "dm1_v1_champion_needs_pc34_compat.h"
#include "dm1_v1_hoc_presented_frame_consumer_pc34_compat.h"
#include "dm1_v1_center_door_render_pc34_compat.h"
#include "dm1_v1_inscription_host_material_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"

#include <stdint.h>
#include "menu_startup_m12.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "session_timer_runtime.h"
#include "memory_magic_pc34_compat.h"
#include "asset_loader_m11.h"
#include "audio_sdl_m11.h"
#include "font_m11.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_light_runtime.h"
#include "dm1_v1_vblank_timing.h"
#include "dm1_v1_save_load.h"
#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "dm1_v1_live_action_effects_pc34_compat.h"
#include "dm1_v1_mouse_routes_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_inventory_slot_placement_pc34_compat.h"
#include "dm1_v1_layout_zones_pc34_compat.h"
#include "dm1_v1_dialog_layout_pc34_compat.h"
#include "dm1_v1_endgame_layout_pc34_compat.h"
#include "dm1_v1_endgame_presentation_pc34_compat.h"
#include "dialog_frontend_pc34_compat.h"
#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"
#include "dm1_v2_camera_controller_pc34.h"
#include "firestaff_retroachievements.h"
#include "firestaff/dm1/v1/resurrection_rename_ui_gate_pc34_compat.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "theron_v1_track02.h"
#include "theron_v1_runtime_admission.h"
#include "theron_v1_track02_campaign_media_discovery.h"
#include "theron_v1_track02_loader_trace_replay_consistency.h"
#include "theron_v1_track02_sector_record_admission.h"
#include "theron_v1_track02_sector_record_corpus_discovery.h"
#include "theron_v1_track02_level_object_descriptor_capture_intake.h"
#include "theron_v1_track02_descriptor_bitmap_palette_capture_intake.h"
#include "theron_v1_track02_dungeon_handoff_capture_plan_admission.h"
#include "theron_v1_track02_live_loader_route_admission.h"
#include "theron_v1_track02_g8_fifo_capture_binding.h"
#include "csb_v1_boot.h"
#include "csb_v1_atari_st_animation_assets.h"
#include "csb_v1_f0128_entrance_runtime_consumer_pc34_compat.h"
#include "csb_v1_startup_entrance_f0128_m11_handoff_pc34_compat.h"
#include "csb_v1_csbwin_dsa_runtime_admission_pc34_compat.h"
#include "csb_v1_startup_runtime_coupling_adapter_pc34_compat.h"
#include "dm1_v1_f0740_f0743_music_source_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FIRESTAFF_M11_GAME_VIEW_PROBE_PRIVATE_HELPERS
/*
 * Legacy M11 probe spellings.  The production input and layout paths use
 * the DM1_V1_* names directly; retaining these aliases keeps the independent
 * source-lock probes tied to those same values instead of duplicating them.
 */
#define M11_DM1_MOUSE_MASK_LEFT DM1_V1_MOUSE_MASK_LEFT_PC34
#define M11_DM1_MOUSE_MASK_RIGHT DM1_V1_MOUSE_MASK_RIGHT_PC34
#define M11_DM1_MOUSE_LIST_INVENTORY DM1_V1_MOUSE_LIST_INVENTORY_PC34
#define M11_DM1_MOUSE_LIST_INTERFACE DM1_V1_MOUSE_LIST_INTERFACE_PC34
#define M11_DM1_MOUSE_LIST_MOVEMENT DM1_V1_MOUSE_LIST_MOVEMENT_PC34
#define M11_DM1_MOUSE_LIST_PANEL_CHEST DM1_V1_MOUSE_LIST_PANEL_CHEST_PC34
#define M11_DM1_MOUSE_SPACE_VIEWPORT DM1_V1_MOUSE_SPACE_VIEWPORT_PC34
#define M11_DM1_MOUSE_SPACE_SCREEN DM1_V1_MOUSE_SPACE_SCREEN_PC34
#define M11_DM1_MOUSE_SPACE_NONE DM1_V1_MOUSE_SPACE_NONE_PC34
#define M11_GameView_SetV1LeaderHandObject \
    DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat
#define M11_GameView_ClearV1LeaderHandObject \
    DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat
#define M11_GameView_GetV1LeaderHandThing \
    DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat
#define M11_GameView_GetV1LeaderHandObjectIconIndex \
    DM1_V1_M11Runtime_GetLeaderHandObjectIconIndexPc34Compat
#define M11_GameView_GetV1LeaderHandObjectName \
    DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat
#define M11_GameView_GetV1InventorySlotIconIndex \
    DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat
#define M11_GameView_DecodeV1InventoryActionHandScrollText \
    DM1_V1_M11Runtime_DecodeInventoryActionHandScrollTextPc34Compat
#define M11_GameView_OpenV1ActionHandChest \
    DM1_V1_M11Runtime_OpenActionHandChestPc34Compat
#define M11_GameView_GetV1OpenChestThing \
    DM1_V1_M11Runtime_GetOpenChestThingPc34Compat
#define M11_GameView_CloseV1OpenChest \
    DM1_V1_M11Runtime_CloseOpenChestPc34Compat

static inline int M11_GameView_GetV1ChampionSmallDamageGraphicId(void) {
    return dm1_v1_graphic_champion_damage_small_pc34();
}

/* Geometry probes consume the same source-owned physical rectangles that
 * F0387/F0394 blit.  Click zones remain separate C011/C013 facts. */
static inline int M11_GameView_GetV1ActionAreaZone(
    int *outX, int *outY, int *outW, int *outH) {
    DM1_V1_ActionAreaRectPc34 rect = dm1_v1_action_area_rect_pc34();
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return rect.w > 0 && rect.h > 0;
}

static inline int M11_GameView_GetV1ActionMenuGraphicZone(
    int rows, int *outX, int *outY, int *outW, int *outH) {
    DM1_V1_ActionAreaRectPc34 rect = dm1_v1_action_menu_graphic_rect_pc34(rows);
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return rect.w > 0 && rect.h > 0;
}

static inline int M11_GameView_GetV1ActionResultZoneId(void) {
    return dm1_v1_action_result_zone_id_pc34();
}

static inline int M11_GameView_GetV1ActionResultZone(
    int *outX, int *outY, int *outW, int *outH) {
    DM1_V1_ActionAreaRectPc34 rect = dm1_v1_action_result_rect_pc34();
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return rect.w > 0 && rect.h > 0;
}

static inline int M11_GameView_GetV1ActionPassZoneId(void) {
    return dm1_v1_action_pass_zone_id_pc34();
}

static inline int M11_GameView_GetV1ActionPassZone(
    int *outX, int *outY, int *outW, int *outH) {
    DM1_V1_ActionAreaRectPc34 rect = dm1_v1_action_pass_rect_pc34();
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return rect.w > 0 && rect.h > 0;
}

static inline int M11_GameView_GetV1SpellAreaZoneId(void) {
    return DM1_V1_SPELL_AREA_ZONE_ID_PC34;
}

static inline int M11_GameView_GetV1SpellAreaZone(
    int *outX, int *outY, int *outW, int *outH) {
    DM1_V1_SpellAreaRectPc34 rect = dm1_v1_spell_area_graphic_rect_pc34();
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return rect.w > 0 && rect.h > 0;
}

static inline int M11_GameView_GetV1SpellAreaBackgroundGraphicId(void) {
    return DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34;
}

static inline int M11_GameView_GetV1SpellAreaLinesGraphicId(void) {
    return DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34;
}

static inline int M11_GameView_GetV1SpellCasterPanelZoneId(void) {
    return DM1_V1_SPELL_CASTER_PANEL_ZONE_ID_PC34;
}

static inline int M11_GameView_GetV1SpellCasterPanelZone(
    int *outX, int *outY, int *outW, int *outH) {
    DM1_V1_SpellAreaRectPc34 rect = dm1_v1_spell_caster_panel_rect_pc34();
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return rect.w > 0 && rect.h > 0;
}

static inline int M11_GameView_GetV1SpellCasterTabZoneId(void) {
    return DM1_V1_SPELL_CASTER_TAB_ZONE_ID_PC34;
}

static inline int M11_GameView_GetV1SpellCasterTabZone(
    int *outX, int *outY, int *outW, int *outH) {
    DM1_V1_SpellAreaRectPc34 rect = dm1_v1_spell_caster_tab_rect_pc34();
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return rect.w > 0 && rect.h > 0;
}

static inline int M11_GameView_GetV1SpellAvailableSymbolParentZoneId(int index) {
    return dm1_v1_spell_available_symbol_parent_zone_id_pc34(index);
}

static inline int M11_GameView_GetV1SpellAvailableSymbolZoneId(int index) {
    return dm1_v1_spell_available_symbol_zone_id_pc34(index);
}

static inline int M11_GameView_GetV1SpellChampionSymbolZoneId(int index) {
    return dm1_v1_spell_champion_symbol_zone_id_pc34(index);
}

static inline int M11_GameView_GetV1SpellCastZoneId(void) {
    return DM1_V1_SPELL_AREA_CAST_ZONE_ID_PC34;
}

static inline int M11_GameView_GetV1SpellRecantZoneId(void) {
    return DM1_V1_SPELL_AREA_RECANT_ZONE_ID_PC34;
}

static inline int M11_GameView_GetV1ChampionBigDamageGraphicId(void) {
    return dm1_v1_graphic_champion_damage_big_pc34();
}

static inline int M11_GameView_GetV1ChampionPortraitGraphicId(void) {
    return dm1_v1_graphic_champion_portraits_pc34();
}

static inline int M11_GameView_GetV1ObjectIconSourceZone(
    int iconIndex, int* outGraphic, int* outX, int* outY, int* outW,
    int* outH) {
    DM1_V1_ObjectIconSourceZonePc34 zone;
    if (!outGraphic || !outX || !outY || !outW || !outH ||
        !dm1_v1_object_icon_source_zone_pc34(iconIndex, &zone)) return 0;
    *outGraphic = zone.graphic_index;
    *outX = zone.x; *outY = zone.y; *outW = zone.w; *outH = zone.h;
    return 1;
}

static inline int M11_GameView_GetF0115C2500C2900Row(
    int relForward, int relSide) {
    return dm1_viewport_3d_f0115_c2500_c2900_row(relForward, relSide);
}

static inline int M11_GameView_GetC2500ObjectRawZonePoint(
    int rowIndex, int relativeCell, int* outX, int* outY) {
    return dm1_viewport_3d_c2500_object_raw_zone_point(
        rowIndex, relativeCell, outX, outY);
}

int M11_GameView_GetV1StatusHandSlotBoxZone(
    int championSlot, int handIndex, int* outX, int* outY, int* outW,
    int* outH);

static inline int M11_GameView_GetV1StatusHandIconZone(
    int championSlot, int handIndex, int* outX, int* outY, int* outW,
    int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_champion_status_hand_icon_rect_pc34(
            championSlot, handIndex, &rect)) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

static inline int M11_GameView_GetV1InventorySourceSlotBoxZone(
    int sourceSlotBoxIndex, int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_InventorySlotBoxZonePc34 zone;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_inventory_source_slot_box_zone_pc34(
            sourceSlotBoxIndex, &zone)) {
        return 0;
    }
    *outX = zone.x; *outY = zone.y; *outW = zone.w; *outH = zone.h;
    return 1;
}

static inline int M11_GameView_GetV1InventorySourceSlotBoxZoneCount(void) {
    return dm1_v1_inventory_source_slot_box_zone_count_pc34();
}

static inline int M11_GameView_GetV1InventoryBackpackSlotZone(
    int backpackOrdinal, int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_InventorySlotBoxZonePc34 zone;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_inventory_backpack_slot_zone_pc34(backpackOrdinal, &zone)) {
        return 0;
    }
    *outX = zone.x;
    *outY = zone.y;
    *outW = zone.w;
    *outH = zone.h;
    return 1;
}

static inline int M11_GameView_GetV1InventoryPanelZone(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect;
    if (!outX || !outY || !outW || !outH) return 0;
    rect = dm1_v1_inventory_panel_rect_pc34();
    if (rect.w <= 0 || rect.h <= 0) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

/* Test-facing adapters retained over the source-owned DM1 helpers.  These
 * expose no M11-local geometry: ReDMCSB PANEL.C uses C023/C025, C503 and
 * C537..C544 through the PC34 compatibility tables. */
static inline int M11_GameView_GetV1OpenScrollPanelGraphicId(void) {
    return dm1_v1_graphic_panel_open_scroll_pc34();
}

static inline int M11_GameView_GetV1ChestSlotBoxZoneCount(void) {
    return dm1_v1_inventory_chest_slot_box_zone_count_pc34();
}

static inline int M11_GameView_GetV1ChestSlotBoxZoneId(int chestOrdinal) {
    return dm1_v1_inventory_chest_slot_box_zone_id_pc34(chestOrdinal);
}

static inline int M11_GameView_GetV1ChestSlotBoxZone(
    int chestOrdinal, int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_InventorySlotBoxZonePc34 zone;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_inventory_chest_slot_box_zone_pc34(chestOrdinal, &zone)) {
        return 0;
    }
    *outX = zone.x; *outY = zone.y; *outW = zone.w; *outH = zone.h;
    return 1;
}

static inline int M11_GameView_GetV1ArrowOrEyeZone(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect;
    if (!outX || !outY || !outW || !outH) return 0;
    rect = dm1_v1_arrow_or_eye_rect_pc34();
    if (rect.w <= 0 || rect.h <= 0) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

/* ReDMCSB: DUNVIEW.C inventory-view backdrop is the viewport rectangle.
 * Keep the legacy probe export source-owned by the DM1 layout helper. */
static inline int M11_GameView_GetV1InventoryBackdropZone(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect;
    if (!outX || !outY || !outW || !outH) return 0;
    rect = dm1_v1_inventory_backdrop_rect_pc34();
    if (rect.w <= 0 || rect.h <= 0) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

static inline int M11_GameView_GetV1MouseCommandForPoint(
    int mouseInputList, int screenX, int screenY, int buttonMask,
    int* outCoordinateSpace, int* outZoneId) {
    return DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
        mouseInputList, screenX, screenY, buttonMask, outCoordinateSpace,
        outZoneId);
}

int M11_GameView_GetV1EndgameTheEndZone(
    int* outX, int* outY, int* outW, int* outH);
int M11_GameView_GetV1EndgameRestartBox(
    int inner, int* outX, int* outY, int* outW, int* outH);
int M11_GameView_GetV1EndgameQuitBox(
    int inner, int* outX, int* outY, int* outW, int* outH);

static inline int M11_GameView_GetV1EndgameChampionMirrorZone(
    int slot, int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_EndgameRectPc34 rect;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_endgame_champion_mirror_rect_pc34(slot, &rect)) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

static inline int M11_GameView_GetV1EndgameChampionPortraitZone(
    int slot, int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_EndgameRectPc34 rect;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_endgame_champion_portrait_rect_pc34(slot, &rect)) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

static inline int M11_GameView_GetV1DamageIndicatorZoneId(int championSlot) {
    return dm1_v1_champion_damage_indicator_zone_id_pc34(championSlot);
}

static inline int M11_GameView_GetV1InventoryDamageIndicatorZoneId(
    int championSlot) {
    return dm1_v1_champion_inventory_damage_indicator_zone_id_pc34(
        championSlot);
}

static inline int M11_GameView_GetV1DamageIndicatorZone(
    int championSlot, int indicatorW, int indicatorH, int* outX, int* outY,
    int* outW, int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_champion_damage_indicator_rect_pc34(
            championSlot, indicatorW, indicatorH, &rect)) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

static inline int M11_GameView_GetV1InventoryDamageIndicatorZone(
    int championSlot, int indicatorW, int indicatorH, int* outX, int* outY,
    int* outW, int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!outX || !outY || !outW || !outH ||
        !dm1_v1_champion_inventory_damage_indicator_rect_pc34(
            championSlot, indicatorW, indicatorH, &rect)) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

static inline int M11_GameView_GetV1MessageAreaZone(
    int* outX, int* outY, int* outW, int* outH) {
    if (!outX || !outY || !outW || !outH) return 0;
    *outX = 0; *outY = 173; *outW = 320; *outH = 27;
    return 1;
}

static inline int M11_GameView_GetV1DialogChoiceTextZone(
    int choiceCount, int choiceIndex, int* outX, int* outY, int* outW,
    int* outH) {
    DM1_V1_DialogRectPc34 rect;
    if (!outX || !outY || !outW || !outH || choiceIndex < 0 ||
        !dm1_v1_dialog_choice_text_rect_pc34(choiceCount, choiceIndex,
                                              &rect)) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}

static inline int M11_GameView_GetV1DialogChoiceHitZone(
    int choiceCount, int choiceIndex, int* outX, int* outY, int* outW,
    int* outH) {
    DM1_V1_DialogRectPc34 rect;
    if (!outX || !outY || !outW || !outH || choiceIndex < 0 ||
        !dm1_v1_dialog_choice_hit_rect_pc34(choiceCount, choiceIndex,
                                             &rect)) return 0;
    *outX = rect.x; *outY = rect.y; *outW = rect.w; *outH = rect.h;
    return 1;
}
#endif

enum {
    M11_GAME_VIEW_PATH_CAPACITY = 512,
    M11_ENDGAME_F0445_REPLAY_CAPACITY = 64,
    M11_THERON_STARTUP_RENDER_ROW_CAPACITY = 80,
    M11_THERON_STARTUP_LAYOUT_LABEL_CAPACITY = 48
};

typedef enum {
    M11_THERON_STARTUP_ELEMENT_TITLE = 1,
    M11_THERON_STARTUP_ELEMENT_CHAPTER,
    M11_THERON_STARTUP_ELEMENT_CONTINUE,
    M11_THERON_STARTUP_ELEMENT_STAGE,
    M11_THERON_STARTUP_ELEMENT_MIRROR,
    M11_THERON_STARTUP_ELEMENT_FORCEFIELD
} M11_TheronStartupElementKind;

typedef struct {
    M11_TheronStartupElementKind kind;
    int phase;
    int cursor;
    int enabled;
    int selected;
    int dungeonId;
    int mirrorIndex;
    int selectedOrder;
    int portraitIndex; /* Theron startup mirror/champion portrait ordinal, -1 when absent. */
    int primaryClass;  /* Theron_ChampionClass for mirror rows, -1 when absent. */
    int saveKind; /* 0=none, 1=TQSV, 2=SRM */
    int saveSlot;
    int x;
    int y;
    int w;
    int h;
    char label[M11_THERON_STARTUP_LAYOUT_LABEL_CAPACITY];
    char decodedName[16];
    char decodedTitle[32];
} M11_TheronStartupElement;

typedef enum {
    M11_GAME_INPUT_IGNORED = 0,
    M11_GAME_INPUT_REDRAW = 1,
    M11_GAME_INPUT_RETURN_TO_MENU = 2,
    M11_GAME_INPUT_RESTART_GAME = 3
} M11_GameInputResult;

typedef enum {
    M11_ENDGAME_F0445_EVENT_NONE = 0,
    M11_ENDGAME_F0445_EVENT_SETUP = 1,
    M11_ENDGAME_F0445_EVENT_FIREBALL_BURST = 2,
    M11_ENDGAME_F0445_EVENT_LORD_ORDER = 3,
    M11_ENDGAME_F0445_EVENT_HARM_BURST = 4,
    M11_ENDGAME_F0445_EVENT_CHAOS_ORDER_SWITCH = 5,
    M11_ENDGAME_F0445_EVENT_FINAL_EXPLOSIONS = 6,
    M11_ENDGAME_F0445_EVENT_GREY_LORD = 7,
    M11_ENDGAME_F0445_EVENT_FLUXCAGE_HIDE = 8,
    M11_ENDGAME_F0445_EVENT_GROUP_CLEANUP = 9,
    M11_ENDGAME_F0445_EVENT_TEXT_MESSAGE = 10
} M11_EndgameF0445ReplayEventType;

typedef enum {
    M11_GAME_SOURCE_BUILTIN_CATALOG = 0,
    M11_GAME_SOURCE_CUSTOM_DUNGEON,
    M11_GAME_SOURCE_DIRECT_DUNGEON,
    M11_GAME_SOURCE_NEXUS_DGN,
    M11_GAME_SOURCE_THERON_TRACK02,
    M11_GAME_SOURCE_CSB_BOOT, /* CSB V1 hand-off: M11 owns the verified
                               * CSB boot profile and runtime save/resume
                               * boundary. The CSB viewport/gameplay bridge
                               * is still narrower than DM1, so M11 dispatch
                               * paths must not fall through to the DM1 world
                               * tick or loader when this source is active. */
    M11_GAME_SOURCE_DM2_BOOT /* DM2 V1 hand-off: M11 owns the launch but the
                              * actual game state + tick + rendering go
                              * through dm2_v1_boot_enter_game() and the
                              * DM2 V1/V2 runtime libraries. The sourceKind
                              * tag tells M11 to skip the DM1 dungeon loader
                              * and the DM1 m11_apply_tick path for this
                              * game (added 2026-06-17). */
} M11_GameSourceKind;

typedef struct {
    const char* title;
    const char* gameId;
    const char* dataDir;
    const char* sourceId;
    const char* dungeonPath;
    const char* verifiedAssetPath; /* Optional: hash-verified single-file launch path. */
    const char* verifiedAssetMd5;  /* Optional: expected MD5 for verifiedAssetPath. */
    const Theron_Track02StartupLoaderReceipt* theronTrack02LoaderReceipt;
    const Theron_V1Track02CampaignMediaDiscoveryReceipt* theronCampaignMedia;
    const Theron_V1Track02CaptureTargetPlan* theronCampaignMediaPlan;
    uint32_t theronCampaignMediaScanEpoch;
    const Theron_V1SrmCampaignReplayReceipt* theronSrmCampaignReplay;
    const Theron_V1SrmLaunchDiscoveryReceipt* theronSrmLaunchDiscovery;
    const Theron_V1Track02LaunchTraceIdentityReceipt* theronLaunchTraceIdentity;
    const Theron_V1Track02TraceBundleReceipt* theronTraceBundle;
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt* theronSectorRecordCorpus;
    const Theron_V1Track02DungeonCapturePlanAdmissionReceipt* theronDungeonCapturePlan;
    const Theron_V1Track02HandoffArtifactCorpusReceipt* theronHandoffArtifactCorpus;
    /* Optional authenticated Track 02 capture manifest. The Theron launch
     * consumes it before publishing its detached runtime receipt. */
    const char* theronTrack02CaptureManifestPath;
    const char* savePath; /* Optional quick-resume save to restore after dungeon init. */
    const char* entranceResumeSavePath; /* Optional validated save for in-entrance Resume buttons. */
    const char* csbImportDm1SavePath; /* Optional CSB utility startup import candidate. */
    uint32_t csbSaveCandidateIdentity; /* M12-selected CSB save discovery identity. */
    int languageIndex;
    int rendererBackend;
    int presentationMode;
    int presentationWidth;
    int presentationHeight;
    int fontScale;        /* Accessibility: font size scale (1..3), 0 = use default */
    /* Jobb F2: bounded launcher-options runtime handoff snapshot from
     * M12_LaunchIntent.launcherOptions.  Consumed by M11_GameView_Start
     * into M11_GameViewState.launcherOptions when launcherOptionsBound
     * is 1; direct (non-launcher) starts leave it unbound and the game
     * view keeps its zero/default options. */
    M12_LauncherRuntimeOptions launcherOptions;
    int launcherOptionsBound;
    int hudLaunchMode;    /* Theron V2 HUD launch-mode selector (M11-side).
                           * Maps onto Theron_V2_HudLaunchMode via
                           * theron_v2_hud_launch_mode_from_m11().
                           * 0=OFF (V1 chrome), 1=OVERLAY, 2=TOUCH,
                           * 3=CONTROLLER. Only consulted when gameId
                           * is "theron"; ignored for DM1/CSB/DM2/Nexus.
                           * Source-lock: see
                           * include/theron_v2_hud_launch_mode_pc34.h
                           * resolution table; presentation-only
                           * contract, never mutates V1 state. */
    M11_GameSourceKind sourceKind;
} M11_GameLaunchSpec;

enum {
    M11_RUNTIME_CATALOG_MAX_ENTRIES = 1024,
    M11_RUNTIME_CATALOG_MSGID_CAPACITY = 96,
    M11_RUNTIME_CATALOG_MSGSTR_CAPACITY = 128
};

typedef struct {
    char msgid[M11_RUNTIME_CATALOG_MSGID_CAPACITY];
    char msgstr[M11_RUNTIME_CATALOG_MSGSTR_CAPACITY];
} M11_RuntimeCatalogEntry;

typedef struct {
    M11_RuntimeCatalogEntry entries[M11_RUNTIME_CATALOG_MAX_ENTRIES];
    int entryCount;
    int loaded;
} M11_RuntimeCatalog;

enum {
    M11_MESSAGE_LOG_CAPACITY = 6,
    M11_MESSAGE_MAX_LENGTH = 80
};

typedef struct {
    char text[M11_MESSAGE_MAX_LENGTH];
    unsigned char color;
} M11_LogEntry;

typedef struct {
    M11_LogEntry entries[M11_MESSAGE_LOG_CAPACITY];
    int writeIndex;
    int count;
} M11_MessageLog;

enum {
    M11_BOOT_RECEIPT_SOURCE_ID_CAPACITY = 32,
    M11_BOOT_RECEIPT_PHASE_CAPACITY = 48,
    M11_BOOT_RECEIPT_ANIMATION_CAPACITY = 48,
    M11_BOOT_RECEIPT_MD5_CAPACITY = 33
};

typedef struct {
    int active;
    M11_GameSourceKind sourceKind;
    char sourceId[M11_BOOT_RECEIPT_SOURCE_ID_CAPACITY];
    char bootAssetMd5[M11_BOOT_RECEIPT_MD5_CAPACITY];
    int startedFromLauncher;
    int dm1StartupIntroBypassed;
    int dm1StartupHandoffExecuted;
    int startupActive;
    char startupPhase[M11_BOOT_RECEIPT_PHASE_CAPACITY];
    int startupFrame;
    char startupAnimation[M11_BOOT_RECEIPT_ANIMATION_CAPACITY];
    int startupAnimationActive;
    int startupTitleFrame;
    int startupTitleFrameMax;
    int startupTitleReady;
    int startupInputReady;
    int startupHudMenuReady;
    int startupHudMenuKind;
    int startupHudMenuOptionCount;
    int startupSelectedCommandId;
    int startupUtilitySelectedActionIndex;
    int csbStartupFullVisualSequenceReady;
    int csbStartupNoFallbackRoutes;
    int csbStartupSequenceCaptureHash;
    int dm1HoCFullGraphicsReady;
    int dm1HoCHostRenderPlanReady;
    int dm1HoCCaptureProofPassed;
    int dm1HoCRuntimeApplyReady;
    int dm1HoCProductionConsumerReady;
    int dm1HoCNoHostFallbackVisuals;
    int dm1HoCRealAssetCapture;
    int dm1HoCMacWindowCapture;
    int dm1HoCReleaseAppCapture;
    int dm1HoCHostCaptureRouteMatches;
    int dm1HoCReleaseCaptureOwnershipReady;
    int dm1HoCHostRenderConsumerReady;
    int dm1HoCM11BootProbeConsumerReady;
    int dm1HoCLaunchPathReady;
    int dm1HoCRequiredAssetCapture;
    int dm1HoCReceiptOnlyConsumerReady;
    int dm1HoCLowerLevelHelpersReady;
    int dm1HoCHostDrawUsesOwnedReceipt;
    int dm1HoCHostDrawConsumesBackingAsset;
    int dm1HoCHostDrawRejectsBackingFallback;
    int dm1HoCHoCAssetCapture;
    int dm1HoCHostWindowCapture;
    int dm1HoCPresentedCapture;
    int dm1HoCPresentedCaptureWidth;
    int dm1HoCPresentedCaptureHeight;
    int dm1HoCPresentedCaptureGeometry;
    int dm1HoCPresentedCapturePixels;
    int dm1HoCPresentedCaptureBytes;
    unsigned int dm1HoCPresentedCaptureHash;
    int dm1HoCPresentedCaptureChainReady;
    unsigned int dm1HoCPresentedCaptureConsumerMask;
    unsigned int dm1HoCPresentedCaptureChainHash;
    int dm1HoCHostCaptureRoutePackaged;
    unsigned int dm1HoCHostCaptureRouteMask;
    unsigned int dm1HoCHostCaptureRouteHash;
    int dm1HoCPresentedCaptureRoutePackaged;
    int dm1HoCOpenedEntranceFrame;
    int dm1HoCHallMirrorOverlay;
    int dm1HoCBlockedEnterUntilChampion;
    /* Live M11 evidence only: static HoC asset receipts must not open a
     * release/app capture route before the active front cell requests C127
     * or F0115 material. */
    int dm1HoCLiveC127MaterialRequest;
    int dm1HoCLiveF0115MaterialRequest;
    int dm1HoCMapWidth;
    int dm1HoCMapHeight;
    int dm1HoCRenderCommandCount;
    int dm1CompleteSupportReady;
    int dm1CompleteSourceVisibleStartup;
    int dm1CompleteEntranceToHoC;
    int dm1CompleteHoCRenderRoute;
    int dm1CompleteHostAppCaptureRoute;
    int dm1CompleteSaveCorpusRoute;
    int dm1CompleteOriginalSaveRoundtripRoute;
    DM1_V1_StartupHoCBootProbeSummary_PC34 dm1HoCBootSummary;
    int startupInitializeV2Runtime;
    int startupInitializeHudRuntime;
    int startupInitializeTouchRuntime;
    int startupHudRuntimeReady;
    int dm2ExtendedSpellGdatReady;
    unsigned int dm2ExtendedSpellGdatDefinedCount;
    unsigned int dm2ExtendedSpellGdatWordHash;
    int csbPresentedFrameCaptureReady;
    int csbPresentedFrameRunningFromMacOSApp;
    int csbPresentedFrameMacWindowReady;
    int csbPresentedFrameWidth;
    int csbPresentedFrameHeight;
    unsigned int csbPresentedFrameHash;
    int levelLoaded;
    int mapIndex;
    int partyX;
    int partyY;
    int partyDir;
    int championCount;
    int runtimeTick;
    uint32_t dm1WorldTick;
} M11_BootProbeReceipt;

typedef struct {
    int active;
    int startedFromLauncher;
    int dm1StartupIntroBypassed;
    int dm1StartupHandoffExecuted;
    char title[64];
    char sourceId[32];
    char bootAssetMd5[M11_BOOT_RECEIPT_MD5_CAPACITY];
    M11_GameSourceKind sourceKind;
    int presentationMode;
    int presentationWidth;
    int presentationHeight;
    /* F10 runtime presentation panel. It changes only M11 presentation
     * state; dungeon, save and source-asset ownership remain untouched. */
    int graphicsPopupActive;
    int graphicsPopupPage;
    int graphicsPopupSelectedRow;
    int fpsOverlayEnabled;
    unsigned int fpsOverlayValue;
    unsigned int fpsOverlayFrameCount;
    uint64_t fpsOverlaySampleStartedMs;
    int hudLaunchMode;    /* Theron V2 HUD launch-mode selector (M11-side,
                           * 0=OFF / 1=OVERLAY / 2=TOUCH / 3=CONTROLLER).
                           * Stored from spec->hudLaunchMode on launch;
                           * consulted by m11_theron_render_v2_hud() so
                           * the HUD overlay obeys the launcher's
                           * launch-mode intent. Presentation-only
                           * contract; never mutates V1 input / champion
                           * / world / save state. See
                           * include/theron_v2_hud_launch_mode_pc34.h. */
    char dungeonPath[M11_GAME_VIEW_PATH_CAPACITY];
    char lastAction[32];
    char lastOutcome[64];
    char inspectTitle[64];
    char inspectDetail[512];
    int v1ObjectDescriptionPanelActive;
    unsigned short v1ObjectDescriptionThing;
    int v1ObjectDescriptionIconIndex;
    /* PANEL.C F0332/F0335/F0336 source-material receipt.  The panel may
     * draw only while its raw Thing record and GRAPHICS.DAT icon source
     * still agree with this receipt. */
    int v1ObjectDescriptionSourceMaterialValid;
    int v1ObjectDescriptionSourceGraphicIndex;
    char v1ObjectDescriptionName[64];
    char v1ObjectDescriptionBody[256];
    int v1ScrollPanelActive;
    unsigned short v1ScrollPanelThing;
    int v1ChampionStatsPanelActive;
    int v1FoodWaterPanelActive;
    /* Pass 42: V1-chrome-mode reroute bookkeeping.  Last payload
     * pushed into the message log from m11_set_status /
     * m11_set_inspect_readout, used to suppress back-to-back
     * duplicate pushes (the renderer may call the setters with
     * identical strings across frames).  Not part of the save
     * format; zero-initialised by memset in M11_GameView_Init. */
    char chromeRerouteLastStatus[96];
    char chromeRerouteLastInspect[128];
    uint32_t lastWorldHash;
    struct TickResult_Compat lastTickResult;
    struct GameWorld_Compat world;
    /* Pass345: live route tokens feed the source-locked DM1 V1
     * COMMAND.C F0380 -> CLIKMENU.C F0365/F0366 movement pipeline before
     * the viewport redraw result is returned to main_loop_m11.c. */
    struct Dm1V1MovementPipelinePc34Compat dm1V1MovementPipeline;
    struct Dm1V1MovementPipelineResultPc34Compat lastDm1V1MovementPipelineResult;
    struct ChampionMirrorCatalog_Compat mirrorCatalog;
    int mirrorCatalogAvailable;
    /* Source-backed champion mirror candidate panel.  Mirrors the
     * G0299_ui_CandidateChampionOrdinal + M568_PANEL_RESURRECT_REINCARNATE
     * flow at M11 state level: selecting a front-cell mirror appends the
     * candidate champion to the party immediately (F0280), records its
     * DUNGEON.DAT mirror ordinal/party index, then confirm/cancel consumes
     * or removes the candidate (F0282). */
    int candidateMirrorOrdinal;
    int candidateMirrorPartyIndex;
    int candidateMirrorPanelActive;
    int candidateMirrorRenameActive;
    DM1_V1_ResurrectionRenameUiGatePc34Compat candidateMirrorRename;
    uint32_t lastPartyMovementTick;
    /* ReDMCSB CHAMPION.C F0316/F0317/F0331 source state for Thieves Eye. */
    DM1_V1_NeedsScentListPc34Compat championScents;
    M11_MessageLog messageLog;
    int resting;
    int partyDead;
    unsigned char championDeathHandledMask;
    uint32_t exploredBits[32]; /* 32 * 32 = 1024 cells tracked per level */

    /* Asset loader for GRAPHICS.DAT-backed rendering */
    M11_AssetLoader assetLoader;
    int assetsAvailable; /* 1 if assetLoader is ready */
    M11_FontState originalFont; /* DM1 font from GRAPHICS.DAT */
    int originalFontAvailable;
    M11_AudioState audioState;
    int audioEventCount;

    /* ── Per-map ornament index cache ──
     * In DM1, each map has a per-map wall/floor/door ornament index
     * table stored in the DUNGEON.DAT metadata area.  The ordinal
     * from the sensor thing is looked up in this table to get the
     * actual ornament graphic index.  We cache these per-map tables
     * here, loaded lazily from DUNGEON.DAT the first time a map is
     * visited.  Ref: ReDMCSB G0261_auc_CurrentMapWallOrnamentIndices. */
    int wallOrnamentIndices[32][16];   /* [mapIndex][ordinal] -> graphic index */
    int floorOrnamentIndices[32][16];  /* [mapIndex][ordinal] -> graphic index */
    int doorOrnamentIndices[32][16];
    int ornamentCacheLoaded[32];       /* 1 if loaded for this map */

    /* Spell casting UI state */
    int spellPanelOpen;          /* 1 when rune entry panel is visible */
    int spellRuneRow;            /* current rune row (0..3) = power/element/form/class */
    struct RuneSequence_Compat spellBuffer; /* runes entered so far */
    /* ReDMCSB DEFS.H Champion.Symbols[5]/SymbolStep plus
     * G0514_i_MagicCasterChampionIndex.  spellBuffer/spellRuneRow remain
     * the active-caster presentation view for existing M11 consumers; this
     * record is the source-owned per-caster storage behind C109/F0394. */
    DM1_SpellCastingState dm1SpellCasting;

    /* ── Creature animation state ── */
    /* Global animation tick — incremented each game tick, drives all
     * creature frame cycling and attack flash timers. */
    uint32_t animTick;

    /* Damage-flash timer.  Set to M11_DAMAGE_FLASH_DURATION when the
     * party takes creature melee damage.  Decremented each tick.
     * While > 0, the viewport border flashes red. */
    int damageFlashTimer;

    /* Per-champion damage indicator state.
     * When a champion takes damage, the corresponding timer is set to
     * M11_DAMAGE_FLASH_DURATION and the amount is recorded.  While > 0,
     * the GRAPHICS.DAT damage-to-champion overlay (graphic 15, 45×7)
     * is drawn on top of the champion's status box with the damage
     * number.  Ref: ReDMCSB CHAMPION.C F0291. */
    int championDamageTimer[4];  /* per-slot countdown */
    int championDamageAmount[4]; /* last damage dealt */

    /* Front-cell attack indicator timer.  Set to M11_ATTACK_CUE_DURATION
     * when a creature in the front cell (depth 0) attacks.  Decremented
     * each tick.  While > 0, draw slash-mark overlay on the viewport. */
    int attackCueTimer;

    /* Creature type that last attacked (for attack-cue sprite). */
    int attackCueCreatureType;

    /* Creature-hit overlay state (GRAPHICS.DAT graphic 14).
     * When the party deals melee damage to a creature, this timer is set
     * and the damage amount recorded.  While > 0, graphic 14 is drawn
     * centered on the viewport with the damage number overlaid.
     * Ref: ReDMCSB MELEE.C — C014_GRAPHIC_DAMAGE_TO_CREATURE. */
    int creatureHitOverlayTimer;
    int creatureHitDamageAmount;
    /* Transient visual echo for the source movement-arrow panel.  Keyboard
     * movement keys (arrows, WASD, Home/End, Q/E) enter M11 as the same
     * M12_MENU_INPUT_* tokens as mouse/touch arrow clicks, so this stores
     * the C068..C073 arrow zone mask that should be highlighted on the next
     * V1 chrome redraw.  Presentation-only; not part of any save format. */
    int v1MovementArrowVisualMask;
    int v1MovementArrowVisualTicks;

    /* ── Full-screen overlay state ── */
    int mapOverlayActive;        /* 1 when full-screen map is displayed */
    int inventoryPanelActive;    /* 1 when full inventory grid is displayed */
    int inventorySelectedSlot;   /* currently highlighted slot index (-1 = none) */
    int csbGameFrozen;           /* CSB C147/C148 freeze/unfreeze keyboard state */
    int csbDiskMenuActive;       /* CSB Ctrl-S disk-menu overlay placeholder */
    /* ReDMCSB PANEL.C F0349 food mouth animation.  The source draws four
     * icon frames into C545_ZONE_MOUTH with an 8-delay gate after a
     * leader-hand food object is removed.  These fields preserve that
     * transient visual across M11 full-frame redraws. */
    int v1MouthVisualIconIndex;  /* current C205/C206 icon to blit; 0 = none */
    int v1MouthAnimationFrameCount;
    int v1MouthAnimationFrameIndex;
    int v1MouthAnimationDelayRemaining;
    int v1MouthAnimationIcons[4];
    int v1MouthAnimationDelays[4];

    /* ── Endgame / dialog flow state ── */
    /* Set to 1 when ORCH_GAME_WON / EMIT_GAME_WON fires.  Blocks all
     * gameplay input; only ESC (return to menu) is accepted. */
    int gameWon;
    uint32_t gameWonTick;  /* tick when victory was detected */
    int endgameDoNotDrawFluxcages;
    int endgameFinalDelayTicks;
    int endgameRestartAllowed;
    int endgameRestartRequested;
    int endgameCalledWithTrue;
    int endgameChaosOrderSwitchCount;
    int endgameFuseSequenceUpdateTicks;
    int endgameFuseSequenceTotalUpdateTicks;
    int endgameFuseSequenceFrameReplayTicks;
    int endgameFuseSequenceFrameReplayRemainingTicks;
    int endgameFuseSequenceReplayCursor;
    int endgameFuseSequenceReplayEventCount;
    int endgameFuseSequenceReplayTypes[M11_ENDGAME_F0445_REPLAY_CAPACITY];
    int endgameFuseSequenceReplayAttacks[M11_ENDGAME_F0445_REPLAY_CAPACITY];
    int endgameFuseSequenceReplayCreatureTypes[M11_ENDGAME_F0445_REPLAY_CAPACITY];
    /* F0446 waits 780 ticks immediately after each source text-message
     * F0445 redraw, rather than batching all message delays at the end. */
    int endgameFuseSequenceReplayDelayTicks[M11_ENDGAME_F0445_REPLAY_CAPACITY];
    int endgameFuseSequenceCurrentReplayType;
    int endgameFuseSequenceCurrentReplayAttack;
    int endgameFuseSequenceCurrentReplayCreatureType;
    int endgameTextMessageDelayTicks;
    int endgameFuseSequenceDelayTicks;
    int endgameFuseSequenceDelayRemainingTicks;
    int endgameFinalDelayPendingTicks;
    int endgameFinalHandoffReady;
    int endgameBuzzRequestCount;

    /* Dialog box overlay for text-plaque inspection.
     * When dialogOverlayActive is 1, a styled dialog panel is rendered
     * on top of the viewport showing dialogOverlayText.  The user
     * dismisses it with any key or click. */
    int dialogOverlayActive;
    int returnToMenuConfirmActive; /* Esc confirmation modal before leaving gameplay */
    /* G2018_ul_LastSaveTime quit-guard (source: ReDMCSB LOADSAVE.C:267,1371,1714).
     * lastSaveTick mirrors G2018; updated by M11_GameView_QuickSave and the
     * Ctrl-S save command.  loadGameTick mirrors G0319_ul_LoadGameTime.
     * The ESC quit handler (m11_game_view.c, M12_MENU_INPUT_BACK case) consults
     * these to decide whether to show the "GAME NOT SAVED" prompt instead of
     * the plain "RETURN TO START MENU?" dialog. */
    uint32_t lastSaveTick;
    uint32_t loadGameTick;
    /* DM1 F0172/F0115 consumes live projectile/effect lists after every
     * handoff. This records provenance only and never selects fallback art. */
    int dm1ViewportRuntimeOrigin;
    /* DRAWVIEW.C F0097 G2123 palette cache. Kept per game view, never as a
     * global host default, so a recreated/reloaded session starts at -1. */
    int dm1ViewportPresentedPaletteIndex;
    /* quitGuardActive: 1 while the GAME-NOT-SAVED prompt is on screen.  When
     * the user picks SAVE&QUIT the dialog handler honors this flag and runs
     * a save before returning to the launcher. */
    int quitGuardActive;
    /* ReDMCSB LOADSAVE.C F0433 owns a four-choice save-disk dialog before
     * writing.  Firestaff maps the selected original save disk to its local
     * save namespace; zero means no such dialog is active. */
    int dm1SaveDiskMenuStage;
    char dialogOverlayText[128];
    int dialogChoiceCount;
    int dialogSelectedChoice;
    char dialogChoices[4][32];
    char localeCode[8];
    M11_RuntimeCatalog localizedCatalog;
    M11_RuntimeCatalog englishCatalog;

    /* V1 presentation mode: when showDebugHUD is 0 (default), the
     * in-game screen omits developer-facing metadata, keybinding
     * helpers, tick counters, and diagnostic square summaries.
     * Set to 1 via FIRESTAFF_DEBUG_HUD=1 environment variable. */
    int showDebugHUD;

    /* Session timer runtime handoff.  Populated by
     * M11_GameView_OpenSelectedMenuEntry() from the launcher's
     * sessionTimerIndex setting (see include/session_timer_runtime.h
     * + src/shared/session_timer_runtime.c).  The M11 main loop ticks
     * this once per second of active gameplay, then consults
     * SessionTimerRuntime_Poll() to decide whether to surface a
     * reminder overlay (REMINDER_DUE) or a forced-pause confirm
     * dialog (FORCED_PAUSE).  This is the in-game side of the
     * M12-owned Session Timer setting; the launcher side persists the
     * setting and exposes the limit/remaining-time helpers. */
    SessionTimerRuntime sessionTimerRuntime;
    /* Latched while the M11 main loop has surfaced a forced-pause
     * confirm dialog and is awaiting user input.  Cleared once the
     * user picks a confirm-dialog choice (Continue / Return to menu)
     * via M11_GameView_ClearSessionTimerForcedPause(). */
    int sessionTimerForcedPauseDialogActive;
    /* Latched while a reminder overlay is on screen.  Cleared when
     * the user dismisses the reminder or the forced-pause dialog
     * takes precedence. */
    int sessionTimerReminderOverlayActive;

    /* Source transient leader-hand object.  Mirrors DM1
     * G4055_s_LeaderHandObject at V1 presentation/runtime level: this
     * is the mouse-held object, distinct from any champion inventory
     * ready/action-hand slot.  `leaderHandObjectPresent == 0` means
     * G4055_s_LeaderHandObject.Thing is C0xFFFF_THING_NONE. */
    int leaderHandObjectPresent;
    unsigned short leaderHandThing;
    int leaderHandIconIndex;
    char leaderHandObjectName[32];
    int pointerPositionKnown;
    int pointerX;
    int pointerY;
    /* COMMAND.C G0449 selects the source slot on button-down; a later
     * button-up may place the carried raw Thing through another G0449 slot. */
    int v1InventoryDragActive;
    int v1InventoryDragSourceSlotBox;

    /* Source inventory open-chest state.  Mirrors ReDMCSB
     * G0426_T_OpenChest at the V1 presentation bridge: THING_NONE means
     * no chest panel is currently open; otherwise the value is the
     * container thing whose action-hand icon must render as C145 while
     * the inventory panel is open.  v1OpenChestOpenedByEye preserves the
     * CHEST.C F0333 lines 43-46 P0694_B_PressingEye branch: the eye route
     * opens the C025 chest panel without drawing C145 into C09. */
    unsigned short v1OpenChestThing;
    int v1OpenChestOpenedByEye;

    /* Acting-champion ordinal.  Mirrors DM1
     * G0506_ui_ActingChampionOrdinal exactly: 0 = no champion is
     * acting (idle action area with four action-hand icon cells,
     * F0387 icon-mode branch).  1..N = champion at index N-1 has
     * been activated by a click on their action-hand cell and the
     * action area is in menu mode (F0387 menu-mode branch,
     * rendering graphic 10 + champion name + up to three action
     * names from the item's ActionSet).
     *
     * Toggled by M11_GameView_HandlePointer when a click falls
     * inside an action-hand icon cell.  Cleared automatically
     * when the acting champion dies, is replaced, or the party
     * enters rest — matches F0388_MENUS_ClearActingChampion
     * semantics (the subset we currently model).
     *
     * Ref: ReDMCSB MENU.C F0389_MENUS_SetActingChampion,
     *      F0388_MENUS_ClearActingChampion,
     *      ACTIDRAW.C F0387_MENUS_DrawActionArea menu-mode branch. */
    unsigned int actingChampionOrdinal;
    /* ReDMCSB MENU.C G0491 + F0407 / TIMELINE.C F0253 bounded
     * action-lock mirror.  DM1 disables a champion's action icon
     * after most F0407 actions and re-enables it when the action
     * timer expires.  M11 keeps the visible lockout as transient state;
     * the paired ChampionState_Compat actionDefense/actionIndex fields carry
     * the ReDMCSB F0391/F0253 defense modifier across runtime/save paths. */
    unsigned char actionDisabledTicks[CHAMPION_MAX_PARTY];
    unsigned char actionDisabledIndex[CHAMPION_MAX_PARTY];
    unsigned char actionEnableSlotOrdinal[CHAMPION_MAX_PARTY];
    unsigned char pendingShootReadyHandRefill[CHAMPION_MAX_PARTY];
    DM1_V1_LiveActionEffectsPc34 dm1LiveActionEffects;

    /* DM1 V1 VBlank-based timing state.
     * Simulates the PAL 50Hz VBlank interrupt handler (VBLANK.C:F0577)
     * and the game loop tick gate (GAMELOOP.C:F0002).
     * Authentic interval: 10 VBlanks * 20ms = 200ms per game tick. */
    DM1_V1_VBlankTimingState vblankTiming;

    /* DM1 V1 Save/Load menu state.
     * Tracks ESC-triggered save dialog matching ReDMCSB
     * C140_COMMAND (LOADSAVE.C F0433). */
    struct DM1SaveMenuContext saveMenu;
    uint32_t dm1GameID;  /* Persistent game ID for save file matching */
    int dm1MusicOn;      /* ReDMCSB G2024_B_PendingMusicOn runtime state */

    DM1_V1_F0740F0743MusicSourcePc34 dm1MusicSource;
    DM1_V1_F0740F0743MusicStatePc34  dm1MusicState;
    DM1_V1_F0740F0743MusicDriverPc34 dm1MusicDriver;
    int dm1MusicSourceBound;

    /* DM1 V2 Phase 5 smooth movement camera state.
     * These fields carry the presentation-layer interpolation offsets
     * produced by dm1_v2_camera_controller_pc34 between V1 movement
     * ticks.  The game state (party.mapX/Y, direction, cooldowns,
     * sensors, creature AI, redraw cadence) is untouched.
     *
     * camera_offset_x/y: pixel-space sub-grid camera nudge (0 when idle).
     *   Source-lock: DUNGEON.C:1371-1391 applies direction-step tables
     *   to produce discrete map coordinates; DUNVIEW.C:8606-8612 renders
     *   from those coordinates without any presentation offset.
     *   camera_offset is V2-only presentation smoothing.
     *
     * camera_interpolated_facing: 8-way facing (0..7) sampled during
     *   a turn animation.  Source-lock: GAMELOOP.C:90 redraws from
     *   G0308_i_PartyDirection (discrete).  V2 turns interpolate the
     *   visual facing without changing creature/sensor timing.
     *
     * camera_duration_ms: configured camera animation duration.
     *   Default 96ms (~2 V1 ticks) from dm1_v2_movement_command_adapter.
     *
     * Source anchors: COMMAND.C:2096-2106 cooldown gate, CLIKMENU.C:278-329
     * collision, MOVESENS.C:752-818 sensor/scent, GAMELOOP.C:69-155
     * timeline/redraw cadence.
     *
     * p5_camera: private camera controller for V2 presentation interpolation.
     *   Initialized in M11_GameView_Init, ticked in m11_apply_dm1_v1_pipeline_tick
     *   after each accepted movement command.  When smoothing is disabled the
     *   camera stays inactive and camera_offset_x/y remain zero (V1 path unchanged).
     *   This must embed the full DM1_V2_CameraController, including completion
     *   callback fields read by dm1_v2_camera_tick(). */
    int camera_offset_x;
    int camera_offset_y;
    int16_t camera_interpolated_facing;
    int camera_duration_ms;
    DM1_V2_CameraController p5_camera;

    /* Nexus V1 engine — active when sourceKind == M11_GAME_SOURCE_NEXUS_DGN.
     * Owned by nexus_v1_launcher.c (singleton). */
    struct Nexus_V1_Engine *nexusEngine;
    void *nexusTitleScreen; /* Nexus_TitleScreen*, owned by M11 startup. */
    Nexus_V1_LightRuntime nexusLightRuntime;
    int nexusLightRuntimeReady;
    struct {
        int level_loaded;
        int party_x, party_y, party_dir;
        int tick_count;
        int title_active;
        int title_loaded;
        int title_frame;
        int champion_select_active;
        int champion_cursor;
        int champion_select_frame;
        int startup_save_select_active;
        int startup_save_selected_row;
        int startup_save_row_count;
        unsigned int startup_save_slot_mask;
        char startup_save_dir[512];
        int startup_runtime_handoff_ready;
        int startup_dgn_render_ready;
        int startup_hud_ready;
        int startup_dgn_render_command_count;
        int startup_dgn_render_blocked;
        int startup_host_caller_ready;
        int startup_host_capture_ready;
        int startup_host_dgn_ready;
        int startup_host_execute_startup_draws;
        int startup_host_execute_dgn_draws;
        int startup_bpk_handoff_consumed;
        int startup_prs3_blocker_consumed;
        int startup_dgn_handoff_consumed;
        int startup_no_fallback_visuals_enforced;
        int startup_suppress_fallback_visuals;
        int startup_suppress_legacy_placeholder_visuals;
        int startup_full_start_package_consumed;
        int startup_bundle_consumed;
        int startup_display_callers_use_package_receipt;
        int startup_saturn_timing_exact;
        int startup_saturn_capture_frames_exact;
        int startup_saturn_warning_frame;
        int startup_saturn_title_capture_frame;
        int startup_saturn_save_capture_frame;
        int startup_saturn_champion_capture_frame;
        int startup_saturn_dungeon_capture_frame;
        int startup_saturn_title_ready_frame;
        int startup_saturn_gameover_capture_frame;
        int startup_host_active_capture_frame;
        int startup_host_saturn_active_capture_frame;
        int startup_host_route_consumes_active_capture_frame;
        int startup_host_route_consumes_dungeon_capture_frame;
        int startup_host_route_capture_matrix_ready;
        int startup_host_route_capture_matrix_exact;
        int startup_host_saturn_non_title_capture_count;
        unsigned int startup_host_saturn_non_title_capture_mask;
        unsigned int startup_host_saturn_expected_capture_mask;
        int startup_title_timing_frame;
        int startup_title_timing_frame_max;
        int startup_title_timing_ready;
        int startup_package_capture_consumed;
        int startup_package_route_matches_capture_route;
        int startup_host_route_consumes_package_route;
        int startup_host_route_consumes_capture_matrix;
        int startup_dgn_route_consumes_startup_package;
        int startup_dgn_route_saturn_capture_exact;
        int startup_host_ownership_route_matches_capture_route;
        int startup_package_route_consumes_host_ownership;
        int startup_dgn_route_consumes_host_ownership;
        int startup_route_consumption_complete;
        int startup_non_title_saturn_capture_route_complete;
        int startup_dungeon_route_consumption_complete;
        int startup_save_route_consumes_package_capture;
        int startup_champion_route_consumes_package_capture;
        int startup_dungeon_route_consumes_package_capture;
        int startup_save_route_saturn_capture_exact;
        int startup_champion_route_saturn_capture_exact;
        int startup_dungeon_route_saturn_capture_exact;
        int startup_save_host_package_route_complete;
        int startup_champion_host_package_route_complete;
        int startup_dungeon_host_package_route_complete;
        unsigned int startup_host_package_route_complete_mask;
        unsigned int startup_host_package_route_expected_mask;
        int startup_host_package_route_matrix_complete;
        int startup_single_saturn_owner_ready;
        int startup_title_menu_capture_route_joined;
        int startup_runtime_dgn_route_joined;
        int startup_blocked_route_suppresses_all_draws;
        int startup_copied_draw_command_count;
        int startup_copied_dgn_render_command_count;
        int startup_copied_dgn_material_plan_complete;
        int startup_dgn_viewport_host_route_ready;
        int startup_dgn_viewport_host_route_status;
        int startup_dgn_viewport_host_package_consumed;
        int startup_dgn_viewport_host_route_consumed;
        int startup_dgn_viewport_host_blocks_runtime;
        int startup_dgn_viewport_host_written_pixels;
        int startup_dgn_viewport_host_rasterized_commands;
        int startup_dgn_viewport_host_material_surfaces;
        /* M11 only presents the DGN plan supplied by the Nexus startup
         * host route. Keeping the accepted plan here lets a champion start
         * or save-resume enter the dungeon without falling back to the
         * generic Nexus viewport. */
        Nexus_V1_DgnRenderCommand
            startup_dgn_render_commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
        int startup_dgn_render_cached_count;
    } nexusState;

    /* Theron's Quest V1 runtime — active when sourceKind ==
     * M11_GAME_SOURCE_THERON_TRACK02. Opaque here so the public M11 state
     * does not expose Theron-private implementation headers. */
    void *theronBootProfile;  /* Theron_V1_BootProfile* */
    void *theronWorld;        /* Theron_V1_World* */
    void *theronViewport;     /* Theron_V1_Viewport* */
    void *theronAssets;       /* TrAssetBundle* */
    /* Scanner-issued CUE -> verified Track 02 -> IPL/stage-two provenance.
     * Present only for strict raw CUE launches; plain media has a zero receipt. */
    Theron_Track02StartupLoaderReceipt theronTrack02LoaderReceipt;
    Theron_Track01CddaHandoff theronTrack01CddaHandoff;
    Theron_Track01CddaStream theronTrack01CddaStream;
    struct {
        int level_loaded;
        int party_x, party_y, party_dir;
        int tick_count;
        int selected_dungeon;
        int companion_count;
        int startup_phase;
        int startup_title_animation_tick;
        int startup_cursor;
        int selected_mirrors_mask;
        int selected_mirror_order[3];
        int save_resume_verdict;
        int save_resume_claim;
        int save_resume_active_slot;
        int save_resume_srm_active_slot;
        int save_resume_srm_import_status;
        int save_resume_srm_current_dungeon;
        int save_resume_srm_current_level;
        int save_resume_srm_quest_mask;
        int save_resume_continue_focus;
        int save_resume_tqsv_slots;
        int save_resume_srm_slots;
        char save_resume_srm_root[512];
        int startup_roster_name_count;
        int startup_roster_name_status;
        int startup_media_ready;
        int startup_media_track02_variant;
        char startup_media_track02_md5[33];
        size_t startup_media_track02_size;
        int startup_bitmap_decode_status;
        int startup_bitmap_sample_count;
        unsigned int startup_bitmap_route_mask;
        size_t startup_bitmap_nonzero_pixel_count;
        uint32_t startup_bitmap_checksum;
        int startup_bitmap_title_route_ready;
        int startup_bitmap_stage_route_ready;
        int startup_bitmap_soul_room_route_ready;
        int startup_bitmap_forcefield_route_ready;
        int startup_bitmap_atlas_ready;
        int startup_bitmap_atlas_route_count;
        unsigned int startup_bitmap_atlas_route_mask;
        size_t startup_bitmap_atlas_tile_count;
        size_t startup_bitmap_atlas_nonzero_pixel_count;
        uint32_t startup_bitmap_atlas_checksum;
        Theron_Track02StartupBitmapAtlas startup_bitmap_atlas;
        int startup_bitmap_title_sample_count;
        int startup_bitmap_stage_sample_count;
        int startup_bitmap_soul_room_sample_count;
        int startup_bitmap_forcefield_sample_count;
        size_t startup_bitmap_title_nonzero_pixel_count;
        size_t startup_bitmap_stage_nonzero_pixel_count;
        size_t startup_bitmap_soul_room_nonzero_pixel_count;
        size_t startup_bitmap_forcefield_nonzero_pixel_count;
        uint32_t startup_bitmap_title_checksum;
        uint32_t startup_bitmap_stage_checksum;
        uint32_t startup_bitmap_soul_room_checksum;
        uint32_t startup_bitmap_forcefield_checksum;
        char startup_roster_names[8][16];
        char startup_roster_titles[8][32];
        int startup_text_prompt_count;
        int startup_text_prompt_status;
        char startup_text_prompt[40];
        /* External capture evidence may only publish route readiness. These
         * flags never authorize a decoder, renderer, or fallback surface. */
        int capture_campaign_admission_valid;
        int capture_campaign_startup_ready;
        int capture_campaign_soul_room_ready;
        int capture_campaign_dungeon_ready;
        int capture_campaign_track02_variant;
        char capture_campaign_track02_md5[33];
        char capture_campaign_trace_md5[33];
        char capture_campaign_bundle_md5[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT][33];
        uint32_t capture_campaign_dungeon_window_checksum;
        /* Descriptor-selected later CD records are opaque provenance only.
         * These fields cannot authorize a level/object decoder or draw. */
        int sector_record_admission_valid;
        int sector_record_dungeon_ready;
        int sector_record_track02_variant;
        char sector_record_track02_md5[33];
        uint32_t sector_record_track02_record;
        uint32_t sector_record_user_data_hash;
        uint32_t sector_record_raw_sector_checksum;
        uint32_t sector_record_campaign_layout_epoch;
        uint32_t sector_record_media_scan_epoch;
        Theron_V1Track02SectorRecordCorpusDiscoveryReceipt sector_record_corpus;
        int sector_record_corpus_bound;
        /* The first dungeon handoff may retain only descriptor-proven record
         * provenance. This is not a decoded level/object world or draw gate. */
        int first_dungeon_record_world_admission_valid;
        int first_dungeon_level_object_opaque_ready;
        int first_dungeon_record_track02_variant;
        char first_dungeon_record_track02_md5[33];
        uint32_t first_dungeon_stage3_record;
        size_t first_dungeon_descriptor_ordinal;
        uint16_t first_dungeon_descriptor_selector;
        uint32_t first_dungeon_descriptor_source_hash;
        uint32_t first_dungeon_resolved_record;
        uint32_t first_dungeon_record_user_data_hash;
        uint32_t first_dungeon_raw_sector_checksum;
        uint16_t first_dungeon_loader_caller_pc;
        uint16_t first_dungeon_loader_return_pc;
        uint8_t first_dungeon_loader_caller_opcode;
        uint16_t first_dungeon_loader_caller_target;
        uint16_t first_dungeon_loader_post_return_pc;
        uint16_t first_dungeon_loader_post_return_next_pc;
        uint8_t first_dungeon_loader_record_cl;
        uint8_t first_dungeon_loader_record_dl;
        uint8_t first_dungeon_loader_record_ch;
        uint8_t first_dungeon_loader_sector_count;
        uint32_t first_dungeon_campaign_layout_epoch;
        uint32_t first_dungeon_media_scan_epoch;
        /* Capture output identities may reach presentation only as a strict
         * no-draw witness. They do not retain palette values or bitmap bytes. */
        int first_dungeon_bitmap_palette_capture_bound;
        int first_dungeon_bitmap_palette_presentation_no_draw;
        uint32_t first_dungeon_bitmap_palette_plan_identity;
        uint32_t first_dungeon_cd_read_record;
        uint32_t first_dungeon_bitmap_palette_descriptor_record;
        size_t first_dungeon_loader_output_raw_offset;
        size_t first_dungeon_loader_output_bytes;
        uint32_t first_dungeon_loader_output_identity;
        uint32_t first_dungeon_palette_output_identity;
        uint32_t first_dungeon_bitmap_transfer_identity;
        size_t first_dungeon_destination_offset;
        size_t first_dungeon_destination_bytes;
        uint32_t first_dungeon_bitmap_destination_identity;
        uint32_t first_dungeon_bitmap_palette_layout_epoch;
        uint32_t first_dungeon_bitmap_palette_scan_epoch;
        int dungeon_capture_plan_bound;
        int dungeon_capture_required;
        int dungeon_capture_resume_ready;
        uint32_t dungeon_capture_plan_identity;
        /* Canonical direct-media receipt retained only while startup is
         * capture-required/no-draw, so an unchanged rescan may advance epoch. */
        Theron_V1Track02RawMediaIntakeReceipt startup_capture_media;
        int startup_capture_media_bound;
        Theron_V1Track02HandoffArtifactCorpusReceipt handoff_artifact_corpus;
        int handoff_artifact_corpus_bound;
        uint32_t handoff_artifact_corpus_scan_epoch;
        /* Immutable G8 FIFO provenance. This is capture-required/no-draw
         * metadata and is intentionally not a loader-output consumer input. */
        Theron_V1Track02G8FifoCaptureBindingReceipt g8_fifo_capture_binding;
        int g8_fifo_capture_binding_bound;
        uint32_t g8_fifo_capture_binding_scan_epoch;
        /* Original later $e009 output bound at launch. This remains opaque
         * CD_READ/loader provenance and can never enable drawing. */
        int handoff_loader_capture_bound;
        uint32_t handoff_loader_capture_record;
        uint16_t handoff_loader_capture_destination;
        uint16_t handoff_loader_capture_caller_pc;
        uint16_t handoff_loader_capture_return_pc;
        uint16_t handoff_loader_capture_post_return_pc;
        uint16_t handoff_loader_capture_post_return_next_pc;
        uint8_t handoff_loader_capture_record_cl;
        uint8_t handoff_loader_capture_record_dl;
        uint8_t handoff_loader_capture_record_ch;
        uint8_t handoff_loader_capture_sector_count;
        size_t handoff_loader_capture_span_bytes;
        uint32_t handoff_loader_capture_span_checksum;
        uint32_t handoff_loader_capture_plan_identity;
        int live_loader_route_admission_valid;
        int live_loader_soul_room_ready;
        int live_loader_dungeon_ready;
        uint32_t live_loader_route_epoch;
        uint32_t live_loader_campaign_layout_epoch;
        uint32_t campaign_media_scan_epoch;
        uint32_t live_loader_media_scan_epoch;
        uint32_t live_loader_capture_plan_identity;
        uint32_t live_loader_final_track02_record;
        size_t live_loader_final_raw_sector;
        char live_loader_source_trace_md5[33];
        char live_loader_event_log_md5[33];
        Theron_V1SrmCampaignReplayReceipt srm_campaign_replay;
        int srm_campaign_replay_bound;
        size_t srm_campaign_replay_size_bound;
        uint32_t srm_campaign_replay_fnv_bound;
        Theron_V1SrmLaunchDiscoveryReceipt srm_launch_discovery;
        int srm_launch_discovery_bound;
        Theron_V1Track02LaunchTraceIdentityReceipt launch_trace_identity;
        int launch_trace_identity_bound;
        Theron_V1Track02TraceBundleReceipt trace_bundle;
        int trace_bundle_bound;
    } theronState;

    /* CSB (Chaos Strikes Back) V1 runtime — active when sourceKind ==
     * M11_GAME_SOURCE_CSB_BOOT. Opaque here so the public M11 state
     * does not expose CSB-private implementation headers. Populated by
     * M11_GameView_Start() after csb_v1_boot_enter_game() succeeds;
     * csbBootProfile owns the live CSB_V1_RuntimeProfile and any loaded
     * DUNGEON.DAT handle. */
    void *csbBootProfile;     /* CSB_V1_BootProfile* */
    /* Fresh CSB starts retain the source-owned C001--C040 session until
     * ENTRANCE.C F0807 has published the terminal live-HUD receipt. A
     * direct PC34 save restore is the documented LOADSAVE.C route and does
     * not replay title/entrance media. */
    void *csbStartupRuntimeSession; /* CSB_V1_StartupRuntimeAssetSession_PC34* */
    void *csbStartupRuntimeAssetSession; /* legacy M11 spelling */
    /* CSB's GRAPHICS.DAT IMG3 floor/ceiling records use a different decoder
     * from the generic DM1 asset cache. These two decoded source rasters are
     * owned by the live M11 session and released on shutdown. */
    unsigned char *csbViewportFloorPixels;
    unsigned char *csbViewportCeilingPixels;
    int csbViewportFloorWidth;
    int csbViewportFloorHeight;
    int csbViewportCeilingWidth;
    int csbViewportCeilingHeight;
    /* PC3.4 GRAPHICS.DAT C093..C107: F0128 wall-set-zero cells. Their
     * dimensions vary by view square, so they cannot use the DM1 test atlas. */
    unsigned char *csbViewportWallPixels[15];
    int csbViewportWallWidths[15];
    int csbViewportWallHeights[15];
    uint32_t csbStartupExpectedPackageIdentity;
    int csbStartupF0128EntranceBound;
    uint32_t csbStartupF0128EntranceSourceTick;
    uint32_t csbStartupF0128EntranceSessionGeneration;
    CSB_V1_ViewportFirstFrameMaterializationReceipt
        csbStartupF0128EntranceMaterialReceipt;
    CSB_V1_ViewportFirstFrameRasterReceiptPc34
        csbStartupF0128EntranceRasterReceipt;
    uint8_t csbStartupF0128EntrancePixels[224 * 136];
    /* Boot-owned release evidence is retained with the source session. M11
     * may present title, entrance, or HUD material only while this receipt
     * and its advancing capture lifecycle still name that same session. */
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34
        csbStartupReleaseAppCaptureReceipt;
    CSB_V1_StartupReleaseLifecycleReceipt_PC34
        csbStartupReleaseLifecycleReceipt;
    /* Ordinary CSB startup has no DSA requirement. A CSBWin DSA-save route
     * opts in here and remains fail-closed unless this receipt stays current. */
    CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34
        csbDsaSaveRuntimeReceipt;
    int csbDsaSaveRuntimeRouteRequired;
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34
        csbDsaRestoredTimerTransactionReceipt;
    int csbDsaRestoredTimerTransactionRequired;
    /* Direct native F0435 reloads retain this separate source receipt. */
    CSB_V1_BootOriginalSaveRuntimeReceipt_PC34
        csbOriginalSaveRuntimeReceipt;
    int csbOriginalSaveRuntimeReceiptRequired;
    int csbStartupTimelineRequired;
    int csbStartupLiveHudAuthorized;
    uint32_t csbStartupTerminalSourceTick;
    uint32_t csbStartupTerminalGeneration;
    /* F0908 owns this exact raw PC34 SWSH sample.  It is copied from the
     * selected CSB package so the draw loop never consults a generic sound
     * bank or an invented cue. */
    unsigned char csbStartupSwooshBytes[9078];
    int csbStartupSwooshBytesBound;
    uint32_t csbStartupSwooshHash;
    int csbStartupSwooshPlayConsumed;
    int csbStartupSwooshReleaseConsumed;
    int csbStartupEntranceMusicTransitionConsumed;
    /* Atari ST ANIMATE.SCR runs at 50 VBlanks/sec while the shared CSB
     * startup state advances at its source-owned 55 ms cadence. Keep the
     * fractional conversion and last source framebuffer here, not in a
     * PC34 title wrapper or generic host animation. */
    uint32_t csbAtariStAnimationVbl;
    uint32_t csbAtariStAnimationEndVbl;
    uint16_t csbAtariStAnimationVblRemainder;
    int csbAtariStAnimationClockStarted;
    int csbAtariStAnimationFrameBound;
    int csbAtariStRuntimeHandoffComplete;
    uint32_t csbAtariStAnimationFrameVbl;
    uint16_t csbAtariStAnimationSoundCount;
    int csbAtariStAnimationSoundPlayed[
        CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS];
    uint16_t csbAtariStAnimationSoundPeriods[
        CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS];
    uint32_t csbAtariStAnimationSoundVbls[
        CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS];
    size_t csbAtariStAnimationSoundBytes[
        CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS];
    uint8_t csbAtariStAnimationSoundData[
        CSB_V1_ATARI_ST_ANIMATION_MAX_PLAYED_SOUNDS][4096];
    uint8_t csbAtariStAnimationPixels[320 * 200];
    uint8_t csbAtariStAnimationPalette[16][3];
    struct {
        int level_loaded;
        int current_level;
        int party_x, party_y, party_dir;
        int tick_count;
        int runtime_object_sprite_drawn_count;
        int runtime_object_icon_drawn_count;
        int runtime_object_marker_drawn_count;
        int runtime_group_sprite_drawn_count;
        int runtime_group_marker_drawn_count;
        int runtime_projectile_sprite_drawn_count;
        int runtime_projectile_material_resolved_count;
        int runtime_projectile_material_icon_drawn_count;
        int runtime_projectile_marker_drawn_count;
        int runtime_post_teleport_projectile_handoff_drawn_count;
        int runtime_post_teleport_projectile_handoff_blocked_count;
        int runtime_explosion_sprite_drawn_count;
        int runtime_explosion_marker_drawn_count;
        int runtime_viewport_source_session_ready;
        uint32_t runtime_viewport_source_session_generation;
        uint32_t runtime_viewport_source_tick;
        uint32_t runtime_viewport_pixel_hash;
        uint32_t runtime_viewport_draw_counts_hash;
        int startup_title_active;
        int startup_title_frame;
        int startup_title_source_step;
        int startup_entrance_active;
        int startup_entrance_frame;
        int startup_entrance_source_step;
        int startup_entrance_dismissed;
        int startup_entrance_last_command;
        int startup_entrance_bonus_requested;
        int startup_entrance_credits_active;
        int startup_entrance_credits_remaining_ticks;
        int startup_entrance_opening_active;
        int c040_panel_session_active;
        uint32_t c040_panel_session_generation;
        uint32_t c040_panel_source_tick;
        int presented_frame_capture_ready;
        int presented_frame_running_from_macos_app;
        int presented_frame_mac_window_ready;
        int presented_frame_width;
        int presented_frame_height;
        uint32_t presented_frame_hash;
        int c040_clear_live_hud_ready;
        uint32_t c040_clear_source_tick;
        uint32_t c040_clear_session_generation;
        int startup_entrance_opening_delay_ticks;
        int startup_entrance_opening_step;
        int startup_entrance_pending_command;
        int startup_entrance_resume_available;
        char startup_entrance_resume_path[512];
        int startup_import_available;
        int startup_import_champion_count;
        int startup_import_utility_state;
        int startup_import_selected_action_index;
        int startup_import_preview_active;
        char startup_import_dm1_save_path[512];
        char startup_import_utility_prompt[192];
    } csbState;

    /* DM2 (Skullkeep) V1 runtime — active when sourceKind ==
     * M11_GAME_SOURCE_DM2_BOOT. Opaque here so the public M11 state
     * does not expose DM2-private implementation headers. Populated by
     * M11_GameView_StartDm2() after dm2_v1_boot_enter_game() succeeds
     * (added 2026-06-17). dm2World carries the live DM2_V1_GameState
     * pointer (or NULL when not initialised); dm2BootProfile is a
     * back-reference for shutdown + future V2 runtime init. The M11
     * render loop and tick paths dispatch on sourceKind and read
     * dm2World when present. */
    void *dm2World;           /* DM2_V1_GameState* — owned by
                               * the static DM2_V1_BootProfile in
                               * M11_GameView_StartDm2(). */
    void *dm2BootProfile;     /* DM2_V1_BootProfile* */
    int dm2SaveDialoguePanelActive;
    struct {
        int level_loaded;
        int party_x, party_y, party_dir;
        int tick_count;
        int startup_menu_active;
        int startup_menu_selected_row;
        int startup_menu_row_count;
        int startup_title_animation_tick;
        uint32_t music_elapsed_us;
        uint32_t music_loop_duration_us;
        uint32_t music_loop_count;
        uint32_t music_events_due;
        int music_schedule_ready;
        int startup_resume_available;
        unsigned int startup_slot_mask;
        char startup_save_root[512];
        uint32_t leader_hand_object;
        uint32_t champion_inventory_objects[4][30];
    } dm2State;
    int dm2ShopSelectedStockIndex;
    int dm2ShopSelectedInventoryIndex;

    /* Accessibility: in-game font size scale (1..3).
     * Set from M12 launcher's fontScale setting via M11_GameLaunchSpec.fontScale.
     * Used by m11_draw_glyph and m11_draw_text_original to scale text rendering.
     * 0 means "use built-in font's default scale from M11_TextStyle" (backward compat). */
    int fontScale;

    /* Jobb F2: launcher-options runtime handoff snapshot.  Stored by
     * M11_GameView_Start from M11_GameLaunchSpec.launcherOptions when
     * the spec carries a bound launcher handoff; unbound for direct
     * (non-launcher) starts.  Read via
     * M11_GameView_GetLauncherRuntimeOptions(). */
    M12_LauncherRuntimeOptions launcherOptions;
    int launcherOptionsBound;

    /* RetroAchievements in-game notification overlay. */
    Firestaff_RA_Overlay retroAchievementsOverlay;
} M11_GameViewState;

#ifdef __cplusplus
static_assert(sizeof(((M11_GameViewState*)0)->p5_camera) ==
                  sizeof(DM1_V2_CameraController),
              "M11 p5_camera must embed the full V2 camera controller");
#else
_Static_assert(sizeof(((M11_GameViewState*)0)->p5_camera) ==
                   sizeof(DM1_V2_CameraController),
               "M11 p5_camera must embed the full V2 camera controller");
#endif

/* Spell casting API */
int M11_GameView_OpenSpellPanel(M11_GameViewState* state);
int M11_GameView_SetCsbEntranceF0128Raster(
    M11_GameViewState *state,
    const CSB_V1_ViewportFirstFrameMaterializationReceipt *material_receipt,
    const CSB_V1_ViewportFirstFrameRasterReceiptPc34 *raster_receipt,
    const uint8_t *viewport_pixels,
    size_t viewport_pixel_count,
    uint32_t source_tick,
    uint32_t session_generation);
int M11_GameView_CloseSpellPanel(M11_GameViewState* state);
int M11_GameView_EnterRune(M11_GameViewState* state, int symbolIndex);
int M11_GameView_CastSpell(M11_GameViewState* state);
int M11_GameView_ClearSpell(M11_GameViewState* state);

/* DM1 V2 Phase 5 smooth movement camera accessors.
 *
 * M11_GameView_SetCameraOffset — called by the V2 presentation lane
 * after each V1 game tick to propagate the camera controller's
 * interpolation state.  Does NOT mutate game state, collision,
 * sensors, creature timing, or redraw cadence.
 *
 * M11_GameView_GetCameraOffset — called by m11_draw_viewport during
 * the render pass to retrieve the interpolated offset/facing for the
 * viewport's view-cone sampling.  When offsets are zero (V1 paths),
 * this is a no-op.
 *
 * Source-lock: see M11_GameViewState.camera_offset_x documentation.
 * These functions are safe for V1 callers (V1 sets offsets to 0). */
void M11_GameView_SetCameraOffset(M11_GameViewState* state,
                                   int offsetX, int offsetY, int16_t facingDir);
void M11_GameView_GetCameraOffset(const M11_GameViewState* state,
                                   int* outOffsetX, int* outOffsetY, int16_t* outFacingDir);

void M11_GameView_Init(M11_GameViewState* state);
void M11_GameView_Shutdown(M11_GameViewState* state);
int M11_GameView_Start(M11_GameViewState* state, const M11_GameLaunchSpec* spec);
int M11_GameView_ResolveNexusRuntimeDataDir(const M11_GameLaunchSpec* spec,
                                            char* outPath,
                                            int outPathSize);
int M11_GameView_OpenSelectedMenuEntry(M11_GameViewState* state,
                                       const M12_StartupMenuState* menuState);
int M11_GameView_StartDm1(M11_GameViewState* state, const char* dataDir);
int M11_GameView_Dm1StartupIntroBypassed(const M11_GameViewState* state);
int M11_GameView_GetBootProbeReceipt(const M11_GameViewState* state,
                                     M11_BootProbeReceipt* out);
/* Record the already-rendered CSB source framebuffer after M11 presents it.
 * The CSB package layer verifies C001-C005/C017/C040 before retaining facts. */
void M11_GameView_RecordCSBPresentedIndexedFrame(
    M11_GameViewState* state,
    const unsigned char* indexedPixels,
    int width,
    int height,
    int runningFromMacOSAppBundle,
    int macWindowCaptureReady);
/* Source-owned C001-C005/C017/C040 gate for the already drawn M11 page.
 * The caller must present only when this exact current-plan comparison holds. */
int M11_GameView_CSBPresentedFrameMatchesCurrentSource(
    const M11_GameViewState* state,
    const unsigned char* indexedPixels,
    int width,
    int height,
    int specialPalette);
int M11_GameView_BindCSBDSASaveRuntimeHandoff(
    M11_GameViewState *state,
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *receipt);
int M11_GameView_CommitCSBDSARestoredTimerOutcome(
    M11_GameViewState *state,
    const CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 *receipt);
/* Executes one already-restored CSBWin TimerQueue slot through the source
 * runtime. The caller supplies the source timer location and queue slot; M11
 * neither selects a timer nor interprets a DSA program. */
int M11_GameView_ExecuteCSBDSARestoredTimer(
    M11_GameViewState *state,
    const CSB_V1_DSAFilterLocation *location,
    uint16_t queue_slot);
int M11_GameView_GetQuickSavePath(const M11_GameViewState* state,
                                  char* out,
                                  size_t outSize);
int M11_GameView_LoadDm1SavePath(M11_GameViewState* state,
                                 const char* path,
                                 int* outUsedBackup);
/* Adopt an already authenticated, immutable original PC34 F0435 snapshot.
 * `sourcePath` is retained only for the resume receipt; M11 never reopens it.
 */
int M11_GameView_LoadDm1OriginalPc34SaveBytes(M11_GameViewState* state,
                                              const uint8_t* bytes,
                                              size_t size,
                                              const char* sourcePath);
int M11_GameView_ExportQuickSaveAsDM1PC34(const char* quickSavePath,
                                          const char* exportPath);
int M11_GameView_QuickSave(M11_GameViewState* state);
int M11_GameView_QuickLoad(M11_GameViewState* state);
/* Session timer runtime handoff boundary (see session_timer_runtime.h).
 * M11_GameView_InitFromMenuSessionTimer seeds the in-game runtime from
 * the launcher's M12_StartupMenu_SessionTimerLimitMinutes() value;
 * M11_GameView_TickSessionTimer advances the runtime and reports the
 * latest event; M11_GameView_AcknowledgeSessionTimerReminder clears a
 * pending reminder overlay; M11_GameView_ClearSessionTimerForcedPause
 * releases the forced-pause latch so the user can resume gameplay. */
void M11_GameView_InitFromMenuSessionTimer(M11_GameViewState* state,
                                           const M12_StartupMenuState* menu);
/* Jobb F2: launcher-options runtime handoff accessor.  Copies the
 * launcher-options snapshot stored by M11_GameView_Start into `out` and
 * returns 1 when a launcher handoff was bound for this session;
 * returns 0 (and zeroes `out`) for direct starts or NULL input. */
int M11_GameView_GetLauncherRuntimeOptions(
    const M11_GameViewState* state,
    M12_LauncherRuntimeOptions* out);
SessionTimerRuntimeEvent M11_GameView_TickSessionTimer(
    M11_GameViewState* state, int seconds);
void M11_GameView_AcknowledgeSessionTimerReminder(M11_GameViewState* state);
void M11_GameView_ClearSessionTimerForcedPause(M11_GameViewState* state);
int M11_GameView_GetSessionTimerForcedPauseDialogActive(
    const M11_GameViewState* state);
int M11_GameView_GetSessionTimerReminderOverlayActive(
    const M11_GameViewState* state);
int M11_GameView_SetMusicEnabled(M11_GameViewState* state, int enabled);
/* Binds the authenticated raw PC34 SWSH sample for the active CSB package.
 * Invalid/missing source data stays silent; no synthetic audio substitute is
 * used for title or Entrance. */
int M11_GameView_SetCsbStartupSwooshSource(M11_GameViewState* state,
                                           const unsigned char* bytes,
                                           int byteCount,
                                           uint32_t expectedHash);
int M11_GameView_ToggleMusic(M11_GameViewState* state);
int M11_GameView_GetMusicEnabled(const M11_GameViewState* state);
M11_GameInputResult M11_GameView_AdvanceIdleTick(M11_GameViewState* state);
M11_GameInputResult M11_GameView_HandleInput(M11_GameViewState* state,
                                             M12_MenuInput input);
int M11_GameView_InputConsumesDm1V1SourceTick(const M11_GameViewState* state,
                                              M12_MenuInput input);
int M11_GameView_Dm1V1SourceTickReadyForInput(const M11_GameViewState* state);

/* Resolve the host idle cadence for the active source state. CSB startup
 * advances one source VBlank per M11 idle tick and retains the 20 ms cadence. */
uint32_t M11_GameView_IdleTickIntervalMs(const M11_GameViewState* state,
                                         int speedMultiplier);

M11_GameInputResult M11_GameView_HandlePointer(M11_GameViewState* state,
                                               int x,
                                               int y,
                                               int primaryButton);
M11_GameInputResult M11_GameView_HandlePointerMove(M11_GameViewState* state,
                                                   int x,
                                                   int y);
M11_GameInputResult M11_GameView_HandlePointerButton(M11_GameViewState* state,
                                                     int x,
                                                     int y,
                                                     int buttonMask);
M11_GameInputResult M11_GameView_HandlePointerButtonRelease(
    M11_GameViewState* state,
    int x,
    int y,
    int buttonMask);
void M11_GameView_Draw(const M11_GameViewState* state,
                       unsigned char* framebuffer,
                       int framebufferWidth,
                       int framebufferHeight);
/* Draw the F10 runtime graphics panel after the source-owned game frame.
 * It deliberately contains controls only, never substitute game artwork. */
void M11_GameView_DrawGraphicsPopup(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight);
/* FPS is sampled at actual SDL presentation time, independently of source
 * ticks, so the overlay never alters V1 game cadence. */
void M11_GameView_RecordPresentedFrame(M11_GameViewState* state,
                                       uint64_t nowMs);
void M11_GameView_DrawFpsOverlay(const M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight);
int M11_GameView_GetPresentationSpecialPalette(const M11_GameViewState* state);
int M11_GameView_PickupItem(M11_GameViewState* state);
int M11_GameView_DropItem(M11_GameViewState* state);
int M11_GameView_CountChampionItems(const M11_GameViewState* state, int championIndex);
void M11_MessageLog_Push(M11_MessageLog* log, const char* text, unsigned char color);
int M11_GameView_GetMessageLogCount(const M11_GameViewState* state);
const char* M11_GameView_GetMessageLogEntry(const M11_GameViewState* state, int reverseIndex);

/* Post-move environmental transition check (pits, teleporters).
 * Returns 1 if a transition occurred. */
int M11_GameView_CheckPostMoveTransitions(M11_GameViewState* state);

/* Query the computed skill level for a champion via the lifecycle layer.
 * Returns the effective level (>= 0), or -1 on error. */
int M11_GameView_GetSkillLevel(const M11_GameViewState* state,
                               int championIndex,
                               int skillIndex);

int M11_GameView_ProbeF0230ParryAdjustedAttack(
    const M11_GameViewState* state,
    int championIndex,
    int random16,
    int creatureBaseAttack,
    int doubledMapDifficulty);
int M11_GameView_ProbeF0407ShootAttack(
    const M11_GameViewState* state,
    int championIndex,
    int weaponShootAttack);
int M11_GameView_ProbeF0328ThrowAttack(
    const M11_GameViewState* state,
    int championIndex,
    int baseAttack);
int M11_GameView_ProbeF0352PotionEyeDescription(
    const M11_GameViewState* state,
    int championIndex,
    unsigned int thingType,
    unsigned int iconIndex,
    unsigned int potionPower,
    const char* objectName,
    char* outText,
    unsigned int outCapacity);

/* Use the item in the active champion's hand slot (potions, flasks).
 * Returns 1 if an item was consumed/used. */
int M11_GameView_UseItem(M11_GameViewState* state);

/* Process tick emissions: log events, award XP, apply level-ups.
 * Normally called internally after each tick advance; exposed for
 * probe-level verification of emission-driven XP integration. */
void M11_GameView_ProcessTickEmissions(M11_GameViewState* state);

/* Return the source-owned F0337 total dungeon-view light amount.
 * The value is intentionally not clamped: darkness can leave a negative
 * intermediate amount before F0337 selects the darkest palette. */
int M11_GameView_GetLightLevel(const M11_GameViewState* state);

/* Compute the source dungeon-view palette index (0=brightest, 5=darkest).
 * This is the same F0337-backed value the V1 viewport renderer applies. */
int M11_GameView_GetDungeonPaletteIndex(const M11_GameViewState* state);

/* ── Creature animation API ── */

/* Durations in game ticks. */
#define M11_DAMAGE_FLASH_DURATION   4
/* ReDMCSB schedules C12_EVENT_HIDE_DAMAGE_RECEIVED for gameTime+5.
 * Keep champion HUD damage overlays source-locked separately from
 * the red viewport damage flash duration. */
#define M11_CHAMPION_DAMAGE_OVERLAY_DURATION 5
#define M11_ATTACK_CUE_DURATION     3
#define M11_CREATURE_HIT_OVERLAY_DURATION 5  /* ticks to show graphic-14 overlay */
#define M11_CREATURE_ANIM_PERIOD    6  /* ticks per idle-frame cycle */

/* Advance the animation frame counter and decrement timers.
 * Called once per game tick (from AdvanceIdleTick). */
void M11_GameView_TickAnimation(M11_GameViewState* state);

/* Signal that the party just took creature melee damage.
 * Starts the damage-flash and attack-cue timers.
 * creatureType: type index of the attacking creature (for cue sprite). */
void M11_GameView_NotifyDamageFlash(M11_GameViewState* state,
                                    int creatureType);

/* Query animation state (for probes). */
int M11_GameView_GetDamageFlashTimer(const M11_GameViewState* state);
int M11_GameView_GetAttackCueTimer(const M11_GameViewState* state);

/* Trigger a per-champion damage indicator overlay (GRAPHICS.DAT graphic 15).
 * The indicator displays for M11_CHAMPION_DAMAGE_OVERLAY_DURATION ticks. */
void M11_GameView_NotifyChampionDamage(M11_GameViewState* state,
                                       int championSlot,
                                       int damageAmount);
uint32_t M11_GameView_GetAnimTick(const M11_GameViewState* state);

/* Signal that the party just dealt melee damage to a creature.
 * Starts the graphic-14 viewport overlay timer. */
void M11_GameView_NotifyCreatureHit(M11_GameViewState* state,
                                    int damageAmount);
int M11_GameView_GetCreatureHitOverlayTimer(const M11_GameViewState* state);

/* Return the idle-animation frame index (0 or 1) for a given
 * creature type based on the current animTick. */
int M11_GameView_CreatureAnimFrame(const M11_GameViewState* state,
                                   int creatureType);

/* Return the attack-cue creature type (-1 if no active attack). */
int M11_GameView_GetAttackCueCreatureType(const M11_GameViewState* state);

/* ── Dialog / endgame query API ── */

/* Return 1 if the game has been won (Firestaff placed correctly). */
int M11_GameView_IsGameWon(const M11_GameViewState* state);

/* Return the tick at which the game was won (0 if not won). */
uint32_t M11_GameView_GetGameWonTick(const M11_GameViewState* state);

/* Return 1 when F0446 has reached the source endgame fluxcage-hide gate. */
int M11_GameView_GetEndgameDoNotDrawFluxcages(const M11_GameViewState* state);

/* Return 1 when F0446 delay playback has reached the final Endgame(TRUE) gate. */
int M11_GameView_GetEndgameFinalHandoffReady(const M11_GameViewState* state);

/* Return 1 when the final endgame screen should be visible. */
int M11_GameView_GetEndgameFinalPresentationReady(const M11_GameViewState* state);

/* Return 1 when the source endgame restart button has requested a restart. */
int M11_GameView_GetEndgameRestartRequested(const M11_GameViewState* state);

int M11_GameView_GetEndgameFuseReplayCursor(const M11_GameViewState* state);
int M11_GameView_GetEndgameFuseReplayEventCount(const M11_GameViewState* state);
int M11_GameView_GetEndgameFuseReplayCurrentEvent(const M11_GameViewState* state,
                                                  int* outType,
                                                  int* outAttack,
                                                  int* outCreatureType);

int M11_GameView_BuildEndgameFinalPresentationReceipt(
    const M11_GameViewState* state,
    DM1_V1_EndgameFinalPresentationReceiptPc34* outReceipt);

/* Return 1 if a dialog overlay is currently displayed. */
int M11_GameView_IsDialogOverlayActive(const M11_GameViewState* state);

/* Dismiss the dialog overlay if active.  Returns 1 if dismissed. */
int M11_GameView_DismissDialogOverlay(M11_GameViewState* state);

/* Show a dialog overlay with the given text.  Returns 1 on success. */
int M11_GameView_ShowDialogOverlay(M11_GameViewState* state,
                                   const char* text);

/* Show a dialog overlay with up to four source-style choice labels. */
int M11_GameView_ShowDialogOverlayChoices(M11_GameViewState* state,
                                          const char* text,
                                          const char* choice1,
                                          const char* choice2,
                                          const char* choice3,
                                          const char* choice4);

/* Return 1..4 after a choice is selected, or 0 if none was selected. */
int M11_GameView_GetDialogSelectedChoice(const M11_GameViewState* state);

/* ── Full-screen map overlay API ── */

/* Toggle the full-screen map overlay.  Returns 1 if now visible. */
int M11_GameView_ToggleMapOverlay(M11_GameViewState* state);

/* Return 1 if the map overlay is currently displayed. */
int M11_GameView_IsMapOverlayActive(const M11_GameViewState* state);

/* ── Full inventory panel API ── */

/* Toggle the full inventory panel.  Returns 1 if now visible. */
int M11_GameView_ToggleInventoryPanel(M11_GameViewState* state);

/* Return 1 if the inventory panel is currently displayed. */
int M11_GameView_IsInventoryPanelActive(const M11_GameViewState* state);

/* Return the currently selected inventory slot index (-1 = none). */
int M11_GameView_GetInventorySelectedSlot(const M11_GameViewState* state);

/* Return human-readable label for an inventory slot index. */
const char* M11_GameView_SlotName(int slotIndex);

/* ── Action-menu API (DM1 F0387 menu-mode) ── */

/* Return the currently acting champion ordinal (DM1 G0506).
 * 0 = no champion is acting (idle icon-cell mode).
 * 1..N = champion at index N-1 has been activated and the action
 * area is in menu mode. */
unsigned int M11_GameView_GetActingChampionOrdinal(const M11_GameViewState* state);

/* Set the acting champion by index (0..CHAMPION_MAX_PARTY-1).
 * Returns 1 on success, 0 if the champion slot is empty/dead/out
 * of range.  Mirrors F0389_MENUS_SetActingChampion for the bounded
 * subset we currently model (action-area state only; does not yet
 * emit the full champion-draw refresh or icon flag). */
int M11_GameView_SetActingChampion(M11_GameViewState* state, int championIndex);

/* Clear the acting champion, returning the action area to idle
 * icon-cell mode.  Mirrors F0388_MENUS_ClearActingChampion for
 * the bounded action-area subset. */
void M11_GameView_ClearActingChampion(M11_GameViewState* state);

/* Resolve the DM1 ActionSet indices for the currently acting
 * champion.  Returns the 3 action-name indices (0..43, or 255 =
 * C0xFF_ACTION_NONE) into outIndices[0..2].  Returns 1 when the
 * action area is in menu mode and the indices were filled,
 * 0 when idle (outIndices left untouched).
 *
 * Ref: ReDMCSB MENU.C G0489_as_Graphic560_ActionSets,
 *      F0389_MENUS_SetActingChampion action-set lookup. */
int M11_GameView_GetActingActionIndices(const M11_GameViewState* state,
                                        unsigned char outIndices[3]);

/* Look up the DM1 action name for an action index (0..43, or 255).
 * Returns an empty string for C0xFF_ACTION_NONE and a pointer to a
 * static string otherwise.  Verbatim names from
 * G0490_ac_Graphic560_ActionNames (ReDMCSB MENU.C). */
const char* M11_GameView_GetActionName(unsigned char actionIndex);

/* Trigger the DM1 action-row click pathway for the currently acting
 * champion.  Mirrors F0391_MENUS_DidClickTriggerAction for the
 * bounded V1 slice we currently model:
 *
 *   - Returns 0 immediately when no champion is acting or when the
 *     chosen row resolves to C0xFF_ACTION_NONE (DM1 aborts without
 *     clearing the menu so the player can pick a valid row).
 *   - Emits a player-facing "CHAMPION: ACTION" log line in cyan.
 *   - For melee-contact actions (CHOP, PUNCH, KICK, STAB, SWING,
 *     HIT, THRUST, SLASH, BASH, JAB, STUN, HACK, BERZERK, CLEAVE,
 *     MELEE) advances one CMD_ATTACK tick through the existing M10
 *     orchestrator, which resolves damage, creature hit overlays
 *     and combat emissions exactly as the keyboard strike path.
 *   - ALWAYS clears the acting champion at the end (F0391 /
 *     F0388_MENUS_ClearActingChampion semantics), closing the
 *     action menu and restoring idle icon-cell presentation.
 *
 * actionListIndex must be 0..2.  Returns 1 when a tick-level
 * action was committed, 0 otherwise (including the NONE-row early
 * exit).  Ref: ReDMCSB MENU.C F0391, F0407; ACTIDRAW.C F0387. */
int M11_GameView_TriggerActionRow(M11_GameViewState* state,
                                  int actionListIndex);

/* Probe-visibility helpers for DM1 projectile / spell action
 * downstream effects.  These let probes observe that action-menu
 * projectile rows (FIREBALL / LIGHTNING / DISPELL / INVOKE /
 * SHOOT / THROW) actually spawn a projectile into world.projectiles
 * without needing to wire a full ObjectInfo action-hand harness.
 *
 * GetProjectileCount returns the count of live entries in
 * GameWorld.projectiles (wraps world.projectiles.count).
 *
 * TriggerNonMeleeActionByIndex invokes the F0407-style handler
 * for a chosen action-name index (0..43) against the acting
 * champion, bypassing the ActionSet-from-hand resolution.  This
 * is a test helper that mirrors exactly the same dispatch path
 * M11_GameView_TriggerActionRow uses when F0391 selects a
 * non-melee action from the champion's action list: it emits
 * the "CHAMPION: ACTION" log line, runs the bounded V1 handler
 * (which may spawn a projectile, adjust MagicState, etc.),
 * advances a CMD_NONE tick, and clears the acting champion.
 * Melee-contact actions are not routed here (they require the
 * CMD_ATTACK orchestrator path, which already has coverage via
 * the row-click probe invariants).
 *
 * Returns 1 when the handler reported AL1245_B_ActionPerformed=
 * TRUE (i.e. the projectile was spawned, mana was deducted, or
 * a deterministic effect was applied), 0 otherwise.
 *
 * Ref: ReDMCSB MENU.C F0407_MENUS_IsActionPerformed. */
int M11_GameView_GetProjectileCount(const M11_GameViewState* state);
int M11_GameView_TriggerNonMeleeActionByIndex(M11_GameViewState* state,
                                              int championIndex,
                                              int actionIndex);
int M11_GameView_ProbeF0407FuseImmediate(M11_GameViewState* state,
                                         int championIndex);

/* V1 projectile cycle probe hook: drive one tick of the V1
 * projectile advance over all live projectiles.  Normally invoked
 * from M11_GameView_ProcessTickEmissions each orchestrator tick;
 * exposed here so probes can deterministically step projectiles
 * without replaying the full orchestrator pipeline.  Calls
 * F0811_PROJECTILE_Advance_Compat per live slot, applies the new
 * state or despawns on impact (with explosion spawn + damage +
 * log cue as the bounded V1 slice).  Safe on empty lists. */
void M11_GameView_AdvanceProjectilesOnce(M11_GameViewState* state);

/* Probe-visible creature projectile insertion hook.  Drives the same
 * M11 runtime creature-AI launch branch used by AdvanceIdleTick after
 * a group has been located on the current map; returns 1 when a live
 * projectile slot was inserted and first-move scheduled. */
int M11_GameView_ProbeCreatureProjectileRuntimeLaunch(M11_GameViewState* state,
                                                      unsigned short groupThing,
                                                      int groupIndex,
                                                      int groupMapX,
                                                      int groupMapY);

/* Probe the DM1 V1 creature fixed-possession materialization bridge.
 * Mirrors GROUP.C F0186 allocation through F0166-style unused object slots
 * and F0267-style placement from CM1_MAPX_NOT_ON_A_SQUARE onto the target
 * square. Returns the number of object things materialized, or -1 on error. */
int M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
    M11_GameViewState* state,
    int creatureType,
    int sourceCell,
    int mapIndex,
    int mapX,
    int mapY);
int M11_GameView_ProbeCheckCreatureGroupDeathAndDrop(
    M11_GameViewState* state,
    unsigned short groupThing,
    int mapIndex,
    int mapX,
    int mapY);

/* Drives the production CHAMPION.C F0319 death check without advancing an
 * unrelated timeline tick.  Probe-only: callers must provide the live M11
 * world state that the normal tick route owns. */
void M11_GameView_ProbeCheckPartyDeath(M11_GameViewState* state);

/* V1 explosion cycle probe hook: drive one tick of the V1 explosion
 * advance over all live explosion slots.  Normally invoked from
 * M11_GameView_ProcessTickEmissions each orchestrator tick right after
 * the projectile advance; exposed here so probes can deterministically
 * step explosion aftermath without replaying the orchestrator pipeline.
 * Calls F0822_EXPLOSION_Advance_Compat per live slot, applies its
 * champion/group damage actions, and either despawns (one-shot: fire-
 * ball, lightning) or commits the advanced state with the new
 * currentFrame / decayed attack (persistent: poison cloud, smoke).
 * Safe on empty lists. */
void M11_GameView_AdvanceExplosionsOnce(M11_GameViewState* state);

/* Probe shim for the internal m11_summarize_square_things helper so
 * probes can verify that runtime-only projectiles / explosions show
 * up in the viewport cell summary (this is what drives the sprite
 * being drawn as the projectile travels across cells). */
int M11_GameView_CountCellProjectiles(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY);
int M11_GameView_CountCellExplosions(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY);

int M11_GameView_GetWallSetGraphicIndex(int wallSet, int wallSet0GraphicIndex);
int M11_GameView_GetViewportRect(int* outX, int* outY, int* outW, int* outH);
int M11_GameView_GetObjectIconIndexForThing(const M11_GameViewState* state,
                                            unsigned short thingId);
int M11_GameView_GetC3200CreatureZonePoint(int coordSet,
                                           int depthIndex,
                                           int visibleCount,
                                           int slotIndex,
                                           int* outX,
                                           int* outY);
int M11_GameView_GetC3200CreatureSideZonePoint(int coordSet,
                                               int depthIndex,
                                               int sideHint,
                                               int visibleCount,
                                               int slotIndex,
                                               int* outX,
                                               int* outY);
void M11_GameView_GetObjectPileShiftIndices(int pileIndex,
                                            int* outXIndex,
                                            int* outYIndex);
int M11_GameView_GetObjectShiftValue(int shiftSet, int shiftIndex);
unsigned int M11_GameView_GetObjectAspectGraphicInfo(int aspectIndex);
int M11_GameView_GetObjectAspectCoordinateSet(int aspectIndex);
int M11_GameView_ObjectUsesFlipOnRight(int thingType, int subtype,
                                       int relativeCell);
int M11_GameView_GetCreaturePaletteChange(int depthPaletteIndex,
                                          int paletteIndex);

/* ── Creature aspect query API (for probes) ── */

/* Return the coordinate set index (0-10) for a creature type. */
int M11_GameView_GetCreatureCoordinateSet(int creatureType);

/* Return the transparent color index for a creature type. */
int M11_GameView_GetCreatureTransparentColor(int creatureType);

/* Return front-cell viewport placement for a creature duplicate using
 * original Graphic558 center/bottom coordinates.  Exposed for probe
 * verification of original-data-backed coordinate extraction. */
void M11_GameView_GetCreatureFrontSlotPoint(int coordSet,
                                            int depthIndex,
                                            int visibleCount,
                                            int slotIndex,
                                            int* outCenterX,
                                            int* outBottomY);

/* Return the front-pose GRAPHICS.DAT sprite index for a creature at a
 * given depth. Returns 0 for invalid inputs. */
unsigned int M11_GameView_GetCreatureSpriteForDepth(int creatureType, int depthIndex);

/* Return the GRAPHICS.DAT sprite index selected for an actual view using
 * creature direction vs party direction and optional attack state.
 * outMirror receives whether the side pose should be mirrored. */
unsigned int M11_GameView_GetCreatureSpriteForView(int creatureType,
                                                   int depthIndex,
                                                   int creatureDir,
                                                   int partyDir,
                                                   int attacking,
                                                   int* outMirror);

/* Query replacement color palette indices for a creature type.
 * Returns 1 if the creature uses replacement colors, 0 if not.
 * outReplDst9/outReplDst10 receive the target palette indices. */
int M11_GameView_GetCreatureReplacementColors(int creatureType,
                                               int* outReplDst9,
                                               int* outReplDst10);

/* Return the CREATURE_INFO GraphicInfo bitfield for a creature type.
 * Values are extracted verbatim from ReDMCSB
 * G0243_as_Graphic559_CreatureInfo[].GraphicInfo (DM1 PC v3.4).
 * Returns 0 for invalid creature types.
 * See m11_game_view.c for M11_CREATURE_GI_MASK_* bit definitions. */
unsigned int M11_GameView_GetCreatureGraphicInfo(int creatureType);

/* Boolean queries on the source-backed GraphicInfo table.  These mirror
 * the DEFS.H flags that control whether dedicated side/back/attack
 * bitmaps exist and whether the FRONT bitmap should be mirrored when
 * falling back. */
int M11_GameView_CreatureHasSideBitmap(int creatureType);
int M11_GameView_CreatureHasBackBitmap(int creatureType);
int M11_GameView_CreatureHasAttackBitmap(int creatureType);
int M11_GameView_CreatureHasFlipNonAttack(int creatureType);
int M11_GameView_CreatureHasFlipAttack(int creatureType);

/* Source-backed queries for MASK0x0003_ADDITIONAL,
 * MASK0x0080_SPECIAL_D2_FRONT, MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT,
 * MASK0x0400_FLIP_DURING_ATTACK, and the M052/M053 offset amplitude
 * fields of CREATURE_INFO.GraphicInfo.  Reference:
 * ReDMCSB DUNVIEW.C F097 (_LoadGraphics) native-bitmap allocation loop
 * and F1512-render offset computation, plus F0179_GROUP_GetCreature-
 * AspectUpdateTime (GROUP2.C) for runtime flip behaviour. */
int M11_GameView_GetCreatureAdditional(int creatureType);
int M11_GameView_CreatureHasSpecialD2Front(int creatureType);
int M11_GameView_CreatureHasD2FrontIsFlippedFront(int creatureType);
int M11_GameView_CreatureHasFlipDuringAttack(int creatureType);
int M11_GameView_GetCreatureMaxHorizontalOffset(int creatureType);
int M11_GameView_GetCreatureMaxVerticalOffset(int creatureType);

/* Total GRAPHICS.DAT native-bitmap slots F097_xxxx_DUNGEONVIEW_LoadGraphics
 * allocates for this creature in order:
 *   [Front] [Side?] [Back?] [SpecialD2?] [Attack?] [AdditionalFront x N?]
 * The SpecialD2 slot is present when MASK0x0080_SPECIAL_D2_FRONT is set
 * and MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT is clear (guarded by
 * the C06_COMPILE_DM10aEN..DM13bFR block in DUNVIEW.C; BUG0_00 notes
 * this slot is allocated but never read by the renderer).
 * Additional-front slots are only allocated when MASK0x0004_FLIP_NON_ATTACK
 * is clear. */
int M11_GameView_GetCreatureNativeBitmapCount(int creatureType);

/* Return the text rows currently exposed by the Theron V1 startup renderer.
 * This is a render-facing test hook: it intentionally mirrors M11 startup
 * screen labels and cursor/status state without depending on framebuffer OCR. */
int M11_GameView_GetTheronStartupRenderRows(
    const M11_GameViewState* state,
    char rows[][M11_THERON_STARTUP_RENDER_ROW_CAPACITY],
    int maxRows);

/* Machine-readable Theron startup layout.  This is the render-facing
 * contract used before decoded Track 02 menu/mirror art replaces the bounded
 * text screen; callers should use element fields instead of parsing row text. */
int M11_GameView_GetTheronStartupLayout(
    const M11_GameViewState* state,
    M11_TheronStartupElement* elements,
    int maxElements);

/* Binds externally authenticated Track 02 campaign evidence to the active
 * M11 Theron host state. A failed bind clears all capture-ready flags. */
int M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
    M11_GameViewState* state,
    const Theron_V1Track02CaptureCampaignReceipt* campaign,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt* dungeonWindow);
/* Starts the no-draw capture-required lifecycle from one current direct
 * ISO/BIN/CUE receipt and the existing opaque capture plan. */
int M11_GameView_TheronBindTrack02StartupCaptureRequired(
    M11_GameViewState* state,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt* campaignMedia,
    const Theron_V1Track02CaptureTargetPlan* campaignPlan,
    uint32_t campaignMediaScanEpoch);
/* Rechecks the exact external campaign receipt that published capture
 * readiness. Any changed trace, route bundle, or opaque dungeon window clears
 * readiness; this remains a no-draw provenance gate. */
int M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrent(
    M11_GameViewState* state,
    const Theron_V1Track02CaptureCampaignReceipt* campaign,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt* dungeonWindow);
/* Extends capture-currentness to the live direct CUE/BIN/ISO receipt. The
 * capture campaign stays opaque and is cleared on re-scan epoch, layout, plan,
 * or launch-trace drift. */
int M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrentForDirectMedia(
    M11_GameViewState* state,
    const Theron_V1Track02CaptureCampaignReceipt* campaign,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt* dungeonWindow,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt* campaignMedia,
    const Theron_V1Track02RawMediaIntakeReceipt* refreshedMedia,
    const Theron_V1Track02CaptureTargetPlan* campaignPlan,
    uint32_t campaignMediaScanEpoch);

/* Publishes a verified non-startup CD record as opaque no-draw readiness.
 * A failed bind clears this independent receipt and leaves every draw path
 * unchanged. */
int M11_GameView_TheronBindTrack02SectorRecordAdmission(
    M11_GameViewState* state,
    const Theron_V1Track02SectorRecordAdmissionReceipt* receipt);
/* Accepts only a complete hash-first direct corpus discovery and delegates its
 * opaque record receipt to the existing no-draw sector admission. */
int M11_GameView_TheronBindTrack02SectorRecordCorpusDiscovery(
    M11_GameViewState* state,
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt* discovery);
/* Consumes only a READY direct CUE/BIN + coalesced-trace descriptor intake;
 * delegates its opaque descriptor to the existing corpus admission. */
int M11_GameView_TheronBindTrack02LevelObjectDescriptorCaptureIntake(
    M11_GameViewState* state,
    const Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt* receipt);
/* Binds only a READY descriptor-chain bitmap/palette capture identity to the
 * M11 presentation boundary. The resulting state is explicitly no-draw and
 * cannot provide palette entries, bitmap bytes, or render permission. */
int M11_GameView_TheronBindTrack02DescriptorBitmapPaletteCaptureIntake(
    M11_GameViewState* state,
    const Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt* receipt);
/* Returns current opaque presentation provenance only. A true result still
 * means no-draw; it is not a renderer or decoder admission. */
int M11_GameView_TheronTrack02DescriptorBitmapPalettePresentationNoDrawCurrent(
    const M11_GameViewState* state);
int M11_GameView_TheronBindTrack02DungeonCapturePlan(
    M11_GameViewState* state,
    const Theron_V1Track02DungeonCapturePlanAdmissionReceipt* receipt);
/* Joins the imported opaque artifact's dungeon row to the original
 * descriptor-selected $e009 capture retained by the direct sector corpus.
 * It is a live no-draw handoff gate, not a payload or rendering admission. */
int M11_GameView_TheronBindTrack02HandoffLoaderCapture(
    M11_GameViewState* state,
    const Theron_V1Track02HandoffArtifactCorpusReceipt* artifact_corpus,
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt* sector_corpus,
    const Theron_V1Track02CaptureTargetPlan* plan,
    const Theron_V1Track02LaunchTraceIdentityReceipt* trace_identity,
    uint32_t campaign_media_scan_epoch);
/* Binds an immutable G8 FIFO sidecar only as current capture-required/no-draw
 * lifecycle provenance. It cannot promote a route or provide payload bytes. */
int M11_GameView_TheronBindTrack02G8FifoSidecarCaptureRequired(
    M11_GameViewState* state,
    const Theron_V1Track02G8FifoSidecarReceipt* sidecar,
    const Theron_V1Track02HandoffArtifactCorpusReceipt* artifact_corpus,
    uint32_t campaign_media_scan_epoch);
/* Rechecks an opaque later descriptor-selected CD record against the current
 * direct layout and replay tail. It never exposes record bytes or semantics. */
int M11_GameView_TheronTrack02SectorRecordAdmissionCurrentForDirectMedia(
    M11_GameViewState* state,
    const Theron_V1Track02SectorRecordAdmissionReceipt* receipt,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt* campaignMedia,
    const Theron_V1Track02RawMediaIntakeReceipt* refreshedMedia,
    const Theron_V1Track02CaptureTargetPlan* campaignPlan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt* replay,
    uint32_t campaignLayoutEpoch,
    uint32_t campaignMediaScanEpoch);
/* Binds one already observed descriptor-selected later record to the first
 * live dungeon handoff. It is an opaque M11 world/level-object boundary only:
 * missing corpus, media/trace/replay drift, or any semantic/draw flag clears
 * it rather than constructing a level, object, or visual fallback. */
int M11_GameView_TheronBindTrack02FirstDungeonWorldAdmission(
    M11_GameViewState* state,
    const Theron_V1Track02SectorRecordAdmissionReceipt* receipt,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt* campaignMedia,
    const Theron_V1Track02RawMediaIntakeReceipt* refreshedMedia,
    const Theron_V1Track02CaptureTargetPlan* campaignPlan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt* replay,
    uint32_t campaignLayoutEpoch,
    uint32_t campaignMediaScanEpoch);

/* Requires a current direct-only campaign layout and replay receipt plus Soul
 * Room epoch 1 followed by dungeon-handoff epoch 2 from one source-trace and
 * converted HuC6280 event-log identity. This is route readiness only and
 * never enables draw. */
int M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
    M11_GameViewState* state,
    const Theron_V1Track02LiveLoaderRouteAdmissionReceipt* receipt,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt* campaignMedia,
    const Theron_V1Track02RawMediaIntakeReceipt* refreshedMedia,
    const Theron_V1Track02CaptureTargetPlan* campaignPlan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt* replay,
    uint32_t campaignLayoutEpoch,
    uint32_t campaignMediaScanEpoch,
    Theron_V1Track02LiveRouteKind route,
    uint32_t epoch);
int M11_GameView_TheronBindSrmCampaignReplayReceipt(
    M11_GameViewState* state, const Theron_V1SrmCampaignReplayReceipt* receipt);
int M11_GameView_TheronBindSrmLaunchDiscoveryReceipt(
    M11_GameViewState* state, const Theron_V1SrmLaunchDiscoveryReceipt* receipt);

/* Total derived-bitmap cache slots F460_xxxx_START_CalculateDerivedBitmap-
 * CacheSizes reserves for this creature (Front D3 + Front D2 always,
 * +2 for each pose with a dedicated bitmap, +3 per additional front). */
int M11_GameView_GetCreatureDerivedBitmapCount(int creatureType);

/* Return the floor ornament ordinal for a viewport cell position.
 * relForward/relSide are relative to party position/facing. */
int M11_GameView_GetFloorOrnamentOrdinal(const M11_GameViewState* state,
                                         int relForward, int relSide);

/* Source endgame title placement helper for probes.  Mirrors
 * ENDGAME.C:F0444_STARTEND_Endgame spacing: title starts after
 * Champion.Name, with an extra character gap unless the title begins
 * with ',', ';', or '-'. */
int M11_GameView_EndgameTitleXForSourceText(const char* name, const char* title);

int M11_GameView_GetMirrorCatalogCount(const M11_GameViewState* state);
int M11_GameView_GetMirrorNameByOrdinal(const M11_GameViewState* state,
                                        int mirrorOrdinal,
                                        char* outName,
                                        int outSize);
int M11_GameView_GetMirrorTitleByOrdinal(const M11_GameViewState* state,
                                         int mirrorOrdinal,
                                         char* outTitle,
                                         int outSize);
int M11_GameView_RecruitChampionByMirrorOrdinal(M11_GameViewState* state,
                                                int mirrorOrdinal);
int M11_GameView_RecruitChampionByMirrorName(M11_GameViewState* state,
                                             const char* name);
int M11_GameView_GetFrontMirrorOrdinal(const M11_GameViewState* state);
int M11_GameView_CsbF0282ChampionPanelGateActive(
    const M11_GameViewState* state, int* out_front_ordinal,
    int* out_candidate_ordinal, int* out_party_index);
/* Probe compatibility adapter for the source-owned D1C champion-mirror
 * C346 wall-ornament destination.  Coordinates remain viewport-relative. */
int M11_GameView_GetDm1WallOrnamentZone(int* outX,
                                        int* outY,
                                        int* outW,
                                        int* outH);
int M11_GameView_GetD1CWallOrnamentZone(const M11_GameViewState* state,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH);
/* Compatibility-only geometry and material adapters for legacy M11 probes.
 * They delegate to DM1 PC34 layout helpers and never alter runtime drawing. */
int M11_GameView_GetV1StatusHandSlotBoxZone(int slot, int hand,
                                            int* outX, int* outY,
                                            int* outW, int* outH);
int M11_GameView_GetV1DamageNumberOriginPc34(int slot, int amount,
                                             int inventoryChampion,
                                             int* outX, int* outY);
int M11_GameView_GetV1ChampionIconZone(int slot,
                                       int* outX, int* outY,
                                       int* outW, int* outH);
int M11_GameView_GetV1ActionIconCellZone(
    int champion_slot, int* out_x, int* out_y, int* out_w, int* out_h);
/* ReDMCSB ACTIDRAW.C F0386's final action-cell hatch decision.  This
 * consumes the live C11/F0407 disabled-action receipt; it does not create a
 * second cooldown or infer a state from host timing. */
int M11_GameView_ShouldHatchV1ActionIconCell(
    const M11_GameViewState* state, int champion_slot);
int M11_GameView_GetV1ActionMenuRowZone(
    int row, int* out_x, int* out_y, int* out_w, int* out_h);
int M11_GameView_GetV1SlotBoxNormalGraphicId(void);
int M11_GameView_GetV1SlotBoxActingHandGraphicId(void);
int M11_GameView_GetV1StatusHandSlotGraphic(
    const M11_GameViewState* state, int slot, int hand);
int M11_GameView_GetV1StatusNameColor(
    const M11_GameViewState* state, int champion_slot);
int M11_GameView_GetV1StatusNameClearColor(void);
int M11_GameView_GetV1StatusBoxFillColor(void);
int M11_GameView_GetV1StatusBoxZoneId(int champion_slot);
int M11_GameView_GetV1StatusBoxZone(
    int champion_slot, int* out_x, int* out_y, int* out_w, int* out_h);
int M11_GameView_GetV1StatusNameClearZoneId(int champion_slot);
int M11_GameView_GetV1StatusNameTextZoneId(int champion_slot);
int M11_GameView_GetV1StatusNameZone(
    int champion_slot, int* out_x, int* out_y, int* out_w, int* out_h);
int M11_GameView_GetV1StatusNameTextZone(
    int champion_slot, int* out_x, int* out_y, int* out_w, int* out_h);
int M11_GameView_GetV1StatusHandParentZoneId(int champion_slot);
int M11_GameView_GetV1StatusHandZoneId(int champion_slot, int hand_index);
int M11_GameView_GetV1StatusHandZone(
    int champion_slot, int hand_index,
    int* out_x, int* out_y, int* out_w, int* out_h);
int M11_GameView_GetV1StatusBarGraphZoneId(int champion_slot);
int M11_GameView_GetV1StatusBarZoneId(int stat_index);
int M11_GameView_GetV1StatusBarValueZoneId(
    int champion_slot, int stat_index);
int M11_GameView_GetV1StatusBarZone(
    int champion_slot, int stat_index,
    int* out_x, int* out_y, int* out_w, int* out_h);
int M11_GameView_GetV1ChampionBarColor(int champion_slot);
int M11_GameView_GetV1StatusBarBlankColor(void);
int M11_GameView_GetV1StatusBoxBaseGraphic(
    const M11_GameViewState* state, int champion_slot);
int M11_GameView_GetV1DeadStatusBoxGraphicId(void);
int M11_GameView_ProbeCsbRuntimeOverlayDrawStats(
    const M11_GameViewState *state,
    int *out_object_sprite_count,
    int *out_object_icon_count,
    int *out_object_marker_count,
    int *out_group_sprite_count,
    int *out_group_marker_count,
    int *out_projectile_sprite_count,
    int *out_projectile_material_count,
    int *out_projectile_marker_count,
    int *out_explosion_sprite_count,
    int *out_explosion_marker_count);
/* Admit an exact source-owned C14 into the active CSB boot profile.  The
 * ordinary M11 F0128 frame consumes the profile queue with no marker path. */
int M11_GameView_AdmitCsbPostTeleportProjectileImpact(
    M11_GameViewState *state,
    int map_index, int map_x, int map_y, uint16_t projectile_thing,
    int projectile_aspect_ordinal, int side, int coordinate_set);
/* Read-only M11 viewport inspection for HoC false-item regression probes. */
int M11_GameView_ProbeViewportFloorItemCounts(
    const M11_GameViewState* state, int relForward, int relSide,
    int* outMapX, int* outMapY, int* outElementType,
    int* outFloorItemCount, int* outSummaryItemCount);
int M11_GameView_GetDm1HocMenuRouteReceipt(
    const M11_GameViewState* state,
    DM1_V1_EntranceMenuRouteReceiptPc34* outReceipt);
int M11_GameView_SelectFrontMirrorCandidate(M11_GameViewState* state);
int M11_GameView_ConfirmMirrorCandidate(M11_GameViewState* state,
                                        int reincarnate);
int M11_GameView_BeginMirrorCandidateReincarnateRename(M11_GameViewState* state);
int M11_GameView_ApplyMirrorCandidateRenameAscii(M11_GameViewState* state,
                                                 int ch);
int M11_GameView_ApplyMirrorCandidateRenameCommand(M11_GameViewState* state,
                                                   int command);
int M11_GameView_HandleMirrorCandidateRenameClick(M11_GameViewState* state,
                                                  int x,
                                                  int y);
int M11_GameView_CancelMirrorCandidate(M11_GameViewState* state);
int M11_GameView_GetDm2LeaderHandObjectIconZone(int* outX,
                                                int* outY,
                                                int* outW,
                                                int* outH);
int M11_GameView_GetDm2LeaderHandObjectCursorIconZone(
    const M11_GameViewState* state,
    int* outX,
    int* outY,
    int* outW,
    int* outH);
uint32_t M11_GameView_GetDm2LeaderHandObject(const M11_GameViewState* state);
uint32_t M11_GameView_GetDm2InventoryObject(const M11_GameViewState* state,
                                            int championIndex,
                                            int championSlot);
int M11_GameView_Dm2LeaderHandObjectIconAvailable(
    const M11_GameViewState* state);
int DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(M11_GameViewState* state,
                                        unsigned short thing);
void DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(M11_GameViewState* state);
unsigned short DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(const M11_GameViewState* state);
int DM1_V1_M11Runtime_GetLeaderHandObjectIconIndexPc34Compat(const M11_GameViewState* state);
int DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat(const M11_GameViewState* state,
                                             int championSlot);
int DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(const M11_GameViewState* state,
                                           char* out,
                                           int outSize);
int DM1_V1_M11Runtime_DecodeInventoryActionHandScrollTextPc34Compat(
    const M11_GameViewState* state,
    char* out,
    int outSize);
int DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(M11_GameViewState* state);
void DM1_V1_M11Runtime_CloseOpenChestPc34Compat(M11_GameViewState* state);
unsigned short DM1_V1_M11Runtime_GetOpenChestThingPc34Compat(const M11_GameViewState* state);

/* M11_DM1 V1 sub-cell hit mask (BUG-111).  Source-locked per
 * ReDMCSB DEFS.H M550 (DUNGEON.C:1085).  Full-square creatures
 * use 0x0F (all 4 sub-cells); quarter-square / giant / 2x2
 * creatures use 0xF0 (high nibble).  v1 always uses 0x0F
 * because per-sub-cell positioning is deferred to post-M10. */
#define M11_DM1_CELL_OCCUPIED_MASK 0x0Fu
#define M11_DM1_CELL_OCCUPIED_QUARTER 0xF0u

/* ── Forced-pause dialog fit/layout (Firestaff session timer) ───────
 * Not driven by ReDMCSB; this surface is the session-timer escalation
 * overlay that pops when the limit hits zero.  The layout shrinks or
 * widens its box and rewrites its text per state->fontScale (1..3) so
 * the title and prompt lines never overflow the 320x200 framebuffer.
 * Source-locked contract documented in
 * firestaff_dm1_v1_forced_pause_font_scale_fit_probe.c. */
typedef struct M11_ForcedPauseDialogLayout {
    int scale;             /* 1..3 (clamped) */
    int boxX;
    int boxY;
    int boxW;
    int boxH;
    int titleX;
    int titleY;
    int line1X;
    int line1Y;
    int line2X;
    int line2Y;
    char title[64];
    char line1[32];
    char line2[32];
} M11_ForcedPauseDialogLayout;

void M11_GameView_GetForcedPauseDialogLayout(
    const M11_GameViewState* state,
    int framebufferWidth,
    int framebufferHeight,
    M11_ForcedPauseDialogLayout* outLayout);

int M11_GameView_ForcedPauseDialogLayoutMaxTextPixelWidth(
    const M11_ForcedPauseDialogLayout* layout);

/* ── Session-timer reminder banner fit/layout ────────────────────
 * Sibling to M11_ForcedPauseDialogLayout.  Same contract: the
 * reminder banner is a Firestaff-specific, non-ReDMCSB overlay
 * surface whose box dimensions, insets, and text wording shrink or
 * widen so they fit the 320x200 framebuffer at every supported
 * fontScale (1..3).  Unlike the forced-pause dialog (which is a
 * centred modal), the reminder banner is a top-strip overlay that
 * must never paint into the source-owned DM1 dungeon viewport at
 * y=33..168; the fit gate pins bannerY+boxH <= the viewport top
 * (33) so the banner stays in its own bottom-padded rectangle
 * (y=4..31 by default) regardless of fontScale.
 *
 * Source-locked contract documented in
 * firestaff_dm1_v1_reminder_banner_font_scale_fit_probe.c. */
typedef struct M11_ReminderBannerLayout {
    int scale;             /* 1..3 (clamped) */
    int boxX;
    int boxY;
    int boxW;
    int boxH;
    int textX;             /* draw origin for m11_draw_text_centered_in_rect */
    int textY;
    int innerX;            /* text-drawing rect.left */
    int innerY;
    int innerW;            /* text-drawing rect width */
    char line[64];         /* wording that m11_format_session_timer_reminder_line computes */
} M11_ReminderBannerLayout;

void M11_GameView_GetReminderBannerLayout(
    const M11_GameViewState* state,
    int framebufferWidth,
    int framebufferHeight,
    M11_ReminderBannerLayout* outLayout);

int M11_GameView_ReminderBannerLayoutMaxTextPixelWidth(
    const M11_ReminderBannerLayout* layout);

/* Plain ESC return-to-menu confirmation fit/layout.  The source-owned
 * unsaved-game guard continues through the DM dialog-choice path; this
 * helper covers only the Firestaff-owned "RETURN TO START MENU?" modal. */
typedef struct M11_ReturnConfirmDialogLayout {
    int scale;             /* 1..3 (clamped) */
    int boxX;
    int boxY;
    int boxW;
    int boxH;
    int promptX;
    int promptY;
    int choiceY;
    int choiceW;
    char prompt[64];
    char choice0[32];
    char choice1[32];
} M11_ReturnConfirmDialogLayout;

void M11_GameView_GetReturnConfirmDialogLayout(
    const M11_GameViewState* state,
    int framebufferWidth,
    int framebufferHeight,
    M11_ReturnConfirmDialogLayout* outLayout);

int M11_GameView_ReturnConfirmDialogLayoutMaxTextPixelWidth(
    const M11_ReturnConfirmDialogLayout* layout);

/* ── Click hit-test (replaces ReDMCSB F0376_COMMAND_IsPointInBox) ────
 * Returns 1 if (px,py) is inside the closed box {left,right,top,bottom}
 * (inclusive), 0 otherwise.  Mirrors the contract of CLIKVIEW.C:290
 * in ReDMCSB so the modern M11 door-button / wall-ornament /
 * object-pile click pipeline stays source-equivalent.
 */
int m11_point_in_source_box(int px, int py, const int box[4]);

/* Probe-only C708 ownership trace. Hashes cover the exact F0679 destination
 * rectangle (x=216..223, y=57..108) after successive V1 viewport passes. */
typedef struct M11_D2R2WriteTrace {
    unsigned int count;
    unsigned int hashes[16];
    int c707_graphic;
    int c708_graphic;
    int c707_flipped;
    int c708_flipped;
    int c708_materialized_graphic;
    int c708_source_width;
    int c708_source_height;
    int c708_transparent_color;
    unsigned int c708_source_index_writes[16];
    unsigned char c708_source_raw_sample[16];
    unsigned int c708_source_non_nibble_writes;
    unsigned int c708_flipped_immediate_expected;
    unsigned int c708_flipped_immediate_matched;
    unsigned int side_blit_count;
    int side_blit_rel_forward[16];
    int side_blit_rel_side[16];
    int side_blit_graphic[16];
    unsigned int side_blit_hashes[16];
    unsigned int checkpoint_c708_matches[16];
} M11_D2R2WriteTrace;

void M11_GameView_ProbeGetD2R2WriteTrace(M11_D2R2WriteTrace* outTrace);

/* Read-only observation from the last actual DM1 F0115 floor-item blit.
 * It remains invalid until the real M11 asset/material path reaches its
 * final destination geometry. */
typedef struct M11_Dm1FloorItemHostPresentationReceipt {
    int valid;
    int floorItemLane;
    int graphicsId;
    int transparentColor;
    int usesF0791Blit;
    int sourceZone;
    int sourceZoneRow;
    int destinationX;
    int destinationY;
    int destinationW;
    int destinationH;
    int assetWidth;
    int assetHeight;
    /* The final F0791 blit must visibly alter its destination region. */
    unsigned int destinationBeforeFNV1a;
    unsigned int destinationAfterFNV1a;
    int destinationPixelsChanged;
} M11_Dm1FloorItemHostPresentationReceipt;

void M11_GameView_GetDm1FloorItemHostPresentationReceipt(
    M11_Dm1FloorItemHostPresentationReceipt* outReceipt);

/* F0115's F0121/F0124 alcove invocation is a separate wall lane.  It uses
 * the same original C10/F0791 object material but must never impersonate a
 * floor-object capture. */
void M11_GameView_GetDm1AlcoveItemHostPresentationReceipt(
    M11_Dm1FloorItemHostPresentationReceipt* outReceipt);

/* Final M11 capture consumption for a real, non-HoC F0115 floor item.
 * The receipt is frame-local and stays invalid when the current source item
 * cannot reach its GRAPHICS.DAT material blit. */
typedef struct M11_Dm1F0115FloorItemRuntimeCaptureReceipt {
    int valid;
    unsigned int runtimeTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int materialFNV1a;
    M11_Dm1FloorItemHostPresentationReceipt presentation;
} M11_Dm1F0115FloorItemRuntimeCaptureReceipt;

void M11_GameView_GetDm1F0115FloorItemRuntimeCaptureReceipt(
    M11_Dm1F0115FloorItemRuntimeCaptureReceipt* outReceipt);

/* Final M11 capture consumption for F0115's separate, deferred C15 pass.
 * The receipt aggregates only current-frame F0114/M636 GRAPHICS.DAT blits
 * outside HoC; a missing source surface leaves it clear. */
typedef struct M11_Dm1F0115C15RuntimeCaptureReceipt {
    int valid;
    unsigned int runtimeTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int materialFNV1a;
    int requestedMaterialCount;
    int completedMaterialCount;
} M11_Dm1F0115C15RuntimeCaptureReceipt;

void M11_GameView_GetDm1F0115C15RuntimeCaptureReceipt(
    M11_Dm1F0115C15RuntimeCaptureReceipt* outReceipt);

/* Final M11 capture consumption for F0115's C2900 live-projectile lane.
 * It accepts only current-frame M613 or F0142/G0209/M612 GRAPHICS.DAT blits
 * outside HoC; missing or stale material leaves the receipt clear. */
typedef struct M11_Dm1F0115C2900RuntimeCaptureReceipt {
    int valid;
    unsigned int runtimeTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int materialFNV1a;
    int requestedMaterialCount;
    int completedMaterialCount;
} M11_Dm1F0115C2900RuntimeCaptureReceipt;

void M11_GameView_GetDm1F0115C2900RuntimeCaptureReceipt(
    M11_Dm1F0115C2900RuntimeCaptureReceipt* outReceipt);

/* Probe the exact receipt predicate consumed by the HoC capture facts.
 * itemPresent must come from the current F0115 floor-item pass. */
int M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(int itemPresent);

/* Read-only view of the production F0115 viewport cell.  HoC probes use
 * this to verify that compact square lists and mirror filtering do not
 * surface candidate payloads as ordinary floor items. */
int M11_GameView_ProbeViewportFloorItemCounts(
    const M11_GameViewState* state,
    int relativeForward,
    int relativeSide,
    int* outMapX,
    int* outMapY,
    int* outElementType,
    int* outFloorItems,
    int* outSummaryItems);

/* Test probes for the actual F0115 item and projectile blitters. They do not
 * synthesize a receipt; callers must provide an M11 asset-loader cache slot. */
int M11_GameView_ProbeDrawDm1FloorItemHostReceipt(
    M11_GameViewState* state,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);
int M11_GameView_ProbeDrawDm1AlcoveItemForFloorItemReceipt(
    M11_GameViewState* state,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);
int M11_GameView_ProbeDrawDm1ProjectileForFloorItemReceipt(
    M11_GameViewState* state,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);

/* Read-only evidence from a completed DM1 F0115 creature material blit.
 * It records the original GRAPHICS.DAT bitmap and the G0221/G0222 palette
 * selected by the DM1 route; it is invalid until that host blit succeeds. */
typedef struct M11_Dm1CreatureHostPresentationReceipt {
    int valid;
    int creatureLane;
    int creatureType;
    int depthIndex;
    int graphicsId;
    int transparentColor;
    int mirrored;
    int destinationX;
    int destinationY;
    int destinationW;
    int destinationH;
    int assetWidth;
    int assetHeight;
    unsigned int paletteChecksum;
} M11_Dm1CreatureHostPresentationReceipt;

void M11_GameView_GetDm1CreatureHostPresentationReceipt(
    M11_Dm1CreatureHostPresentationReceipt* outReceipt);

/* Test-only entry to the production F0115 creature material route.  The
 * caller must provide a real initialized GRAPHICS.DAT asset loader. */
int M11_GameView_ProbeDrawDm1CreatureHostReceipt(
    M11_GameViewState* state,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);

/* Read one real C04 group candidate through the same F0115 receipt that the
 * M11 creature tick consumes.  This is a read-only corpus probe: it does not
 * construct a dungeon candidate or mutate the runtime. */
int M11_GameView_ProbeDm1F0115CreatureTickCandidate(
    const M11_GameViewState* state,
    int mapIndex,
    int mapX,
    int mapY,
    DM1_F0115WorldGroupCandidatePc34* outCandidate);

/* Read-only evidence from a completed DM1 F0115 C2900 projectile blit.
 * `objectMaterial` distinguishes the F0142 -> G0209 thrown-object branch
 * from the native M613 projectile bitmap branch. */
typedef struct M11_Dm1ProjectileHostPresentationReceipt {
    int valid;
    int projectileLane;
    int objectMaterial;
    int graphicsId;
    int objectAspectIndex;
    int transparentColor;
    int flipFlags;
    int sourceZoneRow;
    int destinationX;
    int destinationY;
    int destinationW;
    int destinationH;
    int assetWidth;
    int assetHeight;
} M11_Dm1ProjectileHostPresentationReceipt;

void M11_GameView_GetDm1ProjectileHostPresentationReceipt(
    M11_Dm1ProjectileHostPresentationReceipt* outReceipt);

/* Test-only entry to the production F0115 thrown-object projectile route.
 * The caller must provide a real initialized GRAPHICS.DAT asset loader. */
int M11_GameView_ProbeDrawDm1ThrownObjectProjectileHostReceipt(
    M11_GameViewState* state,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);

typedef struct M11_Dm1DoorHostPresentationReceipt {
    int valid;
    int depthIndex;
    int doorState;
    int panelVisible;
    int frameCount;
    int blitCount;
    int graphicsId[DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS];
    int destinationX[DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS];
    int destinationY[DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS];
    int width[DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS];
    int height[DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS];
} M11_Dm1DoorHostPresentationReceipt;

void M11_GameView_GetDm1DoorHostPresentationReceipt(
    M11_Dm1DoorHostPresentationReceipt* outReceipt);

/* Test-only entry to the production F0111 center-door material route.
 * It consumes a DM1 receipt and requires real GRAPHICS.DAT assets. */
int M11_GameView_ProbeDrawDm1CenterDoorHostReceipt(
    M11_GameViewState* state,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);

typedef struct M11_Dm1WallOrnamentHostPresentationReceipt {
    int valid;
    int globalOrnamentIndex;
    int viewWallIndex;
    int graphicIndex;
    int destinationX;
    int destinationY;
    int width;
    int height;
    int transparentColor;
    int flipHorizontal;
    int paletteMapValid;
    unsigned char paletteMap[16];
} M11_Dm1WallOrnamentHostPresentationReceipt;

void M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(
    M11_Dm1WallOrnamentHostPresentationReceipt* outReceipt);

/* ReDMCSB DUNVIEW.C F0107:3913-3928 consumes a C127 champion mirror as
 * one C346 backing plus one C026 atlas-cell operation. Published only after
 * both source-backed M11 blits succeed in the current frame. */
typedef struct M11_Dm1HoCMirrorHostPresentationReceipt {
    int valid;
    int renderIndex;
    int backingGraphicIndex;
    int backingSourceWidth;
    int backingSourceHeight;
    int backingDestinationX;
    int backingDestinationY;
    int backingWidth;
    int backingHeight;
    int backingTransparentColor;
    int backingPaletteMapValid;
    unsigned char backingPaletteMap[16];
    int portraitGraphicIndex;
    int portraitSourceX;
    int portraitSourceY;
    int portraitDestinationX;
    int portraitDestinationY;
    int portraitWidth;
    int portraitHeight;
    int portraitTransparentColor;
} M11_Dm1HoCMirrorHostPresentationReceipt;

void M11_GameView_GetDm1HoCMirrorHostPresentationReceipt(
    M11_Dm1HoCMirrorHostPresentationReceipt* outReceipt);

#define M11_DM1_HOC_MIRROR_VIEWPORT_MATERIAL_MAX 16

/* Frame-local F0107/F0115 consumption of live C127 wall facts. D1C records
 * the admitted C346/C026 pair; D1L/D1R records C346 only; D2+ records the
 * source-required no-draw rather than an invented distant mirror. */
typedef struct M11_Dm1HoCMirrorViewportMaterialReceipt {
    int valid;
    int renderIndex;
    int relativeForward;
    int relativeSide;
    int viewWallIndex;
    int globalOrnamentIndex;
    int materialized;
    int backingGraphicIndex;
    int portraitGraphicIndex;
    int suppressChampionPortrait;
    int suppressHostFallbackVisuals;
} M11_Dm1HoCMirrorViewportMaterialReceipt;

typedef struct M11_Dm1HoCMirrorViewportMaterialFrameReceipt {
    int count;
    M11_Dm1HoCMirrorViewportMaterialReceipt
        entries[M11_DM1_HOC_MIRROR_VIEWPORT_MATERIAL_MAX];
} M11_Dm1HoCMirrorViewportMaterialFrameReceipt;

void M11_GameView_GetDm1HoCMirrorViewportMaterialFrameReceipt(
    M11_Dm1HoCMirrorViewportMaterialFrameReceipt* outReceipt);

typedef struct M11_Dm1UnreadableInscriptionHostPresentationReceipt {
    int valid;
    int textStringIndex;
    int viewWallIndex;
    int relativeForward;
    int relativeSide;
    int lineCount;
    int boxHeight;
    int graphicIndex;
    int destinationX;
    int destinationY;
    int width;
    int height;
    int transparentColor;
    int paletteMapValid;
    unsigned char paletteMap[16];
    int textDataWordOffset;
    int textDataWordCount;
    unsigned int textDataFNV1a;
    unsigned int glyphBytesFNV1a;
} M11_Dm1UnreadableInscriptionHostPresentationReceipt;

void M11_GameView_GetDm1UnreadableInscriptionHostPresentationReceipt(
    M11_Dm1UnreadableInscriptionHostPresentationReceipt* outReceipt);

/* DM1 V1 F0128 complete per-square source scheduler, live M11 bridge
 * receipt (ReDMCSB DUNVIEW.C F0128:8318-8561 visit order; per-square
 * functions F0676/F0677:6226-6360, F0678/F0679:6837-6899,
 * F0116-F0127:6361-8317).  Published once per DM1 dungeon-view frame:
 * the live sampled 19-square view is merged into the contract plan, the
 * plan invariants are re-verified, and the F0115 content loop consumes
 * the plan's per-square spans in source visit order.  A frame that
 * cannot build a verified plan keeps the legacy hand-rolled loop and
 * publishes planReady/planDrivenContentLoop = 0; no host substitute
 * plan is ever invented. */
typedef struct M11_Dm1F0128PerSquareSchedulerReceipt {
    int valid;                    /* a DM1 frame evaluated the bridge */
    int planReady;                /* build + verify both succeeded */
    int planDrivenContentLoop;    /* F0115 content loop consumed plan spans */
    int stepCount;                /* merged plan step count */
    unsigned long scheduleHash;   /* plan FNV-1a receipt hash */
    int f0115ContentSquareCount;  /* of D3L..D1C, squares with an F0115 step */
} M11_Dm1F0128PerSquareSchedulerReceipt;

void M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(
    M11_Dm1F0128PerSquareSchedulerReceipt* outReceipt);

/* Test-only entry to the production F0107 side-wall ornament route.
 * The caller must initialize an original PC34 GRAPHICS.DAT loader. */
int M11_GameView_ProbeDrawDm1SideWallOrnamentHostReceipt(
    M11_GameViewState* state,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);

/* Test-only F0107 consumer for a C127 mirror one tile away in D1L/D1R.
 * It admits only the authenticated C346 backing and never draws C026. */
int M11_GameView_ProbeDrawDm1ChampionMirrorSideBackingHostReceipt(
    M11_GameViewState* state,
    int relSide,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight);

/* Test-only read-only count for the production V2 effect seed scan. */
int M11_GameView_ProbeDm1V2LiveEffectSeedCount(
    const M11_GameViewState* state);

typedef struct M11_Dm1InscriptionHostPresentationReceipt {
    int valid;
    int textStringIndex;
    int fontGraphicIndex;
    int transparentColor;
    int glyphByteCount;
    int lineCount;
    unsigned char glyphBytes[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    int textDataWordOffset;
    int textDataWordCount;
    unsigned int textDataFNV1a;
    unsigned int glyphBytesFNV1a;
    unsigned int fontPixelsFNV1a;
    unsigned int sourceCellsFNV1a;
    int glyphSourceWidth;
    int glyphSourceHeight;
    int glyphCellCount;
    int opaqueGlyphPixelCount;
    int transparentGlyphPixelCount;
    int glyphScaleNumerator;
    int glyphScaleDenominator;
    int paletteMapValid;
    /* Final M11 viewport destinations after consuming the M10 F0168/F0107
     * material receipt.  Entries with glyphCount == 0 are unused. */
    int lineDestinationX[DM1_V1_INSCRIPTION_MAX_LINES];
    int lineDestinationY[DM1_V1_INSCRIPTION_MAX_LINES];
    int lineGlyphCount[DM1_V1_INSCRIPTION_MAX_LINES];
} M11_Dm1InscriptionHostPresentationReceipt;

void M11_GameView_GetDm1InscriptionHostPresentationReceipt(
    M11_Dm1InscriptionHostPresentationReceipt* outReceipt);

void M11_GameView_GetDm1HocPresentedFrameConsumerReceipt(
    DM1_V1_HocPresentedFrameConsumerReceiptPc34* outReceipt);

/* Restored diagnostic probe exports (lost to the worktree merge drift;
 * tests/test_m11_overlay_command_queue_block.c consumes them). */
int M11_GameView_ProbeViewportArtifactCounts(const M11_GameViewState* state,
                                             int relForward,
                                             int relSide,
                                             int* outMapX,
                                             int* outMapY,
                                             int* outElementType,
                                             int* outProjectileCount,
                                             int* outExplosionCount,
                                             int* outFirstProjectileGfx,
                                             int* outFirstExplosionType);
int M11_GameView_ProbeViewportRenderMetadata(const M11_GameViewState* state,
                                             int relForward,
                                             int relSide,
                                             int* outMapX,
                                             int* outMapY,
                                             int* outElementType,
                                             int* outWallOrnamentOrdinal,
                                             int* outChampionPortraitOrdinal,
                                             int* outInscriptionTextIndex,
                                             int* outFloorOrnamentOrdinal);
int M11_GameView_ProbeCsbStartupHostViewDrawConsumerReceipt(
    int* outTitleReceiptReady,
    int* outTitleDrawExecuted,
    int* outTitleHudExecuted,
    int* outClosedDoorReceiptReady,
    int* outClosedDoorDrawExecuted,
    int* outClosedDoorHudExecuted,
    int* outUtilityReceiptReady,
    int* outUtilityDrawExecuted,
    int* outUtilityHudExecuted,
    int* outOpeningReceiptReady,
    int* outOpeningDrawExecuted,
    int* outConsumedHostViewOnly,
    int* outSuppressLegacyUtilityFallback,
    int* outPackagedVisualCaptureReady,
    int* outInputConsumesReceiptOnly,
    int* outUtilityInputDispatchReady,
    int* outTitleAssetDrawReady,
    int* outClosedDoorFallbackSuppressed,
    int* outOpeningFrameDrawReady,
    int* outFullVisualSequenceConsumed,
    int* outRuntimeRouteHardeningReady,
    int* outRuntimeRouteHardeningHashReady,
    int* outRuntimeHostCaptureGateReady,
    int* outRuntimeHostCaptureGateHashReady,
    int* outTitleStageRuntimeCaptureReady,
    int* outTitleStageRuntimeCaptureHashReady);
int M11_GameView_ProbeViewportCellClass(const M11_GameViewState* state,
                                        int relForward,
                                        int relSide,
                                        int* outMapX,
                                        int* outMapY,
                                        unsigned char* outRawSquare,
                                        int* outElementType,
                                        int* outEffectiveElementType,
                                        int* outIsWallLike,
                                        int* outIsOpen);
int M11_GameView_ProbeSideWallDrawEligibility(const M11_GameViewState* state,
                                              int relForward,
                                              int relSide,
                                              int* outLegacyLaneClear,
                                              int* outDrawsWithSourceOrder);
int M11_GameView_ProbeDm1NearestBlockingCenterDepth(const M11_GameViewState* state,
                                                    int* outDepthIndex,
                                                    int* outRelForward,
                                                    int* outMapX,
                                                    int* outMapY,
                                                    int* outElementType);
int M11_GameView_ProbeDm1CenterContentVisibleDepthMask(const M11_GameViewState* state,
                                                       int* outDepthMask);
int M11_GameView_GetF0115ViewSquareIndex(int relForward, int relSide);
int M11_GameView_GetObjectSourceScaleIndex(int depthIndex, int relativeCell);
int M11_GameView_GetC2900ProjectileZonePoint(int scaleIndex,
                                             int relativeCell,
                                             int* outX,
                                             int* outY);
int M11_GameView_GetProjectileRawZonePointForRel(int relForward,
                                                 int relSide,
                                                 int relativeCell,
                                                 int* outX,
                                                 int* outY);
int M11_GameView_GetV1MovementArrowZone(int arrowIndex,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH);
/* Restored inventory panel / object description wrapper exports
 * (newest author test state e2f5068c0 consumes them). */
int M11_GameView_GetV1InventoryPanelGraphicId(void);
int M11_GameView_GetV1InventoryBackdropGraphicId(void);
int M11_GameView_GetV1ObjectDescriptionPanelGraphicId(void);
int M11_GameView_GetV1ObjectDescriptionCircleGraphicId(void);
int M11_GameView_GetV1ObjectDescriptionCircleZoneId(void);
int M11_GameView_GetV1ObjectDescriptionCircleZone(int* outX,
                                                   int* outY,
                                                   int* outW,
                                                   int* outH);
int M11_GameView_GetV1ObjectDescriptionIconZoneId(void);
int M11_GameView_GetV1ObjectDescriptionIconZone(int* outX,
                                                 int* outY,
                                                 int* outW,
                                                 int* outH);
int M11_GameView_GetV1ObjectDescriptionNameZoneId(void);
int M11_GameView_GetV1ObjectDescriptionNameZoneForText(int textPixelWidth,
                                                        int textPixelHeight,
                                                        int* outX,
                                                        int* outY,
                                                        int* outW,
                                                        int* outH);
int M11_GameView_GetV1ObjectDescriptionContinuationOrigin(int* outX,
                                                           int* outY);
const char* M11_GameView_GetV1ObjectDescriptionLayoutEvidence(void);
int M11_GameView_GetV1FoodLabelGraphicId(void);
int M11_GameView_GetV1WaterLabelGraphicId(void);
int M11_GameView_GetV1PoisonLabelGraphicId(void);
int M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(int championSlot);
int M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(int sourceSlotBoxIndex);
/* Restored V1 graphic-id wrapper exports (probes/sck and panel tests). */
int M11_GameView_GetV1ActionAreaGraphicId(void);
int M11_GameView_GetV1ArrowOrEyeGraphicId(int pressingEye);
int M11_GameView_GetV1ChampionIconGraphicId(void);
int M11_GameView_GetV1CreatureDamageGraphicId(void);
int M11_GameView_GetV1DialogBackdropGraphicId(void);
int M11_GameView_GetV1EndgameChampionMirrorGraphicId(void);
int M11_GameView_GetV1EndgameTheEndGraphicId(void);
int M11_GameView_GetV1FireShieldBorderGraphicId(void);
int M11_GameView_GetV1MovementArrowsGraphicId(void);
int M11_GameView_GetV1PartyShieldBorderGraphicId(void);
int M11_GameView_GetV1SlotBoxWoundedGraphicId(void);
int M11_GameView_GetV1SpellShieldBorderGraphicId(void);
int M11_GameView_GetV1StatusBoxGraphicId(void);
int M11_GameView_GetV1ViewportBaseGraphic(int layer,
                                          int* outGraphic,
                                          int* outX,
                                          int* outY,
                                          int* outW,
                                          int* outH);

#ifdef __cplusplus
}
#endif





#endif /* FIRESTAFF_M11_GAME_VIEW_H */
