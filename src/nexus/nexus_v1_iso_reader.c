
#include "nexus_v1_iso_reader.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef _WIN32
#define strcasecmp _stricmp
#endif

/* Read one sector's data payload from MODE1/2352 BIN */
static int read_sector(FILE *fp, uint32_t sector, uint8_t *buf) {
    long offset = (long)sector * NEXUS_ISO_SECTOR_SIZE + NEXUS_ISO_DATA_OFFSET;
    if (fseek(fp, offset, SEEK_SET) != 0) return -1;
    return (int)fread(buf, 1, NEXUS_ISO_DATA_SIZE, fp);
}

static uint32_t r32le(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}

/* Parse one ISO 9660 directory record */
static int parse_dir_record(const uint8_t *data, int offset, Nexus_ISOFile *out) {
    int rec_len, name_len, i;
    if (!data || !out) return 0;
    rec_len = data[offset];
    if (rec_len == 0) return 0;

    out->lba = r32le(data + offset + 2);
    out->size = r32le(data + offset + 10);
    out->is_dir = (data[offset + 25] & 0x02) != 0;

    name_len = data[offset + 32];
    memset(out->name, 0, sizeof(out->name));
    for (i = 0; i < name_len && i < 63; i++)
        out->name[i] = data[offset + 33 + i];

    /* Strip version (;1) */
    char *semi = strchr(out->name, ';');
    if (semi) *semi = 0;

    return rec_len;
}

/* Recursively parse directory tree */
static int parse_directory(FILE *fp, uint32_t dir_lba, uint32_t dir_size,
    Nexus_ISOFile *files, int *count, int max_files)
{
    uint8_t sector_buf[NEXUS_ISO_DATA_SIZE];
    int sectors = (dir_size + NEXUS_ISO_DATA_SIZE - 1) / NEXUS_ISO_DATA_SIZE;
    int s, offset;

    for (s = 0; s < sectors; s++) {
        if (read_sector(fp, dir_lba + s, sector_buf) < 0) return -1;

        offset = 0;
        while (offset < NEXUS_ISO_DATA_SIZE && *count < max_files) {
            Nexus_ISOFile entry;
            int rec_len = parse_dir_record(sector_buf, offset, &entry);
            if (rec_len == 0) break;

            if (entry.name[0] && entry.name[0] != 0 && entry.name[0] != 1) {
                if (!entry.is_dir) {
                    files[*count] = entry;
                    (*count)++;
                } else if (strcmp(entry.name, ".") != 0 && strcmp(entry.name, "..") != 0) {
                    /* Recurse into subdirectory */
                    parse_directory(fp, entry.lba, entry.size, files, count, max_files);
                }
            }
            offset += rec_len;
        }
    }
    return 0;
}

int nexus_iso_open(Nexus_ISOReader *reader, const char *bin_path) {
    uint8_t pvd[NEXUS_ISO_DATA_SIZE];
    uint32_t root_lba, root_size;

    if (!reader || !bin_path) return -1;
    memset(reader, 0, sizeof(*reader));
    strncpy(reader->path, bin_path, sizeof(reader->path) - 1);

    reader->fp = fopen(bin_path, "rb");
    if (!reader->fp) return -1;

    /* Read PVD at sector 16 */
    if (read_sector(reader->fp, 16, pvd) < 0) goto fail;
    if (pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) goto fail;

    root_lba = r32le(pvd + 158);
    root_size = r32le(pvd + 166);

    /* Parse file tree */
    reader->file_count = 0;
    parse_directory(reader->fp, root_lba, root_size,
        reader->files, &reader->file_count, NEXUS_ISO_MAX_FILES);

    reader->valid = 1;
    return reader->file_count;

fail:
    fclose(reader->fp);
    reader->fp = NULL;
    return -1;
}

