#include "dm1_v1_fmtowns_tbios_id.h"
#include <string.h>

/* Candidate identifier base addresses observed in real BIOS blobs
 * (from Tsugaru `tbiosid.cpp` physical-memory dumps). */
static const uint32_t k_candidate_bases[] = {
    0x1F750U,   /* V31L22A */
    0x1F3D0U,   /* V31L23A, V31L31_90 */
    0x100000U   /* V31L31_91+ */
};
#define K_CANDIDATE_COUNT (sizeof(k_candidate_bases) / sizeof(k_candidate_bases[0]))

typedef struct {
    dm1_v1_fmtowns_tbios_version_t version;
    const char *ver;
    const char *date;
} known_entry_t;

static const known_entry_t k_known[] = {
    { DM1_V1_FMTOWNS_TBIOS_V31L22A,   "V31L22A", "89/03/08" },
    { DM1_V1_FMTOWNS_TBIOS_V31L23A,   "V31L23A", "90/09/21" },
    { DM1_V1_FMTOWNS_TBIOS_V31L31_90, "V31L31",  "90/11/21" },
    { DM1_V1_FMTOWNS_TBIOS_V31L31_91, "V31L31",  "91/10/05" },
    { DM1_V1_FMTOWNS_TBIOS_V31L31_92, "V31L31",  "92/10/16" },
    { DM1_V1_FMTOWNS_TBIOS_V31L31_93, "V31L31",  "93/01/07" },
    { DM1_V1_FMTOWNS_TBIOS_V31L35,    "V31L35",  "93/10/15" },
    { DM1_V1_FMTOWNS_TBIOS_V31L35_94, "V31L35",  "94/12/03" }
};
#define K_KNOWN_COUNT (sizeof(k_known) / sizeof(k_known[0]))

static void read_field(const uint8_t *src, char dst[9]) {
    unsigned i;
    for (i = 0; i < 8u; ++i) {
        char c = (char)src[i];
        dst[i] = c;
        if (c == '\0') { break; }
    }
    dst[i < 8u ? i : 8u] = '\0';
}

int dm1_v1_fmtowns_tbios_identify_pc34(
        const uint8_t *bios_bytes, size_t bios_size,
        dm1_v1_fmtowns_tbios_identity_t *out) {
    unsigned base_i;
    unsigned known_i;
    if (!bios_bytes || !out) return 0;
    memset(out, 0, sizeof(*out));
    for (base_i = 0; base_i < K_CANDIDATE_COUNT; ++base_i) {
        uint32_t base = k_candidate_bases[base_i];
        char ver[9], date[9], prod[9], subs[9];
        if ((size_t)base + 32u > bios_size) continue;
        read_field(bios_bytes + base + 0u, ver);
        read_field(bios_bytes + base + 8u, date);
        read_field(bios_bytes + base + 16u, prod);
        read_field(bios_bytes + base + 24u, subs);
        if (strcmp(prod, "towns") != 0) continue;
        if (strcmp(subs, "tbios") != 0) continue;
        for (known_i = 0; known_i < K_KNOWN_COUNT; ++known_i) {
            if (strcmp(ver, k_known[known_i].ver) == 0 &&
                strcmp(date, k_known[known_i].date) == 0) {
                out->version = k_known[known_i].version;
                memcpy(out->version_str, ver, 9);
                memcpy(out->date_str, date, 9);
                memcpy(out->product_str, prod, 9);
                memcpy(out->subsystem_str, subs, 9);
                out->identified_at_offset = base;
                return 1;
            }
        }
        /* Product+subsystem matched but version/date didn't — record
         * the strings so a caller can log what it saw, but classify
         * as UNKNOWN so no downstream code assumes compatibility. */
        out->version = DM1_V1_FMTOWNS_TBIOS_UNKNOWN;
        memcpy(out->version_str, ver, 9);
        memcpy(out->date_str, date, 9);
        memcpy(out->product_str, prod, 9);
        memcpy(out->subsystem_str, subs, 9);
        out->identified_at_offset = base;
        return 0;
    }
    return 0;
}

dm1_v1_fmtowns_tbios_version_t
dm1_v1_fmtowns_tbios_required_for_dm1_hma240_pc34(void) {
    /* DM1 HMA-240 is a 1992 disc; the earliest TBIOS that ships every
     * subfunction it uses is V31L31_92. Later versions retain the
     * same ABI at these subfunction slots per Tsugaru's audits. */
    return DM1_V1_FMTOWNS_TBIOS_V31L31_92;
}

int dm1_v1_fmtowns_tbios_meets_pc34(
        dm1_v1_fmtowns_tbios_version_t have,
        dm1_v1_fmtowns_tbios_version_t need) {
    if (have == DM1_V1_FMTOWNS_TBIOS_UNKNOWN) return 0;
    if (need == DM1_V1_FMTOWNS_TBIOS_UNKNOWN) return 0;
    return (int)have >= (int)need;
}
