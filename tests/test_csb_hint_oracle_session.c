/* Source-state contract for ReDMCSB HINTHINT.C.  Synthetic HTC bytes are
 * deliberately used only to exercise the parser/session boundary. */
#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_atari_save_session.h"
#include "csb_hint_oracle_session.h"

#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "check failed %s:%d: %s\n", __FILE__, __LINE__, #x); \
    return 0; } } while (0)

static void be16(uint8_t *p, uint16_t v)
{ p[0] = (uint8_t)(v >> 8u); p[1] = (uint8_t)v; }

static size_t fixture(uint8_t *b, size_t cap)
{
    size_t off = 0u, i;
    CHECK(cap >= 300u);
    be16(b + off, 2u); off += 2u;
    be16(b + off, 13u); off += 2u;
    be16(b + off, 3u); off += 2u;
    be16(b + off, 8u); off += 2u;
    be16(b + off, 6u); off += 2u;
    for (i = 0u; i < 8u; ++i) {
        b[off++] = i == 1u ? 255u : 9u;
        b[off++] = i == 1u ? 255u : 7u;
        b[off++] = 4u; b[off++] = 0u; be16(b + off, (uint16_t)i); off += 2u;
    }
    be16(b + off, 8u); off += 2u;
    be16(b + off, 26u); off += 2u;
    for (i = 0u; i < 8u; ++i) {
        memset(b + off, 0, 26u);
        b[off] = (uint8_t)('A' + i);
        be16(b + off + 22u, (uint16_t)(i == 0u ? 0u : i + 1u));
        be16(b + off + 24u, (uint16_t)(i == 0u ? 2u : 1u));
        off += 26u;
    }
    be16(b + off, 9u); off += 2u;
    for (i = 0u; i < 9u; ++i) { be16(b + off, 1u); off += 2u; }
    memset(b + off, 0, 9u); off += 9u;
    return off;
}

static int test_source_seven_row_cap_and_page_state(void)
{
    uint8_t raw[320];
    CSB_HintOracleHTC htc;
    CSB_HintOracleSession session;
    CSB_HintOracleHTC_Hint hint;
    size_t i;
    CHECK(csb_hint_oracle_htc_parse(raw, fixture(raw, sizeof(raw)), &htc) == 0);
    csb_hint_oracle_session_init(&session);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_AWAIT_LOAD);
    CHECK(csb_hint_oracle_session_select_location(&session, &htc, 4u, 9u, 7u) == 0);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_HINT_LIST);
    CHECK(session.selected_hint_count == 7u);
    for (i = 0u; i < 7u; ++i) CHECK(session.selected_hint_indices[i] == i);
    CHECK(csb_hint_oracle_session_open_hint_row(&session, 7u) ==
          CSB_HINT_ORACLE_SESSION_ERR_ROW);
    CHECK(csb_hint_oracle_session_open_hint_row(&session, 0u) == 0);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_HINT_PAGE);
    CHECK(session.page_number == 1u);
    CHECK(csb_hint_oracle_session_current_hint(&session, &hint) != NULL);
    CHECK(strcmp(hint.name, "A") == 0 && hint.page_count == 2u);
    CHECK(csb_hint_oracle_session_previous_page(&session) ==
          CSB_HINT_ORACLE_SESSION_ERR_PAGE_BOUNDARY);
    CHECK(csb_hint_oracle_session_next_page(&session) == 0 && session.page_number == 2u);
    CHECK(csb_hint_oracle_session_next_page(&session) ==
          CSB_HINT_ORACLE_SESSION_ERR_PAGE_BOUNDARY);
    CHECK(csb_hint_oracle_session_done(&session) == 0);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_HINT_LIST && session.page_number == 0u);
    CHECK(csb_hint_oracle_session_done(&session) == 0);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_AWAIT_LOAD && session.htc == NULL);
    return 1;
}

static int test_no_clue_and_close(void)
{
    uint8_t raw[320]; CSB_HintOracleHTC htc; CSB_HintOracleSession session;
    CHECK(csb_hint_oracle_htc_parse(raw, fixture(raw, sizeof(raw)), &htc) == 0);
    CHECK(csb_hint_oracle_session_select_location(&session, &htc, 3u, 9u, 7u) == 0);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_NO_CLUE);
    CHECK(csb_hint_oracle_session_open_hint_row(&session, 0u) ==
          CSB_HINT_ORACLE_SESSION_ERR_STATE);
    CHECK(csb_hint_oracle_session_done(&session) == 0);
    csb_hint_oracle_session_close(&session);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_CLOSED);
    return 1;
}

static int test_authenticated_atari_receipt_adapter(void)
{
    uint8_t raw[320];
    CSB_HintOracleHTC htc;
    CSB_HintOracleSession session;
    CSB_V1_AtariSaveInfo info;
    memset(&info, 0, sizeof(info));
    CHECK(csb_hint_oracle_htc_parse(raw, fixture(raw, sizeof(raw)), &htc) == 0);
    info.party_map_index = 4;
    info.party_x = 9;
    info.party_y = 7;
    CHECK(csb_hint_oracle_atari_save_session_select(&session, &htc, &info) == 0);
    CHECK(session.state == CSB_HINT_ORACLE_SESSION_HINT_LIST);
    CHECK(session.selected_hint_count == 7u);
    info.party_x = -1;
    CHECK(csb_hint_oracle_atari_save_session_select(&session, &htc, &info) ==
          CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_POSE);
    return 1;
}

int main(void)
{
    int ok = test_source_seven_row_cap_and_page_state() && test_no_clue_and_close() &&
             test_authenticated_atari_receipt_adapter();
    puts(ok ? "csb_hint_oracle_session: PASS" : "csb_hint_oracle_session: FAIL");
    return ok ? 0 : 1;
}
