
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
    char path[512];
    Nexus_ISOFile files[NEXUS_ISO_MAX_FILES];
    int file_count;
    int valid;
} Nexus_ISOReader;

/* Open a Saturn BIN file (Track 1) and parse the ISO 9660 filesystem */
int nexus_iso_open(Nexus_ISOReader *reader, const char *bin_path);

/* Open from CUE file (finds Track 1 BIN automatically) */
int nexus_iso_open_cue(Nexus_ISOReader *reader, const char *cue_path);

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
