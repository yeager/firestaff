/*
 * test_m11_colorblind_m11.c
 *
 * Data-free contract test for the M11 colorblind daltonization
 * module (src/engine/colorblind_m11.c). The module applies a fixed
 * 3x3 linear matrix to incoming RGB / RGBA pixels so HUD and UI
 * elements can be remapped for the three common dichromat modes
 * (deuteranopia / protanopia / tritanopia).  A wrong matrix would
 * silently twist every HUD overlay for users who need it, so this
 * regression pins the exact per-channel math, the OFF-mode
 * short-circuit, and the surface contract.
 */

#include "colorblind_m11.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int cond, const char *name) {
    if (cond) {
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s\n", name);
        ++g_failures;
    }
}

static int clamp_byte_local(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

static void expected_remap(const int m[9], int r, int g, int b,
                           int *rr, int *gg, int *bb) {
    int vr = (m[0] * r + m[1] * g + m[2] * b) / 1000;
    int vg = (m[3] * r + m[4] * g + m[5] * b) / 1000;
    int vb = (m[6] * r + m[7] * g + m[8] * b) / 1000;
    *rr = clamp_byte_local(vr);
    *gg = clamp_byte_local(vg);
    *bb = clamp_byte_local(vb);
}

static const int kDeutero[9] = {
    800, 200,   0,
    258, 742,   0,
      0, 142, 858
};
static const int kProtan[9] = {
    567, 433,   0,
    558, 442,   0,
      0, 242, 758
};
static const int kTritan[9] = {
    950,  50,   0,
      0, 433, 567,
      0, 475, 525
};

static void assert_remap(int mode, const int m[9],
                         int r, int g, int b,
                         const char *label) {
    uint8_t rv = (uint8_t)r;
    uint8_t gv = (uint8_t)g;
    uint8_t bv = (uint8_t)b;
    int er, eg, eb;
    expected_remap(m, r, g, b, &er, &eg, &eb);
    M11_Colorblind_RemapRGB(&rv, &gv, &bv, mode);
    check(rv == (uint8_t)er &&
          gv == (uint8_t)eg &&
          bv == (uint8_t)eb,
          label);
}

static void test_labels(void) {
    const char *l;
    l = M11_Colorblind_GetLabel(M11_COLORBLIND_OFF);
    check(l != NULL && strcmp(l, "Off") == 0,
          "label OFF == Off");
    l = M11_Colorblind_GetLabel(M11_COLORBLIND_DEUTERANOPIA);
    check(l != NULL && strcmp(l, "Deuteranopia") == 0,
          "label DEUTERANOPIA == Deuteranopia");
    l = M11_Colorblind_GetLabel(M11_COLORBLIND_PROTANOPIA);
    check(l != NULL && strcmp(l, "Protanopia") == 0,
          "label PROTANOPIA == Protanopia");
    l = M11_Colorblind_GetLabel(M11_COLORBLIND_TRITANOPIA);
    check(l != NULL && strcmp(l, "Tritanopia") == 0,
          "label TRITANOPIA == Tritanopia");
    check(M11_Colorblind_GetLabel(M11_COLORBLIND_COUNT) == NULL,
          "out-of-range label high -> NULL");
    check(M11_Colorblind_GetLabel(-1) == NULL,
          "out-of-range label -1 -> NULL");
    check(M11_Colorblind_GetLabel(99) == NULL,
          "out-of-range label 99 -> NULL");
}

static void test_identity_detection(void) {
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_OFF) == 1,
          "OFF is identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_DEUTERANOPIA) == 0,
          "DEUTERANOPIA is not identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_PROTANOPIA) == 0,
          "PROTANOPIA is not identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_TRITANOPIA) == 0,
          "TRITANOPIA is not identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_COUNT) == 1,
          "out-of-range high -> identity");
    check(M11_Colorblind_IsIdentity(-7) == 1,
          "out-of-range negative -> identity");
}

static void test_remap_noop(void) {
    uint8_t r = 17, g = 33, b = 9;

    M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_OFF);
    check(r == 17 && g == 33 && b == 9,
          "RemapRGB OFF leaves bytes unchanged");

    r = 17; g = 33; b = 9;
    M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_COUNT);
    check(r == 17 && g == 33 && b == 9,
          "RemapRGB out-of-range high leaves bytes unchanged");

    r = 17; g = 33; b = 9;
    M11_Colorblind_RemapRGB(&r, &g, &b, -3);
    check(r == 17 && g == 33 && b == 9,
          "RemapRGB negative mode leaves bytes unchanged");

    {
        uint8_t anyR = 5;
        M11_Colorblind_RemapRGB(NULL, &anyR, &anyR,
                                M11_COLORBLIND_DEUTERANOPIA);
        check(anyR == 5, "RemapRGB NULL r no write");
    }
    {
        uint8_t anyG = 5;
        M11_Colorblind_RemapRGB(&anyG, NULL, &anyG,
                                M11_COLORBLIND_DEUTERANOPIA);
        check(anyG == 5, "RemapRGB NULL g no write");
    }
    {
        uint8_t anyB = 5;
        M11_Colorblind_RemapRGB(&anyB, &anyB, NULL,
                                M11_COLORBLIND_DEUTERANOPIA);
        check(anyB == 5, "RemapRGB NULL b no write");
    }
    M11_Colorblind_RemapRGB(NULL, NULL, NULL,
                            M11_COLORBLIND_DEUTERANOPIA);
    check(1, "RemapRGB all NULL does not crash");
}

