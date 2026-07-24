#include "csb_v1_utility_flow_pc34_compat.h"
#include "csb_v1_utility_import_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>

static int failures;
static int legacy_import_calls;

/* The confirmation branch does not call these integrations.  Stubs keep this
 * focused state-machine test independent of the concurrently changing runtime
 * link graph. */
int csb_v1_character_import_dm1_save(CSB_V1_PartyState *party,
                                     const char *path)
{
    (void)party;
    (void)path;
    ++legacy_import_calls;
    return -1;
}

void csb_v1_character_init_default(CSB_V1_PartyState *party)
{
    if (party) {
        *party = (CSB_V1_PartyState){0};
    }
}

int csb_v1_import_from_dm1_save_file(CSB_V1_PartyState *party,
                                      const char *path,
                                      CSB_V1_ImportResult *result)
{
    (void)party;
    (void)path;
    (void)result;
    return -1;
}

void csb_v1_runtime_init(CSB_V1_RuntimeProfile *profile, const char *data_dir)
{
    (void)profile;
    (void)data_dir;
}

void csb_v1_runtime_cleanup(CSB_V1_RuntimeProfile *profile)
{
    (void)profile;
}

int csb_v1_runtime_get_party_state(const CSB_V1_RuntimeProfile *profile,
                                   CSB_V1_PartyState *party)
{
    (void)profile;
    (void)party;
    return -1;
}

int csb_v1_runtime_load_game_from_path(CSB_V1_RuntimeProfile *profile,
                                       const char *path)
{
    (void)profile;
    (void)path;
    return -1;
}

int csb_v1_util_check_disk(const char *path)
{
    (void)path;
    return -1;
}

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    CSB_V1_UtilFlowContext flow;

    csb_v1_util_flow_init(&flow);
    flow.imported_party.ChampionCount = 1;
    snprintf(flow.imported_party.Champions[0].Name,
             sizeof(flow.imported_party.Champions[0].Name), "%s", "KEEP");
    flow.imported_champion_count = 1;
    flow.pending_import_party.ChampionCount = 1;
    snprintf(flow.pending_import_party.Champions[0].Name,
             sizeof(flow.pending_import_party.Champions[0].Name), "%s", "NEW");
    flow.pending_import_champion_count = 1;
    flow.pending_import_active = 1;
    flow.state = CSB_V1_UTIL_FLOW_CONFIRM_IMPORT;
    flow.import_confirmed = -1;
    check(csb_v1_util_flow_step(&flow) == 0 &&
              flow.state == CSB_V1_UTIL_FLOW_CONFIRM_IMPORT &&
              flow.imported_party.ChampionCount == 1 &&
              flow.imported_party.Champions[0].Name[0] == 'K',
          "pending confirmation retains preview without replacing party");

    csb_v1_util_flow_confirm_import(&flow, 0);
    check(csb_v1_util_flow_step(&flow) == 0 &&
              flow.state == CSB_V1_UTIL_FLOW_SELECT_ACTION &&
              flow.action == CSB_V1_UTIL_ACTION_EXIT &&
              flow.import_confirmed == 0 &&
              !flow.pending_import_active &&
              flow.imported_party.Champions[0].Name[0] == 'K',
          "explicit rejection rolls candidate back to the utility menu");

    flow.pending_import_party.ChampionCount = 1;
    snprintf(flow.pending_import_party.Champions[0].Name,
             sizeof(flow.pending_import_party.Champions[0].Name), "%s", "NEW");
    flow.pending_import_champion_count = 1;
    flow.pending_import_active = 1;
    flow.state = CSB_V1_UTIL_FLOW_CONFIRM_IMPORT;
    flow.import_confirmed = -1;
    csb_v1_util_flow_confirm_import(&flow, 1);
    check(csb_v1_util_flow_step(&flow) == 0 &&
              flow.state == CSB_V1_UTIL_FLOW_NEW_GAME &&
              flow.import_confirmed == 0 &&
              !flow.pending_import_active &&
              flow.imported_party.Champions[0].Name[0] == 'N',
          "explicit acceptance commits the staged party after confirmation");

    csb_v1_util_flow_init(&flow);
    flow.state = CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS;
    csb_v1_util_flow_set_dm1_path(&flow, "/rejected/DM1.SAV");
    legacy_import_calls = 0;
    check(csb_v1_util_flow_step(&flow) == 0 &&
              flow.state == CSB_V1_UTIL_FLOW_ERROR &&
              flow.last_error == -3,
          "rejected source save stops before the import preview");
    check(legacy_import_calls == 0,
          "rejected source save never falls back to the legacy importer");

    return failures ? 1 : 0;
}
