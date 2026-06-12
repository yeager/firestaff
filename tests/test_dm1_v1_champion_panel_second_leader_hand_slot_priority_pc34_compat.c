#include "firestaff/dm1/v1/champion/dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.h"

#include "dm1_v1_champion_panel_hand_slot_priority_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

void F0658_BlitBitmapIndexToZoneIndexWithTransparency(
    int16_t bitmapIndex,
    int16_t zoneIndex,
    int16_t transparentColor)
{
    (void)bitmapIndex;
    (void)zoneIndex;
    (void)transparentColor;
}

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    }
}

static void expect_u32(const char *id, uint32_t got, uint32_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%08X want=0x%08X at %s\n",
               id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    }
}

static void expect_bool(const char *id, bool got, bool want,
                        const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    }
}

static void test_second_leader_priority_and_redraw(void)
{
    dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t model;

    expect_bool("build.model",
                dm1_v1_champion_panel_second_leader_hand_slot_priority_build_pc34(
                    &model),
                true,
                "contract model builds");

    expect_bool("contract.only", model.contract_only, true,
                "no real GRAPHICS.DAT/DUNGEON.DAT load");
    expect_int("champion.index", model.champion_index, 1,
               "CHAMPION.C F0302:680 slotbox >> 1");
    expect_int("leader.index", model.leader_index, 1,
               "CHAMDRAW.C F0292:843-895 leader name-color cascade");
    expect_int("slotbox.index", model.slot_box_index, 3,
               "CHAMPION.C F0302:677 status hand before inventory");
    expect_int("target.slot", model.target_slot_index, DM1_SLOT_ACTION_HAND,
               "DEFS.H:1878 M070_HAND_SLOT_INDEX");
    expect_bool("transaction.accepted", model.transaction_accepted, true,
                "CHAMPION.C F0302:700-712 helper order");
    expect_bool("transaction.status_route", model.transaction_status_hand_route,
                true, "CHAMPION.C F0302:677-684");
    expect_bool("transaction.inventory_route", model.transaction_inventory_route,
                false, "CHAMPION.C F0302:684-687 not inventory route");
    expect_int("transaction.target_champion",
               model.transaction_target_champion_index, 1,
               "CHAMPION.C F0302:680 slotbox >> 1");
    expect_int("transaction.target_ordinal",
               model.transaction_target_champion_ordinal, 2,
               "COMPILE.H M000_INDEX_TO_ORDINAL");
    expect_int("transaction.target_slot", model.transaction_target_slot_index,
               DM1_SLOT_ACTION_HAND,
               "CHAMPION.C F0302:683 M070_HAND_SLOT_INDEX");
    expect_bool("transaction.leader_is_target",
                model.transaction_leader_is_target, true,
                "leader is second champion");
    expect_bool("transaction.leader_before_storage",
                model.leader_hand_precedes_storage_write, true,
                "CHAMPION.C F0302:688 before F0300/F0301");
    expect_bool("transaction.backpack_before_belt",
                model.backpack_precedes_belt, true,
                "DEFS.H:780-817 C13..C29 before C06..C12 contract chain");
    expect_bool("transaction.f0292", model.f0292_final_draw_state, true,
                "CHAMPION.C F0302:711 final F0292 redraw");
    expect_int("transaction.calls", model.transaction_call_sequence_count, 7,
               "CHAMPION.C F0302:700-712 full helper order");
    expect_int("transaction.call0", model.transaction_call_sequence[0],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34,
               "CHAMPION.C F0302:700");
    expect_int("transaction.call1", model.transaction_call_sequence[1],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_PC34,
               "CHAMPION.C F0302:701-702");
    expect_int("transaction.call2", model.transaction_call_sequence[2],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0300_CLEAR_SLOT_PC34,
               "CHAMPION.C F0302:704-705");
    expect_int("transaction.call3", model.transaction_call_sequence[3],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0297_PUT_LEADER_PC34,
               "CHAMPION.C F0302:706");
    expect_int("transaction.call4", model.transaction_call_sequence[4],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0301_WRITE_SLOT_PC34,
               "CHAMPION.C F0302:708-710");
    expect_int("transaction.call5", model.transaction_call_sequence[5],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0292_DRAW_STATE_PC34,
               "CHAMPION.C F0302:711");
    expect_int("transaction.call6", model.transaction_call_sequence[6],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34,
               "CHAMPION.C F0302:712");
}

