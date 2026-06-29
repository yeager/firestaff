/*
 * test_memory_savegame_pc34_f0417_saveutil_port_pc34_compat.c
 *
 * F0417 SaveUtil minimal-port regression (M10 Phase 15 save-side
 * helper). Pins the byte-level XOR obfuscation + per-section key
 * derivation that ships alongside the LSV-01 byte-stable PC 3.4
 * exporter.
 *
 * The existing LSV-01/LSV-02 regression test
 * (tests/test_dm1_v1_savegame_pc34_native_export_pc34_compat.c)
 * covers the F0798_SAVEGAME_PC34CPSCObfuscate_Compat word-level
 * reversible XOR and the F0417_SAVEUTIL_GetChecksumAndObfuscate
 * citation in its header docs, but the F0417_SAVEUTIL_Port_Hint
 * + F0417_SAVEUTIL_GetChecksumAndObfuscate byte-level helpers
 * exposed by `memory_savegame_pc34_compat.h:466..476` were
 * unguarded. This gate locks:
 *
 *   1. F0417_SAVEUTIL_Port_Hint_Compat:
 *      - NULL hdr returns 0 (early-out).
 *      - When noiseSeed is NULL, the existing noise[] values are
 *        preserved (no-op on the seed array).
 *      - When noiseSeed is supplied, noise[0..9] is copied verbatim
 *        (LE words).
 *      - sectionKeys[0..15] are derived from the noise via the
 *        documented FNV-1a fold (basis 0x811C9DC5, prime 0x01000193).
 *      - sectionChecksums[0..15] are caller-supplied and the helper
 *        never touches them (locked by leaving a sentinel pattern
 *        intact across the call).
 *      - Two calls with the same noise produce identical sectionKeys
 *        (determinism).
 *      - Two different noise seeds produce different sectionKeys
 *        (sensitivity).
 *
 *   2. F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat:
 *      - NULL hdr / NULL buf / bufLen <= 0 all return 0 (no-op).
 *      - The byte-level XOR pass is its own inverse: two consecutive
 *        calls with the same noise seed restore the original bytes
 *        (mirrors the existing F0798 word-level contract for
 *        F0417_SAVEUTIL_GetChecksumAndObfuscate).
 *      - The pass is deterministic: two buffers filled from the same
 *        plaintext + same noise byte out the same.
 *      - Different noise[0] running keys produce different obfuscated
 *        output (so the noise is wired into the running key).
 *      - At least one byte must change after a single obfuscate call
 *        on a non-zero plaintext (sanity: the helper does work).
 *      - The pass clamps bufLen to the visible window (no read past
 *        the caller's buffer; verified by clearing a tail sentinel
 *        and asserting it stays clear after the call).
 *
 * ReDMCSB anchors:
 *   - SAVEHEAD.C F0417_SAVEUTIL_GetChecksumAndObfuscate (READWRIT.C).
 *   - SAVEHEAD.C:44, 97, 104 (the call sites the minimal port mirrors).
 *   - LOADSAVE.C F0433 / F0435 (save / load chain that consumes the
 *     minimal-port header).
 *   - DEFS.H:225-250 (SaveGameHeader_Compat layout).
 *
 * Pure data layer (M10 Phase 15). No UI, no IO, no globals.
 * Build linkage: firestaff_m10 only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_savegame_pc34_compat.h"

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL %s (line %d): %s\n", msg, __LINE__, #cond); \
            exit(1); \
        } \
    } while (0)

/* Test 1: F0417_SAVEUTIL_Port_Hint_Compat NULL guard. */
static void test_f0417_port_hint_null_hdr(void) {
    uint16_t seed[10] = {0x1111u, 0x2222u, 0x3333u, 0x4444u, 0x5555u,
                          0x6666u, 0x7777u, 0x8888u, 0x9999u, 0xAAA0u};
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(NULL, seed) == 0,
          "F0417 port hint: NULL hdr returns 0");
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(NULL, NULL) == 0,
          "F0417 port hint: NULL hdr + NULL seed returns 0");
    puts("  PASS f0417_port_hint_null_hdr");
}

/* Test 2: F0417_SAVEUTIL_Port_Hint_Compat copies the noise seed
 * and derives section keys deterministically. */
