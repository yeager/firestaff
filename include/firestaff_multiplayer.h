
#ifndef FIRESTAFF_MULTIPLAYER_H
#define FIRESTAFF_MULTIPLAYER_H
#include <stdint.h>

/* Multiplayer foundation for DM2-style LAN play.
 * Protocol: simple UDP packets for state sync.
 * Max 4 players (one champion each).
 *
 * Architecture:
 *   Host: runs game loop, broadcasts state
 *   Clients: send inputs, receive state updates
 *   All parties see same dungeon in real-time */

#define MP_MAX_PLAYERS 4
#define MP_PORT 7777
#define MP_PACKET_SIZE 1024
#define MP_MAGIC 0x46535450  /* "FSTP" */

typedef enum {
    MP_ROLE_NONE = 0,
    MP_ROLE_HOST,
    MP_ROLE_CLIENT
} FS_MP_Role;

typedef enum {
    MP_PKT_JOIN = 1,
    MP_PKT_ACCEPT,
    MP_PKT_INPUT,
    MP_PKT_STATE,
    MP_PKT_CHAT,
    MP_PKT_LEAVE
} FS_MP_PacketType;

typedef struct {
    uint32_t magic;
    uint8_t type;
    uint8_t player_id;
    uint16_t seq;
    uint32_t tick;
    uint8_t data[MP_PACKET_SIZE - 12];
} FS_MP_Packet;

typedef struct {
    char name[32];
    int connected;
    int champion_index;
    uint32_t last_seen_tick;
} FS_MP_Player;

typedef struct {
    FS_MP_Role role;
    FS_MP_Player players[MP_MAX_PLAYERS];
    int player_count;
    int local_player_id;
    int socket_fd;
    uint32_t host_addr;
    uint16_t seq_counter;
    int active;
} FS_MP_State;

int fs_mp_host_start(FS_MP_State *mp, const char *player_name);
int fs_mp_client_join(FS_MP_State *mp, const char *host_ip, const char *player_name);
void fs_mp_disconnect(FS_MP_State *mp);
int fs_mp_send_input(FS_MP_State *mp, int command);
int fs_mp_receive(FS_MP_State *mp);
void fs_mp_broadcast_state(FS_MP_State *mp, const uint8_t *state_data, int size);

#endif

