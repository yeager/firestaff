/* CSBWin SaveGame.cpp original-save provenance and DB11 admission test. */
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_csbwin_dungeon_tail.h"
#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"
#include "csbwin_resume_fixture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(expr, name) do { \
    if (expr) { ++passed; printf("PASS: %s\\n", name); } \
    else { ++failed; printf("FAIL: %s\\n", name); } \
} while (0)

static int append_invalid_expool_tail(const char *path)
{
    unsigned char tail[256] = { 0 };
    FILE *fp;

    /* First DB11 bucket points beyond this 64-word EXPOOL page. */
    tail[32u * 4u] = 64u;
    fp = fopen(path, "ab");
    if (!fp) return 0;
    if (fwrite(tail, 1u, sizeof(tail), fp) != sizeof(tail)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int files_equal(const char *left_path, const char *right_path)
{
    FILE *left = fopen(left_path, "rb");
    FILE *right = fopen(right_path, "rb");
    int equal = 1;
    int left_byte;
    int right_byte;

    if (!left || !right) {
        if (left) fclose(left);
        if (right) fclose(right);
        return 0;
    }
    do {
        left_byte = fgetc(left);
        right_byte = fgetc(right);
        if (left_byte != right_byte) equal = 0;
    } while (equal && left_byte != EOF && right_byte != EOF);
    fclose(left);
    fclose(right);
    return equal;
}

static void test_staged_real_csbwin_save(void)
{
    const char *path = getenv("FIRESTAFF_CSBWIN_REAL_SAVE");
    FILE *file;
    long size_long = 0L;
    size_t size;
    uint8_t *bytes;
    CSB_V1_CSBWin512BodyReport body;
    CSB_V1_CSBWinDungeonTailPrefix tail_prefix;
    CSB_V1_CSBWinDungeonTailDatabaseLayout tail_databases;
    CSB_V1_CSBWinLegacyDungeonCandidate *tail_candidate = NULL;
    CSB_V1_CSBWinLegacyResumePrepare tail_resume;
    CSB_V1_CSBWinSaveDiscoveryResult discovery;
    CSB_V1_RuntimeProfile runtime;

    if (!path || path[0] == '\0') {
        printf("SKIP: FIRESTAFF_CSBWIN_REAL_SAVE is not staged\n");
        return;
    }
    file = fopen(path, "rb");
    CHECK(file != NULL, "opens staged CSBWin save corpus");
    if (!file) return;
    CHECK(fseek(file, 0L, SEEK_END) == 0 && (size_long = ftell(file)) > 0L &&
          fseek(file, 0L, SEEK_SET) == 0,
          "measures staged CSBWin save corpus");
    if (size_long <= 0L) {
        fclose(file);
        return;
    }
    size = (size_t)size_long;
    bytes = (uint8_t *)malloc(size);
    CHECK(bytes != NULL, "allocates staged CSBWin save corpus buffer");
    if (!bytes) {
        fclose(file);
        return;
    }
    CHECK(fread(bytes, 1u, size, file) == size,
          "reads staged CSBWin save corpus");
    fclose(file);
    memset(&body, 0, sizeof(body));
    CHECK(csb_v1_csbwin_512_verify_save_body(bytes, size, 10u, &body) ==
              CSB_V1_CSBWIN_512_OK &&
          body.header_valid && body.header.verdict == CSB_V1_CSBWIN_512_VERDICT_CSB &&
          body.header.byte_order == CSB_V1_CSBWIN_512_BYTE_ORDER_BIG_ENDIAN &&
          body.num_character == 2u && body.max_timers == 436u &&
          body.timer_record_size == 10u && body.appended_size == 32655u,
          "legacy CSBGAME2 body authenticates with original 10-byte timers");
    memset(&discovery, 0, sizeof(discovery));
    (void)csb_v1_csbwin_save_loader_boundary_classify(path, bytes, size,
                                                       &discovery);
    CHECK(discovery.xor512_valid && discovery.xor512_body_valid &&
          discovery.xor512_body_report.timer_record_size == 10u &&
          strcmp(discovery.decision_label,
                 "accept_csbwin_512_runtime_handoff_ready") == 0,
          "discovery admits authentic legacy CSBGAME2 body at its 10-byte timer width");

    /* ReDMCSB has no CSBWin GAMEBLOCK1 wrapper, but CSBWin's write/read
     * symmetry is explicit: SaveGame.cpp:1239-1335 writes this exact raw
     * dungeon sequence and SaveGame.cpp:2512-2841 reads it through
     * ReadDatabases().  The verified body gives the only safe tail boundary;
     * the parser must consume the whole legacy stream and its terminal
     * WriteAndChecksum word.  This remains read-only evidence: no decoded
     * database record enters RuntimeProfile until a world-owner handoff is
     * source-locked separately. */
    memset(&tail_prefix, 0, sizeof(tail_prefix));
    memset(&tail_databases, 0, sizeof(tail_databases));
    CHECK(body.appended_offset + body.appended_size == size &&
          body.appended_truncated &&
          body.appended_preserved_size ==
              CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES &&
          csb_v1_csbwin_dungeon_tail_parse_prefix(
              bytes + body.appended_offset, body.appended_size, 0u,
              &tail_prefix) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK &&
          tail_prefix.valid && tail_prefix.level_count == 11u &&
          tail_prefix.text_word_count == 333u &&
          tail_prefix.object_list_length == 1961u &&
          tail_prefix.legacy_cell_flag_bytes == 8763u,
          "legacy CSBGAME2 tail prefix is source-sized from verified body boundary");
    CHECK(csb_v1_csbwin_dungeon_tail_parse_databases(
              bytes + body.appended_offset, body.appended_size,
              &tail_prefix, CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION, 0u, 0u,
              &tail_databases) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK &&
          tail_databases.valid && tail_databases.database_bytes == 18490u &&
          tail_databases.cell_flag_bytes == 8763u &&
          tail_databases.checksum_offset + 2u == body.appended_size &&
          tail_databases.computed_checksum == tail_databases.stored_checksum,
          "legacy CSBGAME2 tail DB0-DB15 spans and checksum authenticate");
    memset(&tail_resume, 0, sizeof(tail_resume));
    CHECK(csb_v1_csbwin_dungeon_tail_prepare_legacy_candidate(
              bytes + body.appended_offset, body.appended_size,
              &tail_candidate) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK &&
          csb_v1_csbwin_dungeon_tail_prepare_legacy_resume(
              tail_candidate, &body, bytes + body.appended_offset,
              body.appended_size, &tail_resume) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK &&
          tail_resume.valid && tail_resume.party_level == 4u &&
          tail_resume.party_x == 22u && tail_resume.party_y == 18u,
          "legacy CSBGAME2 tail prepares a complete private resume transaction");
    csb_v1_csbwin_dungeon_tail_discard_legacy_candidate(tail_candidate);
    tail_candidate = NULL;
    free(bytes);

    /* The tail prepares privately but cannot be published while the live
     * queue lacks source-owned support for this legacy 10-byte TIMER layout. */
    csb_v1_runtime_init(&runtime, NULL);
    runtime.game_time = 919u;
    runtime.party_x = 7;
    CHECK(csb_v1_runtime_apply_csbwin_resume_file(&runtime, path, 0u) != 0 &&
          runtime.game_time == 919u && runtime.party_x == 7 &&
          !runtime.csbwin_save_provenance.valid,
          "legacy CSBGAME2 10-byte timer layout fails closed before runtime import");
    csb_v1_runtime_cleanup(&runtime);
}

int main(void)
{
    const char *good = "firestaff-csbwin-provenance-good.sav";
    const char *bad = "firestaff-csbwin-provenance-bad.sav";
    const char *copy = "firestaff-csbwin-provenance-copy.sav";
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_CSBWinSaveProvenance_PC34 provenance;
    uint32_t preserved_time;
    int preserved_x;

    CHECK(firestaff_test_write_csbwin_resume_fixture(good, 0),
          "writes checksum-verified CSBWin save");
    csb_v1_runtime_init(&runtime, NULL);
    runtime.game_time = 77u;
    runtime.party_x = 3;
    CHECK(csb_v1_runtime_apply_csbwin_resume_file(&runtime, good, 0u) == 0,
          "accepts a complete CSBWin header/body save");
    CHECK(csb_v1_runtime_get_csbwin_save_provenance(&runtime, &provenance) == 0 &&
          provenance.valid && provenance.source_size > 512u &&
          provenance.source_fnv1a != 0u && provenance.core_fnv1a != 0u &&
          provenance.key_verdict == CSB_V1_CSBWIN_512_VERDICT_CSB &&
          strcmp(provenance.source_path, good) == 0,
          "records exact original-save provenance after commit");
    CHECK(csb_v1_runtime_export_csbwin_source_save_to_path(&runtime, copy) == 0 &&
          files_equal(good, copy),
          "source-preserving export retains the complete authenticated CSBWin artifact");

    CHECK(append_invalid_expool_tail(good),
          "changes the source artifact after provenance capture");
    CHECK(csb_v1_runtime_export_csbwin_source_save_to_path(&runtime, copy) != 0,
          "source-preserving export rejects source bytes that drift after resume");

    CHECK(firestaff_test_write_csbwin_resume_fixture(bad, 0),
          "writes second verified CSBWin save");
    CHECK(append_invalid_expool_tail(bad),
          "appends malformed DB11/EXPOOL tail");
    preserved_time = runtime.game_time;
    preserved_x = runtime.party_x;
    CHECK(csb_v1_runtime_apply_csbwin_resume_file(&runtime, bad, 0u) != 0,
          "rejects malformed DB11/EXPOOL before runtime commit");
    CHECK(runtime.game_time == preserved_time && runtime.party_x == preserved_x &&
          runtime.csbwin_save_provenance.valid &&
          strcmp(runtime.csbwin_save_provenance.source_path, good) == 0,
          "failed original-save import rolls back live runtime and provenance");

    test_staged_real_csbwin_save();

    remove(good);
    remove(bad);
    remove(copy);
    printf("%d passed, %d failed\\n", passed, failed);
    return failed != 0;
}
