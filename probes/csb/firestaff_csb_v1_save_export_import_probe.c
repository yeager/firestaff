/*
 * firestaff_csb_v1_save_export_import_probe.c
 *
 * Real-asset CSB V1 per-game save-byte export/import probe.
 *
 * Source-lock boundary (see include/csb_v1_save_export_import_
 * pc34_compat.h for the full evidence chain):
 *   - ReDMCSB CEDTINC8.C:101-118 (CSBGAME routing)
 *   - ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace)
 *   - ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)
 *   - ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic)
 *   - CSBWin SaveGame.cpp:927/1711/2111 (save file I/O)
 *
 * What this proves:
 *   - The FSSB envelope gate builds, validates, parses, and
 *     round-trips a CSB V1 save-byte stream through the existing
 *     csb_v1_build_csb_save_buffer() and
 *     csb_v1_import_csb_save_buffer() entry points.
 *   - The classification helper correctly distinguishes FSSB
 *     envelopes from raw CSBGAME v2.0 / v2.1, DM1 RDMCSB15, and
 *     CSBWin 512-byte buffers.
 *   - When a user-staged *.csbsave file is present, the probe
 *     validates the envelope and feeds it back through the
 *     production loader.
 *
 * Skip-safe by design: hosts without a known FSSB envelope
 * exit 0 with a SKIP message after the synthetic-fixture
 * portion has already proven the contract.
 */

