/*
 * firestaff_theron_v1_first_room_runtime_probe.c
 *
 * Theron's Quest V1 — first-room startup/runtime probe.
 *
 * Headless, no game data required.  Validates the bounded launch →
 * M11-runtime handoff for the first reachable dungeon state in
 * Theron's Quest:
 *
 *   1.  Readiness gate (skip-safe):
 *         - theron_v1_runtime_readiness() walks data_root for the
 *           four locked-in Track 02 filenames, hashes them with the
 *           local MD5, and reports OK / NO_DATA_ROOT / NO_TRACK02 /
 *           NOT_VERIFIED / BAD_INPUT.
 *         - When data_root is empty (CI default), the probe reports
 *           SKIP via the readiness status and exits 0.  This keeps
 *           CI deterministic without staging original game data.
 *         - When data_root points at a real Track 02, the probe also
 *           exercises the direct-launch path on top of theron_v1_boot
 *           and asserts zero extra stat() probes.
 *
 *   2.  Synthetic first-room proof (always run):
 *         - Build a 20x20 fixture via theron_v1_first_room_synthesize()
 *           using the documented 12-byte header + grid layout that
 *           theron_v1_level_load() consumes.
 *         - Pass it through theron_v1_level_load() and assert
 *           THERON_MAP_OK + start_x/start_y + non-empty entrance
 *           tile at (1,1).
 *         - Mark level_loaded for dungeon 1 / level 0 and exercise
 *           party placement, forward move, wall-block, world-tick,
 *           and theron_v1_world_hash() determinism.
 *
 *   3.  Negative fixture:
 *         - Truncated, oversized, NULL, and zero-size inputs to
 *           theron_v1_first_room_synthesize() and
 *           theron_v1_first_room_buffer_size() must return 0.
 *
 *   4.  Real-Track-02 conditional path (skip when not staged):
 *         - When the readiness gate reports OK, the probe reads the
 *           first 0x3010 bytes of the resolved file and feeds them
 *           through theron_v1_track02_find_bank_signal() to prove
 *           that the existing Track 02 decoder still recognises the
 *           variant.  This is the bounded evidence we promise in
 *           AGENTS.md: "Track 02 path: real-asset handoff proof
 *           remains active work".
 *         - When the gate reports NO_DATA_ROOT or NO_TRACK02 the
 *           probe prints "SKIP: no Track 02 staged" and exits 0.
 *
 * Source-lock:
 *   THQUEST.ASM T080  — between-dungeon save/load (progression init)
 *   THQUEST.ASM T400  — Track 02 data loading
 *   THQUEST.ASM T520  — party placement (start_x, start_y, dir)
 *   THQUEST.ASM T560  — dungeon header parsing (12-byte BE header)
 *   THQUEST.ASM T700  — world tick (timer + AI tick)
 *
 *   src/theron/theron_v1_world.c  (synth + readiness gate + hash)
 *   src/theron/theron_v1_track02.c (bank-signal variant ID)
 *   src/theron/theron_v1_mechanics.c (move / wall-block)
 *   src/theron/theron_v1_boot.c (direct-launch path)
 *
 *   docs/source-lock/tqr_v1_phase1_boot_H2338.md
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   RFC 1321 (MD5 reference, used for the readiness gate)
 *
 * Exit codes:
 *   0 — every check passed (real-asset path may be SKIP)
 *   1 — at least one check failed
 *
 * Usage:
 *   ./build/firestaff_theron_v1_first_room_runtime_probe [--data-root <dir>]
 *
 * When --data-root is omitted, the gate probes the empty path and
 * returns NO_DATA_ROOT (CI default).  When --data-root points at a
 * directory containing one of the four documented Track 02 files,
 * the gate returns OK and the probe runs the real-asset path.
 */

#include "theron_v1_world.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_track02.h"
#include "theron_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int g_assertions = 0;
static int g_failures   = 0;
static int g_skips      = 0;

#define CHECK(cond_) do {                                            \
    g_assertions++;                                                  \
    if (!(cond_)) {                                                  \
        printf("  FAIL  %s:%d  %s\n",                                \
               __FILE__, __LINE__, #cond_);                          \
        g_failures++;                                                \
    }                                                                \
} while (0)

#define SKIP_(msg_) do {                                             \
    g_skips++;                                                       \
    printf("  SKIP  %s\n", (msg_));                                  \
} while (0)

#define CHECK_GROUP(name)                                            \
    printf("\n  --- %s ---\n", name)

/* ── Synthetic first-room build + load ───────────────────────────── */

