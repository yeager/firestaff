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

/* V2 A* pathfinding for minimap route display + AI hints */

#define V2_PATH_MAX_NODES 1024
#define V2_PATH_MAX_OPEN 256

typedef struct {
    int x, y, g, h, f, parent;
    int closed;
} V2_PathNode;

static V2_PathNode g_nodes[V2_PATH_MAX_NODES];
static int g_node_count;

static int v2_pathfind_heuristic(int x0, int y0, int x1, int y1) {
    int dx = x0 > x1 ? x0 - x1 : x1 - x0;
    int dy = y0 > y1 ? y0 - y1 : y1 - y0;
    return dx + dy; /* Manhattan distance */
}

int v2_pathfind_astar_step(int sx, int sy, int ex, int ey,
    const int *walkable, int map_w, int map_h,
    int *out_path_x, int *out_path_y, int max_path)
{
    /* Simplified A* for dungeon grids */
    int open[V2_PATH_MAX_OPEN], open_count = 0;
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    int i, current, best, path_len;

    if (!walkable || !out_path_x || !out_path_y) return 0;

    g_node_count = 0;
    g_nodes[0].x = sx; g_nodes[0].y = sy;
    g_nodes[0].g = 0; g_nodes[0].h = v2_pathfind_heuristic(sx, sy, ex, ey);
    g_nodes[0].f = g_nodes[0].h; g_nodes[0].parent = -1; g_nodes[0].closed = 0;
    g_node_count = 1;
    open[open_count++] = 0;

    while (open_count > 0) {
        /* Find lowest f in open */
        best = 0;
        for (i = 1; i < open_count; i++) {
            if (g_nodes[open[i]].f < g_nodes[open[best]].f) best = i;
        }
        current = open[best];
        open[best] = open[--open_count];

        if (g_nodes[current].x == ex && g_nodes[current].y == ey) {
            /* Reconstruct path */
            path_len = 0;
            int n = current;
            while (n >= 0 && path_len < max_path) {
                out_path_x[path_len] = g_nodes[n].x;
                out_path_y[path_len] = g_nodes[n].y;
                path_len++;
                n = g_nodes[n].parent;
            }
            return path_len;
        }

        g_nodes[current].closed = 1;

        for (i = 0; i < 4; i++) {
            int nx = g_nodes[current].x + dx[i];
            int ny = g_nodes[current].y + dy[i];
            int ng, j, found;
            if (nx < 0 || nx >= map_w || ny < 0 || ny >= map_h) continue;
            if (!walkable[ny * map_w + nx]) continue;

            ng = g_nodes[current].g + 1;
            found = -1;
            for (j = 0; j < g_node_count; j++) {
                if (g_nodes[j].x == nx && g_nodes[j].y == ny) { found = j; break; }
            }
            if (found >= 0 && g_nodes[found].closed) continue;
            if (found >= 0 && ng >= g_nodes[found].g) continue;

            if (found < 0 && g_node_count < V2_PATH_MAX_NODES) {
                found = g_node_count++;
                g_nodes[found].x = nx; g_nodes[found].y = ny;
                g_nodes[found].closed = 0;
                if (open_count < V2_PATH_MAX_OPEN) open[open_count++] = found;
            }
            if (found >= 0) {
                g_nodes[found].g = ng;
                g_nodes[found].h = v2_pathfind_heuristic(nx, ny, ex, ey);
                g_nodes[found].f = ng + g_nodes[found].h;
                g_nodes[found].parent = current;
            }
        }
    }
    return 0; /* no path */
}

