/*
 * test_firestaff_x68k_ftl_handoff.c
 *
 * Cross-module test for the X68000 HDM/floppy <-> FTL container
 * handoff invariants declared in
 * include/firestaff_x68k_media_classify.h.
 *
 * This test links two libraries:
 *
 *   - firestaff_ftl_container (greatstone d_ftl.html parser)
 *     to build a synthetic FTL common-header + 3-hunk
 *     container (BSS + DATA + CODE) and verify the parsed
 *     HUNK_BSS metadata reports the documented area_1 memory
 *     size.
 *
 *   - firestaff_x68k_media_classify (DMWeb X68000 geometry)
 *     to verify that an FTL-declared area_1 size fits within
 *     an X68000 2DHD HDM (1232 KB) when it should, and is
 *     rejected when the FTL header would force a load that
 *     cannot fit the disk.
 *
 * Why this is its own test binary instead of another
 * SelfTest case inside firestaff_x68k_media_classify.c:
 * the cross-module handoff depends on the FTL container
 * parser (greatstone d_ftl.html Note 1 / Note 2 / Note 4
 * checksums). We do not want to pull the FTL parser into the
 * X68k media classifier itself — the classifier is meant to
 * be reusable for FTL assets that may or may not be sitting
 * on a real HDM. The handoff test exercises both at once.
 *
 * Source of truth:
 *   - greatstone d_ftl.html "20-byte common header" magic
 *     0x6160 big-endian; 12-byte hunk header; HUNK_BSS
 *     "size of area 1 of hunk 0x011 in memory" at offset 4
 *     of the BSS payload.
 *   - dmweb-free.fr/community/documentation/copy-protection,
 *     "Sharp X68000" section: 2 sides x 77 tracks x 8
 *     sectors x 1024 bytes = 1261568 bytes per HDM image.
 *   - docs/FIRESTAFF_GAP_LIST.md "DM1 X68000 HDM/floppy
 *     media import" gap row.
 *
 * Build:
 *   cc -std=c99 -I include tests/test_firestaff_x68k_ftl_handoff.c \
 *      src/shared/firestaff_x68k_media_classify.c \
 *      src/shared/firestaff_ftl_container.c \
 *      -o test_firestaff_x68k_ftl_handoff
 */

#include "firestaff_ftl_container.h"
#include "firestaff_x68k_media_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The DMWeb-documented full-disk size in bytes. We duplicate
 * it here (instead of including firestaff_x68k_media_classify.h
 * internals) so the test is robust to renames of the
 * constants. */
#define DMWEB_X68K_DISK_BYTES 1261568u

