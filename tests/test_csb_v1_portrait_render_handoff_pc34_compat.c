/*
 * test_csb_v1_portrait_render_handoff_pc34_compat.c
 *
 * Data-free CSB V1 runtime handoff gate for Utility Disk champion portraits.
 * It verifies the narrow chain:
 *
 *   synthetic .CMP -> csb_v1_cmp_import_to_party()
 *   -> csb_v1_runtime_set_party_state()
 *   -> csb_v1_runtime_select_champion_portrait_render_source()
 *
 * Source lock:
 *   - ReDMCSB DEFS.H CMP typedef: 496-byte Utility Disk portrait record.
 *   - ReDMCSB PANEL.C F0354 lines 2195-2239: status-box portrait draws from
 *     M516_CHAMPIONS[ChampionIndex].Portrait.
 *   - ReDMCSB CHAMDRAW.C F0292 lines 731-940: status-box refresh routes
 *     living champions through F0354 before drawing name/action/state.
 *
 * This is not a real Utility Disk asset test and does not claim pixel parity.
 */

#include "csb_v1_cmp_import_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failed = 0;
static int g_passed = 0;

#define CHECK_TRUE(name, expr) do { \
    if (expr) { \
        ++g_passed; \
    } else { \
        ++g_failed; \
        printf("FAIL %s\n", (name)); \
    } \
} while (0)

#define CHECK_INT(name, got, want) do { \
    int _got = (int)(got); \
    int _want = (int)(want); \
    if (_got == _want) { \
        ++g_passed; \
    } else { \
        ++g_failed; \
        printf("FAIL %s got=%d want=%d\n", (name), _got, _want); \
    } \
} while (0)

static void make_cmp(uint8_t cmp[FIRESTAFF_CMP_FILE_SIZE],
                     const char *name,
                     const char *title,
                     uint8_t seed)
{
    memset(cmp, 0, FIRESTAFF_CMP_FILE_SIZE);
    memcpy(cmp + 4, name, strlen(name));
    memcpy(cmp + 4 + FIRESTAFF_CMP_NAME_SIZE, title, strlen(title));
    for (int i = 0; i < 464; ++i) {
        cmp[4 + FIRESTAFF_CMP_NAME_SIZE + FIRESTAFF_CMP_TITLE_SIZE + i] =
            (uint8_t)(seed + i);
    }
}

static CSB_V1_PartyState make_imported_party(void)
{
    uint8_t cmp[FIRESTAFF_CMP_FILE_SIZE];
    CSB_V1_PartyState party;
    csb_v1_character_init_default(&party);

    make_cmp(cmp, "TIGGY", "APPRENTICE", 0x31);
    CHECK_INT("cmp import slot 0",
              csb_v1_cmp_import_to_party(&party, cmp, sizeof(cmp)), 0);

    make_cmp(cmp, "BORIS", "WIZARD", 0x91);
    CHECK_INT("cmp import slot 1",
              csb_v1_cmp_import_to_party(&party, cmp, sizeof(cmp)), 1);

    party.ImportedFromDM1 = 1;
    party.ImportSource = 1;
    party.LeaderIndex = 1;
    party.Champions[0].CurrentHealth = 44;
    party.Champions[0].MaximumHealth = 44;
    party.Champions[1].CurrentHealth = 55;
    party.Champions[1].MaximumHealth = 55;
    return party;
}

static void test_cmp_to_runtime_portrait_render_source(void)
{
    CSB_V1_PartyState party = make_imported_party();
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_ChampionPortraitRenderSource source;

    csb_v1_runtime_init(&runtime, NULL);
    CHECK_INT("runtime set party", csb_v1_runtime_set_party_state(&runtime, &party), 0);
    CHECK_INT("select portrait slot 1",
              csb_v1_runtime_select_champion_portrait_render_source(
                  &runtime, 1, &source), 0);

    CHECK_TRUE("source points at runtime champion portrait",
               source.portrait == runtime.party_state.Champions[1].Portrait);
    CHECK_INT("source byte count", source.portrait_byte_count,
              CSB_V1_PORTRAIT_BYTE_COUNT);
    CHECK_INT("source width", source.portrait_width, CSB_V1_PORTRAIT_WIDTH);
    CHECK_INT("source height", source.portrait_height, CSB_V1_PORTRAIT_HEIGHT);
    CHECK_INT("source byte width", source.portrait_byte_width,
              CSB_V1_PORTRAIT_BYTE_WIDTH);
    CHECK_INT("source champion index", source.champion_index, 1);
    CHECK_INT("source leader flag", source.is_leader, 1);
    CHECK_TRUE("source name", strcmp(source.name, "BORIS") == 0);
    CHECK_TRUE("source title", strcmp(source.title, "WIZARD") == 0);
    CHECK_INT("source first portrait byte", source.portrait[0], 0x91);
    CHECK_INT("source last cmp portrait byte", source.portrait[463],
              (uint8_t)(0x91 + 463));
    CHECK_INT("source padded portrait byte", source.portrait[464], 0);

    CHECK_INT("select portrait slot 0",
              csb_v1_runtime_select_champion_portrait_render_source(
                  &runtime, 0, &source), 0);
    CHECK_TRUE("slot 0 source points at runtime champion portrait",
               source.portrait == runtime.party_state.Champions[0].Portrait);
    CHECK_INT("slot 0 not leader", source.is_leader, 0);
    CHECK_TRUE("slot 0 name", strcmp(source.name, "TIGGY") == 0);
    CHECK_INT("slot 0 first portrait byte", source.portrait[0], 0x31);
}

static void test_invalid_portrait_sources_are_rejected(void)
{
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_ChampionPortraitRenderSource source;
    CSB_V1_PartyState party = make_imported_party();

    memset(&source, 0xA5, sizeof(source));
    csb_v1_runtime_init(&runtime, NULL);
    CHECK_INT("no party rejects portrait source",
              csb_v1_runtime_select_champion_portrait_render_source(
                  &runtime, 0, &source), -1);
    CHECK_TRUE("rejected no-party source pointer cleared", source.portrait == NULL);
    CHECK_INT("rejected no-party source index", source.champion_index, -1);

    CHECK_INT("runtime set party for invalid checks",
              csb_v1_runtime_set_party_state(&runtime, &party), 0);
    CHECK_INT("negative source index rejected",
              csb_v1_runtime_select_champion_portrait_render_source(
                  &runtime, -1, &source), -1);
    CHECK_INT("past champion count rejected",
              csb_v1_runtime_select_champion_portrait_render_source(
                  &runtime, 2, &source), -1);
    CHECK_INT("past max champion count rejected",
              csb_v1_runtime_select_champion_portrait_render_source(
                  &runtime, CSB_V1_MAX_CHAMPIONS, &source), -1);
    CHECK_INT("null source rejected",
              csb_v1_runtime_select_champion_portrait_render_source(
                  &runtime, 0, NULL), -1);
    CHECK_INT("null runtime rejected",
              csb_v1_runtime_select_champion_portrait_render_source(
                  NULL, 0, &source), -1);
}

int main(void)
{
    test_cmp_to_runtime_portrait_render_source();
    test_invalid_portrait_sources_are_rejected();

    printf("[csb_v1_portrait_render_handoff_pc34_compat] %d passed, %d failed\n",
           g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
