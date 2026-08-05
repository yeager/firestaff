#ifndef CSB_V1_FMTOWNS_CD_H
#define CSB_V1_FMTOWNS_CD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB FM Towns CD image parser.
 *
 * The FM Towns CSB disc has:
 *   Track 01: MODE1/2352 data track with ISO 9660 filesystem
 *   Tracks 02-31: CDDA audio (30 tracks)
 *
 * ISO 9660 filesystem ("CHAOS" volume):
 *   CDATA/GRAPHICS.DAT, CDATA/DUNGEON.DAT, CDATA/MINI.DAT  (English)
 *   CJDATA/GRAPHICS.DAT, CJDATA/DUNGEON.DAT, CJDATA/MINI.DAT (Japanese)
 *   PORTRAIT/{name}.CMP  (24 champion portraits)
 *   TITLE.ANM, STORY.ANM, ENDING.ANM  (animations)
 *   CHTWE.EXP, CHTWJ.EXP  (game executables)
 *
 * MODE1/2352 raw sectors: 12 sync + 4 header + 2048 data + 288 EDC/ECC.
 * The 2048-byte user data payload starts at offset 16 in each sector.
 */

#define CSB_FMTOWNS_CD_RAW_SECTOR_SIZE     2352u
#define CSB_FMTOWNS_CD_COOKED_SECTOR_SIZE  2048u
#define CSB_FMTOWNS_CD_SECTOR_DATA_OFFSET  16u
#define CSB_FMTOWNS_CD_PVD_SECTOR          16u
#define CSB_FMTOWNS_CD_MAX_FILES           64
#define CSB_FMTOWNS_CD_MAX_NAME_LEN        32
#define CSB_FMTOWNS_CD_CDDA_FIRST_TRACK    2u
#define CSB_FMTOWNS_CD_CDDA_LAST_TRACK     31u
#define CSB_FMTOWNS_CD_CDDA_TRACK_COUNT    30u

typedef struct {
    char     name[CSB_FMTOWNS_CD_MAX_NAME_LEN];
    char     parent[CSB_FMTOWNS_CD_MAX_NAME_LEN];
    uint32_t lba;
    uint32_t size;
    int      is_directory;
} CSB_V1_FmtownsCdFile;

typedef struct {
    char     volume_id[33];
    uint32_t data_track_sectors;
    int      file_count;
    CSB_V1_FmtownsCdFile files[CSB_FMTOWNS_CD_MAX_FILES];
    int      is_raw_2352;
} CSB_V1_FmtownsCdLayout;

typedef struct {
    uint32_t track_number;
    uint32_t start_sector;
    uint32_t sector_count;
    uint32_t byte_offset;
    uint32_t byte_length;
} CSB_V1_FmtownsCddaTrack;

typedef struct {
    int      valid;
    int      track_count;
    CSB_V1_FmtownsCddaTrack tracks[CSB_FMTOWNS_CD_CDDA_TRACK_COUNT];
} CSB_V1_FmtownsCddaLayout;

/* Probe whether a BIN image is the CSB FM Towns CD (MODE1/2352).
 * Checks for ISO 9660 PVD with "CHAOS" volume ID. */
int csb_v1_fmtowns_cd_probe(const uint8_t *bin_data, size_t bin_size);

/* Parse the ISO 9660 directory structure from a MODE1/2352 BIN image.
 * Returns 0 on success, -1 on error. */
int csb_v1_fmtowns_cd_parse(const uint8_t *bin_data, size_t bin_size,
                             CSB_V1_FmtownsCdLayout *out);

/* Extract a file from the BIN image by its entry.
 * Reads from raw 2352-byte sectors, extracting the 2048-byte user data.
 * Returns 0 on success, -1 on error. */
int csb_v1_fmtowns_cd_extract(const uint8_t *bin_data, size_t bin_size,
                               const CSB_V1_FmtownsCdFile *entry,
                               uint8_t *out_buf, size_t out_buf_size);

/* Find a file by name (case-insensitive, without ISO version suffix).
 * Returns pointer to the entry in layout, or NULL if not found. */
const CSB_V1_FmtownsCdFile *csb_v1_fmtowns_cd_find(
    const CSB_V1_FmtownsCdLayout *layout,
    const char *parent_dir,
    const char *filename);

/* Parse CDDA track layout from a CUE sheet text buffer.
 * Returns 0 on success, -1 on parse error. */
int csb_v1_fmtowns_cdda_parse_cue(const char *cue_text, size_t cue_len,
                                   CSB_V1_FmtownsCddaLayout *out);

/* Extract raw CDDA PCM data for one audio track from the BIN image.
 * Audio is 16-bit signed LE stereo at 44100 Hz. A zero sector_count denotes
 * the final CUE track and is resolved to the raw image end at extraction.
 * Returns the number of bytes written, or -1 on error. */
int csb_v1_fmtowns_cdda_extract(const uint8_t *bin_data, size_t bin_size,
                                 const CSB_V1_FmtownsCddaTrack *track,
                                 uint8_t *out_buf, size_t out_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_CD_H */
