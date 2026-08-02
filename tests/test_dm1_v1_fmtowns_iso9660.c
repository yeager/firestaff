#include "dm1_v1_fmtowns_iso9660.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static void write_both32(uint8_t *p, uint32_t v) {
    write_le32(p, v);
    p[4] = (uint8_t)(v >> 24);
    p[5] = (uint8_t)(v >> 16);
    p[6] = (uint8_t)(v >> 8);
    p[7] = (uint8_t)(v);
}

/* Build a minimal ISO 9660 data track with a root directory containing
 * DATA/GRAPHICS.DAT and DATA/DUNGEON.DAT entries. */
static uint8_t *build_synthetic_iso(size_t *out_size) {
    /* Layout:
     * Sector 0-15: system area (unused)
     * Sector 16: PVD
     * Sector 17: terminator
     * Sector 20: root directory
     * Sector 21: DATA subdirectory
     * Sector 100: GRAPHICS.DAT (4 bytes for test)
     * Sector 101: DUNGEON.DAT (4 bytes for test)
     */
    size_t total = 102 * 2048;
    uint8_t *img = (uint8_t *)calloc(1, total);
    if (!img) return NULL;

    /* PVD at sector 16 */
    uint8_t *pvd = img + 16 * 2048;
    pvd[0] = 0x01;
    memcpy(pvd + 1, "CD001", 5);
    pvd[6] = 0x01; /* version */
    memcpy(pvd + 8, "HMA-240                         ", 32);
    memcpy(pvd + 40, "DUNGEON                         ", 32);
    write_both32(pvd + 80, 102); /* volume space size */

    /* Root directory record at PVD+156 */
    uint8_t *root_rec = pvd + 156;
    root_rec[0] = 34; /* record length */
    write_le32(root_rec + 2, 20); /* extent LBA */
    write_le32(root_rec + 10, 2048); /* size */
    root_rec[25] = 0x02; /* directory flag */
    root_rec[32] = 1; /* name length */
    root_rec[33] = 0x00; /* root */

    /* Terminator at sector 17 */
    uint8_t *term = img + 17 * 2048;
    term[0] = 0xff;
    memcpy(term + 1, "CD001", 5);

    /* Root directory at sector 20 */
    uint8_t *rootdir = img + 20 * 2048;
    size_t pos = 0;

    /* "." entry */
    rootdir[pos] = 34;
    write_le32(rootdir + pos + 2, 20);
    write_le32(rootdir + pos + 10, 2048);
    rootdir[pos + 25] = 0x02;
    rootdir[pos + 32] = 1;
    rootdir[pos + 33] = 0x00;
    pos += 34;

    /* ".." entry */
    rootdir[pos] = 34;
    write_le32(rootdir + pos + 2, 20);
    write_le32(rootdir + pos + 10, 2048);
    rootdir[pos + 25] = 0x02;
    rootdir[pos + 32] = 1;
    rootdir[pos + 33] = 0x01;
    pos += 34;

    /* DATA directory entry */
    const char *dirname = "DATA";
    uint8_t name_len = (uint8_t)strlen(dirname);
    uint8_t rec_len = (uint8_t)(33 + name_len);
    if (rec_len & 1) rec_len++;
    rootdir[pos] = rec_len;
    write_le32(rootdir + pos + 2, 21); /* DATA dir at sector 21 */
    write_le32(rootdir + pos + 10, 2048);
    rootdir[pos + 25] = 0x02;
    rootdir[pos + 32] = name_len;
    memcpy(rootdir + pos + 33, dirname, name_len);
    pos += rec_len;

    /* DATA subdirectory at sector 21 */
    uint8_t *datadir = img + 21 * 2048;
    size_t dpos = 0;

    /* "." */
    datadir[dpos] = 34;
    write_le32(datadir + dpos + 2, 21);
    write_le32(datadir + dpos + 10, 2048);
    datadir[dpos + 25] = 0x02;
    datadir[dpos + 32] = 1;
    datadir[dpos + 33] = 0x00;
    dpos += 34;

    /* ".." */
    datadir[dpos] = 34;
    write_le32(datadir + dpos + 2, 20);
    write_le32(datadir + dpos + 10, 2048);
    datadir[dpos + 25] = 0x02;
    datadir[dpos + 32] = 1;
    datadir[dpos + 33] = 0x01;
    dpos += 34;

    /* GRAPHICS.DAT;1 */
    const char *gfx_name = "GRAPHICS.DAT;1";
    name_len = (uint8_t)strlen(gfx_name);
    rec_len = (uint8_t)(33 + name_len);
    if (rec_len & 1) rec_len++;
    datadir[dpos] = rec_len;
    write_le32(datadir + dpos + 2, 100);
    write_le32(datadir + dpos + 10, 4);
    datadir[dpos + 25] = 0x00;
    datadir[dpos + 32] = name_len;
    memcpy(datadir + dpos + 33, gfx_name, name_len);
    dpos += rec_len;

    /* DUNGEON.DAT;1 */
    const char *dng_name = "DUNGEON.DAT;1";
    name_len = (uint8_t)strlen(dng_name);
    rec_len = (uint8_t)(33 + name_len);
    if (rec_len & 1) rec_len++;
    datadir[dpos] = rec_len;
    write_le32(datadir + dpos + 2, 101);
    write_le32(datadir + dpos + 10, 4);
    datadir[dpos + 25] = 0x00;
    datadir[dpos + 32] = name_len;
    memcpy(datadir + dpos + 33, dng_name, name_len);
    dpos += rec_len;

    /* Write some data at sectors 100 and 101 */
    memcpy(img + 100 * 2048, "GFX!", 4);
    memcpy(img + 101 * 2048, "DNG!", 4);

    *out_size = total;
    return img;
}

