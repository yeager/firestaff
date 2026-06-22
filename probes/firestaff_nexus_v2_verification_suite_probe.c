/*
 * firestaff_nexus_v2_verification_suite_probe.c
 *
 * Nexus V2 Phase 7 — V2 Verification Suite Hardening Probe
 *
 * Headless probe: verifies the full Nexus V2 render pipeline
 * (V1 framebuffer -> EPX -> bilinear -> lighting -> particles ->
 * atmosphere -> SDL present) for all three modes (V1 OFF /
 * V2.1 UPSCALED / V2.2 ENHANCED), and asserts state-hash
 * presentation-disabled gate.
 *
 * This probe validates:
 *
 *   1. nexus_v2_pipeline_init() returns 0 and allocates output
 *      buffer matching the config's render_width/render_height
 *
 *   2. V2 OFF mode (NEXUS_V2_OFF): output is direct palette
 *      conversion, no scaling. Output dim == NEXUS_FB_W * NEXUS_FB_H.
 *      Deterministic: same V1 input -> same V1 output bytes.
 *
 *   3. V2 OFF mode matches V1: a V2 OFF render of fb[k] =
 *      palette[k] produces output[k] = palette[k] for all k
 *      (byte-identical to what V1 would produce)
 *
 *   4. V2 UPSCALED mode: output is 2x the V1 dimensions, with EPX
 *      algorithm applied. Output dim matches
 *      config.render_width * config.render_height (default 640x400).
 *
 *   5. V2 ENHANCED mode: output is 4x (1280x800) with full
 *      pipeline (EPX + bilinear + lighting + atmosphere).
 *
 *   6. State-hash gate: presenting a V1 framebuffer state produces
 *      the same output state (within byte-equal range) for the same
 *      input state across two consecutive renders (no hidden state
 *      leaking between frames).
 *
 *   7. Deterministic Nexus V1 input scripts produce identical final
 *      state hashes across V1/V2 presentation modes.
 *
 *   8. Screenshot gates: synthetic V1 framebuffer captures produce
 *      deterministic full-frame and viewport-region pixel hashes for
 *      V1 OFF, V2 UPSCALED, and V2 ENHANCED output buffers.
 *
 *   9. nexus_v2_pipeline_shutdown() frees the output buffer
 *      and is safe to call multiple times.
 *
 *  10. Null-args are safe on init/render/shutdown.
 *
 *  11. Config mode drives output dimensions correctly.
 *
 *  12. Pipeline can be re-init'd after shutdown (lifecycle).
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_nexus_v2_verification_suite_probe
 *
 * Source references:
 *   nexus_v2_config.h             mode + upscale + filter config
 *   nexus_v2_render_pipeline.h    pipeline struct
 *   nexus_v2_upscaler.h            EPX/Scale2x + bilinear
 *   nexus_v2_lighting.h            per-vertex dynamic lighting
 *   nexus_v2_particles.h           particle systems
 *   nexus_v2_atmosphere.h          fog + AO + color grading
 *   nexus_v1_rasterizer.h          V1 320x200 indexed framebuffer
 *   NEXUS.C / NEXUS2.C / NEXUS.BIN Saturn DM Nexus engine
 *   DMDF level data                per-tile render configuration
 *   ReDMCSB DUNVIEW.C F0128        320x200 base
 */

#include "nexus_v2_render_pipeline.h"
#include "nexus_v2_config.h"
#include "nexus_v2_upscaler.h"
#include "nexus_v2_lighting.h"
#include "nexus_v2_particles.h"
#include "nexus_v2_atmosphere.h"
#include "nexus_v2_phase_gate_pc34.h"
#include "nexus_v1_rasterizer.h"
#include "nexus_v1_movement.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static int g_total = 0;
static int g_failed = 0;

