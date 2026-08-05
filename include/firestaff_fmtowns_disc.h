#ifndef FIRESTAFF_FMTOWNS_DISC_H
#define FIRESTAFF_FMTOWNS_DISC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shared FM Towns disc image reader for DM1 and DM2.
 * Handles both MODE1/2048 (DM1) and MODE1/2352 (DM2) sector formats.
 * ISO 9660 filesystem traversal and CDDA track extraction. */

typedef enum {
    FMTOWNS_SECTOR_2048,  /* MODE1/2048: pure data sectors (DM1) */
    FMTOWNS_SECTOR_2352   /* MODE1/2352: raw sectors with sync/header/ECC (DM2) */
} FmtownsSectorFormat;

#define FMTOWNS_CDDA_SECTOR_SIZE     2352u
#define FMTOWNS_CDDA_SAMPLE_RATE    44100u
#define FMTOWNS_CDDA_CHANNELS          2u
#define FMTOWNS_CDDA_BITS_PER_SAMPLE  16u

typedef struct {
    char     name[64];
    uint32_t lba;
    uint32_t size;
    int      is_directory;
} FmtownsIsoEntry;

#define FMTOWNS_MAX_ISO_ENTRIES 32

typedef struct {
    int                  valid;
    FmtownsSectorFormat  sector_format;
    char                 volume_id[33];
    char                 system_id[33];
    uint32_t             volume_sectors;
    int                  entry_count;
    FmtownsIsoEntry      entries[FMTOWNS_MAX_ISO_ENTRIES];
} FmtownsDiscProbeResult;

/* Probe an ISO 9660 disc image. Scans root directory and up to one
 * level of subdirectories. Returns 0 on success, -1 on failure. */
int fmtowns_disc_probe(const uint8_t *image, size_t image_size,
                       FmtownsSectorFormat format,
                       FmtownsDiscProbeResult *out);

/* Find a file entry by name (case-insensitive, strips ";1" version).
 * Returns pointer to entry within result, or NULL. */
const FmtownsIsoEntry *fmtowns_disc_find(const FmtownsDiscProbeResult *result,
                                         const char *name);

/* Extract a file from the disc image into a malloc'd buffer.
 * Returns 0 on success, -1 on failure. Caller frees *out_data. */
int fmtowns_disc_extract_alloc(const uint8_t *image, size_t image_size,
                               FmtownsSectorFormat format,
                               const FmtownsIsoEntry *entry,
                               uint8_t **out_data, size_t *out_size);

/* Extract raw CDDA PCM data for a range of sectors.
 * CDDA sectors are always 2352 bytes of raw audio regardless of data
 * track format. Returns 0 on success, -1 on failure. Caller frees. */
int fmtowns_cdda_extract(const uint8_t *image, size_t image_size,
                         uint32_t start_sector, uint32_t sector_count,
                         uint8_t **out_pcm, size_t *out_pcm_size);

/* Parse a CUE sheet to extract INDEX 01 sector offsets for each track.
 * track_starts must be at least max_tracks entries.
 * Returns the number of tracks found, or -1 on error. */
int fmtowns_cue_parse_track_starts(const char *cue, size_t cue_size,
                                   uint32_t *track_starts, int max_tracks);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FMTOWNS_DISC_H */
