#include "nexus_v1_multi_level_capture_campaign_launcher.h"
#include "nexus_v1_iso_reader.h"
#include "firestaff_x68k_media_receipt.h"
#include "firestaff_zip_extract.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static int sha256(const char *s) {
    size_t i;
    if (!s || strlen(s) != 64U) return 0;
    for (i = 0; i < 64U; ++i)
        if (!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f') ||
              (s[i] >= 'A' && s[i] <= 'F'))) return 0;
    return 1;
}
/* Retail media is read only and verified by Firestaff's native SHA-256 and
 * ZIP/ISO readers.  No host checksum or extraction tool is required. */
static int hash_file(const char *path, const char *expected) {
    char container[512]; const char *entry;
    struct stat st;
    if (!path || !expected || strlen(path) > 900U) return 0;
    entry = strstr(path, "::");
    if (entry) {
        size_t n = (size_t)(entry - path);
        size_t zip_size = 0u;
        Nexus_ISOReader iso; const Nexus_ISOFile *member; uint8_t *bytes = NULL;
        char digest[65]; int opened;
        if (!n || n >= sizeof(container) || !entry[2]) return 0;
        memcpy(container, path, n); container[n] = '\0';
        if (strstr(container, ".cue") || strstr(container, ".bin") || strstr(container, ".iso")) {
            memset(&iso, 0, sizeof(iso));
            opened = strstr(container, ".cue") ? nexus_iso_open_cue(&iso, container) : nexus_iso_open(&iso, container);
            if (opened <= 0 || !(member = nexus_iso_find(&iso, entry + 2)) || member->size == 0U ||
                !(bytes = (uint8_t *)malloc(member->size)) ||
                nexus_iso_read_file(&iso, member, bytes, (int)member->size) != (int)member->size ||
                firestaff_x68k_media_receipt_sha256_hex(bytes, member->size, digest, sizeof(digest)) != 0) {
                free(bytes);
                nexus_iso_close(&iso);
                return 0;
            }
            free(bytes); nexus_iso_close(&iso); return strcmp(digest, expected) == 0;
        }
        if (stat(container, &st) != 0 || !S_ISREG(st.st_mode) ||
            firestaff_zip_extract_by_name(container, entry + 2, &bytes,
                                          &zip_size) != 0) return 0;
        opened = firestaff_x68k_media_receipt_sha256_hex(bytes, zip_size, digest,
                                                          sizeof(digest)) == 0 &&
                 strcmp(digest, expected) == 0;
        free(bytes);
        return opened;
    } else {
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) return 0;
        return firestaff_x68k_media_receipt_sha256_file_hex(
                   path, container, sizeof(container)) == 0 &&
               strcmp(container, expected) == 0;
    }
}
int nexus_v1_multi_level_capture_campaign_launcher_plan(
    const Nexus_V1_MultiLevelCaptureCampaignLauncherInput *in,
    Nexus_V1_MultiLevelCaptureCampaignLaunchPlan *out)
{
    Nexus_V1_MultiLevelCaptureCampaignLaunchPlan p;
    uint32_t i; size_t j;
    memset(&p, 0, sizeof(p)); p.operator_only = 1;
    if (!out || !in) return 0;
    if (!in->retail_assets_available) { p.skipped_missing_retail_assets = 1; *out = p; return 0; }
    if (!in->operator_opt_in || !in->bios_path || !in->disc_path ||
        !in->menu_bpk_path || !in->dm_bin_path || !sha256(in->bios_sha256) ||
        !sha256(in->disc_sha256) || !sha256(in->menu_bpk_sha256) || !sha256(in->dm_bin_sha256) ||
        !hash_file(in->bios_path, in->bios_sha256) || !hash_file(in->disc_path, in->disc_sha256) ||
        !hash_file(in->menu_bpk_path, in->menu_bpk_sha256) || !hash_file(in->dm_bin_path, in->dm_bin_sha256) ||
        !in->direct_lev_corpus || !in->direct_lev_corpus->valid ||
        !in->direct_lev_corpus->direct_files_only ||
        in->direct_lev_corpus->payload_materialized ||
        !in->dgn || !in->dgn->valid || !in->dgn->opaque_original_capture_only ||
        in->dgn->decoder_promoted || in->dgn->mesh_semantics_permitted || in->dgn->render_permitted ||
        !in->slev || !in->slev->valid || !in->slev->no_dispatch_only ||
        !in->sal || !in->sal->valid || !in->sal->no_playback_only) { *out = p; return 0; }
    memcpy(p.manifest_sha256, in->disc_sha256, 65U);
    for (i = 0; i < 16U; ++i) {
        const Nexus_V1_DgnMultiLevelCaptureCoverage *d = &in->dgn->levels[i];
        const Nexus_V1_LevCorpusDirectLevelIdentity *lev =
            &in->direct_lev_corpus->levels[i];
        const Nexus_V1_SlevTaskBodyCaptureTarget *s = &in->slev->targets[i];
        const Nexus_V1_SalCapturePlanTarget *sal = NULL;
        if (!lev->valid || lev->level_index != i || !lev->byte_count ||
            !lev->fnv1a64 || !lev->md5[0] || !d->valid ||
            !d->opaque_original_capture_covered ||
            d->dgn_fnv1a64 != lev->fnv1a64 || !d->dgn_fnv1a64 ||
            !d->trace_fnv1a64 || !d->trace_size || !s->valid || s->level_index != i) { *out = p; return 0; }
        for (j = 0; j < in->sal->target_count; ++j)
            if (in->sal->targets[j].valid && in->sal->targets[j].level_index == i) { sal = &in->sal->targets[j]; break; }
        if (!sal) { *out = p; return 0; }
        p.jobs[i].valid = 1; p.jobs[i].level_index = i; p.jobs[i].dgn_fnv1a64 = d->dgn_fnv1a64;
        p.jobs[i].trace_fnv1a64 = d->trace_fnv1a64; p.jobs[i].trace_size = d->trace_size;
        p.jobs[i].slev_source_fnv1a64 = s->source_fnv1a64; p.jobs[i].sal_selector = sal->raw_map_selector;
    }
    p.valid = 1; *out = p; return 1;
}
