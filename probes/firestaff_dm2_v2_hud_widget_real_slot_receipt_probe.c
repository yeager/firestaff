/*
 * firestaff_dm2_v2_hud_widget_real_slot_receipt_probe.c
 *
 * Scratch-only receipt for one DM2 V2 HUD chrome slot that is larger
 * than the checked-in synthetic 1x1 fixtures. The probe writes a valid
 * 32x32 RGBA PNG for compass_rose, installs a manifest whose other
 * slots are still placeholders, and proves the asset gate promotes that
 * one chrome slot to REAL while the overall pack remains PARTIAL.
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - include/dm2_v2_hud_widget_assets.h (module under test)
 */

#include "dm2_v2_hud_widget_assets.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#endif

static int s_pass = 0;
static int s_fail = 0;

static int portable_mkdir(const char* path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0777);
#endif
}

static void check(const char* name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_pass++;
    } else {
        printf("  FAIL: %s\n", name);
        s_fail++;
    }
}

static int ensure_dir(const char* path) {
    char tmp[1024];
    size_t len;

    if (!path || path[0] == '\0') return 0;
    len = strlen(path);
    if (len >= sizeof(tmp)) return 0;
    memcpy(tmp, path, len + 1U);
    for (char* p = tmp + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (portable_mkdir(tmp) != 0 && errno != EEXIST) return 0;
            *p = '/';
        }
    }
    if (portable_mkdir(tmp) != 0 && errno != EEXIST) return 0;
    return 1;
}

static void join_path(char* out, size_t out_size,
                      const char* a, const char* b) {
    if (!out || out_size == 0U) return;
    if (!a || a[0] == '\0') {
        snprintf(out, out_size, "%s", b ? b : "");
    } else if (!b || b[0] == '\0') {
        snprintf(out, out_size, "%s", a);
    } else if (a[strlen(a) - 1U] == '/') {
        snprintf(out, out_size, "%s%s", a, b);
    } else {
        snprintf(out, out_size, "%s/%s", a, b);
    }
}

static void dirname_of(char* out, size_t out_size, const char* path) {
    const char* slash;
    size_t len;

    if (!out || out_size == 0U) return;
    out[0] = '\0';
    if (!path) return;
    slash = strrchr(path, '/');
    if (!slash) {
        snprintf(out, out_size, ".");
        return;
    }
    len = (size_t)(slash - path);
    if (len >= out_size) len = out_size - 1U;
    memcpy(out, path, len);
    out[len] = '\0';
}

