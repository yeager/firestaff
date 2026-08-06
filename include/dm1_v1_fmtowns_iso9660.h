#ifndef DM1_V1_FMTOWNS_ISO9660_H
#define DM1_V1_FMTOWNS_ISO9660_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FM Towns DM1 ISO 9660 data track file extractor.
 *
 * The FM Towns DM1 disc (HMA-240) has a MODE1/2048 ISO 9660 data track
 * with this directory structure:
 *
 *   /DATA/GRAPHICS.DAT      (396970 bytes, English)
 *   /DATA/DUNGEON.DAT       (33423 bytes, English)
 *   /JDATA/GRAPHICS.DAT     (396407 bytes, Japanese)
 *   /JDATA/DUNGEON.DAT      (33931 bytes, Japanese)
 *   /EDM.EXP                (310518 bytes, English executable)
 *   /JDM.EXP                (290221 bytes, Japanese executable)
 *
 * Audio tracks 02-20 provide CD audio music.
 *
 *   /AUTOEXEC.BAT             (original boot route)
 *   /TMENU.EXP/.ICN/.INF      (original title/menu program and media)
 *
 * This module inventories source game data and startup/menu files from the raw data track bytes
 * (sector-aligned MODE1/2048 data, as found in a BIN/CUE image where
 * Track 01 is MODE1/2048). It does NOT perform CUE sheet parsing (use
 * firestaff_fmtowns_cd_classify.h for that). */

#define DM1_FMTOWNS_ISO_SECTOR_SIZE    2048U
#define DM1_FMTOWNS_ISO_PVD_SECTOR     16U
#define DM1_FMTOWNS_ISO_MAX_FILES      12
#define DM1_FMTOWNS_ISO_MAX_NAME_LEN   32

typedef struct {
    char     name[DM1_FMTOWNS_ISO_MAX_NAME_LEN];
    uint32_t lba;
    uint32_t size;
    int      is_directory;
} DM1_V1_FmtownsIsoEntry;

typedef struct {
    char     system_id[33];
    char     volume_id[33];
    uint32_t volume_space_size;
    uint32_t root_dir_lba;
    uint32_t root_dir_size;
    int      file_count;
    DM1_V1_FmtownsIsoEntry files[DM1_FMTOWNS_ISO_MAX_FILES];
} DM1_V1_FmtownsIsoLayout;

/* Probe whether the data track looks like the FM Towns DM1 ISO.
 * Checks for CD001 PVD at sector 16 and system ID "HMA-240".
 * data_track points to the start of the MODE1/2048 data track.
 * Returns 1 if recognized. */
int dm1_v1_fmtowns_iso_probe(const uint8_t *data_track, size_t size);

/* Parse the ISO 9660 directory structure and locate the original game-data
 * and startup/menu owners.  Alongside DATA/JDATA GRAPHICS.DAT/DUNGEON.DAT,
 * layout->files retains AUTOEXEC.BAT, EDM.EXP, JDM.EXP and TMENU's EXP/ICN/INF
 * records so callers can bind the FM Towns sequence without a PC34 substitute.
 * Returns 0 on success, -1 on error. */
int dm1_v1_fmtowns_iso_parse(const uint8_t *data_track, size_t size,
                              DM1_V1_FmtownsIsoLayout *out);

/* Extract a file from the data track by its entry.
 * Copies entry->size bytes from the data track at entry->lba into out_buf.
 * Returns 0 on success, -1 if out_buf_size is too small or entry is invalid. */
int dm1_v1_fmtowns_iso_extract(const uint8_t *data_track, size_t track_size,
                                const DM1_V1_FmtownsIsoEntry *entry,
                                uint8_t *out_buf, size_t out_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_ISO9660_H */