static void test_f0417_port_hint_copies_noise_and_derives_keys(void) {
    struct SaveGameHeader_Compat hdr;
    struct SaveGameHeader_Compat copy;
    static const uint16_t seedA[10] = {0x1111u, 0x2222u, 0x3333u, 0x4444u,
                                        0x5555u, 0x6666u, 0x7777u, 0x8888u,
                                        0x9999u, 0xAAA0u};
    static const uint16_t seedB[10] = {0xDEADu, 0xBEEFu, 0xCAFEu, 0xBABEu,
                                        0xF00Du, 0x1234u, 0x5678u, 0x9ABCu,
                                        0xDEF0u, 0x0FEDu};
    int i;
    uint32_t sentinel = 0xDEADBEEFu;

    memset(&hdr, 0, sizeof(hdr));
    memset(&copy, 0, sizeof(copy));

    /* Pre-fill the sectionChecksums with a sentinel so we can prove
     * the helper leaves them alone (caller-supplied). */
    for (i = 0; i < 16; ++i) hdr.sectionChecksums[i] = sentinel;

    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdr, seedA) == 1,
          "F0417 port hint: seedA returns 1");

    /* Noise[0..9] is copied verbatim. */
    for (i = 0; i < 10; ++i) {
        CHECK(hdr.noise[i] == seedA[i],
              "F0417 port hint: noise[i] copies seedA[i]");
    }

    /* sectionChecksums[0..15] untouched by the helper. */
    for (i = 0; i < 16; ++i) {
        CHECK(hdr.sectionChecksums[i] == sentinel,
              "F0417 port hint: sectionChecksums are caller-owned");
    }

    /* Determinism: re-run with the same seed and snapshot the
     * sectionKeys into `copy`. */
    memset(&copy, 0, sizeof(copy));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&copy, seedA) == 1,
          "F0417 port hint: seedA second call returns 1");
    for (i = 0; i < 16; ++i) {
        CHECK(hdr.sectionKeys[i] == copy.sectionKeys[i],
              "F0417 port hint: sectionKeys deterministic for same seed");
    }

    /* Sensitivity: seedB produces different sectionKeys. */
    memset(&copy, 0, sizeof(copy));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&copy, seedB) == 1,
          "F0417 port hint: seedB returns 1");
    {
        int differs = 0;
        for (i = 0; i < 16; ++i) {
            if (hdr.sectionKeys[i] != copy.sectionKeys[i]) {
                differs = 1;
                break;
            }
        }
        CHECK(differs == 1,
              "F0417 port hint: different seed produces different sectionKeys");
    }

    puts("  PASS f0417_port_hint_copies_noise_and_derives_keys");
}

/* Test 3: F0417_SAVEUTIL_Port_Hint_Compat with noiseSeed == NULL
 * is a no-op on the existing noise[] (preserves the prior values). */
static void test_f0417_port_hint_null_seed_preserves_noise(void) {
    struct SaveGameHeader_Compat hdr;
    static const uint16_t seed[10] = {0x0102u, 0x0304u, 0x0506u, 0x0708u,
                                       0x090Au, 0x0B0Cu, 0x0D0Eu, 0x0F10u,
                                       0x1112u, 0x1314u};
    uint16_t keyBefore[16];
    int i;

    memset(&hdr, 0, sizeof(hdr));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdr, seed) == 1,
          "F0417 port hint: priming pass returns 1");
    memcpy(keyBefore, hdr.sectionKeys, sizeof(keyBefore));

    /* Second call with NULL seed: noise must stay, keys re-derived
     * from the same noise -> keys stay bit-identical. */
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdr, NULL) == 1,
          "F0417 port hint: NULL seed returns 1");
    for (i = 0; i < 10; ++i) {
        CHECK(hdr.noise[i] == seed[i],
              "F0417 port hint: NULL seed preserves noise");
    }
    for (i = 0; i < 16; ++i) {
        CHECK(hdr.sectionKeys[i] == keyBefore[i],
              "F0417 port hint: NULL seed keeps sectionKeys stable");
    }

    puts("  PASS f0417_port_hint_null_seed_preserves_noise");
}

/* Test 4: F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat NULL and
 * zero-length guards. */