static uint32_t crc32_update(uint32_t crc, const unsigned char* data,
                             size_t len) {
    crc = ~crc;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint32_t)data[i];
        for (int bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0U - (crc & 1U);
            crc = (crc >> 1) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

static uint32_t adler32_update(const unsigned char* data, size_t len) {
    uint32_t a = 1U;
    uint32_t b = 0U;
    for (size_t i = 0; i < len; ++i) {
        a = (a + data[i]) % 65521U;
        b = (b + a) % 65521U;
    }
    return (b << 16) | a;
}

static int write_u32_be(FILE* fp, uint32_t v) {
    unsigned char b[4];
    b[0] = (unsigned char)((v >> 24) & 0xFFU);
    b[1] = (unsigned char)((v >> 16) & 0xFFU);
    b[2] = (unsigned char)((v >> 8) & 0xFFU);
    b[3] = (unsigned char)(v & 0xFFU);
    return fwrite(b, 1, sizeof(b), fp) == sizeof(b);
}

static int write_chunk(FILE* fp, const char type[4],
                       const unsigned char* data, uint32_t len) {
    uint32_t crc;

    if (!write_u32_be(fp, len)) return 0;
    if (fwrite(type, 1, 4, fp) != 4U) return 0;
    if (len > 0U && fwrite(data, 1, len, fp) != len) return 0;
    crc = crc32_update(0U, (const unsigned char*)type, 4U);
    crc = ~crc;
    for (uint32_t i = 0; i < len; ++i) {
        crc ^= (uint32_t)data[i];
        for (int bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0U - (crc & 1U);
            crc = (crc >> 1) ^ (0xEDB88320U & mask);
        }
    }
    crc = ~crc;
    return write_u32_be(fp, crc);
}

static int write_receipt_png(const char* path, int width, int height) {
    static const unsigned char sig[8] = {
        0x89U, 0x50U, 0x4EU, 0x47U, 0x0DU, 0x0AU, 0x1AU, 0x0AU
    };
    FILE* fp;
    size_t raw_len;
    size_t z_len;
    unsigned char* raw = NULL;
    unsigned char* z = NULL;
    unsigned char ihdr[13];
    uint32_t adler;
    int ok = 0;

    if (!path || width <= 1 || height <= 1) return 0;
    raw_len = (size_t)height * (1U + (size_t)width * 4U);
    if (raw_len > 65535U) return 0;
    z_len = 2U + 5U + raw_len + 4U;
    raw = (unsigned char*)calloc(raw_len, 1U);
    z = (unsigned char*)calloc(z_len, 1U);
    if (!raw || !z) goto cleanup;

    for (int y = 0; y < height; ++y) {
        unsigned char* row = raw + (size_t)y * (1U + (size_t)width * 4U);
        row[0] = 0U;
        for (int x = 0; x < width; ++x) {
            unsigned char* px = row + 1U + (size_t)x * 4U;
            int dx = x - (width / 2);
            int dy = y - (height / 2);
            int arm = (x == width / 2) || (y == height / 2) ||
                      (dx == dy) || (dx == -dy);
            px[0] = arm ? 230U : (unsigned char)(28U + (x % 16));
            px[1] = arm ? 218U : (unsigned char)(46U + (y % 20));
            px[2] = arm ? 104U : 96U;
            px[3] = arm ? 255U : 192U;
        }
    }

    z[0] = 0x78U;
    z[1] = 0x01U;
    z[2] = 0x01U; /* final uncompressed DEFLATE block */
    z[3] = (unsigned char)(raw_len & 0xFFU);
    z[4] = (unsigned char)((raw_len >> 8) & 0xFFU);
    z[5] = (unsigned char)((~raw_len) & 0xFFU);
    z[6] = (unsigned char)(((~raw_len) >> 8) & 0xFFU);
    memcpy(z + 7U, raw, raw_len);
    adler = adler32_update(raw, raw_len);
    z[7U + raw_len] = (unsigned char)((adler >> 24) & 0xFFU);
    z[8U + raw_len] = (unsigned char)((adler >> 16) & 0xFFU);
    z[9U + raw_len] = (unsigned char)((adler >> 8) & 0xFFU);
    z[10U + raw_len] = (unsigned char)(adler & 0xFFU);

    ihdr[0] = (unsigned char)((width >> 24) & 0xFF);
    ihdr[1] = (unsigned char)((width >> 16) & 0xFF);
    ihdr[2] = (unsigned char)((width >> 8) & 0xFF);
    ihdr[3] = (unsigned char)(width & 0xFF);
    ihdr[4] = (unsigned char)((height >> 24) & 0xFF);
    ihdr[5] = (unsigned char)((height >> 16) & 0xFF);
    ihdr[6] = (unsigned char)((height >> 8) & 0xFF);
    ihdr[7] = (unsigned char)(height & 0xFF);
    ihdr[8] = 8U; /* 8-bit */
    ihdr[9] = 6U; /* RGBA */
    ihdr[10] = 0U;
    ihdr[11] = 0U;
    ihdr[12] = 0U;

    fp = fopen(path, "wb");
    if (!fp) goto cleanup;
    ok = fwrite(sig, 1, sizeof(sig), fp) == sizeof(sig) &&
         write_chunk(fp, "IHDR", ihdr, sizeof(ihdr)) &&
         write_chunk(fp, "IDAT", z, (uint32_t)z_len) &&
         write_chunk(fp, "IEND", NULL, 0U);
    if (fclose(fp) != 0) ok = 0;

cleanup:
    free(raw);
    free(z);
    return ok;
}

static int png_ihdr_dimensions(const char* path, int* out_w, int* out_h) {
    unsigned char b[24];
    FILE* fp;

    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(b, 1, sizeof(b), fp) != sizeof(b)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    if (memcmp(b, "\x89PNG\r\n\x1a\n", 8) != 0) return 0;
    if (memcmp(b + 12, "IHDR", 4) != 0) return 0;
    if (out_w) {
        *out_w = ((int)b[16] << 24) | ((int)b[17] << 16) |
                 ((int)b[18] << 8) | (int)b[19];
    }
    if (out_h) {
        *out_h = ((int)b[20] << 24) | ((int)b[21] << 16) |
                 ((int)b[22] << 8) | (int)b[23];
    }
    return 1;
}

static int write_manifest(const char* path) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    fputs("{\n"
          "  \"manifestVersion\": \"1.0.0\",\n"
          "  \"packId\": \"dm2-v2-hud-widget-real-slot-receipt\",\n"
          "  \"comment\": \"Scratch-only 32x32 compass_rose receipt; not finished shipped art.\",\n"
          "  \"hud_widgets\": [\n"
          "    { \"id\": \"inventory_quick_view\", \"generator\": \"placeholder\", \"source_file\": \"placeholder.png\", \"width\": 64, \"height\": 32 },\n"
          "    { \"id\": \"action_prompt\", \"generator\": \"placeholder\", \"source_file\": \"placeholder.png\", \"width\": 48, \"height\": 16 },\n"
          "    { \"id\": \"compass_rose\", \"generator\": \"pbr_hero_receipt\", \"source_file\": \"compass_rose_receipt.png\", \"width\": 32, \"height\": 32 },\n"
          "    { \"id\": \"depth_indicator\", \"generator\": \"placeholder\", \"source_file\": \"placeholder.png\", \"width\": 40, \"height\": 16 },\n"
          "    { \"id\": \"gold_counter\", \"generator\": \"placeholder\", \"source_file\": \"placeholder.png\", \"width\": 56, \"height\": 16 },\n"
          "    { \"id\": \"champion_bar_frame\", \"generator\": \"placeholder\", \"source_file\": \"placeholder.png\", \"width\": 64, \"height\": 8 },\n"
          "    { \"id\": \"action_strip_frame\", \"generator\": \"placeholder\", \"source_file\": \"placeholder.png\", \"width\": 28, \"height\": 28 }\n"
          "  ]\n"
          "}\n", fp);
    if (fclose(fp) != 0) return 0;
    return 1;
}