int nexus_iso_open_cue(Nexus_ISOReader *reader, const char *cue_path) {
    /* Parse CUE to find Track 1 BIN file */
    FILE *cue;
    char line[512], bin_name[256] = {0};
    char bin_path[512];

    if (!reader || !cue_path) return -1;
    cue = fopen(cue_path, "r");
    if (!cue) return -1;

    while (fgets(line, sizeof(line), cue)) {
        /* Find: FILE "something.bin" BINARY */
        char *p = strstr(line, "FILE ");
        if (p) {
            char *q1 = strchr(p, '"');
            if (q1) {
                char *q2 = strchr(q1 + 1, '"');
                if (q2) {
                    int len = (int)(q2 - q1 - 1);
                    if (len > 0 && len < 255) {
                        memcpy(bin_name, q1 + 1, len);
                        bin_name[len] = 0;
                        break; /* Use first FILE entry (Track 1) */
                    }
                }
            }
        }
    }
    fclose(cue);

    if (!bin_name[0]) return -1;

    /* Build full path relative to CUE directory */
    strncpy(bin_path, cue_path, sizeof(bin_path) - 1);
    char *last_slash = strrchr(bin_path, '/');
    if (!last_slash) last_slash = strrchr(bin_path, '\\');
    if (last_slash) {
        strcpy(last_slash + 1, bin_name);
    } else {
        strncpy(bin_path, bin_name, sizeof(bin_path) - 1);
    }

    return nexus_iso_open(reader, bin_path);
}

const Nexus_ISOFile *nexus_iso_find(const Nexus_ISOReader *reader, const char *name) {
    int i;
    if (!reader || !name) return NULL;
    for (i = 0; i < reader->file_count; i++) {
        if (strcasecmp(reader->files[i].name, name) == 0)
            return &reader->files[i];
    }
    return NULL;
}

int nexus_iso_read_file(Nexus_ISOReader *reader, const Nexus_ISOFile *file,
    uint8_t *buffer, int buffer_size)
{
    uint8_t sector_buf[NEXUS_ISO_DATA_SIZE];
    int remaining, offset = 0;
    uint32_t sector;

    if (!reader || !reader->fp || !file || !buffer) return -1;
    if ((int)file->size > buffer_size) return -1;

    remaining = (int)file->size;
    sector = file->lba;

    while (remaining > 0) {
        int chunk = remaining > NEXUS_ISO_DATA_SIZE ? NEXUS_ISO_DATA_SIZE : remaining;
        if (read_sector(reader->fp, sector, sector_buf) < 0) return -1;
        memcpy(buffer + offset, sector_buf, chunk);
        offset += chunk;
        remaining -= chunk;
        sector++;
    }
    return (int)file->size;
}

int nexus_iso_read_file_chunk(Nexus_ISOReader *reader, const Nexus_ISOFile *file,
    int file_offset, uint8_t *buffer, int chunk_size)
{
    uint8_t sector_buf[NEXUS_ISO_DATA_SIZE];
    int read_total = 0;
    uint32_t sector;
    int sector_offset;

    if (!reader || !reader->fp || !file || !buffer) return -1;

    sector = file->lba + (file_offset / NEXUS_ISO_DATA_SIZE);
    sector_offset = file_offset % NEXUS_ISO_DATA_SIZE;

    while (read_total < chunk_size) {
        int avail, to_copy;
        if (read_sector(reader->fp, sector, sector_buf) < 0) return -1;
        avail = NEXUS_ISO_DATA_SIZE - sector_offset;
        to_copy = chunk_size - read_total;
        if (to_copy > avail) to_copy = avail;
        memcpy(buffer + read_total, sector_buf + sector_offset, to_copy);
        read_total += to_copy;
        sector++;
        sector_offset = 0;
    }
    return read_total;
}

int nexus_iso_file_count(const Nexus_ISOReader *reader) {
    return reader ? reader->file_count : 0;
}

void nexus_iso_close(Nexus_ISOReader *reader) {
    if (reader && reader->fp) {
        fclose(reader->fp);
        reader->fp = NULL;
    }
    if (reader) reader->valid = 0;
}

int nexus_iso_is_nexus(const Nexus_ISOReader *reader) {
    /* Check for DM Nexus signature files */
    return reader && reader->valid &&
           nexus_iso_find(reader, "DM.BIN") != NULL &&
           nexus_iso_find(reader, "LEV00.DGN") != NULL;
}