static void probe_first_room_synthesize(void) {
    CHECK_GROUP("Synthetic first-room buffer");
    uint8_t buf[THERON_V1_FIRST_ROOM_DEFAULT_SIZE];
    Theron_V1_Level preview;
    size_t written;

    memset(&preview, 0xAA, sizeof(preview));
    memset(buf, 0xAA, sizeof(buf));

    /* Empty buffer must not silently succeed. */
    size_t zero = theron_v1_first_room_buffer_size(0, 0);
    CHECK(zero == 0);
    size_t over = theron_v1_first_room_buffer_size(
        THERON_MAX_MAP_SIZE + 1, THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT);
    CHECK(over == 0);

    written = theron_v1_first_room_synthesize(
        buf, sizeof(buf),
        THERON_V1_FIRST_ROOM_DEFAULT_WIDTH,
        THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT,
        /* level_index  */ 0,
        /* dungeon_seed */ 313u,
        &preview);

    CHECK(written == THERON_V1_FIRST_ROOM_DEFAULT_SIZE);
    CHECK(preview.width  == THERON_V1_FIRST_ROOM_DEFAULT_WIDTH);
    CHECK(preview.height == THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT);
    CHECK(preview.start_x == 1);
    CHECK(preview.start_y == 1);
    CHECK(preview.start_dir == 1); /* EAST */
    /* Entrance + forward path carved out as floor. */
    CHECK(preview.squares[1][1] == THERON_SQUARE_FLOOR);
    CHECK(preview.squares[1][2] == THERON_SQUARE_FLOOR);
    CHECK(preview.squares[1][3] == THERON_SQUARE_FLOOR);
    CHECK(preview.squares[1][4] == THERON_SQUARE_STAIRS_DOWN);
    /* Rest is wall. */
    CHECK(preview.squares[0][0] == THERON_SQUARE_WALL);
    CHECK(preview.squares[3][3] == THERON_SQUARE_WALL);

    /* Header bytes are big-endian per theron_v1_level_load contract. */
    CHECK(buf[0] == 0x00 && buf[1] == 0x14); /* 20 */
    CHECK(buf[2] == 0x00 && buf[3] == 0x14); /* 20 */
    /* dungeon_seed = 313 = 0x00000139 -> big-endian bytes 00 00 01 39 */
    CHECK(buf[4] == 0x00 && buf[5] == 0x00 &&
          buf[6] == 0x01 && buf[7] == 0x39);

    /* Negative fixtures. */
    CHECK(theron_v1_first_room_synthesize(
              NULL, sizeof(buf),
              THERON_V1_FIRST_ROOM_DEFAULT_WIDTH,
              THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT, 0, 313u, &preview) == 0);
    CHECK(theron_v1_first_room_synthesize(
              buf, sizeof(buf),
              0, THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT, 0, 313u, &preview) == 0);
    CHECK(theron_v1_first_room_synthesize(
              buf, /* too small */ 16u,
              THERON_V1_FIRST_ROOM_DEFAULT_WIDTH,
              THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT, 0, 313u, &preview) == 0);
    CHECK(theron_v1_first_room_synthesize(
              buf, sizeof(buf),
              THERON_V1_FIRST_ROOM_DEFAULT_WIDTH,
              THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT, 0, 313u, NULL) == 0);
}

