#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_anim_chunk_pc34_compat.h"

static void test_chunk_read_null_safety(void)
{
    DM2_V1_AnimChunk chunk;
    assert(dm2_v1_anim_chunk_read(NULL, 0, 0, &chunk) == 0);
    assert(dm2_v1_anim_chunk_read(NULL, 100, 0, NULL) == 0);
    printf("  PASS: chunk_read_null_safety\n");
}

static void test_chunk_read_basic(void)
{
    uint8_t data[] = {
        'A', 'N', 0x00, 0x02, 0xAA, 0xBB, 0xCC, 0xDD
    };
    DM2_V1_AnimChunk chunk;
    assert(dm2_v1_anim_chunk_read(data, sizeof(data), 0, &chunk) == 1);
    assert(chunk.tag == DM2_V1_ANIM_CHUNK_AN);
    assert(chunk.payload_size == 2);
    assert(chunk.payload[0] == 0xAA);
    assert(chunk.payload[1] == 0xBB);
    assert(chunk.trailer == 0xCCDD);
    assert(chunk.file_offset == 0);
    printf("  PASS: chunk_read_basic\n");
}

static void test_chunk_read_truncated(void)
{
    uint8_t data[] = { 'A', 'N', 0x00, 0x08, 0x00, 0x00 };
    DM2_V1_AnimChunk chunk;
    assert(dm2_v1_anim_chunk_read(data, sizeof(data), 0, &chunk) == 0);
    printf("  PASS: chunk_read_truncated\n");
}

static void test_chunk_scan_swoosh_synthetic(void)
{
    uint8_t buf[32];
    DM2_V1_AnimChunkScanReceipt r;
    int pos = 0;

    buf[pos++] = 'A'; buf[pos++] = 'N';
    buf[pos++] = 0x00; buf[pos++] = 0x02;
    buf[pos++] = 0x01; buf[pos++] = 0x02;
    buf[pos++] = 0xAA; buf[pos++] = 0xBB;

    buf[pos++] = 'D'; buf[pos++] = 'O';
    buf[pos++] = 0x00; buf[pos++] = 0x00;
    buf[pos++] = 0x00; buf[pos++] = 0x00;

    assert(dm2_v1_anim_chunk_scan(buf, pos, &r) == 1);
    assert(r.valid == 1);
    assert(r.chunk_count == 2);
    assert(r.an_count == 1);
    assert(r.do_count == 1);
    assert(r.bytes_consumed == (uint32_t)pos);
    printf("  PASS: chunk_scan_swoosh_synthetic\n");
}

static void test_parse_an_header(void)
{
    uint8_t data[] = {
        'A', 'N', 0x00, 0x08,
        0x00, 0x00, 0x01, 0x40, 0x00, 0xC8, 0x00, 0x04,
        0x00, 0x03
    };
    DM2_V1_AnimChunk chunk;
    DM2_V1_AnimAnHeader hdr;

    assert(dm2_v1_anim_chunk_read(data, sizeof(data), 0, &chunk) == 1);
    assert(dm2_v1_anim_parse_an_header(&chunk, &hdr) == 1);
    assert(hdr.width == 320);
    assert(hdr.height == 200);
    assert(hdr.flags == 0x0004);
    assert(hdr.extra == 0x0003);
    printf("  PASS: parse_an_header\n");
}