static void wr16_be(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wr32_be(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

static uint16_t rd16_be(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

/* Build a synthetic FTL container with the requested
 * data_area1_memory_size value encoded in the BSS hunk
 * metadata at offset 4 (greatstone d_ftl.html HUNK_BSS
 * "Structure" row, index 4: "size of area 1 of hunk
 * 0x011 in memory"). The container has the documented
 * common header, 3 hunk headers, a minimal BSS payload
 * (40 bytes of metadata), a small DATA payload, and an
 * uncompressed 4-byte CODE payload. Returns the total
 * container size on success, 0 on insufficient buffer. */
static size_t build_ftl_with_area1_size(uint8_t* buf,
                                        size_t cap,
                                        uint32_t area1_size) {
    if (!buf || cap < 128u) return 0;
    memset(buf, 0, cap);

    const size_t hunk_count = 3u;
    const size_t bss_off = 20u + hunk_count * 12u;
    const size_t bss_size = 40u;
    const size_t data_off = bss_off + bss_size;
    const size_t data_size = 12u;
    const size_t code_off = data_off + data_size;
    const size_t code_size = 4u;
    const size_t total = code_off + code_size;

    /* Common header. */
    wr16_be(buf + 0u, 0x6160u); /* FTL magic, big-endian. */
    wr16_be(buf + 4u, 0x0002u);
    buf[6] = 0x01u;
    buf[11] = 0x01u;
    buf[12] = 0x04u;
    buf[13] = 0x01u;
    wr16_be(buf + 18u, (uint16_t)hunk_count);

    /* Hunk headers. */
    uint8_t* h = buf + 20u;
    wr16_be(h + 0u, 0x0010u); /* HUNK_BSS */
    wr32_be(h + 4u, (uint32_t)bss_off);
    wr32_be(h + 8u, (uint32_t)bss_size);
    h += 12u;
    wr16_be(h + 0u, 0x0011u); /* HUNK_DATA */
    wr32_be(h + 4u, (uint32_t)data_off);
    wr32_be(h + 8u, (uint32_t)data_size);
    h += 12u;
    wr16_be(h + 0u, 0x0012u); /* HUNK_CODE */
    wr32_be(h + 4u, (uint32_t)code_off);
    wr32_be(h + 8u, (uint32_t)code_size);

    /* BSS metadata (40 bytes), per greatstone d_ftl.html
     * "HUNK_BSS Structure". The area_1 in-memory size is
     * the uint32 at offset 4. */
    uint8_t* bss = buf + bss_off;
    wr32_be(bss + 4u, area1_size);
    wr32_be(bss + 20u, (uint32_t)code_size);
    wr32_be(bss + 24u, (uint32_t)data_size);

    /* DATA payload (just a non-zero byte at index 5 so the
     * common checksum varies; we'll write actual checksums
     * after the payload is laid out). */
    uint8_t* data = buf + data_off;
    data[5] = 0x02u;
    data[10] = 0xaau;
    data[11] = 0xbbu;

    /* CODE payload (uncompressed). */
    uint8_t* code = buf + code_off;
    code[0] = 0x12u;
    code[1] = 0x34u;
    code[2] = 0x56u;
    code[3] = 0x78u;

    /* BSS checksum at bss + 34 (skipping bss + 34 itself). */
    uint16_t bss_sum = 0u;
    for (size_t i = 0u; i + 1u < bss_size; i += 2u) {
        if (i == 34u) continue;
        bss_sum = (uint16_t)((bss_sum + rd16_be(bss + i)) % 0xffffu);
    }
    wr16_be(bss + 34u, bss_sum);

    /* DATA checksum (sum of bytes) at bss + 38. */
    uint16_t data_sum = 0u;
    for (size_t i = 0u; i < data_size; ++i) {
        data_sum = (uint16_t)((data_sum + data[i]) % 0xffffu);
    }
    wr16_be(bss + 38u, data_sum);

    /* CODE checksum (sum of bytes) at bss + 36. */
    uint16_t code_sum = 0u;
    for (size_t i = 0u; i < code_size; ++i) {
        code_sum = (uint16_t)((code_sum + code[i]) % 0xffffu);
    }
    wr16_be(bss + 36u, code_sum);

    /* Common header checksum (greatstone d_ftl.html "Note 1"). */
    uint16_t common = 0u;
    for (size_t i = 4u; i < 20u; ++i) {
        common = (uint16_t)((common +
                             (uint32_t)buf[i] * (uint32_t)i) %
                            0xffffu);
    }
    const uint8_t* headers = buf + 20u;
    for (size_t h_idx = 0u; h_idx < hunk_count; ++h_idx) {
        for (size_t j = 0u; j < 12u; ++j) {
            uint32_t mult = (uint32_t)(j + (12u * h_idx) + 1u);
            common = (uint16_t)((common +
                                 (uint32_t)headers[h_idx * 12u + j] *
                                     mult) %
                                0xffffu);
        }
    }
    wr16_be(buf + 2u, common);

    return total;
}

/* Build a synthetic "HDM image" buffer of the requested size
 * whose bytes are mostly zero so the X68k classifier returns
 * media_class == FULL_DISK when given DMWEB_X68K_DISK_BYTES.
 * For other sizes we just return what the classifier would
 * say. */
static uint8_t* alloc_disk_image(size_t bytes) {
    uint8_t* img = (uint8_t*)calloc(1, bytes);
    return img;
}

static int test_handoff_fits_small_area1(void) {
    /* FTL declares a 1-sector area_1 (1024 bytes); the
     * full-disk HDM (1232 KB) easily contains it. */
    uint8_t ftl_buf[256];
    size_t ftl_size = build_ftl_with_area1_size(
        ftl_buf, sizeof(ftl_buf), 1024u);
    if (ftl_size == 0u) {
        fprintf(stderr, "build_ftl_with_area1_size failed\n");
        return 0;
    }

    FirestaffFtlContainer ftl;
    if (FirestaffFtlContainer_Parse(ftl_buf, ftl_size, &ftl) != 0) {
        fprintf(stderr, "FTL parse failed\n");
        return 0;
    }
    if (!ftl.has_bss_metadata) {
        fprintf(stderr, "FTL BSS metadata missing\n");
        return 0;
    }

    uint8_t* hdm = alloc_disk_image(DMWEB_X68K_DISK_BYTES);
    if (!hdm) return 0;
    FirestaffX68kMediaClassifyResult media;
    FirestaffX68kMedia_Classify(hdm, DMWEB_X68K_DISK_BYTES, &media);
    if (media.media_class != FIRESTAFF_X68K_MEDIA_FULL_DISK) {
        fprintf(stderr, "expected full-disk class\n");
        free(hdm);
        return 0;
    }
    int fits = FirestaffX68kMedia_FTLHandoffFits(
        &media, ftl.bss.data_area1_memory_size);
    free(hdm);
    if (fits != 1) {
        fprintf(stderr,
                "expected fits for area_1=1024 on full disk\n");
        return 0;
    }
    return 1;
}

static int test_handoff_fits_exact_disk(void) {
    /* FTL declares the full disk as its area_1 size. This
     * is the most aggressive "fits" case: the FTL resource
     * is exactly the size of the HDM. */
    uint8_t ftl_buf[256];
    size_t ftl_size = build_ftl_with_area1_size(
        ftl_buf, sizeof(ftl_buf), DMWEB_X68K_DISK_BYTES);
    if (ftl_size == 0u) return 0;

    FirestaffFtlContainer ftl;
    if (FirestaffFtlContainer_Parse(ftl_buf, ftl_size, &ftl) != 0) {
        return 0;
    }

    uint8_t* hdm = alloc_disk_image(DMWEB_X68K_DISK_BYTES);
    if (!hdm) return 0;
    FirestaffX68kMediaClassifyResult media;
    FirestaffX68kMedia_Classify(hdm, DMWEB_X68K_DISK_BYTES, &media);
    int fits = FirestaffX68kMedia_FTLHandoffFits(
        &media, ftl.bss.data_area1_memory_size);
    free(hdm);
    if (fits != 1) {
        fprintf(stderr,
                "expected fits for area_1=disk_bytes on full disk\n");
        return 0;
    }
    return 1;
}

static int test_handoff_overflow_rejected(void) {
    /* FTL declares an area_1 size 1 byte larger than the
     * full HDM. The classifier must refuse this handoff. */
    uint8_t ftl_buf[256];
    size_t ftl_size = build_ftl_with_area1_size(
        ftl_buf, sizeof(ftl_buf), DMWEB_X68K_DISK_BYTES + 1u);
    if (ftl_size == 0u) return 0;

    FirestaffFtlContainer ftl;
    if (FirestaffFtlContainer_Parse(ftl_buf, ftl_size, &ftl) != 0) {
        return 0;
    }

    uint8_t* hdm = alloc_disk_image(DMWEB_X68K_DISK_BYTES);
    if (!hdm) return 0;
    FirestaffX68kMediaClassifyResult media;
    FirestaffX68kMedia_Classify(hdm, DMWEB_X68K_DISK_BYTES, &media);
    int fits = FirestaffX68kMedia_FTLHandoffFits(
        &media, ftl.bss.data_area1_memory_size);
    free(hdm);
    if (fits != 0) {
        fprintf(stderr,
                "expected overflow reject for area_1=disk_bytes+1\n");
        return 0;
    }
    return 1;
}

static int test_handoff_overflow_into_single_side(void) {
    /* FTL declares an area_1 size larger than one side but
     * smaller than the full disk. The classifier returns
     * FULL_DISK for the HDM, so the handoff still fits.
     * This verifies we don't conflate single-side size
     * with full-disk size when the HDM is a full disk. */
    uint8_t ftl_buf[256];
    size_t one_side = DMWEB_X68K_DISK_BYTES / 2u;
    size_t ftl_size = build_ftl_with_area1_size(
        ftl_buf, sizeof(ftl_buf), (uint32_t)one_side + 1024u);
    if (ftl_size == 0u) return 0;

    FirestaffFtlContainer ftl;
    if (FirestaffFtlContainer_Parse(ftl_buf, ftl_size, &ftl) != 0) {
        return 0;
    }

    uint8_t* hdm = alloc_disk_image(DMWEB_X68K_DISK_BYTES);
    if (!hdm) return 0;
    FirestaffX68kMediaClassifyResult media;
    FirestaffX68kMedia_Classify(hdm, DMWEB_X68K_DISK_BYTES, &media);
    int fits = FirestaffX68kMedia_FTLHandoffFits(
        &media, ftl.bss.data_area1_memory_size);
    free(hdm);
    if (fits != 1) {
        fprintf(stderr,
                "expected fits for area_1 > side, <= full disk\n");
        return 0;
    }
    return 1;
}

static int test_ftl_payload_on_handoff(void) {
    /* If the buffer is a small FTL payload (not a full
     * HDM), the classifier reports FTL_PRESENT and the
     * handoff is meaningful only as a "do not classify as
     * HDM" signal — the size check itself rejects because
     * the buffer is too small to be a disk image. */
    uint8_t ftl_buf[128];
    size_t ftl_size = build_ftl_with_area1_size(
        ftl_buf, sizeof(ftl_buf), 4096u);
    if (ftl_size == 0u) return 0;

    FirestaffX68kMediaClassifyResult media;
    FirestaffX68kMedia_Classify(ftl_buf, ftl_size, &media);
    if (!FirestaffX68kMedia_IsFTLPayload(media.flags,
                                          media.media_class)) {
        fprintf(stderr,
                "expected FTL payload flag on FTL-magic buffer\n");
        return 0;
    }
    /* The buffer is way smaller than a disk, so any non-zero
     * FTL-declared area_1 size must be rejected. */
    if (FirestaffX68kMedia_FTLHandoffFits(&media, 4096u) != 0) {
        fprintf(stderr,
                "expected FTL payload to reject full HDM-style "
                "handoff\n");
        return 0;
    }
    return 1;
}

int main(void) {
    int total = 0;
    int passed = 0;
#define RUN(test) do { ++total; if (test()) ++passed; } while (0)
    RUN(test_handoff_fits_small_area1);
    RUN(test_handoff_fits_exact_disk);
    RUN(test_handoff_overflow_rejected);
    RUN(test_handoff_overflow_into_single_side);
    RUN(test_ftl_payload_on_handoff);
#undef RUN
    if (passed == total) {
        printf("test_firestaff_x68k_ftl_handoff: PASS\n");
        return 0;
    }
    fprintf(stderr,
            "test_firestaff_x68k_ftl_handoff: %d/%d passed\n",
            passed, total);
    return 1;
}