static void test_probe_null_rejects(void) {
    assert(dm1_v1_fmtowns_iso_probe(NULL, 0) == 0);
}

static void test_probe_too_small_rejects(void) {
    uint8_t buf[100];
    memset(buf, 0, sizeof(buf));
    assert(dm1_v1_fmtowns_iso_probe(buf, sizeof(buf)) == 0);
}

static void test_probe_synthetic_accepts(void) {
    size_t size;
    uint8_t *img = build_synthetic_iso(&size);
    assert(img != NULL);
    assert(dm1_v1_fmtowns_iso_probe(img, size) == 1);
    free(img);
}

static void test_probe_wrong_system_id_rejects(void) {
    size_t size;
    uint8_t *img = build_synthetic_iso(&size);
    assert(img != NULL);
    memcpy(img + 16 * 2048 + 8, "OTHER-ID", 8);
    assert(dm1_v1_fmtowns_iso_probe(img, size) == 0);
    free(img);
}

static void test_parse_finds_game_files(void) {
    size_t size;
    uint8_t *img = build_synthetic_iso(&size);
    assert(img != NULL);

    DM1_V1_FmtownsIsoLayout layout;
    int rc = dm1_v1_fmtowns_iso_parse(img, size, &layout);
    assert(rc == 0);
    assert(strcmp(layout.system_id, "HMA-240") == 0);
    assert(strcmp(layout.volume_id, "DUNGEON") == 0);
    assert(layout.file_count == 2);

    int found_gfx = 0, found_dng = 0;
    for (int i = 0; i < layout.file_count; i++) {
        if (strcmp(layout.files[i].name, "DATA/GRAPHICS.DAT") == 0) {
            assert(layout.files[i].size == 4);
            assert(layout.files[i].lba == 100);
            found_gfx = 1;
        }
        if (strcmp(layout.files[i].name, "DATA/DUNGEON.DAT") == 0) {
            assert(layout.files[i].size == 4);
            assert(layout.files[i].lba == 101);
            found_dng = 1;
        }
    }
    assert(found_gfx);
    assert(found_dng);

    free(img);
}

static void test_extract_file(void) {
    size_t size;
    uint8_t *img = build_synthetic_iso(&size);
    assert(img != NULL);

    DM1_V1_FmtownsIsoLayout layout;
    dm1_v1_fmtowns_iso_parse(img, size, &layout);

    uint8_t buf[8];
    for (int i = 0; i < layout.file_count; i++) {
        if (strcmp(layout.files[i].name, "DATA/GRAPHICS.DAT") == 0) {
            int rc = dm1_v1_fmtowns_iso_extract(img, size, &layout.files[i],
                                                 buf, sizeof(buf));
            assert(rc == 0);
            assert(memcmp(buf, "GFX!", 4) == 0);
        }
    }

    free(img);
}

static void test_extract_too_small_buffer(void) {
    size_t size;
    uint8_t *img = build_synthetic_iso(&size);
    assert(img != NULL);

    DM1_V1_FmtownsIsoLayout layout;
    dm1_v1_fmtowns_iso_parse(img, size, &layout);

    uint8_t buf[2];
    for (int i = 0; i < layout.file_count; i++) {
        if (strcmp(layout.files[i].name, "DATA/GRAPHICS.DAT") == 0) {
            int rc = dm1_v1_fmtowns_iso_extract(img, size, &layout.files[i],
                                                 buf, sizeof(buf));
            assert(rc == -1);
        }
    }

    free(img);
}

int main(void) {
    test_probe_null_rejects();
    test_probe_too_small_rejects();
    test_probe_synthetic_accepts();
    test_probe_wrong_system_id_rejects();
    test_parse_finds_game_files();
    test_extract_file();
    test_extract_too_small_buffer();
    printf("All dm1_v1_fmtowns_iso9660 tests passed.\n");
    return 0;
}