static void test_parse_palette(void)
{
    uint8_t data[4 + 12 + 2];
    DM2_V1_AnimChunk chunk;
    DM2_V1_AnimPalette pal;
    int pos = 0;

    data[pos++] = 'P'; data[pos++] = 'L';
    data[pos++] = 0x00; data[pos++] = 0x0C;
    data[pos++] = 0x00; data[pos++] = 0x00;
    data[pos++] = 0x00; data[pos++] = 0x02;
    data[pos++] = 0x00; data[pos++] = 0x0F;
    data[pos++] = 0x0A; data[pos++] = 0x05;
    data[pos++] = 0x01; data[pos++] = 0x0E;
    data[pos++] = 0x09; data[pos++] = 0x04;
    data[pos++] = 0xAA; data[pos++] = 0xBB;

    assert(dm2_v1_anim_chunk_read(data, pos, 0, &chunk) == 1);
    assert(dm2_v1_anim_parse_palette(&chunk, &pal) == 1);
    assert(pal.start_color == 0);
    assert(pal.num_colors == 2);
    assert(pal.entries[0][0] == 0x00);
    assert(pal.entries[0][1] == 0x0F);
    assert(pal.entries[0][2] == 0x0A);
    assert(pal.entries[0][3] == 0x05);
    assert(pal.entries[1][0] == 0x01);
    assert(pal.entries[1][1] == 0x0E);
    printf("  PASS: parse_palette\n");
}

static uint8_t *load_file(const char *path, size_t *out_size)
{
    FILE *fp = fopen(path, "rb");
    uint8_t *buf;
    long sz;

    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0) { fclose(fp); return NULL; }
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(fp); return NULL; }
    if (fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
        free(buf); fclose(fp); return NULL;
    }
    fclose(fp);
    *out_size = (size_t)sz;
    return buf;
}

static void test_scan_real_swoosh(void)
{
    size_t size;
    uint8_t *data = load_file(
        getenv("HOME")
            ? NULL
            : NULL,
        &size);
    char path[512];
    DM2_V1_AnimChunkScanReceipt r;

    snprintf(path, sizeof(path), "%s/.firestaff/data/dm2-fmtowns-ja/SWOOSH",
             getenv("HOME") ? getenv("HOME") : "/nonexistent");
    data = load_file(path, &size);
    if (!data) {
        printf("  SKIP: scan_real_swoosh (no game data)\n");
        return;
    }

    assert(dm2_v1_anim_chunk_scan(data, size, &r) == 1);
    assert(r.valid == 1);
    assert(r.an_count == 1);
    assert(r.pl_count == 1);
    assert(r.en_count == 1);
    assert(r.dl_count >= 10);
    assert(r.do_count == 1);
    assert(r.bytes_consumed == (uint32_t)size);
    printf("  PASS: scan_real_swoosh (%u chunks, %u DL frames)\n",
           r.chunk_count, r.dl_count);
    free(data);
}

static void test_scan_real_title(void)
{
    char path[512];
    size_t size;
    uint8_t *data;
    DM2_V1_AnimChunkScanReceipt r;

    snprintf(path, sizeof(path), "%s/.firestaff/data/dm2-fmtowns-ja/TITLE",
             getenv("HOME") ? getenv("HOME") : "/nonexistent");
    data = load_file(path, &size);
    if (!data) {
        printf("  SKIP: scan_real_title (no game data)\n");
        return;
    }

    assert(dm2_v1_anim_chunk_scan(data, size, &r) == 1);
    assert(r.valid == 1);
    assert(r.an_count == 1);
    assert(r.pl_count == 1);
    assert(r.en_count >= 1);
    assert(r.dl_count >= 200);
    assert(r.sd_count >= 1);
    assert(r.do_count == 1);
    printf("  PASS: scan_real_title (%u chunks, %u DL, %u SD, %u SO)\n",
           r.chunk_count, r.dl_count, r.sd_count, r.so_count);
    free(data);
}

static void test_scan_real_end(void)
{
    char path[512];
    size_t size;
    uint8_t *data;
    DM2_V1_AnimChunkScanReceipt r;

    snprintf(path, sizeof(path), "%s/.firestaff/data/dm2-fmtowns-ja/END",
             getenv("HOME") ? getenv("HOME") : "/nonexistent");
    data = load_file(path, &size);
    if (!data) {
        printf("  SKIP: scan_real_end (no game data)\n");
        return;
    }

    assert(dm2_v1_anim_chunk_scan(data, size, &r) == 1);
    assert(r.valid == 1);
    assert(r.an_count >= 1);
    assert(r.en_count >= 1);
    assert(r.dl_count >= 300);
    assert(r.do_count == 1);
    printf("  PASS: scan_real_end (%u chunks, %u DL, %u PL, %u EN)\n",
           r.chunk_count, r.dl_count, r.pl_count, r.en_count);
    free(data);
}

