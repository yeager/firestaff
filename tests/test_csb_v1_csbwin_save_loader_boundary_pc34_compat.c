/*
 * test_csb_v1_csbwin_save_loader_boundary_pc34_compat.c
 *
 * Data-free contract tests for the CSB V1 CSBWin save-side
 * loader-boundary evidence gate.
 *
 * Scope:
 *   - For every CSB_V1_CSBWinSaveShape value, build a synthetic
 *     byte buffer and feed it into the existing
 *     csb_v1_import_csb_save_buffer() entry point. Verify the
 *     loader's actual return code matches the documented
 *     accept/reject contract for that shape.
 *   - Lock the contract-table invariants: every shape has a
 *     non-NULL label, every reject-shape has a non-zero
 *     expect_code, every accept-shape has expect_code = 0.
 *   - Lock the accept-shape helper:
 *     csb_v1_csbwin_save_loader_boundary_match() returns the
 *     right CSB_V1_CSBWinSaveShape for v2.0/v2.1 buffers and
 *     CSB_V1_CSBWIN_SHAPE_COUNT for everything else.
 *   - Source-evidence citation chain returns a non-NULL string
 *     naming at least one ReDMCSB file and one CSBWin source.
 *   - Builder determinism: build a v2.0 fixture twice and verify
 *     byte-identical output (no hidden RNG, no stack noise).
 *
 * Non-claims:
 *   - No real CSBWin / DM1 save bytes are loaded.
 *   - No CSBWin 512-byte obfuscation-key decoder is added; the
 *     test merely asserts the documented contract that the
 *     CSBWIN_512_* shapes are loader-rejected.
 *   - No M11/M12 wiring. The launcher / engine uses the verdict
 *     when (and only when) it decides to expose an import
 *     button for CSBWin saves.
 */

#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                                 \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); }                      \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); }                      \
} while (0)

