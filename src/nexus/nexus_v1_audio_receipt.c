#include "nexus_v1_audio_receipt.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t sal_size;
    const char *sal_sha256;
    uint32_t map_size;
    const char *map_sha256;
} Nexus_V1_AudioExpectedRow;

/* Nexus audio receipt table.
 *
 * Source evidence:
 * - docs/NEXUS_FILE_CLASSIFICATION.md:19-22 identifies SNDLEV00-15.SAL
 *   and SNDLEV00-15.MAP as 16 per-level sound banks + 16 mapping tables.
 * - docs/VERIFIED_HASHES.md:154-185 records the verified size/SHA256
 *   pairs for every SNDLEV##.SAL and SNDLEV##.MAP asset.
 * - docs/nexus_audio_format.md records CD-DA tracks 2-9 and the
 *   2-levels-per-track mapping.
 *
 * This module is a receipt/classification gate only. It does not parse SAL,
 * decode MAP tables, read CD sectors, or start audio playback.
 */
static const Nexus_V1_AudioExpectedRow g_expected[NEXUS_V1_AUDIO_LEVEL_COUNT] = {
    {297082u, "3864e979306727ed3759066947fa0bf61da32d29464efbcdbdd044194fe02d79", 66u, "169de6fcb3c3966c6ca5f0a5d044add9d390bc5f1ce0f497bd352c5810c11716"},
    {297082u, "3864e979306727ed3759066947fa0bf61da32d29464efbcdbdd044194fe02d79", 66u, "169de6fcb3c3966c6ca5f0a5d044add9d390bc5f1ce0f497bd352c5810c11716"},
    {315126u, "d30edfbb6ba2e5082461e243b5899a463522e99619b9d52c681087b99164941a", 74u, "5ceb5f342132ebf9cb784a24366fd6db7638eefe798d2fb1cc3c4d82947782b5"},
    {357112u, "a7df9688656bafa210ed07ee1edd79b5111095f6e0815d3009b9dda885145d1e", 82u, "92c5bbc414d260a1a263dba939c831dbbe5d26519e333f6b1f43d054096b9316"},
    {378192u, "c980709ec95e0f09b6def8ac96ec3179f1330b2b0bbd655218f57d26352ac63d", 82u, "8b64d2b441c7a089b809e7848a89c76be19fbe98e7be9b193d069f07a898cc4d"},
    {335928u, "da91e969ef78c6c4c063d672e67eb0a89f8630790068e0c08f626f53d62a54fd", 82u, "c2cd225eb66199bc5b9c34c253f3c72b86650c1f6abb6037acc83b65f05e67ed"},
    {436904u, "8badfd8195b2e1786684917ec6d409e7864f0f42a600e997bd0f368e2a2ac889", 82u, "7c244e37db13114143aae118fcbc50595af3f7f12f20df1ece6c42863ebc6707"},
    {350658u, "6df55f770904b05f9c7dfd5f6c850449d22d137b3427f99f3c69c39fddf5bd5c", 82u, "081a5ecc258d11a8fc7bad0dd43e9ff49b2b947392f1b87fd0202297e211b659"},
    {469710u, "424eb9c84894019002a4998cca89ec88aafa37e310c2c4eee760be480d94a50d", 90u, "c0b86c4f2f10540eb5ff88485d620321e6d4e72c45fa4d27040b3fc42a081688"},
    {416918u, "857ec3d1caea11915f6e36555f16474dfa44ed5bf9cf7bc20d487688f07f00d3", 74u, "3c5e2722e9445e85584a935b0feea3c662b66732bbe4d5793354247097a4ad63"},
    {419550u, "c45a7e9942deb3347e8679adf48aa0932db4433e7a655bf10bd6b60fcfb5306f", 82u, "be423f44dba19c3d4cdb1b202d0e238b5912a01dc01f395e61db3c557703f6b2"},
    {390272u, "b88ebe0db03355581771008acfe0b418ca53a951a5c7bf054681462614f3b509", 82u, "6dc8fb6155a9f65e26322df6f39023d12cce31629326e3fc9a5b3363fd6bccc1"},
    {388508u, "3a38ccb478db04a9999eb4b4cd5a429a5abeecae27fc142a1c06df112d26a906", 82u, "4a0accbb49b987be87559c2fa00db5e2f99d97feb6665305ab34f0773c5861ee"},
    {393044u, "4e7cbfd3fa7a95198b3e6e7ecbb484bc114f288a884dbfeda7a9873d54081397", 82u, "dd923153d4c7b74bc24f2c89bcffc23a4b6cf91e9c7d6e4a65860280aa230d96"},
    {441498u, "3bc017258e3ef32034757c04b26159f05af1972bb18b5039d72bb13eded4b8e9", 82u, "4fbba4db48b635028dc5cca6fc301a04d4f8ab9501d83f56e7f9a94de240ac53"},
    {374216u, "5fbae7ccd954b1150620f8a62110dea4af58d1acff3041ef51c47f662fb58c18", 74u, "7a6bb1fe86dcdfd545e4efb96613d1ac77a17075783a2defa84635236447dcfe"}
};