static void test_f0417_obfuscate_null_and_zero(void) {
    struct SaveGameHeader_Compat hdr;
    static const uint16_t seed[10] = {0xAAAAu, 0xBBBBu, 0xCCCCu, 0xDDDDu,
                                       0xEEEEu, 0xFFFFu, 0x0001u, 0x0002u,
                                       0x0003u, 0x0004u};
    unsigned char buf[8] = {0x11u, 0x22u, 0x33u, 0x44u,
                            0x55u, 0x66u, 0x77u, 0x88u};

    memset(&hdr, 0, sizeof(hdr));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdr, seed) == 1,
          "F0417 obfuscate: priming pass returns 1");

    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(NULL, buf, 8) == 0,
          "F0417 obfuscate: NULL hdr returns 0");
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdr, NULL, 8) == 0,
          "F0417 obfuscate: NULL buf returns 0");
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdr, buf, 0) == 0,
          "F0417 obfuscate: bufLen=0 returns 0");
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdr, buf, -1) == 0,
          "F0417 obfuscate: bufLen<0 returns 0");

    /* The buffer must stay intact when the helper bails out on a
     * bad argument. */
    {
        static const unsigned char pristine[8] = {0x11u, 0x22u, 0x33u, 0x44u,
                                                   0x55u, 0x66u, 0x77u, 0x88u};
        CHECK(memcmp(buf, pristine, sizeof(pristine)) == 0,
              "F0417 obfuscate: bad-input buffers are untouched");
    }

    puts("  PASS f0417_obfuscate_null_and_zero");
}

/* Test 5: F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat is its own
 * inverse (XOR pass mirrors the F0417 reversible contract). */
static void test_f0417_obfuscate_reversible(void) {
    struct SaveGameHeader_Compat hdr;
    static const uint16_t seed[10] = {0x1234u, 0x5678u, 0x9ABCu, 0xDEF0u,
                                       0x1357u, 0x2468u, 0x9ACEu, 0xBDF0u,
                                       0xCDEFu, 0x0FEDu};
    unsigned char buf[16];
    unsigned char bufCopy[16];
    int i;

    memset(&hdr, 0, sizeof(hdr));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdr, seed) == 1,
          "F0417 obfuscate: priming pass returns 1");

    for (i = 0; i < 16; ++i) buf[i] = (unsigned char)(0x40u ^ (unsigned char)i);
    memcpy(bufCopy, buf, sizeof(buf));

    /* First pass: obfuscate. */
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdr, buf, 16) == 1,
          "F0417 obfuscate: first pass returns 1");
    CHECK(memcmp(buf, bufCopy, sizeof(buf)) != 0,
          "F0417 obfuscate: first pass changes buffer bytes");

    /* Second pass with the same header restores the original. */
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdr, buf, 16) == 1,
          "F0417 obfuscate: second pass returns 1");
    CHECK(memcmp(buf, bufCopy, sizeof(buf)) == 0,
          "F0417 obfuscate: second pass restores original bytes");

    puts("  PASS f0417_obfuscate_reversible");
}

/* Test 6: F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat is
 * deterministic for the same noise + same plaintext. */
static void test_f0417_obfuscate_deterministic(void) {
    struct SaveGameHeader_Compat hdrA;
    struct SaveGameHeader_Compat hdrB;
    static const uint16_t seed[10] = {0xCAFEu, 0xBABEu, 0xF00Du, 0x0BADu,
                                       0xDEADu, 0xBEEFu, 0x1337u, 0x4242u,
                                       0x9001u, 0xABCDu};
    unsigned char bufA[32];
    unsigned char bufB[32];
    int i;

    memset(&hdrA, 0, sizeof(hdrA));
    memset(&hdrB, 0, sizeof(hdrB));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdrA, seed) == 1,
          "F0417 obfuscate: priming hdrA returns 1");
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdrB, seed) == 1,
          "F0417 obfuscate: priming hdrB returns 1");

    for (i = 0; i < 32; ++i) {
        bufA[i] = (unsigned char)(0x80u + (unsigned char)i);
        bufB[i] = (unsigned char)(0x80u + (unsigned char)i);
    }

    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdrA, bufA, 32) == 1,
          "F0417 obfuscate: hdrA pass returns 1");
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdrB, bufB, 32) == 1,
          "F0417 obfuscate: hdrB pass returns 1");

    CHECK(memcmp(bufA, bufB, sizeof(bufA)) == 0,
          "F0417 obfuscate: deterministic output across two headers");

    puts("  PASS f0417_obfuscate_deterministic");
}

/* Test 7: F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat is sensitive
 * to the noise seed (different noise[0] changes the running key). */
