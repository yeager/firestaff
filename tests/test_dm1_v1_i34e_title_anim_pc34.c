#include "dm1_v1_i34e_title_anim_pc34.h"
#include "firestaff_x68k_media_receipt.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void put_be16(uint8_t *p, unsigned value) {
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)value;
}

static void put_record(uint8_t *p, const char id[2], unsigned bytes,
                       unsigned attr) {
    p[0] = (uint8_t)id[0]; p[1] = (uint8_t)id[1];
    put_be16(p + 2, bytes); put_be16(p + 4, attr);
}

static void test_stream(void) {
    uint8_t data[14 + 6 + 6 + 6 + 66 + 6 + 504 + 6];
    size_t pos = 0u;
    unsigned i;
    DM1_V1_I34ETitleAnimPc34 state;
    DM1_V1_I34ETitleAnimFramePc34 frame;
    memset(data, 0, sizeof(data));
    data[0] = 'A'; data[1] = 'N'; data[2] = 8;
    put_be16(data + 6, 320); put_be16(data + 8, 200); put_be16(data + 10, 4);
    pos = 14u;
    put_record(data + pos, "BR", 0, 1); pos += 6u;
    put_record(data + pos, "P8", 0, 0); pos += 6u;
    put_record(data + pos, "PL", 66, 0); pos += 6u;
    put_be16(data + pos, 16u);
    for (i = 0u; i < 16u; ++i) {
        data[pos + 2u + i * 4u] = (uint8_t)i;
        data[pos + 3u + i * 4u] = (uint8_t)i;
        data[pos + 4u + i * 4u] = (uint8_t)(15u - i);
        data[pos + 5u + i * 4u] = 3u;
    }
    pos += 66u;
    put_record(data + pos, "EN", 504, 7); pos += 6u;
    put_be16(data + pos, 320); put_be16(data + pos + 2, 200); pos += 4u;
    for (i = 0u; i < 250u; ++i) {
        data[pos++] = 0x83u;
        data[pos++] = 255u;
    }
    put_record(data + pos, "DO", 0, 0); pos += 6u;
    assert(pos == sizeof(data));
    assert(dm1_v1_i34e_title_anim_pc34_init(&state, data, sizeof(data)));
    assert(dm1_v1_i34e_title_anim_pc34_step(&state, &frame) == 1);
    assert(frame.frame_index == 0u && frame.source_delay_vblanks == 7u);
    assert(frame.break_allowed && frame.pixels[0] == 3u &&
           frame.pixels[63999] == 3u);
    assert(frame.palette[4].r == 19u && frame.palette[4].g == 47u &&
           frame.palette[4].b == 15u);
    assert(dm1_v1_i34e_title_anim_pc34_step(&state, &frame) == 0);
    assert(state.finished);
    assert(!dm1_v1_i34e_title_anim_pc34_init(&state, data, 13u));
}

static void test_source_ordered_erase(void) {
    uint8_t pixels[64000];
    DM1_V1_I34ETitleErasePc34 state;
    unsigned frames = 1u;
    memset(pixels, 7, sizeof(pixels));
    dm1_v1_i34e_title_erase_pc34_init(&state);
    assert(state.valid && state.lfsr == 1u);
    assert(dm1_v1_i34e_title_erase_pc34_step(&state, pixels,
                                              sizeof(pixels), 1024u) == 1);
    assert(pixels[1] == 0u && pixels[0] == 7u);
    do {
        int result = dm1_v1_i34e_title_erase_pc34_step(
            &state, pixels, sizeof(pixels), 1024u);
        ++frames;
        assert(result == 0 || result == 1);
        if (result == 0) break;
    } while (frames < 100u);
    assert(state.finished && state.lfsr == 1u);
    assert(state.lfsr_steps == 65535u && state.pixels_cleared == 64000u);
    assert(frames == 64u);
    assert(pixels[0] == 0u && pixels[63999] == 0u);
    assert(dm1_v1_i34e_title_erase_pc34_step(&state, pixels,
                                              sizeof(pixels), 1024u) == 0);
    assert(dm1_v1_i34e_title_erase_pc34_step(&state, pixels,
                                              sizeof(pixels) - 1u, 1u) == -1);
}

static int test_authentic_media(void) {
    const char *path = getenv("FIRESTAFF_DM1_I34E_TITLE");
    FILE *file;
    uint8_t *data;
    long size;
    char digest[65];
    DM1_V1_I34ETitleAnimPc34 state;
    DM1_V1_I34ETitleAnimFramePc34 frame;
    unsigned frames = 0u;
    int result;
    int sha_ok, hash_ok, init_ok;
    if (!path || !path[0]) return 77;
    file = fopen(path, "rb");
    if (!file) return 77;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        size > 1024L * 1024L || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file); return 1;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1u, (size_t)size, file) != (size_t)size) {
        free(data); fclose(file); return 1;
    }
    fclose(file);
    sha_ok = firestaff_x68k_media_receipt_sha256_hex(data, (size_t)size,
                                                       digest, sizeof(digest)) == 0;
    hash_ok = strcmp(digest, "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745") == 0;
    init_ok = dm1_v1_i34e_title_anim_pc34_init(&state, data, (size_t)size);
    if (!sha_ok || !hash_ok || !init_ok) {
        fprintf(stderr, "FAIL: authentic TITLE digest=%s size=%ld sha=%d hash=%d init=%d\n",
                digest, size, sha_ok, hash_ok, init_ok);
        free(data); return 1;
    }
    while ((result = dm1_v1_i34e_title_anim_pc34_step(&state, &frame)) == 1)
        ++frames;
    free(data);
    if (result != 0 || frames != 53u || !state.finished) {
        fprintf(stderr, "FAIL: authentic TITLE result=%d frames=%u offset=%zu size=%zu\n",
                result, frames, state.offset, state.size);
        return 1;
    }
    printf("PASS: authenticated I34E TITLE stream, %u frames\n", frames);
    return 0;
}

int main(void) {
    test_stream();
    test_source_ordered_erase();
    {
        int result = test_authentic_media();
        if (result == 77) {
            puts("SKIP: set FIRESTAFF_DM1_I34E_TITLE for authentic-media verification");
            return 77;
        }
        if (result != 0) return result;
    }
    puts("PASS: I34E TITLE parser and bounded F8288 player");
    return 0;
}
