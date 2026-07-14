/*
 * Original Saturn PRS3 loader-media evidence probe.
 *
 * This is deliberately not a decoder. It hash-gates the verified Japanese
 * Track 1 DM.BIN + MENU.BPK pair, inventories executable-side PRS3 markers,
 * and compares their bounded headers with the menu archive. The original
 * executable currently proves that PRS3 is known to the game binary, but no
 * SH-2 control-flow reconstruction has connected either marker to an opcode
 * reader or a termination branch. Keep decoder promotion false until that
 * independent proof exists.
 *
 * ReDMCSB has no Saturn/Nexus implementation. Provenance is the hash-locked
 * Track 1 corpus in docs/VERIFIED_HASHES.md and
 * docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md.
 */

#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_bpk_archive.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEXUS_PRS3_LOADER_DM_MD5 "e88d60859f65f08fa622e1992b02280f"
#define NEXUS_PRS3_LOADER_MENU_MD5 "c2776768ff25287c79013a1452253ca0"
#define NEXUS_PRS3_LOADER_DM_SIZE 555144U
#define NEXUS_PRS3_LOADER_MENU_SIZE 89060U

/* These are byte locations in the MD5-verified Japanese DM.BIN only. */
#define NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET 85356U
#define NEXUS_PRS3_LOADER_EMBEDDED_FRAME_OFFSET 231668U

static int g_failures;

static void check(int condition, const char *message) {
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++g_failures; }
}

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp;
    long file_size;
    uint8_t *data;
    *out_data = NULL;
    *out_size = 0U;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0L, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0L ||
        fseek(fp, 0L, SEEK_SET) != 0) { fclose(fp); return 0; }
    data = (uint8_t *)malloc((size_t)file_size);
    if (!data) { fclose(fp); return 0; }
    if (fread(data, 1U, (size_t)file_size, fp) != (size_t)file_size) {
        free(data); fclose(fp); return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)file_size;
    return 1;
}

static size_t count_magic(const uint8_t *data, size_t size, const char magic[4]) {
    size_t count = 0U, i;
    if (!data || size < 4U) return 0U;
    for (i = 0U; i + 4U <= size; ++i)
        if (memcmp(data + i, magic, 4U) == 0) ++count;
    return count;
}

static int menu_has_header(const uint8_t *menu, size_t menu_size,
                           const uint8_t *header, size_t header_size) {
    size_t i;
    if (!menu || !header || header_size == 0U || header_size > menu_size) return 0;
    for (i = 0U; i + header_size <= menu_size; ++i)
        if (memcmp(menu + i, header, header_size) == 0) return 1;
    return 0;
}

int main(int argc, char **argv) {
    const char *data_dir = argc > 1 ? argv[1] : NULL;
    char default_dir[1024], dm_path[1200], menu_path[1200];
    char dm_md5[33], menu_md5[33];
    const char *home;
    uint8_t *dm = NULL, *menu = NULL;
    size_t dm_size = 0U, menu_size = 0U;
    const uint8_t *embedded;

    if (!data_dir) {
        home = getenv("HOME");
        if (!home || snprintf(default_dir, sizeof(default_dir),
                              "%s/.firestaff/data/nexus", home) <= 0) {
            puts("SKIP: no Nexus data directory argument or HOME");
            return 0;
        }
        data_dir = default_dir;
    }
    if (snprintf(dm_path, sizeof(dm_path), "%s/DM.BIN", data_dir) <= 0 ||
        snprintf(menu_path, sizeof(menu_path), "%s/MENU.BPK", data_dir) <= 0) {
        fprintf(stderr, "FAIL: Nexus data path is too long\n");
        return 1;
    }
    if (!read_file(dm_path, &dm, &dm_size) || !read_file(menu_path, &menu, &menu_size)) {
        puts("SKIP: hash-verified DM.BIN + MENU.BPK pair is not available");
        free(dm); free(menu); return 0;
    }

    check(dm_size == NEXUS_PRS3_LOADER_DM_SIZE,
          "DM.BIN has the locked Japanese Track 1 size");
    check(menu_size == NEXUS_PRS3_LOADER_MENU_SIZE,
          "MENU.BPK has the locked Japanese Track 1 size");
    check(firestaff_x68k_media_receipt_md5_hex(dm, dm_size, dm_md5, sizeof(dm_md5)) == 0 &&
              strcmp(dm_md5, NEXUS_PRS3_LOADER_DM_MD5) == 0,
          "DM.BIN matches its locked MD5");
    check(firestaff_x68k_media_receipt_md5_hex(menu, menu_size, menu_md5, sizeof(menu_md5)) == 0 &&
              strcmp(menu_md5, NEXUS_PRS3_LOADER_MENU_MD5) == 0,
          "MENU.BPK matches its locked MD5");
    if (g_failures == 0) {
        check(count_magic(dm, dm_size, "PRS3") == 2U,
              "DM.BIN contains exactly two PRS3 markers");
        check(count_magic(menu, menu_size, "PRS3") == 162U,
              "MENU.BPK contains its 162 PRS3 surface markers");
        check(NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET + 4U <= dm_size &&
                  memcmp(dm + NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET, "PRS3", 4U) == 0,
              "DM.BIN code-region PRS3 marker remains at its locked offset");
        check(NEXUS_PRS3_LOADER_EMBEDDED_FRAME_OFFSET +
                  NEXUS_V1_BPK_PRS3_HEADER_BYTES + 4U <= dm_size &&
                  memcmp(dm + NEXUS_PRS3_LOADER_EMBEDDED_FRAME_OFFSET, "PRS3", 4U) == 0,
              "DM.BIN embedded PRS3 record remains at its locked offset");
        embedded = dm + NEXUS_PRS3_LOADER_EMBEDDED_FRAME_OFFSET;
        check(read_be32(embedded + 4U) == NEXUS_V1_BPK_PRS3_VERSION &&
                  read_be32(embedded + 8U) == 4096U && read_be32(embedded + 12U) == 997U,
              "embedded PRS3 record retains version, 4096 target, and 997 first word");
        check(!menu_has_header(menu, menu_size, embedded, NEXUS_V1_BPK_PRS3_HEADER_BYTES),
              "embedded DM.BIN PRS3 header is not a MENU.BPK surface header");
    }
    puts("RECEIPT: original media proves PRS3 markers only; opcode and termination grammar remain unproven.");
    puts("RECEIPT: decoder promotion=0, runtime route remains blocked-prs3, synthetic fallback=0.");
    free(dm); free(menu);
    return g_failures == 0 ? 0 : 1;
}
