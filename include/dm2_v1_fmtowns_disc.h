#ifndef DM2_V1_FMTOWNS_DISC_H
#define DM2_V1_FMTOWNS_DISC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FM Towns disc image extractor for DM2 (Victor HME-242).
 *
 * The FM Towns DM2 disc is a mixed-mode CD with:
 *   Track 1: MODE1/2352 data track containing an ISO 9660 filesystem
 *   Tracks 2-8: CDDA audio tracks (in-game music)
 *
 * Data track layout (2352 bytes/sector, raw):
 *   Sync (12) + Header (4) + UserData (2048) + ECC/EDC (288)
 *
 * The ISO 9660 filesystem contains:
 *   DATA/CD.DAT        (40 bytes)   — CDDA track table
 *   DATA/DUNGEON.DAT   (37954 bytes) — dungeon data
 *   DATA/GRAPHICS.DAT  (2783791 bytes) — GDAT v4 assets
 *
 * Source: ISO 9660 primary volume descriptor analysis of HME-242 disc image.
 * Volume ID: "DUNGEONMASTER2_SKULLKEEP", System ID: "HME-242". */

#define DM2_FMTOWNS_SECTOR_SIZE         2352u
#define DM2_FMTOWNS_SECTOR_HEADER_SIZE    16u
#define DM2_FMTOWNS_SECTOR_DATA_SIZE   2048u

/* ISO 9660 file entry extracted from the disc image. */
typedef struct {
    char     name[64];
    uint32_t lba;
    uint32_t size;
    int      is_directory;
} DM2_V1_FmtownsIsoEntry;

/* Result of probing a disc image for DM2 FM Towns game files. */
typedef struct {
    int      valid;
    char     volume_id[33];
    char     system_id[33];
    uint32_t data_track_sectors;

    DM2_V1_FmtownsIsoEntry cd_dat;
    DM2_V1_FmtownsIsoEntry dungeon_dat;
    DM2_V1_FmtownsIsoEntry graphics_dat;

    int has_cd_dat;
    int has_dungeon_dat;
    int has_graphics_dat;
} DM2_V1_FmtownsDiscReceipt;

/* Probe a raw MODE1/2352 disc image for the DM2 FM Towns ISO 9660 filesystem.
 * The image must start at sector 0 of the data track.
 * Returns 0 on success and fills receipt, -1 on failure. */
int dm2_v1_fmtowns_disc_probe(const uint8_t *image, size_t image_size,
                              DM2_V1_FmtownsDiscReceipt *out);

/* Extract a file from the disc image into a caller-allocated buffer.
 * entry must come from a successful probe.
 * buf must be at least entry->size bytes.
 * Returns 0 on success, -1 on failure. */
int dm2_v1_fmtowns_disc_extract(const uint8_t *image, size_t image_size,
                                const DM2_V1_FmtownsIsoEntry *entry,
                                uint8_t *buf, size_t buf_size);

/* Extract a file from the disc image, allocating the buffer.
 * Caller must free(*out_data) on success.
 * Returns 0 on success, -1 on failure. */
int dm2_v1_fmtowns_disc_extract_alloc(const uint8_t *image, size_t image_size,
                                      const DM2_V1_FmtownsIsoEntry *entry,
                                      uint8_t **out_data, size_t *out_size);

/* CDDA audio track layout from the CUE sheet.
 * Track 1 = data, Tracks 2-8 = CDDA audio (raw 16-bit stereo 44100Hz).
 * CDDA sectors use the full 2352 bytes for audio (no header/ECC). */

#define DM2_FMTOWNS_CDDA_FIRST_TRACK    2u
#define DM2_FMTOWNS_CDDA_LAST_TRACK     8u
#define DM2_FMTOWNS_CDDA_SAMPLE_RATE    44100u
#define DM2_FMTOWNS_CDDA_CHANNELS       2u
#define DM2_FMTOWNS_CDDA_BITS           16u
#define DM2_FMTOWNS_CDDA_BYTES_PER_SEC  176400u

typedef struct {
    uint8_t  disc_track;
    uint32_t start_sector;
    uint32_t sector_count;
    uint32_t pcm_bytes;
    float    duration_sec;
} DM2_V1_FmtownsCddaTrackInfo;

/* Get CDDA track info for disc track number (2-8).
 * data_track_end_sector is the first sector of track 2 (from CUE or probe).
 * total_disc_sectors is the total sector count of the disc image.
 * Returns 0 on success, -1 if track_num is out of range. */
int dm2_v1_fmtowns_cdda_track_info(int track_num,
                                   const uint32_t *track_start_sectors,
                                   int track_count,
                                   uint32_t total_disc_sectors,
                                   DM2_V1_FmtownsCddaTrackInfo *out);

/* Extract raw CDDA PCM data for a disc track.
 * Returns 0 on success, -1 on failure.
 * Caller must free(*out_pcm) on success. */
int dm2_v1_fmtowns_cdda_extract(const uint8_t *image, size_t image_size,
                                uint32_t start_sector, uint32_t sector_count,
                                uint8_t **out_pcm, size_t *out_pcm_size);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_FMTOWNS_DISC_H */
