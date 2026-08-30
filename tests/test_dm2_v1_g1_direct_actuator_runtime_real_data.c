/* Canonical PC File_header must not enter the legacy G1 actuator path. */
#include "dm2_v1_dungeon_loader.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, int *out_size)
{
    FILE *file;
    long size;
    unsigned char *bytes;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) != 39437L || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)size;
    return bytes;
}

static unsigned char *read_dungeon(int argc, char **argv, int *out_size)
{
    const char *archive;
    uint8_t *bytes = NULL;
    size_t size = 0u;

    if (argc >= 2) return read_file(argv[1], out_size);
    archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    if (!archive || !archive[0] ||
        firestaff_zip_extract_by_suffix(archive, "data/dungeon.dat", &bytes,
                                        &size) != 0 ||
        !bytes || size != 39437u) {
        free(bytes);
        *out_size = 0;
        return NULL;
    }
    *out_size = (int)size;
    return bytes;
}

int main(int argc, char **argv)
{
    int size;
    unsigned char *bytes;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1RuntimeMapActuatorReceipt receipt;

    bytes = read_dungeon(argc, argv, &size);
    if (!bytes) {
        puts("SKIP: canonical DM2 DOS DUNGEON.DAT is unavailable");
        return 0;
    }
    if (bytes[0] || bytes[1] || bytes[2] != 0x47 || bytes[3] != 0x31 ||
        bytes[4] != 44 || bytes[5] || bytes[6] != 28 || bytes[7] ||
        dm2_v1_dungeon_load(&dungeon, bytes, size)) {
        free(bytes);
        fputs("FAIL: canonical File_header input was not accepted\n", stderr);
        return 1;
    }
    free(bytes);
    receipt.committed = -1;
    if (dm2_v1_dungeon_materialize_g1_runtime_map_actuators(&dungeon, 0,
                                                             &receipt) ||
        receipt.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: File_header entered legacy G1 actuator path\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: real File_header leaves legacy G1 actuator path unavailable");
    return 0;
}
