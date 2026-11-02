#ifndef DM2_V1_ACTION_HAND_POINTER_REAL_MEDIA_CHECK_H
#define DM2_V1_ACTION_HAND_POINTER_REAL_MEDIA_CHECK_H

#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"
#include "m11_game_view.h"

#include <stdio.h>

typedef struct {
    int position;
    int hand;
    int hero;
} DM2_V1_ActionHandPointerCandidate;

/* Exercise the same RAW4-expanded hand rectangles used by the active M11
 * pointer path. Candidate actions and expected champions come from the live
 * original-media party; no test hero, item, or rectangle is synthesized. */
static int dm2_v1_test_action_hand_pointer_real_media(
    M11_GameViewState *view, const char *platform_name)
{
    DM2_V1_ActionHandPointerCandidate candidates[8];
    const DM2_V1_BootProfile *profile;
    int candidate_count = 0;
    int hand_mask = 0;
    int party_dir;

    if (!view || !view->dm2BootProfile || !platform_name) return 0;
    profile = (const DM2_V1_BootProfile *)view->dm2BootProfile;
    party_dir = dm2_v1_runtime_get_party_dir();
    if (party_dir < 0 || party_dir > 3) {
        fprintf(stderr, "FAIL: %s live party direction is invalid\n",
                platform_name);
        return 0;
    }

    for (int position = 0; position < 4; ++position) {
        const int party_position = (party_dir + position) & 3;
        int hero_index = -1;
        for (int hero = 0; hero < 4; ++hero) {
            DM2_V1_RuntimeSourceHeroStateReceipt source_hero;
            if (dm2_v1_runtime_get_source_hero_state(
                    (uint8_t)hero, &source_hero) && source_hero.cur_hp > 0 &&
                source_hero.party_position == party_position) {
                hero_index = hero;
                break;
            }
        }
        if (hero_index < 0) continue;

        for (int hand = 0; hand < 2; ++hand) {
            const uint16_t rect_id = (uint16_t)(
                (hand == 1 ? 0x70u : 0x74u) + (unsigned int)position);
            DM2_V1_BootExpandedRectReceipt rect;
            if (!dm2_v1_boot_query_expanded_rect_receipt(
                    profile, rect_id, &rect) || !rect.valid ||
                rect.rect.w <= 0 || rect.rect.h <= 0) {
                fprintf(stderr,
                        "FAIL: %s authentic RAW4 action rectangle 0x%02x is unavailable\n",
                        platform_name, (unsigned int)rect_id);
                return 0;
            }

            /* Probe eligibility through the source runtime, then restore a
             * different live action before clicking the corresponding M11
             * rectangle. This proves the pointer route changes the owner. */
            if (!dm2_v1_runtime_activate_action_hand_at_position(
                    position, hand))
                continue;
            if (dm2_v1_runtime_get_active_champion_index() != hero_index ||
                dm2_v1_runtime_get_active_hand() != hand) {
                fprintf(stderr,
                        "FAIL: %s source hand preflight mapped slot %d hand %d to the wrong champion\n",
                        platform_name, position, hand);
                return 0;
            }
            candidates[candidate_count++] = (DM2_V1_ActionHandPointerCandidate){
                position, hand, hero_index
            };
            hand_mask |= 1 << hand;
        }
    }

    if (candidate_count == 0 || hand_mask != 3) {
        fprintf(stderr,
                "FAIL: %s authentic party does not expose actionable items in both hands (mask=%d candidates=%d)\n",
                platform_name, hand_mask, candidate_count);
        return 0;
    }

    for (int i = 0; i < candidate_count; ++i) {
        const DM2_V1_ActionHandPointerCandidate target = candidates[i];
        DM2_V1_BootExpandedRectReceipt rect;
        int baseline_found = 0;
        int click_result;
        if (!dm2_v1_boot_query_expanded_rect_receipt(
                profile, (uint16_t)((target.hand == 1 ? 0x70u : 0x74u) +
                                    (unsigned int)target.position), &rect))
            return 0;
        for (int j = 0; j < candidate_count; ++j) {
            const DM2_V1_ActionHandPointerCandidate baseline = candidates[j];
            if (baseline.hero == target.hero && baseline.hand == target.hand)
                continue;
            if (dm2_v1_runtime_activate_action_hand_at_position(
                    baseline.position, baseline.hand) &&
                dm2_v1_runtime_get_active_champion_index() == baseline.hero &&
                dm2_v1_runtime_get_active_hand() == baseline.hand) {
                baseline_found = 1;
                break;
            }
        }
        if (!baseline_found) {
            fprintf(stderr,
                    "FAIL: %s could not establish an alternate source action before pointer dispatch\n",
                    platform_name);
            return 0;
        }

        click_result = M11_GameView_HandlePointerButton(
            view, rect.rect.x + rect.rect.w / 2,
            rect.rect.y + rect.rect.h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
        if (click_result != M11_GAME_INPUT_REDRAW ||
            dm2_v1_runtime_get_active_champion_index() != target.hero ||
            dm2_v1_runtime_get_active_hand() != target.hand) {
            fprintf(stderr,
                    "FAIL: %s click on RAW4 0x%02x did not select source champion %d hand %d (result=%d active=%d/%d)\n",
                    platform_name,
                    (unsigned int)((target.hand == 1 ? 0x70u : 0x74u) +
                                   (unsigned int)target.position),
                    target.hero, target.hand, click_result,
                    dm2_v1_runtime_get_active_champion_index(),
                    dm2_v1_runtime_get_active_hand());
            return 0;
        }
    }

    printf("PASS: %s authentic RAW4 hand zones route every actionable champion hand through M11\n",
           platform_name);
    return 1;
}

#endif