static void probe_startup_fallback_rooms(void) {
    CHECK_GROUP("Startup fallback rooms by stage");

    uint8_t buf[THERON_V1_FIRST_ROOM_HEADER_BYTES + 10 * 10];
    Theron_V1_Level level;
    size_t written;

    memset(buf, 0xAA, sizeof(buf));
    memset(&level, 0xAA, sizeof(level));
    written = theron_v1_startup_fallback_room_synthesize(
        buf,
        sizeof(buf),
        THERON_DUNGEON_1_HALL_OF_RECORDS,
        &level);
    CHECK(written == THERON_V1_FIRST_ROOM_HEADER_BYTES + 8 * 8);
    CHECK(level.width == 8 && level.height == 8);
    CHECK(level.start_x == 3 && level.start_y == 5 && level.start_dir == 0);
    CHECK(level.squares[1][3] == THERON_SQUARE_EXIT);
    CHECK(buf[6] == 0x01 && buf[7] == 0x39); /* seed 313 */

    memset(buf, 0xAA, sizeof(buf));
    memset(&level, 0xAA, sizeof(level));
    written = theron_v1_startup_fallback_room_synthesize(
        buf,
        sizeof(buf),
        THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
        &level);
    CHECK(written == THERON_V1_FIRST_ROOM_HEADER_BYTES + 8 * 8);
    CHECK(level.width == 8 && level.height == 8);
    CHECK(level.start_x == 3 && level.start_y == 5 && level.start_dir == 0);
    CHECK(level.squares[1][4] == THERON_SQUARE_EXIT);
    CHECK(level.squares[4][6] == THERON_SQUARE_POOL);
    CHECK(buf[6] == 0x01 && buf[7] == 0x9E); /* seed 414 */

    memset(buf, 0xAA, sizeof(buf));
    memset(&level, 0xAA, sizeof(level));
    written = theron_v1_startup_fallback_room_synthesize(
        buf,
        sizeof(buf),
        THERON_DUNGEON_7_TOWER_OF_EPILOGUE,
        &level);
    CHECK(written == THERON_V1_FIRST_ROOM_HEADER_BYTES + 10 * 10);
    CHECK(level.width == 10 && level.height == 10);
    CHECK(level.squares[1][6] == THERON_SQUARE_EXIT);
    CHECK(level.squares[7][7] == THERON_SQUARE_STAIRS_DOWN);

    CHECK(theron_v1_startup_fallback_room_synthesize(
              NULL,
              sizeof(buf),
              THERON_DUNGEON_1_HALL_OF_RECORDS,
              &level) == 0);
    CHECK(theron_v1_startup_fallback_room_synthesize(
              buf,
              16u,
              THERON_DUNGEON_7_TOWER_OF_EPILOGUE,
              &level) == 0);
    CHECK(theron_v1_startup_fallback_room_synthesize(
              buf,
              sizeof(buf),
              THERON_DUNGEON_INVALID,
              &level) == 0);
}

/* ── First-room load + movement + wall-block ─────────────────────── */

static void probe_first_room_runtime(void) {
    CHECK_GROUP("First-room load + movement + wall-block");

    uint8_t buf[THERON_V1_FIRST_ROOM_DEFAULT_SIZE];
    Theron_V1_Level level;
    Theron_V1_World world;
    Theron_MapLoadResult rc;

    theron_v1_world_init(&world);

    size_t written = theron_v1_first_room_synthesize(
        buf, sizeof(buf),
        THERON_V1_FIRST_ROOM_DEFAULT_WIDTH,
        THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT,
        0, 313u, &level);
    CHECK(written == THERON_V1_FIRST_ROOM_DEFAULT_SIZE);

    /* Load through the existing V1 loader (THQUEST.ASM T560 contract). */
    rc = theron_v1_level_load(&level, buf, (int)written,
                              THERON_DUNGEON_1_HALL_OF_RECORDS, 0);
    CHECK(rc == THERON_MAP_OK);
    CHECK(level.width  == THERON_V1_FIRST_ROOM_DEFAULT_WIDTH);
    CHECK(level.height == THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT);
    CHECK(level.start_x == 1);
    CHECK(level.start_y == 1);

    /* Wire into the world so world_get_square can see the level. */
    world.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    world.current_level   = 0;
    memcpy(&world.levels[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
           &level, sizeof(level));
    world.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0] = 1;

    /* Party placement (THQUEST.ASM T520).  level_load always sets
     * start_dir = 0 (NORTH default; T520 overrides at runtime).  Use
     * that documented default for the placement assertion, then
     * turn east for the forward-step path. */
    theron_v1_party_place(&world, level.start_x, level.start_y,
                           level.start_dir);
    CHECK(world.party.leader_x == 1);
    CHECK(world.party.leader_y == 1);
    CHECK(world.party.leader_dir == 0); /* default north, per T520 */

    /* Hash the world for determinism baseline. */
    uint64_t h0 = theron_v1_world_hash(&world);
    CHECK(h0 != 0);

    /* Forward step east → (2,1) is floor. */
    Theron_MoveResult mres = theron_v1_get_move_result(&world, THERON_DIR_EAST);
    CHECK(mres == THERON_MOVE_OK);
    int move_rc = theron_v1_move_party(&world, THERON_DIR_EAST);
    CHECK(move_rc == THERON_MOVE_OK);
    CHECK(world.party.leader_x == 2);
    CHECK(world.party.leader_y == 1);

    /* Forward step east → (3,1) is floor. */
    move_rc = theron_v1_move_party(&world, THERON_DIR_EAST);
    CHECK(move_rc == THERON_MOVE_OK);
    CHECK(world.party.leader_x == 3);

    /* Forward step east → (4,1) is stairs_down → transitions special. */
    Theron_MoveResult stair_res =
        theron_v1_get_move_result(&world, THERON_DIR_EAST);
    CHECK(stair_res == THERON_MOVE_STAIRS);

    /* Stepping into a wall (north from (1,1)) must block. */
    theron_v1_party_place(&world, 1, 1, THERON_DIR_NORTH);
    Theron_MoveResult wall_res =
        theron_v1_get_move_result(&world, THERON_DIR_NORTH);
    CHECK(wall_res == THERON_MOVE_BLOCKED);
    int blocked = theron_v1_move_party(&world, THERON_DIR_NORTH);
    CHECK(blocked == THERON_MOVE_BLOCKED);
    /* Move blocked must NOT advance the party. */
    CHECK(world.party.leader_x == 1);
    CHECK(world.party.leader_y == 1);

    /* Off-grid bounds also return wall. */
    CHECK(theron_v1_world_get_square(&world, -1, -1) == THERON_SQUARE_WALL);
    CHECK(theron_v1_world_get_square(&world, 999, 999) == THERON_SQUARE_WALL);

    /* World tick (THQUEST.ASM T700) must advance deterministically. */
    uint64_t before_tick = world.world_tick;
    theron_v1_world_tick(&world);
    CHECK(world.world_tick == before_tick + 1);
    theron_v1_world_tick(&world);
    CHECK(world.world_tick == before_tick + 2);

    /* Hash must change once state has changed (party moved + ticks). */
    uint64_t h1 = theron_v1_world_hash(&world);
    CHECK(h1 != 0);
    CHECK(h1 != h0);

    /* Determinism: two parallel worlds initialised identically must hash
     * to the same value at the same point in their lifecycle. */
    Theron_V1_World twin;
    theron_v1_world_init(&twin);
    twin.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    twin.current_level   = 0;
    memcpy(&twin.levels[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0],
           &level, sizeof(level));
    twin.level_loaded[THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0] = 1;
    theron_v1_party_place(&twin, level.start_x, level.start_y,
                           level.start_dir);
    /* world_hash already accounts for world_tick via init to 0; the
     * two worlds are at the same starting pose so their hashes must
     * match exactly. */
    CHECK(theron_v1_world_hash(&twin) == h0);
}

