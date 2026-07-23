#include "csb_v1_f0726_f0745_media_filename_raw_pc34_compat.h"

#include <string.h>

typedef struct CSB_V1_MediaFilenameSpecPc34 {
    CSB_V1_MediaFilenameFunctionPc34 id;
    unsigned graphics : 1, palette : 1, zone : 1, music : 1, package : 1, names : 1;
    const char *evidence;
} CSB_V1_MediaFilenameSpecPc34;

static const CSB_V1_MediaFilenameSpecPc34 k_specs[] = {
    { CSB_V1_MEDIA_FILENAME_F0731, 1, 0, 1, 0, 0, 0, "ReDMCSB INVRTZON.C F0731 InvertZone" },
    { CSB_V1_MEDIA_FILENAME_F0732, 1, 0, 1, 0, 0, 0, "ReDMCSB FILLBOX.C F0732 FillScreenArea" },
    { CSB_V1_MEDIA_FILENAME_F0733, 1, 0, 1, 0, 0, 0, "ReDMCSB FILLBOX.C F0733 FillZoneByIndex" },
    { CSB_V1_MEDIA_FILENAME_F0734, 1, 0, 1, 0, 0, 0, "ReDMCSB PANEL.C F0734 ClearZoneInInventory" },
    { CSB_V1_MEDIA_FILENAME_F0735, 1, 0, 1, 0, 0, 0, "ReDMCSB FILLBOX.C F0735 FillViewportBox" },
    { CSB_V1_MEDIA_FILENAME_F0738, 0, 0, 0, 1, 1, 0, "ReDMCSB MUSIC.C F0738 MUSIC_Continue" },
    { CSB_V1_MEDIA_FILENAME_F0739, 0, 0, 0, 1, 1, 0, "ReDMCSB MUSIC.C F0739 MUSIC_Stop" },
    { CSB_V1_MEDIA_FILENAME_F0740, 0, 0, 0, 1, 1, 0, "ReDMCSB MUSIC.C F0740 MUSIC_Pause" },
    { CSB_V1_MEDIA_FILENAME_F0741, 0, 0, 0, 1, 1, 0, "ReDMCSB MUSIC.C F0741 MUSIC_PlayGameMusic" },
    { CSB_V1_MEDIA_FILENAME_F0742, 0, 0, 0, 1, 1, 0, "ReDMCSB MUSIC.C F0742 MUSIC_Play" },
    { CSB_V1_MEDIA_FILENAME_F0743, 0, 0, 0, 1, 1, 0, "ReDMCSB MUSIC.C F0743 MUSIC_Fade" },
    { CSB_V1_MEDIA_FILENAME_F0744, 0, 0, 0, 0, 1, 1, "ReDMCSB FILENAME.C F0744 ReplaceTildeInFileName" },
    { CSB_V1_MEDIA_FILENAME_F0745, 0, 0, 0, 0, 1, 1, "ReDMCSB FILENAME.C F0745 SetFileNamesAccordingToLanguage" }
};

static int has(const uint8_t *bytes, size_t size, uint32_t identity) {
    return bytes != NULL && size != 0 && identity != 0;
}

int csb_v1_f0726_f0745_media_filename_audit_pc34(const CSB_V1_MediaFilenameRawMaterialPc34 *raw, CSB_V1_MediaFilenameFunctionPc34 id, CSB_V1_MediaFilenameAuditReceiptPc34 *out) {
    size_t index;
    const CSB_V1_MediaFilenameSpecPc34 *spec = NULL;
    if (out == NULL) return 0;
    memset(out, 0, sizeof(*out));
    for (index = 0; index < sizeof(k_specs) / sizeof(k_specs[0]); ++index)
        if (k_specs[index].id == id) { spec = &k_specs[index]; break; }
    if (spec == NULL || raw == NULL || !raw->authenticated_pc34 ||
        (spec->graphics && !has(raw->graphics, raw->graphics_size, raw->graphics_identity)) ||
        (spec->palette && !has(raw->palette, raw->palette_size, raw->palette_identity)) ||
        (spec->zone && !has(raw->zone, raw->zone_size, raw->zone_identity)) ||
        (spec->music && !has(raw->music, raw->music_size, raw->music_identity)) ||
        (spec->package && !has(raw->package, raw->package_size, raw->package_identity)) ||
        (spec->names && !has(raw->file_names, raw->file_names_size, raw->file_names_identity))) return 0;
    out->raw_material_admitted = 1;
    out->existing_runtime_owner_preserved = 1;
    out->graphics_required = spec->graphics; out->palette_required = spec->palette;
    out->zone_required = spec->zone; out->music_required = spec->music;
    out->package_required = spec->package; out->file_names_required = spec->names;
    out->read_only_query = 1; out->runtime_execution_blocked = 1;
    out->platform_behavior_fail_closed = 1; out->function_id = id;
    out->source_evidence = spec->evidence;
    return 1;
}