static void test_second_leader_hud_cascade(void)
{
    dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t model;

    (void)dm1_v1_champion_panel_second_leader_hand_slot_priority_build_pc34(
        &model);

    expect_int("status.zone", model.status_zone, 152,
               "DEFS.H:3783-3786 C151..C154");
    expect_int("status.x", model.status_x, 69,
               "CHAMDRAW.C F0292:771-789 C152 champion 1 zone");
    expect_int("status.y", model.status_y, 0,
               "CHAMDRAW.C F0292:771-789 status-box fill");
    expect_int("status.width", model.status_width, 67,
               "DEFS.H:3783-3786 C152 67x29");
    expect_int("status.height", model.status_height, 29,
               "DEFS.H:3783-3786 C152 67x29");
    expect_int("status.fill", model.status_fill_color, DM1_COLOR_DARKEST_GRAY,
               "CHAMDRAW.C F0292:784-789 fill C12");
    expect_int("name.zone", model.name_zone, 160,
               "DEFS.H:3787-3791 C160 champion 1 status-box name");
    expect_int("name.text.zone", model.name_text_zone, 164,
               "DEFS.H:3791 C163 + champion index");
    expect_int("name.leader.color", model.leader_name_color, DM1_COLOR_YELLOW,
               "CHAMDRAW.C F0292:850-851 PC34 leader C11");
    expect_int("name.nonleader.color", model.nonleader_name_color,
               DM1_COLOR_GOLD,
               "CHAMDRAW.C F0292:850-851 PC34 nonleader C09");

    expect_int("hp.zone", model.hp_bar_zone, 196,
               "CHAMDRAW.C F0287:307 C195 + champion index");
    expect_int("hp.x", model.hp_bar_x, 115,
               "CHAMDRAW.C F0287:307-342 champion 1 HP bar");
    expect_int("hp.y", model.hp_bar_y, 2,
               "CHAMDRAW.C F0287:307-342 row 2 bar zone");
    expect_int("hp.blank.height", model.hp_blank_height, 13,
               "CHAMDRAW.C F0287:320-326 C12 blank split");
    expect_int("hp.fill.height", model.hp_fill_height, 12,
               "CHAMDRAW.C F0287:335-342 fill split");
    expect_int("hp.blank.color", model.hp_blank_color, DM1_COLOR_DARKEST_GRAY,
               "CHAMDRAW.C F0287:326 blank C12");
    expect_int("hp.fill.color", model.hp_fill_color, DM1_COLOR_YELLOW,
               "G0046 champion 1 color");

    expect_int("icon.zone", model.icon_zone, 114,
               "DEFS.H:3779-3782 C113..C116 champion-icon zones");
    expect_int("icon.width", model.icon_width, 19,
               "CHAMDRAW.C F0292:1019-1051 / F0622 19x14");
    expect_int("icon.height", model.icon_height, 14,
               "CHAMDRAW.C F0292:1019-1051 / F0622 19x14");
    expect_int("icon.fill", model.icon_fill_color, DM1_COLOR_YELLOW,
               "CHAMDRAW.C F0292:1022 champion color fill");

    expect_int("action.hand.zone", model.action_hand_zone, 214,
               "DEFS.H:3802-3803 C214 champion 1 action hand");
    expect_int("action.hand.x", model.action_hand_x, 93,
               "CHAMDRAW.C F0291:632-651 champion 1 action hand origin");
    expect_int("action.hand.y", model.action_hand_y, 10,
               "CHAMDRAW.C F0291:632-651 status hand row");
    expect_int("action.hand.width", model.action_hand_width, 18,
               "CHAMDRAW.C F0291:653-672 C033/C034/C035 slot box");
    expect_int("action.hand.height", model.action_hand_height, 18,
               "CHAMDRAW.C F0291:653-672 C033/C034/C035 slot box");
    expect_int("action.hand.graphic", model.action_hand_graphic,
               DM1_GFX_SLOT_ACTING,
               "CHAMDRAW.C F0291:648-651 acting action-hand C035");
}

static void test_source_evidence_and_hash(void)
{
    dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t model;
    const char *source =
        dm1_v1_champion_panel_second_leader_hand_slot_priority_source_pc34();

    (void)dm1_v1_champion_panel_second_leader_hand_slot_priority_build_pc34(
        &model);

    expect_contains("source.f0302", source, "F0302:677-684",
                    "CHAMPION.C F0302 status hand route");
    expect_contains("source.f0297", source, "F0297/F0298:243-298",
                    "CHAMPION.C leader hand side effects");
    expect_contains("source.f0300", source, "F0300:511-515",
                    "CHAMPION.C C30/G0425 clear");
    expect_contains("source.f0301", source, "F0301:606-614",
                    "CHAMPION.C slot write");
    expect_contains("source.f0287", source, "F0287:307-342",
                    "CHAMDRAW.C bar fill/blank");
    expect_contains("source.f0291", source, "F0291:632-651",
                    "CHAMDRAW.C hand-slot graphics");
    expect_contains("source.f0292.status", source, "F0292:771-815",
                    "CHAMDRAW.C status-box fill");
    expect_contains("source.f0292.name", source, "F0292:843-895",
                    "CHAMDRAW.C name-color cascade");
    expect_contains("source.f0292.icon", source, "F0292:1019-1051",
                    "CHAMDRAW.C champion icon");
    expect_contains("source.defs", source, "DEFS.H:780-817",
                    "DEFS.H slot constants");
    expect_contains("source.zones", source, "3779-3807",
                    "DEFS.H zone constants");
    expect_u32("deterministic.hash", model.deterministic_hash, 0xB540AE26u,
               "pass764 deterministic contract hash");
}

int main(void)
{
    dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t model;

    test_second_leader_priority_and_redraw();
    test_second_leader_hud_cascade();
    test_source_evidence_and_hash();

    (void)dm1_v1_champion_panel_second_leader_hand_slot_priority_build_pc34(
        &model);
    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    printf("DeterministicHash: 0x%08X\n",
           (unsigned)model.deterministic_hash);
    if (g_assertions < 60) {
        printf("FAIL assertion floor got=%d want>=60\n", g_assertions);
        return 1;
    }
    if (g_failures) {
        return 1;
    }
    printf("DM1_V1_CHAMPION_PANEL_SECOND_LEADER_HAND_SLOT_PRIORITY_PC34_COMPAT_OK\n");
    return 0;
}
