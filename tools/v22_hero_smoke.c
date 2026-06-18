/*
 * v22_hero_smoke.c - V2.2 modern asset pack hero smoke test
 *
 * Verifies that:
 *   1. m11_v22_modern_assets_available() returns 1 (asset pack discovered)
 *   2. m11_v22_validate_manifest() returns 1 (strict validator happy with
 *      v1.1+ top-level category format)
 *   3. m11_v22_set_installed(1) + m11_v22_get_installed() == 1
 *      (M12 simulation)
 *
 * Path: build as a small standalone C program that links against
 * libfirestaff_v2.a (which holds m11_v22_* symbols). See
 * tools/CMakeLists.txt v22_hero_smoke target for the link command.
 */
#include <stdio.h>

extern void m11_v22_set_manifest_path(const char* data_dir);
extern int  m11_v22_modern_assets_available(void);
extern int  m11_v22_validate_manifest(const char* manifest_path);
extern void m11_v22_set_installed(int installed);
extern int  m11_v22_get_installed(void);

int main(void) {
    int passed = 0, total = 0;
    const char* data_dir = "/Users/bosse/.firestaff/data/dm1";
    const char* manifest_path = "/Users/bosse/.firestaff/assets/dm1/modern/modern_asset_manifest.json";

    printf("=== V2.2 modern asset pack hero smoke ===\n");
    m11_v22_set_manifest_path(data_dir);

    int avail = m11_v22_modern_assets_available();
    printf("modern_assets_available: %d\n", avail);
    total++; passed += (avail == 1);

    /* v1.1+ manifest uses top-level category keys (wall_shapes, floor_shapes, ...)
     * which is what the strict validator expects. */
    int valid = m11_v22_validate_manifest(manifest_path);
    printf("validate_manifest (v1.1 top-level format): %d\n", valid);
    total++; passed += (valid == 1);

    /* Simulate M12: install = available */
    m11_v22_set_installed(avail);
    int installed = m11_v22_get_installed();
    printf("get_installed (after M12 simulation): %d\n", installed);
    total++; passed += (installed == 1);

    printf("\n=== %d/%d PASS ===\n", passed, total);
    return (passed == total) ? 0 : 1;
}