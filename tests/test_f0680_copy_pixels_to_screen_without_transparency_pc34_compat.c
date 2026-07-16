#include "f0680_copy_pixels_to_screen_without_transparency_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    const uint8_t source[] = {0xa0u, 0x5fu, 0x19u};
    uint8_t destination[8];
    uint8_t before[8];
    int ok = 1;

    memset(destination, 0xcc, sizeof(destination));
    ok &= check(f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
                    source, sizeof(source), 0u, destination, sizeof(destination), 1u,
                    6u) == 1,
                "accepts complete packed source and destination pixel ranges");
    ok &= check(destination[0] == 0xcc && destination[1] == 0x0au &&
                    destination[2] == 0x00u && destination[3] == 0x05u &&
                    destination[4] == 0x0fu && destination[5] == 0x01u &&
                    destination[6] == 0x09u && destination[7] == 0xcc,
                "expands high then low nibbles and copies zero without transparency");

    memset(destination, 0xcc, sizeof(destination));
    ok &= check(f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
                    source, sizeof(source), 1u, destination, sizeof(destination), 2u,
                    3u) == 1 &&
                    destination[1] == 0xcc && destination[2] == 0x00u &&
                    destination[3] == 0x05u && destination[4] == 0x0fu &&
                    destination[5] == 0xcc,
                "accepts odd source and nonzero destination pixel offsets");

    memset(destination, 0xa5, sizeof(destination));
    memcpy(before, destination, sizeof(destination));
    ok &= check(f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
                    source, 2u, 0u, destination, sizeof(destination), 0u, 5u) == 0 &&
                    f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
                    source, sizeof(source), 0u, destination, 4u, 0u, 5u) == 0 &&
                    f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
                    source, sizeof(source), 6u, destination, sizeof(destination), 0u,
                    1u) == 0 &&
                    memcmp(destination, before, sizeof(destination)) == 0,
                "rejects incomplete source or destination ranges without mutation");
    ok &= check(f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
                    NULL, sizeof(source), 0u, destination, sizeof(destination), 0u, 1u) ==
                    0 &&
                    f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
                    source, sizeof(source), 0u, NULL, sizeof(destination), 0u, 1u) == 0,
                "rejects missing caller-owned source or destination bytes");
    ok &= check(strstr(f0680_copy_pixels_to_screen_without_transparency_pc34_compat_source_evidence(),
                       "ANIMIMG.C:269") != NULL,
                "source evidence identifies the exact ANIMIMG call site");

    if (!ok) {
        return 1;
    }
    puts("PASS f0680_copy_pixels_to_screen_without_transparency_pc34_compat");
    return 0;
}