static void test_f0417_obfuscate_sensitive_to_noise(void) {
    struct SaveGameHeader_Compat hdrA;
    struct SaveGameHeader_Compat hdrB;
    static const uint16_t seedA[10] = {0x0001u, 0x0001u, 0x0001u, 0x0001u,
                                        0x0001u, 0x0001u, 0x0001u, 0x0001u,
                                        0x0001u, 0x0001u};
    static const uint16_t seedB[10] = {0xFFFFu, 0xFFFFu, 0xFFFFu, 0xFFFFu,
                                        0xFFFFu, 0xFFFFu, 0xFFFFu, 0xFFFFu,
                                        0xFFFFu, 0xFFFFu};
    unsigned char bufA[16];
    unsigned char bufB[16];
    int i;
    int differs = 0;

    memset(&hdrA, 0, sizeof(hdrA));
    memset(&hdrB, 0, sizeof(hdrB));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdrA, seedA) == 1,
          "F0417 obfuscate: priming seedA returns 1");
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdrB, seedB) == 1,
          "F0417 obfuscate: priming seedB returns 1");

    for (i = 0; i < 16; ++i) {
        bufA[i] = (unsigned char)(0xA0u + (unsigned char)i);
        bufB[i] = (unsigned char)(0xA0u + (unsigned char)i);
    }

    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdrA, bufA, 16) == 1,
          "F0417 obfuscate: seedA pass returns 1");
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdrB, bufB, 16) == 1,
          "F0417 obfuscate: seedB pass returns 1");

    /* At least one byte must differ after obfuscation, proving the
     * noise seed actually drives the running key (the documented
     * `runningKey = noise[0]` seed). */
    for (i = 0; i < 16; ++i) {
        if (bufA[i] != bufB[i]) {
            differs = 1;
            break;
        }
    }
    CHECK(differs == 1,
          "F0417 obfuscate: noise seed changes obfuscated output");

    puts("  PASS f0417_obfuscate_sensitive_to_noise");
}

/* Test 8: F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat respects the
 * caller's bufLen window — a tail sentinel stays untouched. */
static void test_f0417_obfuscate_window_clamp(void) {
    struct SaveGameHeader_Compat hdr;
    static const uint16_t seed[10] = {0x4242u, 0x4242u, 0x4242u, 0x4242u,
                                       0x4242u, 0x4242u, 0x4242u, 0x4242u,
                                       0x4242u, 0x4242u};
    unsigned char buf[24];
    static const unsigned char pristine[24] = {
        0x10u, 0x20u, 0x30u, 0x40u, 0x50u, 0x60u, 0x70u, 0x80u,
        0x90u, 0xA0u, 0xB0u, 0xC0u, 0xD0u, 0xE0u, 0xF0u, 0x11u,
        0x99u, 0x99u, 0x99u, 0x99u, 0x99u, 0x99u, 0x99u, 0x99u
    };
    int i;

    memset(&hdr, 0, sizeof(hdr));
    CHECK(F0417_SAVEUTIL_Port_Hint_Compat(&hdr, seed) == 1,
          "F0417 obfuscate: priming pass returns 1");

    memcpy(buf, pristine, sizeof(pristine));

    /* Obfuscate only the first 16 bytes — the trailing 8 sentinel
     * bytes must stay byte-identical to pristine[16..23]. */
    CHECK(F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(&hdr, buf, 16) == 1,
          "F0417 obfuscate: 16-byte pass returns 1");
    for (i = 16; i < 24; ++i) {
        CHECK(buf[i] == pristine[i],
              "F0417 obfuscate: bytes outside bufLen are untouched");
    }
    /* Sanity: at least one byte inside the window changed. */
    {
        int changedInside = 0;
        for (i = 0; i < 16; ++i) {
            if (buf[i] != pristine[i]) {
                changedInside = 1;
                break;
            }
        }
        CHECK(changedInside == 1,
              "F0417 obfuscate: bytes inside bufLen change");
    }

    puts("  PASS f0417_obfuscate_window_clamp");
}

int main(void) {
    puts("# memory_savegame_pc34_f0417_saveutil_port_pc34_compat");
    test_f0417_port_hint_null_hdr();
    test_f0417_port_hint_copies_noise_and_derives_keys();
    test_f0417_port_hint_null_seed_preserves_noise();
    test_f0417_obfuscate_null_and_zero();
    test_f0417_obfuscate_reversible();
    test_f0417_obfuscate_deterministic();
    test_f0417_obfuscate_sensitive_to_noise();
    test_f0417_obfuscate_window_clamp();
    puts("PASS memory_savegame_pc34_f0417_saveutil_port_pc34_compat");
    return 0;
}
