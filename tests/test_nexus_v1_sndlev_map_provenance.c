#include "nexus_v1_sndlev_map_provenance.h"
#include "nexus_v1_iso_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    for (index = 0U; index < count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int open_retail_iso_if_present(const char *directory,
                                      Nexus_ISOReader *out_iso)
{
    static const char *const cue_names[] = {
        "Dungeon Master Nexus (Japan).cue",
        "Dungeon Master Nexus (English).cue",
        NULL
    };
    char path[1024];
    int index;

    if (!directory || !directory[0] || !out_iso) return 0;
    memset(out_iso, 0, sizeof(*out_iso));
    if (strlen(directory) >= 4U &&
        strcmp(directory + strlen(directory) - 4U, ".cue") == 0)
        return nexus_iso_open_cue(out_iso, directory) > 0;
    for (index = 0; cue_names[index]; ++index) {
        snprintf(path, sizeof(path), "%s/%s", directory, cue_names[index]);
        if (nexus_iso_open_cue(out_iso, path) > 0) return 1;
    }
    return 0;
}

static int verify_real_corpus(void)
{
    const char *directory = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_ISOReader iso;
    int iso_opened;
    char path[1024];
    char member_name[32];
    uint8_t bytes[128];
    unsigned level;

    /* Keep the focused unit test runnable without user-supplied game data. */
    if (!directory || !directory[0]) return 1;
    iso_opened = open_retail_iso_if_present(directory, &iso);
    for (level = 0U; level < 16U; ++level) {
        FILE *file;
        const Nexus_ISOFile *member = NULL;
        size_t size;
        Nexus_V1_SndlevMapProvenanceReceipt receipt;
        snprintf(member_name, sizeof(member_name), "SNDLEV%02u.MAP", level);
        if (snprintf(path, sizeof(path), "%s/SNDLEV%02u.MAP", directory,
                     level) <= 0 ||
            (file = fopen(path, "rb")) == NULL) {
            if (!iso_opened ||
                (member = nexus_iso_find(&iso, member_name)) == NULL ||
                member->size > sizeof(bytes) ||
                nexus_iso_read_file(&iso, member, bytes,
                                    (int)sizeof(bytes)) != (int)member->size) {
                if (iso_opened) nexus_iso_close(&iso);
                return 0;
            }
            size = member->size;
        } else {
            size = fread(bytes, 1U, sizeof(bytes), file);
            if (ferror(file) || fclose(file) != 0) {
                if (iso_opened) nexus_iso_close(&iso);
                return 0;
            }
        }
        if (size < 10U || size > 90U ||
            nexus_v1_sndlev_map_provenance_parse(
                bytes, size, fnv1a64(bytes, size), &receipt) == 0 ||
            !receipt.valid || receipt.header_length != 0U ||
            receipt.table_offset != 0U || receipt.table_length != size - 2U ||
            receipt.terminator_offset != size - 2U || !receipt.record_count) {
            if (iso_opened) nexus_iso_close(&iso);
            return 0;
        }
    }
    if (iso_opened) nexus_iso_close(&iso);
    return 1;
}

int main(void)
{
    uint8_t bytes[10] = {0};
    Nexus_V1_SndlevMapProvenanceReceipt table;
    Nexus_V1_SndlevMapRowProvenanceReceipt row;
    uint64_t source_fnv;

    bytes[0] = 0x20U;
    bytes[8] = 0xffU;
    bytes[9] = 0xffU;
    source_fnv = fnv1a64(bytes, sizeof(bytes));
    if (!nexus_v1_sndlev_map_provenance_parse(
            bytes, sizeof(bytes), source_fnv, &table) || !table.valid ||
        table.record_count != 1U || table.terminator_offset != 8U ||
        table.playback_permitted ||
        !nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), source_fnv, 0U, &row) || !row.valid ||
        row.row_index != 0U || row.row_offset != 0U || row.row_length != 8U ||
        row.table_fnv1a64 != table.table_fnv1a64 || row.playback_permitted)
        return 1;

    bytes[0] ^= 1U;
    if (nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), source_fnv, 0U, &row)) return 1;
    bytes[0] ^= 1U;
    if (nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), fnv1a64(bytes, sizeof(bytes)), 1U, &row) ||
        nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), source_fnv ^ UINT64_C(1), 0U, &row)) return 1;

    bytes[8] = 0U;
    if (nexus_v1_sndlev_map_provenance_parse(
            bytes, sizeof(bytes), fnv1a64(bytes, sizeof(bytes)), &table)) return 1;
    if (!verify_real_corpus()) return 1;
    puts("test_nexus_v1_sndlev_map_provenance: PASS");
    return 0;
}