static void test_remap_math_deuteranopia(void) {
    assert_remap(M11_COLORBLIND_DEUTERANOPIA, kDeutero,
                 255, 0, 0, "DEUT: pure red");
    assert_remap(M11_COLORBLIND_DEUTERANOPIA, kDeutero,
                 0, 255, 0, "DEUT: pure green");
    assert_remap(M11_COLORBLIND_DEUTERANOPIA, kDeutero,
                 0, 0, 255, "DEUT: pure blue");
    assert_remap(M11_COLORBLIND_DEUTERANOPIA, kDeutero,
                 0, 0, 0, "DEUT: pure black");
    assert_remap(M11_COLORBLIND_DEUTERANOPIA, kDeutero,
                 255, 255, 255, "DEUT: pure white");
    assert_remap(M11_COLORBLIND_DEUTERANOPIA, kDeutero,
                 128, 128, 128, "DEUT: mid grey");
    assert_remap(M11_COLORBLIND_DEUTERANOPIA, kDeutero,
                 255, 255, 0, "DEUT: yellow");
}

static void test_remap_math_protanopia(void) {
    assert_remap(M11_COLORBLIND_PROTANOPIA, kProtan,
                 255, 0, 0, "PROT: pure red");
    assert_remap(M11_COLORBLIND_PROTANOPIA, kProtan,
                 0, 255, 0, "PROT: pure green");
    assert_remap(M11_COLORBLIND_PROTANOPIA, kProtan,
                 0, 0, 255, "PROT: pure blue");
    assert_remap(M11_COLORBLIND_PROTANOPIA, kProtan,
                 0, 0, 0, "PROT: pure black");
    assert_remap(M11_COLORBLIND_PROTANOPIA, kProtan,
                 255, 255, 255, "PROT: pure white");
    assert_remap(M11_COLORBLIND_PROTANOPIA, kProtan,
                 128, 128, 128, "PROT: mid grey");
    assert_remap(M11_COLORBLIND_PROTANOPIA, kProtan,
                 255, 255, 0, "PROT: yellow");
}

static void test_remap_math_tritanopia(void) {
    assert_remap(M11_COLORBLIND_TRITANOPIA, kTritan,
                 255, 0, 0, "TRIT: pure red");
    assert_remap(M11_COLORBLIND_TRITANOPIA, kTritan,
                 0, 255, 0, "TRIT: pure green");
    assert_remap(M11_COLORBLIND_TRITANOPIA, kTritan,
                 0, 0, 255, "TRIT: pure blue");
    assert_remap(M11_COLORBLIND_TRITANOPIA, kTritan,
                 0, 0, 0, "TRIT: pure black");
    assert_remap(M11_COLORBLIND_TRITANOPIA, kTritan,
                 255, 255, 255, "TRIT: pure white");
    assert_remap(M11_COLORBLIND_TRITANOPIA, kTritan,
                 128, 128, 128, "TRIT: mid grey");
    assert_remap(M11_COLORBLIND_TRITANOPIA, kTritan,
                 255, 255, 0, "TRIT: yellow");
}

static void test_remap_range_edges(void) {
    {
        uint8_t r = 0, g = 255, b = 255;
        int er, eg, eb;
        expected_remap(kDeutero, 0, 255, 255, &er, &eg, &eb);
        M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_DEUTERANOPIA);
        check((int)r == er && (int)g == eg && (int)b == eb,
              "DEUT: cyan range edge matches expected");
    }
    {
        uint8_t r = 255, g = 0, b = 255;
        int er, eg, eb;
        expected_remap(kProtan, 255, 0, 255, &er, &eg, &eb);
        M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_PROTANOPIA);
        check((int)r == er && (int)g == eg && (int)b == eb,
              "PROT: magenta range edge matches expected");
    }
    {
        uint8_t r = 255, g = 255, b = 0;
        int er, eg, eb;
        expected_remap(kTritan, 255, 255, 0, &er, &eg, &eb);
        M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_TRITANOPIA);
        check((int)r == er && (int)g == eg && (int)b == eb,
              "TRIT: yellow range edge matches expected");
    }
}

