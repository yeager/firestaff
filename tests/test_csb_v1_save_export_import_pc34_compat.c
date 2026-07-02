/*
 * test_csb_v1_save_export_import_pc34_compat.c
 *
 * Data-free contract tests for the CSB V1 per-game save-byte
 * export/import compatibility proof.
 *
 * Scope:
 *   - classify every documented save-byte shape (FSSB envelope,
 *     raw CSBGAME v2.0, raw CSBGAME v2.1, raw DM1 RDMCSB15,
 *     raw CSBWin 512-byte) plus the unknown / too-small
 *     fall-through.
 *   - CRC-32 / ISO 3309 over known vectors (zlib's documented
 *     "123456789" -> 0xCBF43926 reference value).
 *   - Envelope header magic + version + payload_kind + reserved
 *     + payload_len + CRC + source_path invariants.
 *   - csb_v1_save_export_validate_envelope() rejects every
 *     malformed envelope (bad magic, bad version, bad
 *     payload_kind, reserved != 0, length overflow, CRC
 *     mismatch, truncated).
 *   - csb_v1_save_export_roundtrip() builds a CSB v2.0 / v2.1
 *     payload via csb_v1_build_csb_save_buffer, wraps it in an
 *     FSSB envelope, and then csb_v1_save_export_import_envelope()
 *     feeds the inner payload back through the production
 *     csb_v1_import_csb_save_buffer() loader with a deterministic
 *     party. This is the load-bearing round-trip test — if the
 *     export/import contract ever broke, this is where it
 *     surfaces.
 *   - Builder determinism: two envelopes built from the same
 *     party are byte-identical (no hidden RNG, no stack noise).
 *   - Source-evidence citation chain returns a non-NULL string
 *     naming at least one ReDMCSB file and one CSBWin source.
 *
 * Non-claims:
 *   - No real CSBWin / DM1 save bytes are loaded.
 *   - No CSBWin 512-byte obfuscation-key decoder is added.
 *   - No M11/M12 wiring. The launcher / engine uses this gate
 *     when (and only when) it decides to expose a per-game
 *     save-byte export/import action.
 */

#include "csb_v1_save_export_import_pc34_compat.h"
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

/* Build a deterministic 2-champion party for round-trip tests.
 * The data is meaningless as a "real" party — we only care that
 * the bytes round-trip through the production loader. */
static void make_synthetic_party(CSB_V1_PartyState *party,
                                  int champion_count)
{
    int i;
    int s;
    int sk;

    memset(party, 0, sizeof(*party));
    party->ChampionCount = champion_count;
    party->ImportedFromDM1 = 0;
    party->ImportSource    = 0;
    party->PartyDirection  = 0;
    party->LeaderIndex     = 0;
    party->MagicCasterIndex = -1;
    party->PartyMapX       = 1;
    party->PartyMapY       = 2;

    for (i = 0; i < champion_count; ++i) {
        CSB_V1_Champion *c = &party->Champions[i];
        csb_v1_champion_init(c);
        snprintf(c->Name, sizeof(c->Name), "CHAMP%d", i);
        c->CurrentHealth   = (int16_t)(40 + i * 10);
        c->MaximumHealth   = (int16_t)(60 + i * 10);
        c->CurrentStamina  = 80;
        c->MaximumStamina  = 80;
        c->CurrentMana     = 30;
        c->MaximumMana     = 30;
        for (s = 0; s < CSB_V1_STAT_COUNT; ++s) {
            c->Statistics[s][CSB_V1_STAT_MIN] = 1;
            /* csb_v1_import_csb_save_buffer() clamps cur to >= 30
             * per the DM1/CSB import convention, so use values
             * above 30 to make the round-trip lossless. */
            c->Statistics[s][CSB_V1_STAT_CUR] = (uint16_t)(35 + i * 2 + s);
            c->Statistics[s][CSB_V1_STAT_MAX] = (uint16_t)(45 + i * 2 + s);
        }
        for (sk = 0; sk < CSB_V1_SKILL_COUNT; ++sk) {
            c->Skills[sk] = (uint8_t)(sk + 1);
        }
        for (s = 0; s < CSB_V1_SLOT_COUNT; ++s) {
            c->Slots[s] = (uint16_t)s;
        }
    }
}

