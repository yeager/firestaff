#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    CAPTURE_W = 320,
    CAPTURE_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136
};

typedef struct WallCollisionCapture {
    const char* label;
    const char* action;
    const char* screenshot;
    int result;
    int mapIndex;
    int mapX;
    int mapY;
    int direction;
    int command;
    int turnApplied;
    int stepApplied;
    int movementBlocked;
    int viewportDirty;
} WallCollisionCapture;

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') return;
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

static void dump_vga_ppm(const char* path, const unsigned char* fb) {
    FILE* f;
    int px;
    if (!path || !fb) return;
    f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", CAPTURE_W, CAPTURE_H);
    for (px = 0; px < CAPTURE_W * CAPTURE_H; ++px) {
        unsigned char raw = fb[px];
        unsigned char idx = M11_FB_DECODE_INDEX(raw);
        int level = M11_FB_DECODE_LEVEL(raw);
        const unsigned char* rgb;
        if (level < 0) level = 0;
        if (level >= M11_PALETTE_LEVELS) level = M11_PALETTE_LEVELS - 1;
        rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

static void dump_vga_viewport_ppm(const char* path, const unsigned char* fb) {
    FILE* f;
    int x;
    int y;
    if (!path || !fb) return;
    f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", VIEWPORT_W, VIEWPORT_H);
    for (y = 0; y < VIEWPORT_H; ++y) {
        for (x = 0; x < VIEWPORT_W; ++x) {
            unsigned char raw = fb[(VIEWPORT_Y + y) * CAPTURE_W + (VIEWPORT_X + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            int level = M11_FB_DECODE_LEVEL(raw);
            const unsigned char* rgb;
            if (level < 0) level = 0;
            if (level >= M11_PALETTE_LEVELS) level = M11_PALETTE_LEVELS - 1;
            rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
}

static void snapshot(M11_GameViewState* game,
                     const char* outDir,
                     const char* label,
                     const char* action,
                     M11_GameInputResult result,
                     WallCollisionCapture* out) {
    char ppmPath[1024];
    unsigned char framebuffer[CAPTURE_W * CAPTURE_H];
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(game, framebuffer, CAPTURE_W, CAPTURE_H);
    snprintf(ppmPath, sizeof(ppmPath), "%s/%s.ppm", outDir, label);
    dump_vga_ppm(ppmPath, framebuffer);
    snprintf(ppmPath, sizeof(ppmPath), "%s/%s_viewport_224x136.ppm", outDir, label);
    dump_vga_viewport_ppm(ppmPath, framebuffer);

    memset(out, 0, sizeof(*out));
    out->label = label;
    out->action = action;
    out->screenshot = label;
    out->result = (int)result;
    out->mapIndex = game->world.party.mapIndex;
    out->mapX = game->world.party.mapX;
    out->mapY = game->world.party.mapY;
    out->direction = game->world.party.direction;
    out->command = game->lastDm1V1MovementPipelineResult.core.queue.command;
    out->turnApplied = game->lastDm1V1MovementPipelineResult.core.turnApplied;
    out->stepApplied = game->lastDm1V1MovementPipelineResult.core.stepApplied;
    out->movementBlocked = game->lastDm1V1MovementPipelineResult.core.movementBlocked;
    out->viewportDirty = game->lastDm1V1MovementPipelineResult.viewportDirty;
}

static int write_manifest(const char* outDir,
                          const WallCollisionCapture* rows,
                          int count) {
    char jsonPath[1024];
    char mdPath[1024];
    FILE* js;
    FILE* md;
    int i;
    snprintf(jsonPath, sizeof(jsonPath), "%s/dm1_v1_wall_collision_runtime_capture.json", outDir);
    snprintf(mdPath, sizeof(mdPath), "%s/dm1_v1_wall_collision_runtime_capture.md", outDir);
    js = fopen(jsonPath, "w");
    md = fopen(mdPath, "w");
    if (!js || !md) {
        if (js) fclose(js);
        if (md) fclose(md);
        return 0;
    }

    fprintf(js, "{\n");
    fprintf(js, "  \"schema\": \"firestaff.dm1_v1_wall_collision_runtime_capture.v1\",\n");
    fprintf(js, "  \"sourceEvidence\": [\n");
    fprintf(js, "    \"DUNGEON.C:1371-1391 F0150 relative coordinate update\",\n");
    fprintf(js, "    \"CLIKMENU.C:180-347 F0366 step dispatch and wall/door blocker path\",\n");
    fprintf(js, "    \"MOVESENS.C:438-444 F0267 accepted movement coordinate mutation\",\n");
    fprintf(js, "    \"DUNVIEW.C:8318-8618 F0128 viewport redraw from party map/x/y/direction\",\n");
    fprintf(js, "    \"COORD.C:1693-1722 PC34 viewport origin/224x136 dimensions\",\n");
    fprintf(js, "    \"DRAWVIEW.C:842-857 F0097 presents G0296 through C007_ZONE_VIEWPORT\"\n");
    fprintf(js, "  ],\n");
    fprintf(js, "  \"honesty\": \"Firestaff deterministic runtime capture with exact state coordinates, full-frame PPM screenshots, and source-geometry viewport crop PPMs; not an original DOS pixel-parity claim.\",\n");
    fprintf(js, "  \"captures\": [\n");

    fprintf(md, "# DM1 V1 wall/collision runtime capture\n\n");
    fprintf(md, "Deterministic Firestaff runtime repro for wall/collision reports. Each row records exact map/x/y/direction, movement pipeline state, a full-frame PPM, and a source-geometry viewport crop PPM. This is capture readiness only; it does not claim original DOS pixel parity.\n\n");
    fprintf(md, "| label | action | result | map | x | y | dir | command | turn | step | blocked | dirty | screenshot | viewport crop |\n");
    fprintf(md, "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |\n");

    for (i = 0; i < count; ++i) {
        const WallCollisionCapture* r = &rows[i];
        fprintf(js,
                "    {\"label\":\"%s\",\"action\":\"%s\",\"result\":%d,\"party\":{\"mapIndex\":%d,\"mapX\":%d,\"mapY\":%d,\"direction\":%d},\"pipeline\":{\"command\":%d,\"turnApplied\":%d,\"stepApplied\":%d,\"movementBlocked\":%d,\"viewportDirty\":%d},\"screenshot\":\"%s.ppm\",\"viewportCrop\":\"%s_viewport_224x136.ppm\"}%s\n",
                r->label, r->action, r->result, r->mapIndex, r->mapX, r->mapY,
                r->direction, r->command, r->turnApplied, r->stepApplied,
                r->movementBlocked, r->viewportDirty, r->screenshot, r->screenshot,
                i == count - 1 ? "" : ",");
        fprintf(md, "| %s | %s | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | `%s.ppm` | `%s_viewport_224x136.ppm` |\n",
                r->label, r->action, r->result, r->mapIndex, r->mapX, r->mapY,
                r->direction, r->command, r->turnApplied, r->stepApplied,
                r->movementBlocked, r->viewportDirty, r->screenshot, r->screenshot);
    }
    fprintf(js, "  ]\n}\n");
    fclose(js);
    fclose(md);
    printf("wrote %s and %s\n", jsonPath, mdPath);
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir = argc > 1 ? argv[1] : NULL;
    const char* outDir = argc > 2 ? argv[2] : "wall-collision-capture";
    M11_GameViewState game;
    WallCollisionCapture rows[4];
    int rowCount = 0;
    int ok = 1;
    M11_GameInputResult result;

    if (!dataDir || dataDir[0] == '\0') {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR OUT_DIR\n", argv[0]);
        return 2;
    }

    ensure_output_dir(outDir);
    M11_GameView_Init(&game);
    if (!M11_GameView_StartDm1(&game, dataDir)) {
        fprintf(stderr, "SKIP dm1_v1_wall_collision_runtime_capture: DM1 data not available at %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    snapshot(&game, outDir, "01_start_south_1_3", "start", M11_GAME_INPUT_REDRAW, &rows[rowCount++]);

    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    snapshot(&game, outDir, "02_turn_right_west_1_3", "turn_right", result, &rows[rowCount++]);

    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    snapshot(&game, outDir, "03_blocked_west_wall_1_3", "forward_into_west_wall", result, &rows[rowCount++]);

    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_LEFT);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    snapshot(&game, outDir, "04_forward_south_1_4", "turn_left_then_forward_south", result, &rows[rowCount++]);

    ok &= (rows[0].mapIndex == 0 && rows[0].mapX == 1 && rows[0].mapY == 3 && rows[0].direction == 2);
    ok &= (rows[1].mapX == 1 && rows[1].mapY == 3 && rows[1].direction == 3 && rows[1].turnApplied == 1);
    ok &= (rows[2].mapX == 1 && rows[2].mapY == 3 && rows[2].direction == 3 && rows[2].movementBlocked == 1 && rows[2].stepApplied == 0);
    ok &= (rows[3].mapX == 1 && rows[3].mapY == 4 && rows[3].direction == 2 && rows[3].stepApplied == 1 && rows[3].movementBlocked == 0);
    ok &= write_manifest(outDir, rows, rowCount);

    M11_GameView_Shutdown(&game);
    printf("%s dm1_v1_wall_collision_runtime_capture rows=%d out=%s\n",
           ok ? "PASS" : "FAIL", rowCount, outDir);
    return ok ? 0 : 1;
}