static int ascii_ieq(char a, char b) {
    return tolower((unsigned char)a) == tolower((unsigned char)b);
}

static int suffix_ieq(const char *s, const char *suffix) {
    size_t s_len;
    size_t suffix_len;
    size_t i;

    if (!s || !suffix) return 0;
    s_len = strlen(s);
    suffix_len = strlen(suffix);
    if (s_len < suffix_len) return 0;
    s += s_len - suffix_len;
    for (i = 0; i < suffix_len; ++i) {
        if (!ascii_ieq(s[i], suffix[i])) return 0;
    }
    return 1;
}

static const char *basename_ptr(const char *path) {
    const char *base;
    const char *p;

    if (!path) return NULL;
    base = path;
    for (p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    return base;
}

static int parse_sndlev_name(const char *path,
                             Nexus_V1_AudioKind *out_kind,
                             int *out_level) {
    const char *name = basename_ptr(path);
    int tens;
    int ones;

    if (!name || strlen(name) != 12u) return NEXUS_V1_AUDIO_ERR_BAD_NAME;
    if (!ascii_ieq(name[0], 'S') ||
        !ascii_ieq(name[1], 'N') ||
        !ascii_ieq(name[2], 'D') ||
        !ascii_ieq(name[3], 'L') ||
        !ascii_ieq(name[4], 'E') ||
        !ascii_ieq(name[5], 'V')) {
        return NEXUS_V1_AUDIO_ERR_BAD_NAME;
    }
    if (!isdigit((unsigned char)name[6]) ||
        !isdigit((unsigned char)name[7]) ||
        name[8] != '.') {
        return NEXUS_V1_AUDIO_ERR_BAD_NAME;
    }

    tens = name[6] - '0';
    ones = name[7] - '0';
    *out_level = (tens * 10) + ones;
    if (*out_level < 0 || *out_level >= NEXUS_V1_AUDIO_LEVEL_COUNT) {
        return NEXUS_V1_AUDIO_ERR_BAD_LEVEL;
    }

    if (suffix_ieq(name, ".SAL")) {
        *out_kind = NEXUS_V1_AUDIO_KIND_SAL_BANK;
    } else if (suffix_ieq(name, ".MAP")) {
        *out_kind = NEXUS_V1_AUDIO_KIND_MAP_TABLE;
    } else {
        return NEXUS_V1_AUDIO_ERR_BAD_NAME;
    }

    return NEXUS_V1_AUDIO_OK;
}

static void fill_expected(Nexus_V1_AudioKind kind,
                          int level_index,
                          Nexus_V1_AudioReceipt *out) {
    const Nexus_V1_AudioExpectedRow *row = &g_expected[level_index];

    memset(out, 0, sizeof(*out));
    out->kind = kind;
    out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_NAME_ONLY;
    out->level_index = level_index;
    out->cd_track = nexus_v1_audio_cd_track_for_level_receipt(level_index);

    if (kind == NEXUS_V1_AUDIO_KIND_SAL_BANK) {
        out->expected_size = row->sal_size;
        out->expected_sha256 = row->sal_sha256;
        snprintf(out->expected_name, sizeof(out->expected_name),
                 "SNDLEV%02d.SAL", level_index);
    } else if (kind == NEXUS_V1_AUDIO_KIND_MAP_TABLE) {
        out->expected_size = row->map_size;
        out->expected_sha256 = row->map_sha256;
        snprintf(out->expected_name, sizeof(out->expected_name),
                 "SNDLEV%02d.MAP", level_index);
    }
}

int nexus_v1_audio_expected_asset(Nexus_V1_AudioKind kind,
                                  int level_index,
                                  Nexus_V1_AudioReceipt *out) {
    if (!out) return NEXUS_V1_AUDIO_ERR_NULL;
    memset(out, 0, sizeof(*out));
    if (level_index < 0 || level_index >= NEXUS_V1_AUDIO_LEVEL_COUNT) {
        return NEXUS_V1_AUDIO_ERR_BAD_LEVEL;
    }
    if (kind != NEXUS_V1_AUDIO_KIND_SAL_BANK &&
        kind != NEXUS_V1_AUDIO_KIND_MAP_TABLE) {
        return NEXUS_V1_AUDIO_ERR_BAD_KIND;
    }

    fill_expected(kind, level_index, out);
    return NEXUS_V1_AUDIO_OK;
}

int nexus_v1_audio_classify_file(const char *path,
                                 uint32_t size,
                                 const char *sha256,
                                 Nexus_V1_AudioReceipt *out) {
    Nexus_V1_AudioKind kind = NEXUS_V1_AUDIO_KIND_UNKNOWN;
    int level = -1;
    int rc;

    if (!path || !out) return NEXUS_V1_AUDIO_ERR_NULL;
    memset(out, 0, sizeof(*out));

    rc = parse_sndlev_name(path, &kind, &level);
    if (rc != NEXUS_V1_AUDIO_OK) return rc;

    fill_expected(kind, level, out);
    out->observed_size = size;

    if (size == 0u) {
        out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_NAME_ONLY;
        return NEXUS_V1_AUDIO_OK;
    }
    if (size != out->expected_size) {
        out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_SIZE_MISMATCH;
        return NEXUS_V1_AUDIO_OK;
    }

    out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH;
    if (sha256 && sha256[0] != '\0') {
        out->receipt_class =
            strcmp(sha256, out->expected_sha256) == 0
                ? NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH
                : NEXUS_V1_AUDIO_RECEIPT_HASH_MISMATCH;
    }

    return NEXUS_V1_AUDIO_OK;
}

int nexus_v1_audio_classify_cdda_layout(int data_track_count,
                                        int audio_track_count,
                                        int first_audio_track,
                                        int last_audio_track,
                                        Nexus_V1_CddaLayoutReceipt *out) {
    if (!out) return NEXUS_V1_AUDIO_ERR_NULL;
    memset(out, 0, sizeof(*out));

    out->data_track_count = data_track_count;
    out->audio_track_count = audio_track_count;
    out->first_audio_track = first_audio_track;
    out->last_audio_track = last_audio_track;

    if (data_track_count < 0 ||
        audio_track_count < 0 ||
        first_audio_track < 0 ||
        last_audio_track < 0 ||
        last_audio_track < first_audio_track) {
        out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_UNKNOWN;
        return NEXUS_V1_AUDIO_ERR_BOUNDS;
    }

    if (data_track_count == 1 &&
        audio_track_count == NEXUS_V1_AUDIO_CDDA_TRACK_COUNT &&
        first_audio_track == NEXUS_V1_AUDIO_CDDA_TRACK_FIRST &&
        last_audio_track == NEXUS_V1_AUDIO_CDDA_TRACK_LAST) {
        out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_MATCH;
    } else if (data_track_count == 1 &&
               first_audio_track <= NEXUS_V1_AUDIO_CDDA_TRACK_FIRST &&
               last_audio_track >= NEXUS_V1_AUDIO_CDDA_TRACK_FIRST &&
               audio_track_count > 0) {
        out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_PARTIAL;
    } else {
        out->receipt_class = NEXUS_V1_AUDIO_RECEIPT_UNKNOWN;
    }

    return NEXUS_V1_AUDIO_OK;
}

int nexus_v1_audio_cd_track_for_level_receipt(int level_index) {
    if (level_index < 0 || level_index >= NEXUS_V1_AUDIO_LEVEL_COUNT) {
        return -1;
    }
    /* Same documented mapping as src/nexus/nexus_v1_game.c:
     * tracks 2-9, one CD-DA track for each pair of dungeon levels. */
    return NEXUS_V1_AUDIO_CDDA_TRACK_FIRST + (level_index / 2);
}

int nexus_v1_audio_decode_supported(Nexus_V1_AudioKind kind) {
    (void)kind;
    return 0;
}

const char *nexus_v1_audio_kind_name(Nexus_V1_AudioKind kind) {
    switch (kind) {
    case NEXUS_V1_AUDIO_KIND_SAL_BANK: return "sal-bank";
    case NEXUS_V1_AUDIO_KIND_MAP_TABLE: return "map-table";
    case NEXUS_V1_AUDIO_KIND_CDDA_LAYOUT: return "cdda-layout";
    case NEXUS_V1_AUDIO_KIND_UNKNOWN:
    default: return "unknown";
    }
}

const char *nexus_v1_audio_receipt_class_name(Nexus_V1_AudioReceiptClass cls) {
    switch (cls) {
    case NEXUS_V1_AUDIO_RECEIPT_NAME_ONLY: return "name-only";
    case NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH: return "size-match";
    case NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH: return "verified-hash";
    case NEXUS_V1_AUDIO_RECEIPT_SIZE_MISMATCH: return "size-mismatch";
    case NEXUS_V1_AUDIO_RECEIPT_HASH_MISMATCH: return "hash-mismatch";
    case NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_MATCH: return "cdda-layout-match";
    case NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_PARTIAL: return "cdda-layout-partial";
    case NEXUS_V1_AUDIO_RECEIPT_UNKNOWN:
    default: return "unknown";
    }
}

const char *nexus_v1_audio_status_string(int status) {
    switch (status) {
    case NEXUS_V1_AUDIO_OK: return "ok";
    case NEXUS_V1_AUDIO_ERR_NULL: return "null";
    case NEXUS_V1_AUDIO_ERR_BAD_NAME: return "bad-name";
    case NEXUS_V1_AUDIO_ERR_BAD_LEVEL: return "bad-level";
    case NEXUS_V1_AUDIO_ERR_BAD_KIND: return "bad-kind";
    case NEXUS_V1_AUDIO_ERR_BOUNDS: return "bad-bounds";
    default: return "unknown-status";
    }
}

const char *nexus_v1_audio_source_evidence(void) {
    return
        "docs/NEXUS_FILE_CLASSIFICATION.md: SNDLEV00-15.SAL/.MAP inventory\n"
        "docs/VERIFIED_HASHES.md:154-185 verified SAL/MAP sizes + SHA256\n"
        "docs/nexus_audio_format.md: CD-DA tracks 2-9, two levels per track\n"
        "src/nexus/nexus_v1_game.c: nexus_v1_cd_track_for_level mapping\n"
        "Boundary: receipt/classification only; no SAL decode, MAP parse, "
        "CD sector read, or playback binding.\n";
}