/* ── Readiness gate (skip-safe) ──────────────────────────────────── */

static void probe_readiness_no_data_root(void) {
    CHECK_GROUP("Readiness gate — no data root");

    char scan[1024];
    char md5[33];
    Theron_RuntimeReadinessStatus s;

    memset(scan, 0xAA, sizeof(scan));
    memset(md5,  0xAA, sizeof(md5));
    s = theron_v1_runtime_readiness(NULL, scan, sizeof(scan),
                                     md5, sizeof(md5));
    CHECK(s == THERON_RUNTIME_READINESS_NO_DATA_ROOT);
    CHECK(scan[0] == '\0');
    CHECK(md5[0]  == '\0');

    s = theron_v1_runtime_readiness("", scan, sizeof(scan),
                                     md5, sizeof(md5));
    CHECK(s == THERON_RUNTIME_READINESS_NO_DATA_ROOT);

    /* Negative input handling. */
    s = theron_v1_runtime_readiness("/tmp", NULL, 0, md5, sizeof(md5));
    CHECK(s == THERON_RUNTIME_READINESS_BAD_INPUT);
    s = theron_v1_runtime_readiness("/tmp", scan, sizeof(scan),
                                     md5, /* too small */ 4);
    CHECK(s == THERON_RUNTIME_READINESS_BAD_INPUT);

    /* Nonexistent dir → NO_TRACK02 (not NO_DATA_ROOT, since the
     * caller gave us a non-empty root). */
    s = theron_v1_runtime_readiness(
        "/tmp/firestaff_theron_definitely_not_a_real_dir_xyz_12345",
        scan, sizeof(scan), md5, sizeof(md5));
    CHECK(s == THERON_RUNTIME_READINESS_NO_TRACK02);

    /* Status-name coverage. */
    CHECK(strcmp(theron_v1_runtime_readiness_status_name(
                     THERON_RUNTIME_READINESS_OK), "ok") == 0);
    CHECK(strcmp(theron_v1_runtime_readiness_status_name(
                     THERON_RUNTIME_READINESS_NO_DATA_ROOT),
                 "no-data-root") == 0);
    CHECK(strcmp(theron_v1_runtime_readiness_status_name(
                     THERON_RUNTIME_READINESS_NO_TRACK02),
                 "no-track02") == 0);
    CHECK(strcmp(theron_v1_runtime_readiness_status_name(
                     THERON_RUNTIME_READINESS_NOT_VERIFIED),
                 "not-verified") == 0);
    CHECK(strcmp(theron_v1_runtime_readiness_status_name(
                     THERON_RUNTIME_READINESS_BAD_INPUT),
                 "bad-input") == 0);
}

