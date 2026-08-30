#ifndef FIRESTAFF_DM1_PROBE_DATA_DIR_H
#define FIRESTAFF_DM1_PROBE_DATA_DIR_H

/*
 * Shared PC 3.4 data-directory resolver for the DM1 real-data probes.
 *
 * ctest invokes these probes with FIRESTAFF_DM1_WORKSPACE_DATA_DIR, which
 * defaults to ~/.firestaff/data/dm1 -- the documented workspace root. That
 * root holds the distribution archives plus extracted trees such as
 * dos_extract/<release>/DATA, NOT loose DAT files. A probe that uses the
 * argument verbatim therefore loads no dungeon at all and every downstream
 * assertion fails against an empty framebuffer or an unloaded world.
 *
 * Thirteen probes each carried their own copy of this walk and the candidate
 * lists had drifted apart: some tried DATA and the dos_extract layouts, some
 * only tried "<dir>/dm1", and one had no resolver whatsoever. Three separate
 * probe failures during the 2026-08-09 sweep traced back to exactly that
 * divergence, each presenting as a different symptom (empty D1C portrait
 * rect, missing mirror ordinal, blank damage-indicator zone). This header is
 * the single owner of the candidate list so a layout that works for one
 * probe works for all of them.
 *
 * Measured effect when this landed: the four probes whose lists were too
 * short -- dialog_choice_overlay_fit, fullscreen_map_font_scale_fit,
 * utility_panel_font_scale_fit and inventory_panel_font_scale_fit -- went
 * from red to green, because they could finally locate the extracted DOS
 * data.  (The consolidation commit reported them as still failing; that
 * measurement was taken against stale binaries that had not yet been rebuilt
 * against this header.)
 *
 * The resolver never invents data: it only selects a directory that actually
 * contains both GRAPHICS.DAT and DUNGEON.DAT, and returns the caller's
 * original argument unchanged when no candidate qualifies, leaving the
 * probe's own missing-data diagnostic to fire.
 */

#include <stdio.h>
#include <stddef.h>

static int firestaff_dm1_probe_file_exists(const char *path) {
    FILE *f;
    if (!path || !path[0]) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* A directory qualifies only when both canonical PC 3.4 payloads are present. */
static int firestaff_dm1_probe_is_pc34_data_dir(const char *path) {
    char graphicsPath[1024];
    char dungeonPath[1024];
    if (!path || !path[0]) return 0;
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/GRAPHICS.DAT", path);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", path);
    return firestaff_dm1_probe_file_exists(graphicsPath) &&
           firestaff_dm1_probe_file_exists(dungeonPath);
}

/*
 * Resolve `dataDir` to a directory holding GRAPHICS.DAT and DUNGEON.DAT.
 *
 * Returns `out` (filled) when a candidate qualifies, otherwise returns
 * `dataDir` unchanged. `out` must have room for `outSize` bytes.
 */
static const char *firestaff_dm1_probe_narrow_data_dir(const char *dataDir,
                                                       char *out,
                                                       size_t outSize) {
    /* Union of every list that had drifted across the individual probes,
     * ordered most-specific-first so an explicit DATA directory always wins
     * over a release root. */
    static const char *const kCandidates[] = {
        "DATA",
        /* The shared workspace also carries the authenticated multilingual
         * PC 3.4 installation beside the archive collection.  Runtime
         * probes are invoked both with the workspace root and its dm1 child;
         * keep this real-data location in the common resolver instead of
         * letting individual probes render against an unloaded game view. */
        "dm1-multilingual",
        "../dm1-multilingual",
        "dm1",
        "dm1/DATA",
        "dos_extract/Dungeon-Master_DOS_EN_Version-34/DATA",
        "dos_extract/Dungeon-Master_DOS_EN_Version-34",
        "dm1/dos_extract/Dungeon-Master_DOS_EN_Version-34/DATA",
        "dm1/dos_extract/Dungeon-Master_DOS_EN_Version-34",
        NULL
    };
    size_t i;

    if (!dataDir || !out || outSize == 0U) return dataDir;

    if (firestaff_dm1_probe_is_pc34_data_dir(dataDir)) {
        snprintf(out, outSize, "%s", dataDir);
        return out;
    }
    for (i = 0; kCandidates[i] != NULL; ++i) {
        char candidate[1024];
        snprintf(candidate, sizeof(candidate), "%s/%s", dataDir, kCandidates[i]);
        if (firestaff_dm1_probe_is_pc34_data_dir(candidate)) {
            snprintf(out, outSize, "%s", candidate);
            return out;
        }
    }
    return dataDir;
}

#endif /* FIRESTAFF_DM1_PROBE_DATA_DIR_H */
