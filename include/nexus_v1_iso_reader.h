
#ifndef NEXUS_V1_ISO_READER_H
#define NEXUS_V1_ISO_READER_H
#include <stdint.h>
#include <stdio.h>

/* Read DM Nexus game files directly from Saturn CUE/BIN disc image.
 * No manual extraction needed — Firestaff opens the ISO at runtime.
 *
 * Supports: MODE1/2352 (raw Saturn CD format).
 * Track 1 = data (ISO 9660), Track 2-9 = audio (Red Book). */

#define NEXUS_ISO_SECTOR_SIZE 2352
#define NEXUS_ISO_DATA_OFFSET 16
#define NEXUS_ISO_DATA_SIZE 2048
#define NEXUS_ISO_MAX_FILES 256

typedef struct {
    char name[64];
    uint32_t lba;
    uint32_t size;
    int is_dir;
} Nexus_ISOFile;

typedef struct {
    FILE *fp;
    uint8_t *memory;
    size_t memory_size;
    char path[512];
    Nexus_ISOFile files[NEXUS_ISO_MAX_FILES];
    int file_count;
    int valid;
    int sector_size;
    int data_offset;
} Nexus_ISOReader;

typedef struct {
    int valid;
    int declared_file_count;
    int present_file_count;
    int missing_file_count;
} Nexus_ISO_CueMediaReceipt;

/* Open a Saturn BIN file (Track 1) and parse the ISO 9660 filesystem */
int nexus_iso_open(Nexus_ISOReader *reader, const char *bin_path);
int nexus_iso_open_memory(Nexus_ISOReader *reader, uint8_t *data,
                          size_t data_size, const char *source_name);

/* Open from CUE file (finds Track 1 BIN automatically) */
int nexus_iso_open_cue(Nexus_ISOReader *reader, const char *cue_path);

/* Recover the owning CUE for an already-selected Nexus data track.  The
 * result is admitted only when exactly one sibling CUE opens that exact
 * physical payload as its Nexus ISO track; a merely similarly named CUE is
 * never substituted.  This preserves the original CDDA declarations when a
 * launcher first matched Track 1 rather than the CUE itself. */
int nexus_iso_find_cue_for_data_track(const char *data_track_path,
                                      char *out_cue_path,
                                      int out_cue_path_size);

/* Check every FILE payload named by a CUE sheet. This is deliberately
 * separate from nexus_iso_open_cue(): a valid ISO data track does not prove
 * that external CDDA payloads are present. */
int nexus_iso_cue_media_receipt(const char *cue_path,
                                Nexus_ISO_CueMediaReceipt *out);

/* Resolve one declared Red Book AUDIO track to its original CUE payload.
 * This is a source binding only: callers receive the referenced BIN path but
 * no PCM decoder or host playback permission.  Returns 0 only when exactly
 * one AUDIO declaration for `track_number` names a readable payload; -1
 * covers malformed/missing/ambiguous tracks and bad arguments. */
int nexus_iso_cue_audio_track_path(const char *cue_path,
                                   int track_number,
                                   char *out_path,
                                   int out_path_size);

/* Resolve a declared AUDIO track from a CUE stored inside its original ZIP.
 * The returned ``archive.zip::member.bin`` is provenance only: it names the
 * validated archive member and never materializes it to disk. */
int nexus_iso_zip_cue_audio_track_path(const char *zip_path,
                                       int track_number,
                                       char *out_path,
                                       int out_path_size);

/* Find a file by name (case-insensitive) */
const Nexus_ISOFile *nexus_iso_find(const Nexus_ISOReader *reader, const char *name);

/* Read file data into buffer */
int nexus_iso_read_file(Nexus_ISOReader *reader, const Nexus_ISOFile *file,
    uint8_t *buffer, int buffer_size);

/* Read file data with streaming (for large files) */
int nexus_iso_read_file_chunk(Nexus_ISOReader *reader, const Nexus_ISOFile *file,
    int offset, uint8_t *buffer, int chunk_size);

/* List all files */
int nexus_iso_file_count(const Nexus_ISOReader *reader);

/* Close */
void nexus_iso_close(Nexus_ISOReader *reader);

/* Validate: check if this is a DM Nexus disc */
int nexus_iso_is_nexus(const Nexus_ISOReader *reader);

#endif