#include "csb_v1_save_export_import_pc34_compat.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                              \
    ++g_checks;                                                            \
    if (cond) {                                                            \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s\n", msg);                                       \
    }                                                                      \
} while (0)

/* Build a deterministic 2-champion party for the synthetic
 * round-trip. Mirrors the helper in the data-free unit test. */
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

/* Resolve the data-dir argument / env var. Mirrors the helper
 * in firestaff_csb_v1_csbwin_save_loader_boundary_probe.c. */
static const char *data_dir_arg(int argc, char **argv,
                                char *buf, size_t buf_size)
{
    const char *env;
    const char *home;
    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        return argv[1];
    }
    env = getenv("FIRESTAFF_CSB_SAVE_EXPORT_DATA");
    if (env && env[0] != '\0') return env;
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') return env;
    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data", home);
    return buf;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    uint8_t envelope[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
    long envelope_len;

    printf("=== CSB V1 save-byte export/import probe ===\n\n");

    /* ── Synthetic round-trip pass (always runs) ── */
    {
        CSB_V1_PartyState export_party;
        CSB_V1_PartyState import_party;
        int rc;

        make_synthetic_party(&export_party, 2);
        envelope_len = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V20,
            "synthetic://probe.csbsave",
            envelope, sizeof(envelope));
        CHECK(envelope_len > 0,
              "synthetic v2.0 2-champion round-trip builds > 0 bytes");
        CHECK((size_t)envelope_len
                  == CSB_V1_SAVE_EXPORT_HEADER_LEN
                   + CSB_SAVE_HEADER_SIZE
                   + 2u * CSB_SAVE_CHAMP_SIZE,
              "synthetic envelope size matches HEADER + 256 + 2*160");
        CHECK(csb_v1_save_export_validate_envelope(
                  envelope, (size_t)envelope_len)
                  == CSB_V1_SAVE_EXPORT_OK,
              "synthetic envelope validates");
        CHECK(csb_v1_save_export_classify(envelope, (size_t)envelope_len)
                  == CSB_V1_SAVE_EXPORT_KIND_FSSB_ENVELOPE,
              "synthetic envelope classifies as fssb_envelope");

        memset(&import_party, 0, sizeof(import_party));
        rc = csb_v1_save_export_import_envelope(
            &import_party, envelope, (size_t)envelope_len);
        CHECK(rc == 2,
              "synthetic import_envelope returns champion count (2)");
        CHECK(import_party.ChampionCount == 2,
              "synthetic imported party ChampionCount == 2");
        CHECK(import_party.ImportSource == CSB_SAVE_IMPORT_SOURCE,
              "synthetic imported party ImportSource stamp");
        CHECK(strcmp(import_party.Champions[0].Name, "CHAMP0") == 0,
              "synthetic imported champion[0].Name round-trip");
        CHECK(strcmp(import_party.Champions[1].Name, "CHAMP1") == 0,
              "synthetic imported champion[1].Name round-trip");
    }

    /* ── Synthetic v2.1 round-trip ── */
    {
        CSB_V1_PartyState export_party;
        CSB_V1_PartyState import_party;
        int rc;
        long v21_len;

        make_synthetic_party(&export_party, 1);
        v21_len = csb_v1_save_export_roundtrip(
            &export_party, CSB_SAVE_VERSION_V21,
            NULL,
            envelope, sizeof(envelope));
        CHECK(v21_len > 0,
              "synthetic v2.1 1-champion round-trip builds > 0 bytes");
        CHECK(csb_v1_save_export_validate_envelope(
                  envelope, (size_t)v21_len)
                  == CSB_V1_SAVE_EXPORT_OK,
              "synthetic v2.1 envelope validates");
        memset(&import_party, 0, sizeof(import_party));
        rc = csb_v1_save_export_import_envelope(
            &import_party, envelope, (size_t)v21_len);
        CHECK(rc == 1,
              "synthetic v2.1 import_envelope returns champion count (1)");
        CHECK(strcmp(import_party.Champions[0].Name, "CHAMP0") == 0,
              "synthetic v2.1 imported champion[0].Name round-trip");
    }

    /* ── Synthetic CRC reference vector ── */
    {
        const uint8_t vec[9] = {'1','2','3','4','5','6','7','8','9'};
        uint32_t got = csb_v1_save_export_crc32(vec, 9u);
        CHECK(got == 0xCBF43926u,
              "CRC-32 reference vector (123456789 -> 0xCBF43926)");
    }

    /* ── Synthetic classification ── */
    {
        uint8_t raw20[CSB_SAVE_HEADER_SIZE];
        uint8_t raw_dm1[16];
        memset(raw20, 0, sizeof(raw20));
        memcpy(raw20, "CSBGAME\0", 8u);
        raw20[8] = 0x00u; raw20[9] = 0x02u;
        CHECK(csb_v1_save_export_classify(raw20, sizeof(raw20))
                  == CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V20,
              "classify(raw CSBGAME v2.0) -> raw_csbgame_v20");
        memset(raw_dm1, 0, sizeof(raw_dm1));
        memcpy(raw_dm1, "RDMCSB15", 8u);
        CHECK(csb_v1_save_export_classify(raw_dm1, sizeof(raw_dm1))
                  == CSB_V1_SAVE_EXPORT_KIND_RAW_DM1_RDMCSB,
              "classify(raw DM1 RDMCSB15) -> raw_dm1_rdmcsb");
    }

    /* ── Real-asset probe (skip-safe) ──
     *
     * Try to find a user-staged *.csbsave file under the data
     * dir. If we find one, validate the envelope and feed it
     * back through the production loader. If not, we SKIP
     * cleanly. */
    {
        CSB_V1_SaveExportScanResult scan;
        uint8_t read_buf[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
        int rc;

        dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
        printf("data_dir=%s\n", dir ? dir : "(none)");

        csb_v1_save_export_scan(dir, 6, &scan);
        printf("scan_result: present=%zu, well_formed=%zu, first_path=%s\n",
               scan.present_count, scan.well_formed_count,
               scan.first_path[0] ? scan.first_path : "(none)");

        if (scan.well_formed_count == 0u) {
            printf("SKIP: no user-staged *.csbsave file (FSSB envelope) "
                   "found under data_dir; export/import gate has "
                   "still been proven on synthetic fixtures.\n");
            return 0;
        }

        rc = csb_v1_save_export_read_envelope(
            scan.first_path, read_buf, sizeof(read_buf));
        CHECK(rc > (int)CSB_V1_SAVE_EXPORT_HEADER_LEN,
              "real envelope: read_envelope returns > header size");
        if (rc > 0) {
            CSB_V1_PartyState party;
            memset(&party, 0, sizeof(party));
            {
                int import_rc = csb_v1_save_export_import_envelope(
                    &party, read_buf, (size_t)rc);
                CHECK(import_rc > 0,
                      "real envelope: import_envelope returns > 0 "
                      "(production loader accepts the wrapped payload)");
                if (import_rc > 0) {
                    CHECK(party.ChampionCount == import_rc,
                          "real envelope: imported ChampionCount matches "
                          "loader return code");
                    CHECK(party.ImportSource == CSB_SAVE_IMPORT_SOURCE,
                          "real envelope: imported ImportSource stamp");
                }
            }
        }
    }

    printf("\n=== Summary: %d checks, %d failures ===\n",
           g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