int main(void)
{
    uint8_t scratch[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
    size_t scratch_size = sizeof(scratch);
    long rc_long;
    int rc_int;

    printf("=== CSB V1 save-byte export/import gate ===\n\n");

    /* ── Envelope size invariant ── */
    {
        size_t s = csb_v1_save_export_envelope_size(0u);
        CHECK(s == 0u, "envelope_size(0) == 0");
        s = csb_v1_save_export_envelope_size(CSB_V1_SAVE_EXPORT_MAX_PAYLOAD);
        CHECK(s == CSB_V1_SAVE_EXPORT_HEADER_LEN + CSB_V1_SAVE_EXPORT_MAX_PAYLOAD,
              "envelope_size(MAX) == HEADER + MAX");
        s = csb_v1_save_export_envelope_size(CSB_V1_SAVE_EXPORT_MAX_PAYLOAD + 1u);
        CHECK(s == 0u, "envelope_size overflow -> 0");
        CHECK(CSB_V1_SAVE_EXPORT_HEADER_LEN == 88u,
              "fixed envelope header is exactly 88 bytes");
    }

    /* ── CRC-32 reference vector ── */
    {
        /* zlib's documented reference: CRC32("123456789") == 0xCBF43926. */
        const uint8_t vec[9] = {'1','2','3','4','5','6','7','8','9'};
        uint32_t got = csb_v1_save_export_crc32(vec, 9u);
        char msg[80];
        snprintf(msg, sizeof(msg),
                 "CRC32(123456789) == 0xCBF43926 (got 0x%08X)", got);
        CHECK(got == 0xCBF43926u, msg);

        /* Empty input -> 0 (initial/final XOR with 0xFFFFFFFF). */
        got = csb_v1_save_export_crc32(NULL, 0u);
        CHECK(got == 0u, "CRC32(NULL, 0) == 0");
    }

    /* ── Classification ── */
    {
        uint8_t fssb[CSB_V1_SAVE_EXPORT_HEADER_LEN];
        uint8_t raw20[CSB_SAVE_HEADER_SIZE];
        uint8_t raw21[CSB_SAVE_HEADER_SIZE];
        uint8_t raw_dm1[16];
        uint8_t raw_csbwin[CSB_SAVE_HEADER_SIZE];

        memset(fssb, 0, sizeof(fssb));
        memcpy(fssb, CSB_V1_SAVE_EXPORT_MAGIC, CSB_V1_SAVE_EXPORT_MAGIC_LEN);

        memset(raw20, 0, sizeof(raw20));
        memcpy(raw20, "CSBGAME\0", 8u);
        raw20[8] = 0x00u; raw20[9] = 0x02u; raw20[10] = 0x00u; raw20[11] = 0x00u;

        memset(raw21, 0, sizeof(raw21));
        memcpy(raw21, "CSBGAME\0", 8u);
        raw21[8] = 0x01u; raw21[9] = 0x02u; raw21[10] = 0x00u; raw21[11] = 0x00u;

        memset(raw_dm1, 0, sizeof(raw_dm1));
        memcpy(raw_dm1, "RDMCSB15", 8u);

        memset(raw_csbwin, 0, sizeof(raw_csbwin));
        memcpy(raw_csbwin, "CSB\x01", 4u);

        CHECK(csb_v1_save_export_classify(fssb, sizeof(fssb))
                  == CSB_V1_SAVE_EXPORT_KIND_FSSB_ENVELOPE,
              "classify(FSSB) -> fssb_envelope");
        CHECK(csb_v1_save_export_classify(raw20, sizeof(raw20))
                  == CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V20,
              "classify(CSBGAME v2.0) -> raw_csbgame_v20");
        CHECK(csb_v1_save_export_classify(raw21, sizeof(raw21))
                  == CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V21,
              "classify(CSBGAME v2.1) -> raw_csbgame_v21");
        CHECK(csb_v1_save_export_classify(raw_dm1, sizeof(raw_dm1))
                  == CSB_V1_SAVE_EXPORT_KIND_RAW_DM1_RDMCSB,
              "classify(DM1 RDMCSB15) -> raw_dm1_rdmcsb");
        CHECK(csb_v1_save_export_classify(raw_csbwin, sizeof(raw_csbwin))
                  == CSB_V1_SAVE_EXPORT_KIND_RAW_CSBWIN_512,
              "classify(CSBWin 512-byte CSB\\1) -> raw_csbwin_512");

        /* Unknown / too-small fall-through. */
        {
            uint8_t junk[8] = {0};
            CHECK(csb_v1_save_export_classify(junk, sizeof(junk))
                      == CSB_V1_SAVE_EXPORT_KIND_UNKNOWN,
                  "classify(unknown 8-byte) -> unknown");
            CHECK(csb_v1_save_export_classify(NULL, 0u)
                      == CSB_V1_SAVE_EXPORT_KIND_UNKNOWN,
                  "classify(NULL, 0) -> unknown");
            CHECK(csb_v1_save_export_classify(junk, 3u)
                      == CSB_V1_SAVE_EXPORT_KIND_UNKNOWN,
                  "classify(< 4 bytes) -> unknown");
        }

        /* Kind-name helper. */
        CHECK(strcmp(csb_v1_save_export_kind_name(
                         CSB_V1_SAVE_EXPORT_KIND_FSSB_ENVELOPE),
                     "fssb_envelope") == 0,
              "kind_name(FSSB) -> fssb_envelope");
        CHECK(strcmp(csb_v1_save_export_kind_name(
                         CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V20),
                     "raw_csbgame_v20") == 0,
              "kind_name(RAW v2.0) -> raw_csbgame_v20");
        CHECK(strcmp(csb_v1_save_export_kind_name(
                         CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V21),
                     "raw_csbgame_v21") == 0,
              "kind_name(RAW v2.1) -> raw_csbgame_v21");
        CHECK(strcmp(csb_v1_save_export_kind_name(
                         CSB_V1_SAVE_EXPORT_KIND_RAW_DM1_RDMCSB),
                     "raw_dm1_rdmcsb") == 0,
              "kind_name(RAW DM1) -> raw_dm1_rdmcsb");
        CHECK(strcmp(csb_v1_save_export_kind_name(
                         CSB_V1_SAVE_EXPORT_KIND_RAW_CSBWIN_512),
                     "raw_csbwin_512") == 0,
              "kind_name(RAW CSBWin 512) -> raw_csbwin_512");
        CHECK(strcmp(csb_v1_save_export_kind_name(
                         CSB_V1_SAVE_EXPORT_KIND_UNKNOWN),
                     "unknown") == 0,
              "kind_name(UNKNOWN) -> unknown");
    }

    /* ── Build a synthetic envelope and validate it ── */
    {
        uint8_t payload[CSB_SAVE_HEADER_SIZE + 2u * CSB_SAVE_CHAMP_SIZE];
        memset(payload, 0, sizeof(payload));
        memcpy(payload, "CSBGAME\0", 8u);
        payload[CSB_SAVE_HDR_OFF_VERSION]     = 0x00u;
        payload[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02u;  /* 0x200 */
        payload[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 2u;

        rc_long = csb_v1_save_export_build_envelope(
            payload, sizeof(payload), 0u,
            "/tmp/synthetic.csbsave",
            scratch, scratch_size);
        CHECK(rc_long > 0, "build_envelope on synthetic payload returns > 0");
        CHECK((size_t)rc_long == CSB_V1_SAVE_EXPORT_HEADER_LEN + sizeof(payload),
              "build_envelope total == HEADER + payload_len");
        CHECK(csb_v1_save_export_classify(scratch, (size_t)rc_long)
                  == CSB_V1_SAVE_EXPORT_KIND_FSSB_ENVELOPE,
              "synthetic envelope classifies as fssb_envelope");
        CHECK(csb_v1_save_export_validate_envelope(scratch, (size_t)rc_long)
                  == CSB_V1_SAVE_EXPORT_OK,
              "synthetic envelope validates");

        /* parse_header round-trip. */
        {
            CSB_V1_SaveExportHeader hdr;
            int rc = csb_v1_save_export_parse_header(
                scratch, (size_t)rc_long, &hdr);
            CHECK(rc == CSB_V1_SAVE_EXPORT_OK, "parse_header returns OK");
            CHECK(hdr.manifest_version == CSB_V1_SAVE_EXPORT_MANIFEST_VERSION,
                  "parse_header.manifest_version == v1");
            CHECK(hdr.game_id == CSB_V1_SAVE_EXPORT_GAME_ID,
                  "parse_header.game_id == CSB\\1");
            CHECK(hdr.payload_kind == 0u,
                  "parse_header.payload_kind == 0 (v2.0)");
            CHECK(hdr.reserved == 0u,
                  "parse_header.reserved == 0");
            CHECK(hdr.payload_len == sizeof(payload),
                  "parse_header.payload_len matches");
            CHECK(strcmp(hdr.source_path, "/tmp/synthetic.csbsave") == 0,
                  "parse_header.source_path round-trip");
        }

        /* Truncated envelope -> ERR_BUF_TOO_SMALL. */
        CHECK(csb_v1_save_export_validate_envelope(scratch, 4u)
                  == CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL,
              "validate(< header) -> ERR_BUF_TOO_SMALL");
        CHECK(csb_v1_save_export_validate_envelope(scratch, 50u)
                  == CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL,
              "validate(< header) -> ERR_BUF_TOO_SMALL (50 bytes)");

        /* Bad magic -> ERR_BAD_VERSION (we use BAD_VERSION because
         * the magic is in the version field slot of the parser). */
        {
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN];
            memcpy(bad, scratch, sizeof(bad));
            bad[0] = 'Z';
            CHECK(csb_v1_save_export_validate_envelope(bad, sizeof(bad))
                      == CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION,
                  "validate(bad magic) -> ERR_BAD_VERSION");
        }

        /* Bad version -> ERR_BAD_VERSION. */
        {
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN];
            memcpy(bad, scratch, sizeof(bad));
            /* manifest_version is at bytes [4..7]; bump to a bogus value. */
            bad[4] = 0xFFu;
            CHECK(csb_v1_save_export_validate_envelope(bad, sizeof(bad))
                      == CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION,
                  "validate(bad manifest_version) -> ERR_BAD_VERSION");
        }

        /* Bad payload_kind -> ERR_BAD_PAYLOAD_KIND. */
        {
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN];
            memcpy(bad, scratch, sizeof(bad));
            /* payload_kind is at bytes [12..13]; set to 0xFFFF. */
            bad[12] = 0xFFu;
            bad[13] = 0xFFu;
            CHECK(csb_v1_save_export_validate_envelope(bad, sizeof(bad))
                      == CSB_V1_SAVE_EXPORT_ERR_BAD_PAYLOAD_KIND,
                  "validate(bad payload_kind) -> ERR_BAD_PAYLOAD_KIND");
        }

        /* Non-zero reserved -> ERR_BAD_VERSION (the parser uses
         * BAD_VERSION for any reserved/version/magic mismatch). */
        {
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN];
            memcpy(bad, scratch, sizeof(bad));
            /* reserved is at bytes [14..15]; set to 0x0001. */
            bad[14] = 0x01u;
            CHECK(csb_v1_save_export_validate_envelope(bad, sizeof(bad))
                      == CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION,
                  "validate(reserved != 0) -> ERR_BAD_VERSION");
        }

        /* payload_len = 0 -> ERR_BUF_TOO_SMALL. */
        {
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN];
            memcpy(bad, scratch, sizeof(bad));
            /* payload_len is at bytes [16..19]; set to 0. */
            bad[16] = 0u; bad[17] = 0u; bad[18] = 0u; bad[19] = 0u;
            CHECK(csb_v1_save_export_validate_envelope(bad, sizeof(bad))
                      == CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL,
                  "validate(payload_len == 0) -> ERR_BUF_TOO_SMALL");
        }

        /* payload_len overflow -> ERR_BUF_TOO_SMALL. */
        {
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN];
            memcpy(bad, scratch, sizeof(bad));
            /* payload_len is at bytes [16..19]; set to MAX+1. */
            bad[16] = 0x01u; bad[17] = 0x00u;
            bad[18] = 0x00u; bad[19] = 0x10u;  /* 0x10000001 */
            CHECK(csb_v1_save_export_validate_envelope(bad, sizeof(bad))
                      == CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL,
                  "validate(payload_len overflow) -> ERR_BUF_TOO_SMALL");
        }

        /* payload_crc mismatch -> ERR_BAD_CRC. We rebuild a
         * small envelope for this test so we don't have to
         * truncate a 576-byte payload (the truncation check
         * would fire before the CRC check). */
        {
            uint8_t small_payload[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
            uint8_t small_envelope[CSB_V1_SAVE_EXPORT_HEADER_LEN + 16];
            long small_len;
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN + 16];
            size_t bad_size = CSB_V1_SAVE_EXPORT_HEADER_LEN + 16;

            small_len = csb_v1_save_export_build_envelope(
                small_payload, sizeof(small_payload), 0u,
                "/tmp/small.csbsave",
                small_envelope, sizeof(small_envelope));
            CHECK(small_len == (long)bad_size,
                  "small envelope build size matches HEADER + 16");

            memcpy(bad, small_envelope, bad_size);
            /* Flip a single bit in payload_crc (byte 21). */
            bad[21] ^= 0x01u;
            CHECK(csb_v1_save_export_validate_envelope(bad, bad_size)
                      == CSB_V1_SAVE_EXPORT_ERR_BAD_CRC,
                  "validate(bad CRC) -> ERR_BAD_CRC");
        }

        /* payload byte flipped -> ERR_BAD_CRC. Same small
         * envelope. */
        {
            uint8_t small_payload[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
            uint8_t small_envelope[CSB_V1_SAVE_EXPORT_HEADER_LEN + 16];
            long small_len;
            uint8_t bad[CSB_V1_SAVE_EXPORT_HEADER_LEN + 16];
            size_t bad_size = CSB_V1_SAVE_EXPORT_HEADER_LEN + 16;

            small_len = csb_v1_save_export_build_envelope(
                small_payload, sizeof(small_payload), 0u,
                "/tmp/small.csbsave",
                small_envelope, sizeof(small_envelope));
            CHECK(small_len == (long)bad_size,
                  "small envelope rebuild matches HEADER + 16");

            memcpy(bad, small_envelope, bad_size);
            bad[CSB_V1_SAVE_EXPORT_HEADER_LEN + 0u] ^= 0x01u;
            CHECK(csb_v1_save_export_validate_envelope(bad, bad_size)
                      == CSB_V1_SAVE_EXPORT_ERR_BAD_CRC,
                  "validate(flipped payload byte) -> ERR_BAD_CRC");
        }
    }

    /* ── Round-trip through the production CSB V1 loader ── */
    {
        CSB_V1_PartyState export_party;
        CSB_V1_PartyState import_party;
        long envelope_len;

        make_synthetic_party(&export_party, 2);
        envelope_len = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V20,
            "/tmp/roundtrip.csbsave",
            scratch, scratch_size);
        CHECK(envelope_len > 0,
              "roundtrip(2-champion v2.0 party) -> envelope > 0 bytes");
        CHECK((size_t)envelope_len
                  == CSB_V1_SAVE_EXPORT_HEADER_LEN
                   + CSB_SAVE_HEADER_SIZE
                   + 2u * CSB_SAVE_CHAMP_SIZE,
              "roundtrip envelope == HEADER + 256 + 2*160 bytes");
        CHECK(csb_v1_save_export_validate_envelope(
                  scratch, (size_t)envelope_len)
                  == CSB_V1_SAVE_EXPORT_OK,
              "roundtrip envelope validates");

        memset(&import_party, 0, sizeof(import_party));
        rc_int = csb_v1_save_export_import_envelope(
            &import_party, scratch, (size_t)envelope_len);
        CHECK(rc_int == 2,
              "import_envelope returns champion count (2)");
        CHECK(import_party.ChampionCount == 2,
              "imported party ChampionCount == 2");
        CHECK(import_party.ImportedFromDM1 == 0,
              "imported party ImportedFromDM1 == 0 (CSB save, not DM1)");
        CHECK(import_party.ImportSource == CSB_SAVE_IMPORT_SOURCE,
              "imported party ImportSource == CSB_SAVE_IMPORT_SOURCE");
        CHECK(strcmp(import_party.Champions[0].Name, "CHAMP0") == 0,
              "imported champion[0].Name round-trip");
        CHECK(strcmp(import_party.Champions[1].Name, "CHAMP1") == 0,
              "imported champion[1].Name round-trip");
        CHECK(import_party.Champions[0].CurrentHealth
                  == export_party.Champions[0].CurrentHealth,
              "imported champion[0].CurrentHealth round-trip");
        CHECK(import_party.Champions[0].MaximumHealth
                  == export_party.Champions[0].MaximumHealth,
              "imported champion[0].MaximumHealth round-trip");
        CHECK(import_party.Champions[1].CurrentHealth
                  == export_party.Champions[1].CurrentHealth,
              "imported champion[1].CurrentHealth round-trip");
        {
            int s;
            int ok_stats = 1;
            for (s = 0; s < CSB_V1_STAT_COUNT; ++s) {
                if (import_party.Champions[0].Statistics[s][CSB_V1_STAT_CUR]
                        != export_party.Champions[0].Statistics[s][CSB_V1_STAT_CUR]) {
                    ok_stats = 0;
                }
                if (import_party.Champions[0].Statistics[s][CSB_V1_STAT_MAX]
                        != export_party.Champions[0].Statistics[s][CSB_V1_STAT_MAX]) {
                    ok_stats = 0;
                }
            }
            CHECK(ok_stats, "imported champion[0].Statistics[cur/max] round-trip");
        }
        {
            int sk;
            int ok_skills = 1;
            for (sk = 0; sk < CSB_V1_SKILL_COUNT; ++sk) {
                if (import_party.Champions[0].Skills[sk]
                        != export_party.Champions[0].Skills[sk]) {
                    ok_skills = 0;
                }
            }
            CHECK(ok_skills, "imported champion[0].Skills round-trip");
        }
    }

    /* ── Round-trip via v2.1 ── */
    {
        CSB_V1_PartyState export_party;
        CSB_V1_PartyState import_party;
        long envelope_len;

        make_synthetic_party(&export_party, 1);
        envelope_len = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V21,
            NULL,
            scratch, scratch_size);
        CHECK(envelope_len > 0,
              "roundtrip(1-champion v2.1 party) -> envelope > 0 bytes");
        CHECK(csb_v1_save_export_validate_envelope(
                  scratch, (size_t)envelope_len)
                  == CSB_V1_SAVE_EXPORT_OK,
              "v2.1 envelope validates");

        {
            CSB_V1_SaveExportHeader hdr;
            CHECK(csb_v1_save_export_parse_header(
                      scratch, (size_t)envelope_len, &hdr)
                      == CSB_V1_SAVE_EXPORT_OK,
                  "v2.1 envelope parse_header OK");
            CHECK(hdr.payload_kind == 1u,
                  "v2.1 envelope payload_kind == 1");
            CHECK(strcmp(hdr.source_path, "(synthetic)") == 0,
                  "NULL source_path -> (synthetic)");
        }

        memset(&import_party, 0, sizeof(import_party));
        rc_int = csb_v1_save_export_import_envelope(
            &import_party, scratch, (size_t)envelope_len);
        CHECK(rc_int == 1,
              "import_envelope returns champion count (1)");
        CHECK(import_party.ChampionCount == 1,
              "v2.1 imported party ChampionCount == 1");
        CHECK(strcmp(import_party.Champions[0].Name, "CHAMP0") == 0,
              "v2.1 imported champion[0].Name round-trip");
    }

    /* ── Round-trip via 4-champion party (MAX payload size). ── */
    {
        CSB_V1_PartyState export_party;
        CSB_V1_PartyState import_party;
        long envelope_len;

        make_synthetic_party(&export_party, CSB_V1_MAX_CHAMPIONS);
        envelope_len = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V20,
            "memory://4champ.csbsave",
            scratch, scratch_size);
        CHECK(envelope_len > 0,
              "roundtrip(4-champion v2.0 party) -> envelope > 0 bytes");

        memset(&import_party, 0, sizeof(import_party));
        rc_int = csb_v1_save_export_import_envelope(
            &import_party, scratch, (size_t)envelope_len);
        CHECK(rc_int == CSB_V1_MAX_CHAMPIONS,
              "import_envelope returns 4 (MAX_CHAMPIONS)");
        CHECK(import_party.ChampionCount == CSB_V1_MAX_CHAMPIONS,
              "4-champion imported party ChampionCount == 4");
    }

    /* ── Bad-party rejection ── */
    {
        CSB_V1_PartyState bad_party;
        long envelope_len;

        make_synthetic_party(&bad_party, 2);
        bad_party.ChampionCount = 0;  /* invalid */
        envelope_len = csb_v1_save_export_roundtrip(
            &bad_party, CSB_SAVE_VERSION_V20,
            NULL,
            scratch, scratch_size);
        CHECK(envelope_len == CSB_V1_SAVE_EXPORT_ERR_BAD_PARTY,
              "roundtrip(0 champions) -> ERR_BAD_PARTY");

        make_synthetic_party(&bad_party, 2);
        bad_party.ChampionCount = CSB_V1_MAX_CHAMPIONS + 1;
        envelope_len = csb_v1_save_export_roundtrip(
            &bad_party, CSB_SAVE_VERSION_V20,
            NULL,
            scratch, scratch_size);
        CHECK(envelope_len == CSB_V1_SAVE_EXPORT_ERR_BAD_PARTY,
              "roundtrip(> MAX champions) -> ERR_BAD_PARTY");

        envelope_len = csb_v1_save_export_roundtrip(
            NULL, CSB_SAVE_VERSION_V20, NULL,
            scratch, scratch_size);
        CHECK(envelope_len == CSB_V1_SAVE_EXPORT_ERR_NULL,
              "roundtrip(NULL party) -> ERR_NULL");

        make_synthetic_party(&bad_party, 2);
        envelope_len = csb_v1_save_export_roundtrip(
            &bad_party, 0x999u, NULL,
            scratch, scratch_size);
        CHECK(envelope_len == CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION,
              "roundtrip(bad csb_version) -> ERR_BAD_VERSION");
    }

    /* ── Builder determinism: two envelopes built from the same
     *    party are byte-identical (no hidden RNG, no stack
     *    noise). ── */
    {
        CSB_V1_PartyState export_party;
        uint8_t a[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
        uint8_t b[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
        long sa, sb;

        make_synthetic_party(&export_party, 3);
        sa = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V20,
            "determinism.csbsave",
            a, sizeof(a));
        sb = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V20,
            "determinism.csbsave",
            b, sizeof(b));
        CHECK(sa == sb, "two roundtrip builds return the same size");
        CHECK(sa > 0 && memcmp(a, b, (size_t)sa) == 0,
              "two roundtrip builds return byte-identical bytes");
    }

    /* ── import_envelope rejects malformed input ── */
    {
        CSB_V1_PartyState party;
        CHECK(csb_v1_save_export_import_envelope(NULL, scratch, 100u)
                  == CSB_V1_SAVE_EXPORT_ERR_NULL,
              "import_envelope(NULL party) -> ERR_NULL");
        CHECK(csb_v1_save_export_import_envelope(&party, NULL, 0u)
                  == CSB_V1_SAVE_EXPORT_ERR_NULL,
              "import_envelope(NULL bytes) -> ERR_NULL");
        CHECK(csb_v1_save_export_import_envelope(&party, scratch, 4u)
                  == CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL,
              "import_envelope(< header) -> ERR_BUF_TOO_SMALL");
    }

    /* ── File I/O: write_envelope / read_envelope round-trip ── */
    {
        CSB_V1_PartyState export_party;
        CSB_V1_PartyState import_party;
        long envelope_len;
        const char *tmp_path = "/tmp/firestaff_csb_v1_export_import_test.csbsave";
        uint8_t read_buf[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
        int rc;

        make_synthetic_party(&export_party, 2);
        envelope_len = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V20,
            tmp_path,
            scratch, scratch_size);
        CHECK(envelope_len > 0,
              "file roundtrip: built envelope > 0 bytes");

        rc = csb_v1_save_export_write_envelope(tmp_path, scratch,
                                                (size_t)envelope_len);
        CHECK(rc == CSB_V1_SAVE_EXPORT_OK,
              "write_envelope returns OK");

        rc = csb_v1_save_export_read_envelope(
            tmp_path, read_buf, sizeof(read_buf));
        CHECK(rc == (int)envelope_len,
              "read_envelope returns the same size we wrote");

        memset(&import_party, 0, sizeof(import_party));
        rc = csb_v1_save_export_import_envelope(
            &import_party, read_buf, (size_t)rc);
        CHECK(rc == 2,
              "file roundtrip: import_envelope returns champion count (2)");
        CHECK(strcmp(import_party.Champions[0].Name, "CHAMP0") == 0,
              "file roundtrip: imported champion[0].Name round-trip");

        /* Negative I/O paths. */
        CHECK(csb_v1_save_export_write_envelope(
                  "/no/such/directory/x.csbsave", scratch, 100u)
                  == CSB_V1_SAVE_EXPORT_ERR_IO,
              "write_envelope(bad path) -> ERR_IO");
        CHECK(csb_v1_save_export_read_envelope(
                  "/no/such/directory/x.csbsave",
                  read_buf, sizeof(read_buf))
                  == CSB_V1_SAVE_EXPORT_ERR_IO,
              "read_envelope(bad path) -> ERR_IO");
    }

    /* ── Source-evidence citation chain ── */
    {
        const char *ev = csb_v1_save_export_source_evidence();
        CHECK(ev != NULL, "source_evidence returns non-NULL");
        CHECK(strstr(ev, "ReDMCSB") != NULL,
              "source_evidence names ReDMCSB source file");
        CHECK(strstr(ev, "CSBWin") != NULL,
              "source_evidence names CSBWin source file");
        CHECK(strstr(ev, "csb_v1_save_import_path") != NULL,
              "source_evidence names the production loader header");
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