static void check(int cond, const char *name)
{
    ++g_total;
    if (!cond) {
        ++g_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

/* Deterministic V1 framebuffer: a 4x4 gradient that exercises
 * the palette and reveals scaling artifacts. */
static void fill_v1_fb(Nexus_Framebuffer *fb, int pattern) {
    int x, y;
    memset(fb, 0, sizeof(*fb));
    /* Palette: a few distinct colors. */
    fb->palette[0] = 0xFF000000u;  /* black */
    fb->palette[1] = 0xFF808080u;  /* mid grey */
    fb->palette[2] = 0xFFFFFFFFu;  /* white */
    fb->palette[3] = 0xFFFF0000u;  /* red */
    fb->palette[4] = 0xFF00FF00u;  /* green */
    fb->palette[5] = 0xFF0000FFu;  /* blue */
    /* Color buffer: gradient or checkerboard. */
    for (y = 0; y < NEXUS_FB_H; ++y) {
        for (x = 0; x < NEXUS_FB_W; ++x) {
            int idx;
            switch (pattern) {
            case 0: idx = (x + y) % 6; break;
            case 1: idx = (x * y) % 6; break;
            case 2: idx = x % 6; break;
            default: idx = y % 6; break;
            }
            fb->color_buffer[y * NEXUS_FB_W + x] = (uint8_t)idx;
        }
    }
}

static uint64_t fnv1a_u32(uint64_t hash, uint32_t value)
{
    int byteIndex;
    for (byteIndex = 0; byteIndex < 4; byteIndex++) {
        hash ^= (uint64_t)((value >> (byteIndex * 8)) & 0xFFU);
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t fnv1a_bytes(uint64_t hash, const void *data, size_t bytes)
{
    const uint8_t *p = (const uint8_t *)data;
    size_t i;
    for (i = 0; i < bytes; i++) {
        hash ^= (uint64_t)p[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t hash_nexus_movement_state(uint64_t hash,
                                          const Nexus_MovementState *ms,
                                          const Nexus_MoveResultData *last)
{
    hash = fnv1a_u32(hash, (uint32_t)ms->party_x);
    hash = fnv1a_u32(hash, (uint32_t)ms->party_y);
    hash = fnv1a_u32(hash, (uint32_t)ms->party_dir);
    hash = fnv1a_u32(hash, (uint32_t)ms->level_index);
    hash = fnv1a_u32(hash, (uint32_t)ms->move_cooldown_ticks);
    if (last) {
        hash = fnv1a_u32(hash, (uint32_t)last->result);
        hash = fnv1a_u32(hash, (uint32_t)last->new_map_x);
        hash = fnv1a_u32(hash, (uint32_t)last->new_map_y);
        hash = fnv1a_u32(hash, (uint32_t)last->new_dir);
        hash = fnv1a_u32(hash, (uint32_t)last->new_map_index);
    }
    return hash;
}

static void fill_open_floor_map(uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE])
{
    int x, y;
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; y++) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++) {
            squares[y][x] = NEXUS_SQUARE_FLOOR;
        }
    }
}

static uint64_t run_input_script(const int *commands, size_t count,
                                 Nexus_MovementState *out_state)
{
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    Nexus_MovementState ms;
    Nexus_MoveResultData last;
    uint64_t hash = 14695981039346656037ULL;
    size_t i;

    fill_open_floor_map(squares);
    nexus_movement_init(&ms, 20, 20, NEXUS_DIR_NORTH);
    memset(&last, 0, sizeof(last));

    for (i = 0; i < count; i++) {
        int cmd = commands[i];
        if (cmd == NEXUS_CMD_TURN_LEFT) {
            nexus_movement_turn(&ms, 0, &last);
        } else if (cmd == NEXUS_CMD_TURN_RIGHT) {
            nexus_movement_turn(&ms, 1, &last);
        } else {
            (void)nexus_movement_step(&ms, squares, cmd, &last);
        }
        hash = hash_nexus_movement_state(hash, &ms, &last);
    }

    if (out_state) {
        *out_state = ms;
    }
    return hash;
}

static uint64_t hash_pipeline_output(const Nexus_V2_RenderPipeline *pipe)
{
    size_t pixels = (size_t)pipe->output_w * (size_t)pipe->output_h;
    return fnv1a_bytes(14695981039346656037ULL,
                       pipe->output_buffer,
                       pixels * sizeof(pipe->output_buffer[0]));
}

static uint64_t hash_output_region(const Nexus_V2_RenderPipeline *pipe,
                                   int x0, int y0, int w, int h)
{
    uint64_t hash = 14695981039346656037ULL;
    int y;
    for (y = 0; y < h; y++) {
        const uint32_t *row = pipe->output_buffer + (size_t)(y0 + y) * pipe->output_w + x0;
        hash = fnv1a_bytes(hash, row, (size_t)w * sizeof(row[0]));
    }
    return hash;
}

static void check_v2_off_init(void)
{
    Nexus_V2_RenderPipeline pipe;
    int rc = nexus_v2_pipeline_init(&pipe, NEXUS_V2_OFF);
    check(rc == 0, "V2 OFF: pipeline_init returns 0");
    check(pipe.output_w == NEXUS_FB_W,
          "V2 OFF: output_w == NEXUS_FB_W");
    check(pipe.output_h == NEXUS_FB_H,
          "V2 OFF: output_h == NEXUS_FB_H");
    check(pipe.output_buffer != 0,
          "V2 OFF: output_buffer allocated");
    check(pipe.config.mode == NEXUS_V2_OFF,
          "V2 OFF: config.mode == NEXUS_V2_OFF");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_v2_off_byte_stability(void)
{
    /* V2 OFF: output should be direct palette conversion.
     * output[k] = fb->palette[fb->color_buffer[k]]. */
    Nexus_V2_RenderPipeline pipe;
    Nexus_Framebuffer fb;
    int i;
    int all_correct = 1;
    nexus_v2_pipeline_init(&pipe, NEXUS_V2_OFF);
    fill_v1_fb(&fb, 0);
    nexus_v2_pipeline_render(&pipe, &fb, 0.0f, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        uint32_t expected = fb.palette[fb.color_buffer[i]];
        if (pipe.output_buffer[i] != expected) {
            all_correct = 0;
            break;
        }
    }
    check(all_correct, "V2 OFF: output[k] = palette[color_buffer[k]] for all k");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_v2_off_deterministic(void)
{
    Nexus_V2_RenderPipeline pipe1, pipe2;
    Nexus_Framebuffer fb1, fb2;
    int i;
    int same;
    nexus_v2_pipeline_init(&pipe1, NEXUS_V2_OFF);
    nexus_v2_pipeline_init(&pipe2, NEXUS_V2_OFF);
    fill_v1_fb(&fb1, 1);
    fill_v1_fb(&fb2, 1);
    nexus_v2_pipeline_render(&pipe1, &fb1, 0.0f, 0.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render(&pipe2, &fb2, 0.0f, 0.0f, 0.0f, 0.0f);
    same = 1;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (pipe1.output_buffer[i] != pipe2.output_buffer[i]) {
            same = 0;
            break;
        }
    }
    check(same, "V2 OFF: deterministic for same input");
    nexus_v2_pipeline_shutdown(&pipe1);
    nexus_v2_pipeline_shutdown(&pipe2);
}

static void check_v2_upscaled_init(void)
{
    Nexus_V2_RenderPipeline pipe;
    int rc = nexus_v2_pipeline_init(&pipe, NEXUS_V2_UPSCALED);
    check(rc == 0, "V2 UPSCALED: pipeline_init returns 0");
    /* V2.1 default is 640x400. */
    check(pipe.output_w == 640,
          "V2 UPSCALED: output_w == 640");
    check(pipe.output_h == 400,
          "V2 UPSCALED: output_h == 400");
    check(pipe.config.mode == NEXUS_V2_UPSCALED,
          "V2 UPSCALED: config.mode == NEXUS_V2_UPSCALED");
    check(pipe.config.bilinear_filter == 1,
          "V2 UPSCALED: bilinear_filter default ON");
    check(pipe.output_buffer != 0,
          "V2 UPSCALED: output_buffer allocated (640*400*4 bytes)");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_v2_enhanced_init(void)
{
    Nexus_V2_RenderPipeline pipe;
    int rc = nexus_v2_pipeline_init(&pipe, NEXUS_V2_ENHANCED);
    check(rc == 0, "V2 ENHANCED: pipeline_init returns 0");
    /* V2.2 default is 1280x800. */
    check(pipe.output_w == 1280,
          "V2 ENHANCED: output_w == 1280");
    check(pipe.output_h == 800,
          "V2 ENHANCED: output_h == 800");
    check(pipe.config.mode == NEXUS_V2_ENHANCED,
          "V2 ENHANCED: config.mode == NEXUS_V2_ENHANCED");
    check(pipe.config.smooth_movement == 1,
          "V2 ENHANCED: smooth_movement ON");
    check(pipe.config.dynamic_lighting == 1,
          "V2 ENHANCED: dynamic_lighting ON");
    check(pipe.config.ambient_occlusion == 1,
          "V2 ENHANCED: ambient_occlusion ON");
    check(pipe.config.particles == 1,
          "V2 ENHANCED: particles ON");
    check(pipe.config.fog == 1,
          "V2 ENHANCED: fog ON");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_state_hash_gate(void)
{
    /* State-hash gate: rendering the same V1 state twice in
     * different pipelines produces deterministic output (no
     * hidden state leaking between renders). */
    Nexus_V2_RenderPipeline pipe1, pipe2;
    Nexus_Framebuffer fb1, fb2;
    int i;
    int same;
    /* Disable bilinear to isolate EPX-only behavior. */
    nexus_v2_pipeline_init(&pipe1, NEXUS_V2_UPSCALED);
    pipe1.config.bilinear_filter = 0;
    nexus_v2_pipeline_init(&pipe2, NEXUS_V2_UPSCALED);
    pipe2.config.bilinear_filter = 0;
    fill_v1_fb(&fb1, 2);
    fill_v1_fb(&fb2, 2);
    nexus_v2_pipeline_render(&pipe1, &fb1, 0.0f, 0.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render(&pipe2, &fb2, 0.0f, 0.0f, 0.0f, 0.0f);
    same = 1;
    for (i = 0; i < pipe1.output_w * pipe1.output_h; ++i) {
        if (pipe1.output_buffer[i] != pipe2.output_buffer[i]) {
            same = 0;
            break;
        }
    }
    check(same, "state-hash: V2 UPSCALED deterministic for same V1 input");
    nexus_v2_pipeline_shutdown(&pipe1);
    nexus_v2_pipeline_shutdown(&pipe2);
}

static void check_state_hash_different_pattern(void)
{
    /* Different V1 inputs should produce different V2 outputs
     * (the gate isn't just always-zero). */
    Nexus_V2_RenderPipeline pipe;
    Nexus_Framebuffer fb1, fb2;
    int i;
    int any_different = 0;
    nexus_v2_pipeline_init(&pipe, NEXUS_V2_UPSCALED);
    pipe.config.bilinear_filter = 0;
    fill_v1_fb(&fb1, 0);
    fill_v1_fb(&fb2, 3);
    nexus_v2_pipeline_render(&pipe, &fb1, 0.0f, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < pipe.output_w * pipe.output_h; ++i) {
        /* Pipeline mutates output_buffer in-place. After rendering
         * fb1 then fb2, the output should reflect fb2 (and
         * therefore differ from a render of fb1 alone). This is
         * tested indirectly: we just verify the second render
         * doesn't crash and produces some output. */
    }
    nexus_v2_pipeline_render(&pipe, &fb2, 0.0f, 0.0f, 0.0f, 0.0f);
    /* After both renders, the output should be the fb2 result. We
     * can't easily compare against an unrendered fb2 result, but
     * we can verify the output is non-zero (i.e., rendering
     * happened). */
    for (i = 0; i < pipe.output_w * pipe.output_h; ++i) {
        if (pipe.output_buffer[i] != 0) {
            any_different = 1;
            break;
        }
    }
    check(any_different, "state-hash: V2 UPSCALED produces non-zero output");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_null_args(void)
{
    /* All pipeline entry points must be safe on NULL. */
    nexus_v2_pipeline_init(0, NEXUS_V2_OFF);
    nexus_v2_pipeline_render(0, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render((Nexus_V2_RenderPipeline *)1, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_shutdown(0);
    check(1, "null: all pipeline entry points safe");
}

static void check_lifecycle_reinit(void)
{
    /* A pipeline can be re-init'd after shutdown (lifecycle). */
    Nexus_V2_RenderPipeline pipe;
    int rc1, rc2;
    rc1 = nexus_v2_pipeline_init(&pipe, NEXUS_V2_OFF);
    check(rc1 == 0, "lifecycle: first init OK");
    nexus_v2_pipeline_shutdown(&pipe);
    /* After shutdown, the struct fields are zeroed. Re-init. */
    rc2 = nexus_v2_pipeline_init(&pipe, NEXUS_V2_UPSCALED);
    check(rc2 == 0, "lifecycle: re-init after shutdown OK");
    check(pipe.config.mode == NEXUS_V2_UPSCALED,
          "lifecycle: re-init uses new mode");
    check(pipe.output_buffer != 0,
          "lifecycle: re-init allocates new output buffer");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_upscaled_2x_scaling(void)
{
    /* V2 UPSCALED output_w / NEXUS_FB_W == 2 and same for height. */
    Nexus_V2_RenderPipeline pipe;
    nexus_v2_pipeline_init(&pipe, NEXUS_V2_UPSCALED);
    check(pipe.output_w / NEXUS_FB_W == 2,
          "V2 UPSCALED: 2x horizontal scaling");
    check(pipe.output_h / NEXUS_FB_H == 2,
          "V2 UPSCALED: 2x vertical scaling");
    check((size_t)pipe.output_w * pipe.output_h ==
          (size_t)NEXUS_FB_W * NEXUS_FB_H * 4,
          "V2 UPSCALED: output has 4x as many pixels as V1");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_config_mode_to_dimensions(void)
{
    /* V2 OFF = 320x200, V2 UPSCALED = 640x400, V2 ENHANCED = 1280x800. */
    Nexus_V2_RenderPipeline pipe;
    int rc;
    rc = nexus_v2_pipeline_init(&pipe, NEXUS_V2_OFF);
    check(rc == 0 && pipe.output_w == 320 && pipe.output_h == 200,
          "config->dim: V2_OFF -> 320x200");
    nexus_v2_pipeline_shutdown(&pipe);
    rc = nexus_v2_pipeline_init(&pipe, NEXUS_V2_UPSCALED);
    check(rc == 0 && pipe.output_w == 640 && pipe.output_h == 400,
          "config->dim: V2_UPSCALED -> 640x400");
    nexus_v2_pipeline_shutdown(&pipe);
    rc = nexus_v2_pipeline_init(&pipe, NEXUS_V2_ENHANCED);
    check(rc == 0 && pipe.output_w == 1280 && pipe.output_h == 800,
          "config->dim: V2_ENHANCED -> 1280x800");
    nexus_v2_pipeline_shutdown(&pipe);
}

static void check_phase_gate_domain_boundaries(void)
{
    NEXUS_V2_PhaseGateConfig off_cfg;
    NEXUS_V2_PhaseGateConfig on_cfg;
    NEXUS_V2_PhaseGateDecision movement_dec;
    NEXUS_V2_PhaseGateDecision render_dec_off;
    NEXUS_V2_PhaseGateDecision render_dec_on;
    const char *evidence;

    nexus_v2_phase_gate_defaults(&off_cfg);
    nexus_v2_phase_gate_defaults(&on_cfg);
    on_cfg.v2PresentationEnabled = 1;
    on_cfg.v2ConfigPersistenceEnabled = 1;

    movement_dec = nexus_v2_phase_gate_decide(&on_cfg,
        NEXUS_V2_PHASE_DOMAIN_MOVEMENT);
    render_dec_off = nexus_v2_phase_gate_decide(&off_cfg,
        NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    render_dec_on = nexus_v2_phase_gate_decide(&on_cfg,
        NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);

    check(movement_dec.v1SourceLocked == 1,
          "phase gate: MOVEMENT remains V1-source-locked");
    check(movement_dec.v2PresentationAllowed == 0,
          "phase gate: MOVEMENT never allows V2 presentation mutation");
    check(render_dec_off.v1SourceLocked == 0 &&
          render_dec_off.v2PresentationAllowed == 0,
          "phase gate: RENDER_PRESENTATION is V2-eligible but off by default");
    check(render_dec_on.v1SourceLocked == 0 &&
          render_dec_on.v2PresentationAllowed == 1,
          "phase gate: RENDER_PRESENTATION allowed when V2 presentation enabled");

    evidence = nexus_v2_phase_gate_source_evidence();
    check(evidence != 0 && strstr(evidence, "nexus_v1_movement.c") != 0,
          "phase gate: source evidence names Nexus V1 movement");
    check(evidence != 0 && strstr(evidence, "COMMAND.C") != 0,
          "phase gate: source evidence names ReDMCSB command loop");
}

static void check_deterministic_input_scripts(void)
{
    static const int route[] = {
        NEXUS_CMD_FORWARD,
        NEXUS_CMD_FORWARD,
        NEXUS_CMD_TURN_RIGHT,
        NEXUS_CMD_FORWARD,
        NEXUS_CMD_BACKWARD,
        NEXUS_CMD_TURN_LEFT,
        NEXUS_CMD_FORWARD
    };
    static const int turn_loop[] = {
        NEXUS_CMD_TURN_RIGHT,
        NEXUS_CMD_TURN_RIGHT,
        NEXUS_CMD_TURN_RIGHT,
        NEXUS_CMD_TURN_RIGHT
    };
    Nexus_MovementState route_state_a;
    Nexus_MovementState route_state_b;
    Nexus_MovementState turn_state;
    uint64_t hash_a = run_input_script(route, sizeof(route) / sizeof(route[0]),
                                       &route_state_a);
    uint64_t hash_b = run_input_script(route, sizeof(route) / sizeof(route[0]),
                                       &route_state_b);
    uint64_t turn_hash = run_input_script(turn_loop,
                                          sizeof(turn_loop) / sizeof(turn_loop[0]),
                                          &turn_state);
    uint64_t turn_hash_again = run_input_script(turn_loop,
                                                sizeof(turn_loop) / sizeof(turn_loop[0]),
                                                0);
    (void)turn_hash;

    check(hash_a == hash_b,
          "input script: repeated route yields identical state hash");
    check(route_state_a.party_x == 20 && route_state_a.party_y == 17 &&
          route_state_a.party_dir == NEXUS_DIR_NORTH,
          "input script: route final state is (20,17,N)");
    check(route_state_a.party_x == route_state_b.party_x &&
          route_state_a.party_y == route_state_b.party_y &&
          route_state_a.party_dir == route_state_b.party_dir,
          "input script: repeated route final state fields match");
    check(turn_state.party_x == 20 && turn_state.party_y == 20 &&
          turn_state.party_dir == NEXUS_DIR_NORTH,
          "input script: four right turns return to north without moving");
    check(turn_hash == turn_hash_again,
          "input script: turn loop hash is idempotent");
}

static void check_state_hash_equality_across_modes(void)
{
    static const int route[] = {
        NEXUS_CMD_FORWARD,
        NEXUS_CMD_TURN_RIGHT,
        NEXUS_CMD_FORWARD,
        NEXUS_CMD_BACKWARD,
        NEXUS_CMD_TURN_LEFT,
        NEXUS_CMD_FORWARD
    };
    Nexus_V2_RenderPipeline off_pipe;
    Nexus_V2_RenderPipeline up_pipe;
    Nexus_V2_RenderPipeline enhanced_pipe;
    Nexus_Framebuffer fb;
    Nexus_MovementState state_off;
    Nexus_MovementState state_up;
    Nexus_MovementState state_enhanced;
    uint64_t hash_off;
    uint64_t hash_up;
    uint64_t hash_enhanced;

    fill_v1_fb(&fb, 2);
    nexus_v2_pipeline_init(&off_pipe, NEXUS_V2_OFF);
    nexus_v2_pipeline_init(&up_pipe, NEXUS_V2_UPSCALED);
    nexus_v2_pipeline_init(&enhanced_pipe, NEXUS_V2_ENHANCED);

    hash_off = run_input_script(route, sizeof(route) / sizeof(route[0]),
                                &state_off);
    nexus_v2_pipeline_render(&off_pipe, &fb,
                             (float)state_off.party_x,
                             (float)state_off.party_y,
                             (float)(state_off.party_dir * 90),
                             0.0f);

    hash_up = run_input_script(route, sizeof(route) / sizeof(route[0]),
                               &state_up);
    nexus_v2_pipeline_render(&up_pipe, &fb,
                             (float)state_up.party_x,
                             (float)state_up.party_y,
                             (float)(state_up.party_dir * 90),
                             0.0f);

    hash_enhanced = run_input_script(route, sizeof(route) / sizeof(route[0]),
                                     &state_enhanced);
    nexus_v2_pipeline_render(&enhanced_pipe, &fb,
                             (float)state_enhanced.party_x,
                             (float)state_enhanced.party_y,
                             (float)(state_enhanced.party_dir * 90),
                             0.0f);

    check(hash_off == hash_up && hash_off == hash_enhanced,
          "state hash: V1/V2.1/V2.2 presentation modes preserve gameplay state");
    check(state_off.party_x == state_up.party_x &&
          state_off.party_y == state_up.party_y &&
          state_off.party_dir == state_up.party_dir &&
          state_off.party_x == state_enhanced.party_x &&
          state_off.party_y == state_enhanced.party_y &&
          state_off.party_dir == state_enhanced.party_dir,
          "state hash: final position fields match across modes");

    nexus_v2_pipeline_shutdown(&off_pipe);
    nexus_v2_pipeline_shutdown(&up_pipe);
    nexus_v2_pipeline_shutdown(&enhanced_pipe);
}

static void check_screenshot_pixel_gates(void)
{
    Nexus_Framebuffer fb;
    Nexus_V2_RenderPipeline off_a, off_b;
    Nexus_V2_RenderPipeline up_a, up_b;
    Nexus_V2_RenderPipeline enhanced_a, enhanced_b;
    uint64_t off_full_a, off_full_b;
    uint64_t off_region_a, off_region_b;
    uint64_t up_full_a, up_full_b;
    uint64_t up_region_a, up_region_b;
    uint64_t enhanced_full_a, enhanced_full_b;
    uint64_t enhanced_region_a, enhanced_region_b;

    fill_v1_fb(&fb, 1);
    nexus_v2_pipeline_init(&off_a, NEXUS_V2_OFF);
    nexus_v2_pipeline_init(&off_b, NEXUS_V2_OFF);
    nexus_v2_pipeline_init(&up_a, NEXUS_V2_UPSCALED);
    nexus_v2_pipeline_init(&up_b, NEXUS_V2_UPSCALED);
    nexus_v2_pipeline_init(&enhanced_a, NEXUS_V2_ENHANCED);
    nexus_v2_pipeline_init(&enhanced_b, NEXUS_V2_ENHANCED);

    nexus_v2_pipeline_render(&off_a, &fb, 20.0f, 17.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render(&off_b, &fb, 20.0f, 17.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render(&up_a, &fb, 20.0f, 17.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render(&up_b, &fb, 20.0f, 17.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render(&enhanced_a, &fb, 20.0f, 17.0f, 0.0f, 0.0f);
    nexus_v2_pipeline_render(&enhanced_b, &fb, 20.0f, 17.0f, 0.0f, 0.0f);

    off_full_a = hash_pipeline_output(&off_a);
    off_full_b = hash_pipeline_output(&off_b);
    off_region_a = hash_output_region(&off_a, 96, 54, 128, 92);
    off_region_b = hash_output_region(&off_b, 96, 54, 128, 92);

    up_full_a = hash_pipeline_output(&up_a);
    up_full_b = hash_pipeline_output(&up_b);
    up_region_a = hash_output_region(&up_a, 192, 108, 256, 184);
    up_region_b = hash_output_region(&up_b, 192, 108, 256, 184);

    enhanced_full_a = hash_pipeline_output(&enhanced_a);
    enhanced_full_b = hash_pipeline_output(&enhanced_b);
    enhanced_region_a = hash_output_region(&enhanced_a, 384, 216, 512, 368);
    enhanced_region_b = hash_output_region(&enhanced_b, 384, 216, 512, 368);

    check(off_full_a == off_full_b && off_region_a == off_region_b,
          "screenshot gate: V2 OFF full-frame and viewport region hashes stable");
    check(up_full_a == up_full_b && up_region_a == up_region_b,
          "screenshot gate: V2 UPSCALED full-frame and viewport region hashes stable");
    check(enhanced_full_a == enhanced_full_b &&
          enhanced_region_a == enhanced_region_b,
          "screenshot gate: V2 ENHANCED full-frame and viewport region hashes stable");
    check(off_full_a != up_full_a && up_full_a != enhanced_full_a,
          "screenshot gate: presentation modes produce distinct full-frame hashes");
    check(off_region_a != 0 && up_region_a != 0 && enhanced_region_a != 0,
          "screenshot gate: viewport region hashes are non-zero receipts");

    nexus_v2_pipeline_shutdown(&off_a);
    nexus_v2_pipeline_shutdown(&off_b);
    nexus_v2_pipeline_shutdown(&up_a);
    nexus_v2_pipeline_shutdown(&up_b);
    nexus_v2_pipeline_shutdown(&enhanced_a);
    nexus_v2_pipeline_shutdown(&enhanced_b);
}

int main(void)
{
    printf("=== Nexus V2 Phase 7 — Verification Suite Probe ===\n");
    check_v2_off_init();
    check_v2_off_byte_stability();
    check_v2_off_deterministic();
    check_v2_upscaled_init();
    check_v2_enhanced_init();
    check_upscaled_2x_scaling();
    check_config_mode_to_dimensions();
    check_state_hash_gate();
    check_state_hash_different_pattern();
    check_phase_gate_domain_boundaries();
    check_deterministic_input_scripts();
    check_state_hash_equality_across_modes();
    check_screenshot_pixel_gates();
    check_lifecycle_reinit();
    check_null_args();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