static void test_decode_en_keyframe_real(void)
{
    char path[512];
    size_t size;
    uint8_t *data;
    DM2_V1_AnimChunk chunk;
    uint32_t pos;
    uint8_t *framebuf;
    uint16_t w, h;
    size_t fb_size = 320 * 200 / 2;

    snprintf(path, sizeof(path), "%s/.firestaff/data/dm2-fmtowns-ja/SWOOSH",
             getenv("HOME") ? getenv("HOME") : "/nonexistent");
    data = load_file(path, &size);
    if (!data) {
        printf("  SKIP: decode_en_keyframe_real (no game data)\n");
        return;
    }

    framebuf = (uint8_t *)calloc(1, fb_size);
    assert(framebuf);

    pos = 0;
    while (dm2_v1_anim_chunk_read(data, size, pos, &chunk)) {
        if (chunk.tag == DM2_V1_ANIM_CHUNK_EN)
            break;
        pos += 4 + chunk.payload_size + 2;
    }
    assert(chunk.tag == DM2_V1_ANIM_CHUNK_EN);
    assert(dm2_v1_anim_decode_en_keyframe(&chunk, framebuf, fb_size,
                                          &w, &h) == 1);
    assert(w == 320);
    assert(h == 200);
    printf("  PASS: decode_en_keyframe_real (w=%u h=%u)\n", w, h);

    free(framebuf);
    free(data);
}

static void test_apply_dl_delta_real(void)
{
    char path[512];
    size_t size;
    uint8_t *data;
    DM2_V1_AnimChunk chunk;
    uint32_t pos;
    uint8_t *framebuf;
    uint16_t w = 0, h = 0;
    size_t fb_size = 320 * 200 / 2;
    int en_found = 0;
    int dl_applied = 0;

    snprintf(path, sizeof(path), "%s/.firestaff/data/dm2-fmtowns-ja/SWOOSH",
             getenv("HOME") ? getenv("HOME") : "/nonexistent");
    data = load_file(path, &size);
    if (!data) {
        printf("  SKIP: apply_dl_delta_real (no game data)\n");
        return;
    }

    framebuf = (uint8_t *)calloc(1, fb_size);
    assert(framebuf);

    pos = 0;
    while (dm2_v1_anim_chunk_read(data, size, pos, &chunk)) {
        if (chunk.tag == DM2_V1_ANIM_CHUNK_EN && !en_found) {
            assert(dm2_v1_anim_decode_en_keyframe(&chunk, framebuf,
                                                  fb_size, &w, &h) == 1);
            en_found = 1;
        } else if (chunk.tag == DM2_V1_ANIM_CHUNK_DL && en_found) {
            assert(dm2_v1_anim_apply_dl_delta(&chunk, framebuf,
                                              fb_size, w, h) == 1);
            dl_applied++;
        }
        pos += 4 + chunk.payload_size + 2;
    }
    assert(en_found);
    assert(dl_applied >= 10);
    printf("  PASS: apply_dl_delta_real (%d DL frames applied)\n",
           dl_applied);

    free(framebuf);
    free(data);
}

int main(void)
{
    printf("test_dm2_v1_anim_chunk_pc34_compat:\n");
    test_chunk_read_null_safety();
    test_chunk_read_basic();
    test_chunk_read_truncated();
    test_chunk_scan_swoosh_synthetic();
    test_parse_an_header();
    test_parse_palette();
    test_scan_real_swoosh();
    test_scan_real_title();
    test_scan_real_end();
    test_decode_en_keyframe_real();
    test_apply_dl_delta_real();
    printf("All anim chunk tests passed.\n");
    return 0;
}
