#include "dm1_v2_pathfinding_pc34.h"

static M11_V2_PathNode g_open_list[256];
static int g_open_count = 0;

void v2_path_init(void) {
    memset(g_open_list, 0, sizeof(g_open_list));
    g_open_count = 0;
}

void v2_path_clear(void) {
    memset(g_open_list, 0, sizeof(g_open_list));
    g_open_count = 0;
}

static int find_node_in_open(int x, int y) {
    for (int i = 0; i < g_open_count; ++i) {
        if (g_open_list[i].open && g_open_list[i].x == x && g_open_list[i].y == y) {
            return i;
        }
    }
    return -1;
}

static int find_lowest_f_node(void) {
    int best_idx = -1;
    int best_f = 2147483647;
    for (int i = 0; i < g_open_count; ++i) {
        if (g_open_list[i].open && g_open_list[i].f < best_f) {
            best_f = g_open_list[i].f;
            best_idx = i;
        }
    }
    return best_idx;
}

bool v2_path_find(int* map, int map_w, int map_h, int sx, int sy, int gx, int gy, M11_V2_Path* out) {
    if (!map || !out || map_w <= 0 || map_h <= 0) return false;
    if (sx < 0 || sx >= map_w || sy < 0 || sy >= map_h) return false;
    if (gx < 0 || gx >= map_w || gy < 0 || gy >= map_h) return false;
    if (map[sy * map_w + sx] != 0 || map[gy * map_w + gx] != 0) return false;

    v2_path_clear();

    if (g_open_count >= 256) return false;
    g_open_list[g_open_count].x = sx;
    g_open_list[g_open_count].y = sy;
    g_open_list[g_open_count].g = 0;
    g_open_list[g_open_count].h = abs(sx - gx) + abs(sy - gy);
    g_open_list[g_open_count].f = g_open_list[g_open_count].g + g_open_list[g_open_count].h;
    g_open_list[g_open_count].parent_idx = -1;
    g_open_list[g_open_count].open = true;
    g_open_list[g_open_count].closed = false;
    g_open_count++;

    int dirs[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

    while (g_open_count > 0) {
        int curr_idx = find_lowest_f_node();
        if (curr_idx == -1) break;

        M11_V2_PathNode* curr = &g_open_list[curr_idx];

        if (curr->x == gx && curr->y == gy) {
            int len = 0;
            int p = curr_idx;
            while (p != -1 && len < 128) {
                out->steps[len][0] = g_open_list[p].x;
                out->steps[len][1] = g_open_list[p].y;
                len++;
                p = g_open_list[p].parent_idx;
            }
            for (int i = 0; i < len / 2; ++i) {
                int tmp_x = out->steps[i][0];
                int tmp_y = out->steps[i][1];
                out->steps[i][0] = out->steps[len - 1 - i][0];
                out->steps[i][1] = out->steps[len - 1 - i][1];
                out->steps[len - 1 - i][0] = tmp_x;
                out->steps[len - 1 - i][1] = tmp_y;
            }
            out->length = len;
            out->valid = true;
            return true;
        }

        curr->open = false;
        curr->closed = true;

        for (int d = 0; d < 4; ++d) {
            int nx = curr->x + dirs[d][0];
            int ny = curr->y + dirs[d][1];

            if (nx < 0 || nx >= map_w || ny < 0 || ny >= map_h) continue;
            if (map[ny * map_w + nx] != 0) continue;

            int neighbor_idx = find_node_in_open(nx, ny);
            int tentative_g = curr->g + 1;

            if (neighbor_idx == -1) {
                if (g_open_count >= 256) continue;
                g_open_list[g_open_count].x = nx;
                g_open_list[g_open_count].y = ny;
                g_open_list[g_open_count].g = tentative_g;
                g_open_list[g_open_count].h = abs(nx - gx) + abs(ny - gy);
                g_open_list[g_open_count].f = g_open_list[g_open_count].g + g_open_list[g_open_count].h;
                g_open_list[g_open_count].parent_idx = curr_idx;
                g_open_list[g_open_count].open = true;
                g_open_list[g_open_count].closed = false;
                g_open_count++;
            } else {
                if (tentative_g < g_open_list[neighbor_idx].g) {
                    g_open_list[neighbor_idx].g = tentative_g;
                    g_open_list[neighbor_idx].f = g_open_list[neighbor_idx].g + g_open_list[neighbor_idx].h;
                    g_open_list[neighbor_idx].parent_idx = curr_idx;
                }
            }
        }
    }

    out->valid = false;
    out->length = 0;
    return false;
}
