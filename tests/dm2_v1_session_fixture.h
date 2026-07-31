#ifndef FIRESTAFF_TEST_DM2_V1_SESSION_FIXTURE_H
#define FIRESTAFF_TEST_DM2_V1_SESSION_FIXTURE_H

/* Test-only session data. This is deliberately outside src/: production DM2
 * starts only through original GAME_LOAD/LOAD_NEW_DUNGEON ownership. */

#include "dm2_v1_new_game.h"

#include <string.h>

static inline void dm2_v1_test_session_fixture_new(DM2_V1_SessionState *session)
{
    static const char *const names[4] = {
        "Fixture fighter", "Fixture ninja", "Fixture priest", "Fixture wizard"
    };
    static const uint8_t portraits[4] = { 0u, 3u, 4u, 7u };
    static const uint16_t hit_points[4] = { 60u, 45u, 40u, 30u };

    if (!session) return;
    memset(session, 0, sizeof(*session));
    session->rng_seed = 257u;
    session->party_x = 15u;
    session->party_y = 15u;
    session->party_dir = 0u;
    session->party_level = 0u;
    session->outdoor_mode = 0u;
    session->gold = 100u;
    session->time_of_day_minutes = 720u;
    session->champion_count = 4u;
    session->leader_index = 0u;
    for (unsigned int i = 0u; i < 4u; ++i) {
        DM2_ChampionRecord *record =
            (DM2_ChampionRecord *)session->champion_data[i];
        strncpy(record->first_name, names[i],
                DM2_CHAMPION_NAME_FIRST_LEN - 1u);
        record->portrait_index = portraits[i];
        record->squad_position = (uint8_t)i;
        record->cur_hp = hit_points[i];
        record->max_hp = hit_points[i];
    }
}

#endif