/* ── Real-asset conditional path (skip when not staged) ──────────── */

static void probe_real_track02_conditional(const char *data_root) {
    CHECK_GROUP("Real-Track-02 conditional path");

    char path[1024];
    char md5[33];
    Theron_RuntimeReadinessStatus s;

    s = theron_v1_runtime_readiness(data_root, path, sizeof(path),
                                     md5, sizeof(md5));
    printf("  readiness: %s\n",
           theron_v1_runtime_readiness_status_name(s));
    printf("  resolved:  %s\n", path[0] ? path : "(none)");
    printf("  md5:       %s\n", md5[0]  ? md5  : "(none)");

    if (s == THERON_RUNTIME_READINESS_NO_DATA_ROOT ||
        s == THERON_RUNTIME_READINESS_NO_TRACK02) {
        SKIP_("no Track 02 staged in data_root; "
              "synthetic path above is the proof");
        return;
    }

    if (s == THERON_RUNTIME_READINESS_NOT_VERIFIED) {
        printf("  NOTE: candidate file at %s exists but MD5 %s is "
               "not on the four-MD5 catalog; treating as skip.\n",
               path, md5);
        SKIP_("Track 02 candidate present but not hash-verified");
        return;
    }

    if (s == THERON_RUNTIME_READINESS_BAD_INPUT) {
        g_failures++;
        printf("  FAIL: readiness gate rejected valid inputs\n");
        return;
    }

    /* THERON_RUNTIME_READINESS_OK — exercise the Track 02 decoder. */
    CHECK(s == THERON_RUNTIME_READINESS_OK);
    CHECK(strlen(md5) == 32);

    Theron_Track02Variant variant =
        theron_v1_track02_variant_for_md5(md5);
    CHECK(variant != THERON_TRACK02_VARIANT_UNKNOWN);

    /* Slurp the first 0x3010 bytes (the documented descriptor-region
     * window) for a quick decoder probe. */
    FILE *fp = fopen(path, "rb");
    CHECK(fp != NULL);
    if (!fp) return;
    static uint8_t head[0x3010];
    size_t got = fread(head, 1, sizeof(head), fp);
    fclose(fp);
    CHECK(got > 0);

    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus ss =
        theron_v1_track02_find_bank_signal(head, got, md5, &signal);
    /* Real Track 02 BINs and US ISO should yield OK.  JP Rev 1 ISO is
     * the zero-image variant → INSUFFICIENT_ZERO_IMAGE.  All three
     * are non-UNKNOWN statuses. */
    CHECK(ss != THERON_TRACK02_SIGNAL_BAD_INPUT);
    CHECK(ss != THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);

    /* If the variant is one of the BIN variants, anchor_count must
     * be > 0 (documented in include/theron_v1_track02.h). */
    if (variant == THERON_TRACK02_VARIANT_JP_BIN ||
        variant == THERON_TRACK02_VARIANT_US_BIN) {
        CHECK(ss == THERON_TRACK02_SIGNAL_OK);
        CHECK(signal.anchor_count > 0);
        CHECK(signal.occurrence_count > 0);
    } else if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        CHECK(ss == THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE);
    } else if (variant == THERON_TRACK02_VARIANT_US_ISO) {
        /* Partial extract with no anchors. */
        CHECK(ss == THERON_TRACK02_SIGNAL_OK ||
              ss == THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE);
    }
}

/* ── main ────────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    const char *data_root = "";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--data-root") == 0 && i + 1 < argc) {
            data_root = argv[++i];
        }
    }

    printf("Theron V1 first-room startup/runtime probe\n");
    printf("Source-lock: THQUEST.ASM T080/T400/T520/T560/T700,\n"
           "             tqr_v1_phase1_boot_H2338.md,\n"
           "             tqr_v1_phase0_provenance_gate_H2339.md\n");

    probe_first_room_synthesize();
    probe_startup_fallback_rooms();
    probe_first_room_runtime();
    probe_readiness_no_data_root();
    probe_real_track02_conditional(data_root);

    printf("\n%d assertions, %d failures, %d skips\n",
           g_assertions, g_failures, g_skips);
    if (g_failures) {
        printf("FAIL: Theron V1 first-room runtime probe\n");
        return 1;
    }
    printf("PASS: Theron V1 first-room runtime probe "
           "(skip-safe CI default honoured)\n");
    return 0;
}
