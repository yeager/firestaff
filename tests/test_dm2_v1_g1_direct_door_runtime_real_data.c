/* Canonical PC File_header must not enter the legacy G1 door path. */
#include "dm2_v1_dungeon_loader.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb"); long n; unsigned char *b;
    *out_size = 0;
    if (!f || fseek(f, 0, SEEK_END) || (n = ftell(f)) != 39437L ||
        fseek(f, 0, SEEK_SET)) { if (f) fclose(f); return NULL; }
    b = malloc((size_t)n);
    if (!b || fread(b, 1, (size_t)n, f) != (size_t)n) { free(b); fclose(f); return NULL; }
    fclose(f); *out_size = (int)n; return b;
}
int main(int argc, char **argv) {
    const char *path = argc >= 2 ? argv[1] : NULL; int size;
    unsigned char *bytes; DM2_V1_DungeonData dungeon;
    DM2_V1_G1RuntimeMapDoorReceipt receipt;
    if (!path) { puts("SKIP: no local canonical DM2 data"); return 0; }
    bytes = read_file(path, &size);
    if (!bytes || bytes[0] || bytes[1] || bytes[2] != 0x47 || bytes[3] != 0x31 ||
        bytes[4] != 44 || bytes[5] || bytes[6] != 28 || bytes[7] ||
        dm2_v1_dungeon_load(&dungeon, bytes, size) != 0) {
        free(bytes); fputs("FAIL: canonical File_header input was not accepted\n", stderr); return 1;
    }
    free(bytes); receipt.committed = -1;
    if (dm2_v1_dungeon_materialize_g1_runtime_map_doors(&dungeon, 0, &receipt) ||
        receipt.committed != -1) {
        dm2_v1_dungeon_free(&dungeon); fputs("FAIL: File_header entered legacy G1 door path\n", stderr); return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: File_header leaves legacy G1 door path unavailable"); return 0;
}