int main(void)
{
    const CSB_V1_CSBWinSaveShapeContract *table = NULL;
    size_t table_count = 0u;
    size_t i;
    size_t accept_pass = 0u;
    size_t reject_pass = 0u;

    printf("=== CSB V1 CSBWin save loader-boundary contract ===\n\n");

    /* ── Contract table invariants ── */
    table = csb_v1_csbwin_save_loader_boundary_contract(&table_count);
    printf("contract_table_count = %zu (accept=%zu, reject=%zu)\n\n",
           table_count,
           csb_v1_csbwin_save_loader_boundary_accept_count(),
           csb_v1_csbwin_save_loader_boundary_reject_count());

    CHECK(table_count == (size_t)CSB_V1_CSBWIN_SHAPE_COUNT,
          "contract table covers every CSB_V1_CSBWinSaveShape");
    CHECK(csb_v1_csbwin_save_loader_boundary_accept_count() == 3u,
          "exactly 3 accept-shapes (CSB v2.0, v2.1, .bak payload)");
    CHECK(csb_v1_csbwin_save_loader_boundary_reject_count() == 11u,
          "exactly 11 reject-shapes (everything else)");
    {
        int all_labels_non_null = 1;
        int all_accept_have_zero = 1;
        int all_reject_have_code = 1;
        int all_evidence_non_null = 1;
        for (i = 0u; i < table_count; ++i) {
            if (!table[i].label) all_labels_non_null = 0;
            if (!table[i].source_evidence) all_evidence_non_null = 0;
            if (table[i].expect_accept && table[i].expect_code != 0)
                all_accept_have_zero = 0;
            if (!table[i].expect_accept && table[i].expect_code == 0)
                all_reject_have_code = 0;
        }
        CHECK(all_labels_non_null, "every contract row has a label");
        CHECK(all_evidence_non_null,
              "every contract row has a source_evidence citation");
        CHECK(all_accept_have_zero,
              "every accept-shape has expect_code == 0");
        CHECK(all_reject_have_code,
              "every reject-shape has expect_code != 0");
    }

    /* ── Per-shape loader-boundary check ──
     *
     * For every shape: (a) build the synthetic fixture via the
     * public builder, (b) run the public convenience check that
     * builds + tests internally, and (c) assert contract_match
     * is 1 and the documented accept/reject verdict is met. */
    for (i = 0u; i < table_count; ++i) {
        CSB_V1_CSBWinSaveShape shape = table[i].shape;
        CSB_V1_CSBWinLoaderBoundaryResult res;
        int rc;
        const char *shape_name =
            csb_v1_csbwin_save_loader_boundary_shape_name(shape);
        char msg[200];

        rc = csb_v1_csbwin_save_loader_boundary_check_shape(shape, &res);
        snprintf(msg, sizeof(msg),
                 "%s: check_shape rc=%d, loader_code=%d, contract_match=%d",
                 shape_name, rc, res.loader_code, res.contract_match);
        CHECK(rc == res.loader_code,
              msg);  /* check API returns loader_code */

        if (table[i].expect_accept) {
            snprintf(msg, sizeof(msg),
                     "%s: loader returns positive count (%d)",
                     shape_name, res.loader_code);
            CHECK(res.loader_code > 0, msg);
            snprintf(msg, sizeof(msg),
                     "%s: contract_match == 1 (accept shape)",
                     shape_name);
            CHECK(res.contract_match == 1, msg);
            ++accept_pass;
        } else {
            snprintf(msg, sizeof(msg),
                     "%s: loader returns expect_code (%d == %d)",
                     shape_name, res.loader_code,
                     table[i].expect_code);
            CHECK(res.loader_code == table[i].expect_code, msg);
            snprintf(msg, sizeof(msg),
                     "%s: contract_match == 1 (reject shape)",
                     shape_name);
            CHECK(res.contract_match == 1, msg);
            ++reject_pass;
        }
    }
    CHECK(accept_pass == csb_v1_csbwin_save_loader_boundary_accept_count(),
          "all accept-shapes loader-passed");
    CHECK(reject_pass == csb_v1_csbwin_save_loader_boundary_reject_count(),
          "all reject-shapes loader-rejected");

    /* ── Accept-shapes report a champion count that matches the
     *    fixture (CSB v2.0 / v2.1 with champ_count = 1 → 1). ── */
    {
        CSB_V1_CSBWinLoaderBoundaryResult res;
        int rc = csb_v1_csbwin_save_loader_boundary_check_shape(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, &res);
        CHECK(rc == 1, "CSB v2.0 fixture loader returns 1 champion");
        CHECK(res.champion_count == 1,
              "CSB v2.0 fixture champion_count == 1");

        rc = csb_v1_csbwin_save_loader_boundary_check_shape(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V21, &res);
        CHECK(rc == 1, "CSB v2.1 fixture loader returns 1 champion");
        CHECK(res.champion_count == 1,
              "CSB v2.1 fixture champion_count == 1");
    }

    /* ── Builder determinism: a second build is byte-identical
     *    to the first (no hidden RNG, no stack noise). ── */
    {
        uint8_t a[1024];
        uint8_t b[1024];
        size_t sa = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, a, sizeof(a));
        size_t sb = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, b, sizeof(b));
        CHECK(sa == sb, "two v2.0 builds return the same size");
        CHECK(sa > 0u && memcmp(a, b, sa) == 0,
              "two v2.0 builds return byte-identical bytes");
    }

    /* ── csb_v1_csbwin_save_loader_boundary_match() helper ── */
    {
        CSB_V1_CSBWinSaveShape matched;
        uint8_t scratch[1024];
        size_t s;

        /* CSB v2.0 → CSB_V1_CSBWIN_SHAPE_CSBGAME_V20. */
        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, scratch, sizeof(scratch));
        CHECK(s > 0u, "v2.0 fixture built for match() check");
        matched = csb_v1_csbwin_save_loader_boundary_match(scratch, s);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_CSBGAME_V20,
              "match() recognises CSB v2.0 buffer");

        /* CSB v2.1 → CSB_V1_CSBWIN_SHAPE_CSBGAME_V21. */
        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V21, scratch, sizeof(scratch));
        matched = csb_v1_csbwin_save_loader_boundary_match(scratch, s);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_CSBGAME_V21,
              "match() recognises CSB v2.1 buffer");

        /* DM1 raw RDMCSB15 — must NOT match any accept shape. */
        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15, scratch, sizeof(scratch));
        matched = csb_v1_csbwin_save_loader_boundary_match(scratch, s);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_COUNT,
              "match() rejects DM1 raw RDMCSB15 (no accept-shape)");

        /* CSBWin 512-byte CSB\\1 — must NOT match any accept shape. */
        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1, scratch, sizeof(scratch));
        matched = csb_v1_csbwin_save_loader_boundary_match(scratch, s);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_COUNT,
              "match() rejects CSBWin 512-byte CSB\\1 (no accept-shape)");

        /* CSBWin 512-byte CEDT — must NOT match any accept shape. */
        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT, scratch, sizeof(scratch));
        matched = csb_v1_csbwin_save_loader_boundary_match(scratch, s);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_COUNT,
              "match() rejects CSBWin 512-byte CEDT (no accept-shape)");

        /* NULL bytes / zero length. */
        matched = csb_v1_csbwin_save_loader_boundary_match(NULL, 0u);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_COUNT,
              "match() rejects NULL bytes");
        matched = csb_v1_csbwin_save_loader_boundary_match(scratch, 0u);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_COUNT,
              "match() rejects zero-length buffer");

        /* A header-only CSB v2.0 buffer (champ_count=0, no records)
         * passes the magic+version pre-check but must NOT be
         * treated as fully loadable — match() only validates magic
         * and version; the full check via the loader correctly
         * rejects with ERR_NO_CHAMPIONS. We assert that match()
         * recognises the magic+version here, and verify the full
         * loader-boundary check rejects it in the per-shape loop
         * above. */
        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0,
            scratch, sizeof(scratch));
        matched = csb_v1_csbwin_save_loader_boundary_match(scratch, s);
        CHECK(matched == CSB_V1_CSBWIN_SHAPE_CSBGAME_V20,
              "match() recognises header-only CSB v2.0 magic+version");
    }

    /* ── Discovery/classification gate for staged filenames ── */
    {
        CSB_V1_CSBWinSaveDiscoveryResult disc;
        uint8_t scratch[1024];
        size_t s;
        int rc;

        CHECK(csb_v1_csbwin_save_loader_boundary_file_kind(
                  "/tmp/Custom/CSBGAME.DAT") ==
              CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_DAT,
              "file_kind recognises uppercase CSBGAME.DAT basename");
        CHECK(csb_v1_csbwin_save_loader_boundary_file_kind(
                  "C:\\CSB\\DMSAVE.BAK") ==
              CSB_V1_CSBWIN_SAVE_FILE_DMSAVE_BAK,
              "file_kind recognises backslash DMSAVE.BAK basename");
        CHECK(csb_v1_csbwin_save_loader_boundary_file_kind(
                  "/tmp/not_a_save.bin") ==
              CSB_V1_CSBWIN_SAVE_FILE_NONE,
              "file_kind rejects unrelated basename");

        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, scratch, sizeof(scratch));
        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/CSBGAME.DAT", scratch, s, &disc);
        CHECK(rc > 0, "classify CSBGAME.DAT v2.0 returns loader accept");
        CHECK(disc.filename_candidate == 1,
              "classify CSBGAME.DAT marks filename candidate");
        CHECK(disc.shape == CSB_V1_CSBWIN_SHAPE_CSBGAME_V20,
              "classify CSBGAME.DAT v2.0 shape");
        CHECK(disc.should_attempt_import == 1,
              "classify CSBGAME.DAT v2.0 is import-ready");
        CHECK(strcmp(disc.decision_label, "accept_loader_ready") == 0,
              "classify CSBGAME.DAT v2.0 decision label");

        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/csbgame.bak", scratch, s, &disc);
        CHECK(rc > 0, "classify csbgame.bak v2.0 returns loader accept");
        CHECK(disc.shape == CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD,
              "classify .bak v2.0 maps to bak payload shape");
        CHECK(disc.should_attempt_import == 1,
              "classify .bak v2.0 is import-ready");

        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V21, scratch, sizeof(scratch));
        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/DMSAVE.DAT", scratch, s, &disc);
        CHECK(rc > 0, "classify DMSAVE.DAT v2.1 returns loader accept");
        CHECK(disc.shape == CSB_V1_CSBWIN_SHAPE_CSBGAME_V21,
              "classify DMSAVE.DAT v2.1 shape");
        CHECK(disc.should_attempt_import == 1,
              "classify DMSAVE.DAT v2.1 is import-ready");

        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15, scratch, sizeof(scratch));
        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/dmsave.dat", scratch, s, &disc);
        CHECK(rc == CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
              "classify dmsave.dat RDMCSB15 returns BAD_MAGIC");
        CHECK(disc.shape == CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15,
              "classify dmsave.dat RDMCSB15 shape");
        CHECK(disc.should_attempt_import == 0,
              "classify dmsave.dat RDMCSB15 is not import-ready");
        CHECK(strcmp(disc.decision_label,
                     "reject_dm1_raw_needs_conversion") == 0,
              "classify dmsave.dat RDMCSB15 decision label");

        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1, scratch, sizeof(scratch));
        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "C:\\CSB\\DMSAVE.BAK", scratch, s, &disc);
        CHECK(rc == CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
              "classify DMSAVE.BAK CSBWin 512 returns BAD_MAGIC");
        CHECK(disc.file_kind == CSB_V1_CSBWIN_SAVE_FILE_DMSAVE_BAK,
              "classify DMSAVE.BAK preserves filename kind");
        CHECK(disc.shape == CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1,
              "classify DMSAVE.BAK CSBWin 512 shape");
        CHECK(strcmp(disc.decision_label,
                     "reject_csbwin_512_needs_decoder") == 0,
              "classify DMSAVE.BAK CSBWin 512 decision label");

        s = csb_v1_csbwin_save_loader_boundary_build_fixture(
            CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, scratch, sizeof(scratch));
        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/not_a_save.bin", scratch, s, &disc);
        CHECK(rc > 0,
              "classify valid CSBGAME bytes under unrelated name still runs loader");
        CHECK(disc.filename_candidate == 0,
              "classify unrelated name is not a filename candidate");
        CHECK(disc.should_attempt_import == 0,
              "classify unrelated name is not import-ready");
        CHECK(strcmp(disc.decision_label,
                     "reject_non_csbwin_save_filename") == 0,
              "classify unrelated name decision label");

        memcpy(scratch, "CSBGAME\0", 8);
        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/csbgame.dat", scratch, 8u, &disc);
        CHECK(rc == CSB_SAVE_IMPORT_ERR_TRUNCATED,
              "classify short CSBGAME header returns TRUNCATED");
        CHECK(disc.shape == CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS,
              "classify short CSBGAME header shape");
        CHECK(strcmp(disc.decision_label, "reject_truncated") == 0,
              "classify short CSBGAME header decision label");

        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/csbgame.dat", NULL, 0u, &disc);
        CHECK(rc == CSB_SAVE_IMPORT_ERR_NULL,
              "classify NULL bytes returns ERR_NULL");
        CHECK(disc.shape == CSB_V1_CSBWIN_SHAPE_COUNT,
              "classify NULL bytes shape_count");
        CHECK(strcmp(disc.decision_label, "reject_no_bytes") == 0,
              "classify NULL bytes decision label");

        rc = csb_v1_csbwin_save_loader_boundary_classify(
            "/tmp/csbgame.dat", scratch, s, NULL);
        CHECK(rc == CSB_SAVE_IMPORT_ERR_NULL,
              "classify NULL out returns ERR_NULL");
    }

    /* ── Source-evidence citation chain ── */
    {
        const char *ev = csb_v1_csbwin_save_loader_boundary_source_evidence();
        CHECK(ev != NULL, "source_evidence returns non-NULL");
        CHECK(strstr(ev, "ReDMCSB") != NULL,
              "source_evidence names ReDMCSB source file");
        CHECK(strstr(ev, "CSBWin") != NULL,
              "source_evidence names CSBWin source file");
        CHECK(strstr(ev, "csb_v1_save_import_path") != NULL,
              "source_evidence names the existing loader header");
    }

    /* ── Shape-name helper ── */
    {
        CHECK(strcmp(csb_v1_csbwin_save_loader_boundary_shape_name(
                         CSB_V1_CSBWIN_SHAPE_CSBGAME_V20),
                     "csbgame_v20") == 0,
              "shape name CSB v2.0 = csbgame_v20");
        CHECK(strcmp(csb_v1_csbwin_save_loader_boundary_shape_name(
                         CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15),
                     "dm1_raw_rdmcsb15") == 0,
              "shape name DM1 raw = dm1_raw_rdmcsb15");
        CHECK(strcmp(csb_v1_csbwin_save_loader_boundary_shape_name(
                         CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD),
                     "csbgame_v20_bak_payload") == 0,
              "shape name .bak payload = csbgame_v20_bak_payload");
        CHECK(strcmp(csb_v1_csbwin_save_loader_boundary_shape_name(
                         (CSB_V1_CSBWinSaveShape)999),
                     "unknown") == 0,
              "shape name out-of-range -> unknown");
    }

    /* ── Loader-boundary check with custom bytes (not the
     *    auto-built fixture). Used to verify NULL-handling,
     *    accept-shape on a hand-rolled CSBGAME\0 buffer, and
     *    reject-shape on a non-CSBWin-shape we did not pre-
     *    enumerate. ── */
    {
        CSB_V1_CSBWinLoaderBoundaryResult res;

        /* Hand-rolled CSB v2.0 buffer with champ_count = 2. */
        uint8_t hand[CSB_SAVE_HEADER_SIZE + 2u * CSB_SAVE_CHAMP_SIZE + 16u];
        size_t hand_size = (size_t)CSB_SAVE_HEADER_SIZE
                         + 2u * (size_t)CSB_SAVE_CHAMP_SIZE;
        memset(hand, 0, hand_size);
        memcpy(hand + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0", 8);
        hand[CSB_SAVE_HDR_OFF_VERSION]     = 0x00u;
        hand[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02u;  /* 0x200 */
        hand[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 2u;
        {
            int rc = csb_v1_csbwin_save_loader_boundary_check(
                hand, hand_size,
                CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, &res);
            CHECK(rc > 0 && rc == 2,
                  "hand-rolled 2-champion v2.0 buffer loader-accepts (rc=2)");
            CHECK(res.contract_match == 1,
                  "hand-rolled 2-champion v2.0 buffer contract_match=1");
        }

        /* NULL bytes — should fail closed. */
        {
            int rc = csb_v1_csbwin_save_loader_boundary_check(
                NULL, 0u,
                CSB_V1_CSBWIN_SHAPE_CSBGAME_V20, &res);
            CHECK(rc == CSB_SAVE_IMPORT_ERR_NULL,
                  "NULL bytes -> loader ERR_NULL");
        }

        /* Bytes that look like CSBGAME\0 but with a different
         * shape enum value (UNKNOWN variant). The loader still
         * accepts the buffer; the contract_match must be 0
         * because the documented contract for that shape is
         * "n/a". We use a v2.0 buffer but label it as the
         * CDSA shape — the bytes don't have the CDSA marker, so
         * the loader would actually accept them; the
         * contract_match must reflect the documented mismatch. */
        {
            int rc = csb_v1_csbwin_save_loader_boundary_check(
                hand, hand_size,
                CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA, &res);
            CHECK(rc > 0,
                  "v2.0 bytes mislabeled as CDSA: loader still accepts");
            CHECK(res.contract_match == 0,
                  "v2.0 bytes mislabeled as CDSA: contract_match=0");
        }
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
