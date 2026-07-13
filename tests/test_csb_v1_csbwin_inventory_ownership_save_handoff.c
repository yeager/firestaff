/* CSBWin CHARDESC inventory/cursor ownership save regression.
 * Source: CSBWin SaveGame.cpp:1023-1032,1802-1808; CSBCode.cpp:6830-6865. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void make_verified_summary(CSB_V1_CSBWin512BodyReport *summary)
{
    memset(summary, 0, sizeof(*summary));
    summary->header_valid = 1;
    summary->sections_verified = CSB_V1_CSBWIN_512_SECTION_COUNT;
    summary->num_character = 2u;
    summary->party_facing = 1u;
    summary->hand_char = 1u;
    summary->magic_caster = 0u;
    summary->object_in_hand = 0x1401u;
    summary->champions[0].valid = 1;
    summary->champions[0].hp = 100;
    summary->champions[0].max_hp = 100;
    summary->champions[0].possessions[0] = 0x1802u;
    summary->champions[1].valid = 1;
    summary->champions[1].hp = 90;
    summary->champions[1].max_hp = 100;
    summary->champions[1].possessions[1] = 0x1c03u;
}

int main(void)
{
    CSB_V1_CSBWin512BodyReport summary;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeProfile snapshot;

    make_verified_summary(&summary);
    csb_v1_runtime_init(&profile, NULL);
    check(csb_v1_runtime_apply_csbwin_resume_report(&profile, &summary) == 0 &&
              profile.party_state_valid &&
              profile.party_state.LeaderIndex == 1 &&
              profile.party_state.LeaderHandThing == 0x1401u &&
              profile.party_state.Champions[0].Slots[0] == 0x1802u &&
              profile.party_state.Champions[1].Slots[1] == 0x1c03u,
          "verified GAMEBLOCK2 cursor and CHARDESC inventory import together");

    make_verified_summary(&summary);
    summary.champions[0].possessions[2] = THING_ENDOFLIST;
    snapshot = profile;
    check(csb_v1_runtime_apply_csbwin_resume_report(&profile, &summary) == -1 &&
              memcmp(&profile, &snapshot, sizeof(profile)) == 0,
          "end-of-list sentinel in a saved inventory slot rejects atomically");

    make_verified_summary(&summary);
    summary.hand_char = summary.num_character;
    snapshot = profile;
    check(csb_v1_runtime_apply_csbwin_resume_report(&profile, &summary) == -1 &&
              memcmp(&profile, &snapshot, sizeof(profile)) == 0,
          "held cursor RN without a saved hand owner rejects atomically");

    return failures == 0 ? 0 : 1;
}