int main(void) {
    char manifest_path[1024];
    char manifest_dir[1024];
    char chrome_dir[1024];
    char png_path[1024];
    DM2_V2_HudWidgetSlotInfo info;
    int total = 0;
    int real = 0;
    int png_w = 0;
    int png_h = 0;

    printf("=== DM2 V2 HUD Widget Real Slot Receipt probe ===\n");

    (void)system("rm -rf /tmp/scratch/dm2_hwa_real_slot_receipt_probe");
    (void)system("mkdir -p /tmp/scratch/dm2_hwa_real_slot_receipt_probe/firestaff-data/dm2");
    dm2_v2_hud_widget_assets_set_manifest_path(
        "/tmp/scratch/dm2_hwa_real_slot_receipt_probe/firestaff-data/dm2");

    snprintf(manifest_path, sizeof(manifest_path), "%s",
             dm2_v2_hud_widget_assets_get_manifest_path());
    dirname_of(manifest_dir, sizeof(manifest_dir), manifest_path);
    join_path(chrome_dir, sizeof(chrome_dir), manifest_dir, "hud_chrome");
    join_path(png_path, sizeof(png_path), chrome_dir, "compass_rose_receipt.png");

    check("manifest directory created", ensure_dir(manifest_dir));
    check("hud_chrome directory created", ensure_dir(chrome_dir));
    check("write one 32x32 compass_rose receipt PNG",
          write_receipt_png(png_path, 32, 32));
    check("receipt PNG IHDR is readable",
          png_ihdr_dimensions(png_path, &png_w, &png_h));
    check("receipt PNG dimensions are 32x32, not 1x1",
          png_w == 32 && png_h == 32);
    check("write placeholder-plus-receipt manifest",
          write_manifest(manifest_path));

    check("manifest validates structurally",
          dm2_v2_hud_widget_assets_validate_manifest(NULL) == 1);
    check("receipt pack -> PARTIAL gate",
          dm2_v2_hud_widget_assets_gate() == DM2_V2_HUD_WIDGET_GATE_PARTIAL);
    check("PARTIAL receipt -> installed=1",
          dm2_v2_hud_widget_assets_get_installed() == 1);
    real = dm2_v2_hud_widget_assets_real_count(&total);
    check("receipt pack real_count=1", real == 1);
    check("receipt pack declares all 7 slots", total == 7);

    check("compass_rose slot -> REAL",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_COMPASS_ROSE) ==
              DM2_V2_HUD_WIDGET_CLASS_REAL);
    check("compass_rose disables procedural placeholder",
          dm2_v2_hud_widget_assets_uses_placeholder(
              DM2_V2_HUD_WIDGET_COMPASS_ROSE) == 0);
    check("inventory_quick_view remains PLACEHOLDER",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("action_strip_frame remains PLACEHOLDER",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME) ==
              DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER);
    check("placeholder slots still use procedural fallback",
          dm2_v2_hud_widget_assets_uses_placeholder(
              DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME) == 1);

    check("compass_rose slot info resolved from hud_chrome",
          dm2_v2_hud_widget_assets_get_slot_info(
              DM2_V2_HUD_WIDGET_COMPASS_ROSE, &info) == 1 &&
          info.file_exists == 1 &&
          strcmp(info.category, "hud_chrome") == 0 &&
          strstr(info.resolved_path, "/hud_chrome/compass_rose_receipt.png") != NULL);
    check("compass_rose slot info records receipt generator",
          strcmp(info.generator, "pbr_hero_receipt") == 0);
    check("compass_rose manifest dimensions match PNG IHDR",
          info.width == png_w && info.height == png_h);
    check("compass_rose receipt is larger than synthetic 1x1 fixtures",
          info.width > 1 && info.height > 1);

    {
        const char* ev = dm2_v2_hud_widget_assets_source_evidence();
        check("source evidence cites DM2 HUD source",
              ev && strstr(ev, "SKULL.ASM T560") != NULL);
        check("source evidence keeps no-finished-art boundary",
              ev && strstr(ev, "finished PBR widget art") != NULL);
    }

    (void)system("rm -rf /tmp/scratch/dm2_hwa_real_slot_receipt_probe");
    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
