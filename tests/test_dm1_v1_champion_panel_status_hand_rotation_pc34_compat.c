#include "firestaff/dm1/v1/champion/dm1_v1_champion_panel_status_hand_rotation_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

void F0658_BlitBitmapIndexToZoneIndexWithTransparency(int16_t bitmapIndex, int16_t zoneIndex, int16_t transparentColor)
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

static void expect_u16(const char *id, uint16_t got, uint16_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04X want=0x%04X at %s\n", id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    }
}

static void expect_u32(const char *id, uint32_t got, uint32_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%08X want=0x%08X at %s\n", id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_contains(const char *id, const char *haystack, const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    }
}

int main(void)
{
    dm1_v1_champion_panel_status_hand_rotation_model_t model;
    const char *source;
    const int expected_graphics[3] = { DM1_GFX_SLOT_NORMAL, DM1_GFX_SLOT_WOUNDED, DM1_GFX_SLOT_ACTING };
    int i;

    expect_bool("build", dm1_v1_champion_panel_status_hand_rotation_build_pc34(&model), true, "contract model");
    source = dm1_v1_champion_panel_status_hand_rotation_source_pc34();
    expect_bool("contract.only", model.contract_only, true, "contract only");
    expect_bool("contract.disjoint.priority", model.disjoint_from_slot_priority_gate, true, "not slot priority");
    expect_bool("contract.disjoint.second", model.disjoint_from_second_leader_gate, true, "not pass764");
    expect_int("chain.count", model.source_chain_count, 7, "source chain");
    expect_contains("source.f0297", source, "F0297:243-298", "CHAMPION.C");
    expect_contains("source.f0298", source, "F0298:270-298", "CHAMPION.C");
    expect_contains("source.f0300", source, "F0300:511-515", "CHAMPION.C");
    expect_contains("source.f0301", source, "F0301:606-614", "CHAMPION.C");
    expect_contains("source.f0302", source, "F0302:662-714", "CHAMPION.C");
    expect_contains("source.f0291", source, "F0291:632-673", "CHAMDRAW.C");
    expect_contains("source.f0292", source, "F0292:771-895", "CHAMDRAW.C");
    expect_contains("source.defs", source, "DEFS.H:2088 C30/C033/C034/C035/M070/M516", "DEFS.H");

    for (i = 0; i < DM1_V1_CP_STATUS_HAND_ROTATION_CHAMPION_COUNT_PC34; ++i) {
        const dm1_v1_champion_panel_status_hand_rotation_frame_t *f = &model.frames[i];
        int base_x = i * DM1_STATUS_BOX_SPACING;
        int slot_box = (i * 2) + DM1_SLOT_ACTION_HAND;
        expect_int("frame.before", f->leader_before, (i + 3) & 3, "leader switch");
        expect_int("frame.after", f->leader_after, i, "new leader");
        expect_int("frame.m516.index", f->m516_champion_index, i, "M516");
        expect_int("frame.m516.zone", f->m516_status_hand_zone_pointer, 211 + slot_box, "M516 zone");
        expect_int("frame.slotbox", f->slot_box_index, slot_box, "F0302 slotbox");
        expect_int("frame.hand", f->hand_slot_index, DM1_SLOT_ACTION_HAND, "M070");
        expect_int("frame.status.zone", f->status_box_zone, 151 + i, "F0292 status");
        expect_int("frame.name.zone", f->status_name_zone, 159 + i, "F0292 name");
        expect_int("frame.text.zone", f->status_text_zone, 163 + i, "F0292 text");
        expect_int("frame.action.zone", f->action_hand_zone, 212 + (i * 2), "F0291 zone");
        expect_int("frame.action.x", f->action_hand_x, base_x + 24, "F0291 x");
        expect_int("frame.action.y", f->action_hand_y, 10, "F0291 y");
        expect_int("frame.blit.right", f->f0291_blit_right, base_x + 41, "18px span");
        expect_int("frame.text.dest", f->f0292_text_zone, f->status_text_zone, "same leader text");
        expect_int("frame.border37", f->border_graphic_shield, 37, "C037");
        expect_int("frame.border38", f->border_graphic_fire_shield, 38, "C038");
        expect_int("frame.border39", f->border_graphic_spell_shield, 39, "C039");
        expect_u16("frame.leader.after", f->leader_hand_after, f->slot_thing_before, "F0300/F0297");
        expect_u16("frame.slot.after", f->slot_thing_after, f->leader_hand_before, "F0301");
        expect_bool("frame.accepted", f->transaction_accepted, true, "F0302");
        expect_bool("frame.status.route", f->transaction_status_route, true, "F0302 status route");
        expect_bool("frame.newleader", f->transaction_uses_new_leader_m516, true, "new leader M516");
        expect_bool("frame.c30.g0425", f->transaction_uses_c30_g0425_when_chest_slot, true, "C30/G0425");
        expect_bool("frame.g0426", f->transaction_preserves_g0426_open_chest_marker, true, "G0426");
        expect_bool("frame.order1", f->f0302_resolved_before_f0291, true, "F0302 before F0291");
        expect_bool("frame.order2", f->f0291_resolved_before_f0292, true, "F0291 before F0292");
        expect_bool("frame.same", f->f0292_destination_matches_f0291_frame, true, "same frame");
        expect_u32("frame.out.left", f->outside_left_after, f->outside_left_before, "outside span");
        expect_u32("frame.out.right", f->outside_right_after, f->outside_right_before, "outside span");
    }

    for (i = 0; i < DM1_V1_CP_STATUS_HAND_ROTATION_ICON_STATE_COUNT_PC34; ++i) {
        const dm1_v1_champion_panel_status_hand_rotation_icon_t *ic = &model.icons[i];
        expect_int("icon.leader", ic->leader_index, 2, "new leader hand");
        expect_u16("icon.thing", ic->actual_hand_thing, (uint16_t)(0x6200u + (uint16_t)i), "actual hand");
        expect_int("icon.graphic", ic->selected_graphic, expected_graphics[i], "C033/C034/C035");
        expect_int("icon.zone", ic->selected_zone, 216, "C216");
        expect_bool("icon.actual", ic->selected_from_actual_new_leader_hand, true, "actual source");
    }
    expect_u16("icon.normal.wounds", model.icons[0].wounds, 0u, "C033");
    expect_u16("icon.wounded.wounds", model.icons[1].wounds, (uint16_t)(1u << DM1_SLOT_ACTION_HAND), "C034");
    expect_int("icon.acting.ordinal", model.icons[2].acting_ordinal, 3, "C035");
    expect_int("close.leader", model.close_path.leader_index, 2, "close leader");
    expect_int("close.zone.before", model.close_path.action_hand_zone_before, 216, "rotated before");
    expect_int("close.zone.after", model.close_path.action_hand_zone_after, -1, "rotated cleared");
    expect_u16("close.g0426.after", model.close_path.g0426_open_chest_after, DM1_V1_CP_STATUS_HAND_ROTATION_NONE_THING_PC34, "G0426 clear");
    expect_u16("close.g0425.after", model.close_path.g0425_chest_slot_after, DM1_V1_CP_STATUS_HAND_ROTATION_NONE_THING_PC34, "G0425 clear");
    expect_bool("close.rot.before", model.close_path.rotated_state_before, true, "before");
    expect_bool("close.rot.after", model.close_path.rotated_state_after, false, "after");
    expect_bool("close.f0300", model.close_path.f0300_clears_c30_or_m516_slot_before_close, true, "F0300");
    expect_bool("close.f0301", model.close_path.f0301_skipped_for_empty_close, true, "F0301");
    expect_bool("close.f0292", model.close_path.f0292_close_redraw_after_clear, true, "F0292");
    expect_u32("close.out.left", model.close_path.outside_left_after, model.close_path.outside_left_before, "outside");
    expect_u32("close.out.right", model.close_path.outside_right_after, model.close_path.outside_right_before, "outside");
    expect_u32("deterministic.hash", model.deterministic_hash, 0xC57063A4u, "pass765 hash");
    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    printf("DeterministicHash: 0x%08X\n", (unsigned)model.deterministic_hash);
    if (g_assertions < 80) return 1;
    if (g_failures) return 1;
    printf("DM1_V1_CHAMPION_PANEL_STATUS_HAND_ROTATION_PC34_COMPAT_OK\n");
    return 0;
}