static void test_apply_rgba_safety(void) {
    uint8_t pixels[16] = {
        10, 20, 30, 40,  50, 60, 70, 80,
        80, 70, 60, 50,  30, 20, 10, 90
    };
    uint8_t original[16];
    memcpy(original, pixels, sizeof(original));

    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, NULL, 2, 2);
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, pixels, 0, 2);
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, pixels, 2, 0);
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, pixels, -1, 2);
    check(memcmp(pixels, original, sizeof(original)) == 0,
          "ApplyRGBA safety paths leave buffer byte-identical");

    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_OFF, pixels, 2, 2);
    check(memcmp(pixels, original, sizeof(original)) == 0,
          "ApplyRGBA OFF leaves buffer byte-identical");

    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_COUNT, pixels, 2, 2);
    M11_Colorblind_ApplyRGBA(-7, pixels, 2, 2);
    check(memcmp(pixels, original, sizeof(original)) == 0,
          "ApplyRGBA out-of-range leaves buffer byte-identical");

    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, pixels, 2, 2);
    check(pixels[3] == 40 && pixels[7] == 80 &&
          pixels[11] == 50 && pixels[15] == 90,
          "ApplyRGBA DEUT preserves alpha channel");
}

static void test_apply_rgba_matches_remaprgb(void) {
    uint8_t single[4] = { 200, 100, 50, 255 };
    int er, eg, eb;

    expected_remap(kDeutero, 200, 100, 50, &er, &eg, &eb);
    {
        uint8_t r = single[0], g = single[1], b = single[2];
        M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_DEUTERANOPIA);
        check((int)r == er && (int)g == eg && (int)b == eb,
              "RemapRGB DEUT(200,100,50) matches expected");
    }
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, single, 1, 1);
    check((int)single[0] == er && (int)single[1] == eg && (int)single[2] == eb,
          "ApplyRGBA single pixel DEUT matches RemapRGB");
    check(single[3] == 255, "ApplyRGBA single pixel alpha untouched");

    {
        uint8_t buf[12] = {
            255, 0,   0,   200,
              0, 255, 0,   201,
              0, 0,   255, 202
        };
        uint8_t bufCopy[12];
        memcpy(bufCopy, buf, sizeof(bufCopy));
        M11_Colorblind_ApplyRGBA(M11_COLORBLIND_PROTANOPIA, buf, 3, 1);
        for (int i = 0; i < 3; ++i) {
            char tag[64];
            expected_remap(kProtan,
                           bufCopy[i*4 + 0],
                           bufCopy[i*4 + 1],
                           bufCopy[i*4 + 2],
                           &er, &eg, &eb);
            snprintf(tag, sizeof(tag),
                     "PROT pixel %d matches RemapRGB", i);
            check((int)buf[i*4 + 0] == er &&
                  (int)buf[i*4 + 1] == eg &&
                  (int)buf[i*4 + 2] == eb,
                  tag);
            snprintf(tag, sizeof(tag),
                     "PROT pixel %d alpha preserved (200+i)", i);
            check(buf[i*4 + 3] == (uint8_t)(200 + i), tag);
        }
    }

    {
        uint8_t buf[16] = {
            100, 150, 200, 7,
             50, 100, 150, 9,
            200, 100,  50, 11,
             25,  50,  75, 13
        };
        uint8_t bufCopy[16];
        int idx;
        memcpy(bufCopy, buf, sizeof(bufCopy));
        M11_Colorblind_ApplyRGBA(M11_COLORBLIND_TRITANOPIA, buf, 2, 2);
        for (idx = 0; idx < 4; ++idx) {
            char tag[64];
            expected_remap(kTritan,
                           bufCopy[idx*4 + 0],
                           bufCopy[idx*4 + 1],
                           bufCopy[idx*4 + 2],
                           &er, &eg, &eb);
            snprintf(tag, sizeof(tag),
                     "TRIT 2x2 pixel %d matches RemapRGB", idx);
            check((int)buf[idx*4 + 0] == er &&
                  (int)buf[idx*4 + 1] == eg &&
                  (int)buf[idx*4 + 2] == eb,
                  tag);
            snprintf(tag, sizeof(tag),
                     "TRIT 2x2 alpha byte preserved (%d)", idx);
            check(buf[idx*4 + 3] == bufCopy[idx*4 + 3], tag);
        }
    }
}

int main(void) {
    test_labels();
    test_identity_detection();
    test_remap_noop();
    test_remap_math_deuteranopia();
    test_remap_math_protanopia();
    test_remap_math_tritanopia();
    test_remap_range_edges();
    test_apply_rgba_safety();
    test_apply_rgba_matches_remaprgb();
    if (g_failures) {
        printf("test_m11_colorblind_m11: FAIL %d\n", g_failures);
        return 1;
    }
    puts("test_m11_colorblind_m11: PASS");
    return 0;
}
