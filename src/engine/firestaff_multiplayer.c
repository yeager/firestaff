
#include "firestaff_multiplayer.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define closesocket close
#endif

static int create_udp_socket(int port) {
    struct sockaddr_in addr;
    int fd;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    fd = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    /* Non-blocking */
#ifdef _WIN32
    {unsigned long mode = 1; ioctlsocket(fd, FIONBIO, &mode);}
#else
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif

    if (port > 0) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            closesocket(fd);
            return -1;
        }
    }
    return fd;
}

int fs_mp_host_start(FS_MP_State *mp, const char *player_name) {
    if (!mp) return -1;
    memset(mp, 0, sizeof(*mp));
    mp->role = MP_ROLE_HOST;
    mp->socket_fd = create_udp_socket(MP_PORT);
    if (mp->socket_fd < 0) {
        printf("MP: failed to bind port %d\n", MP_PORT);
        return -1;
    }
    strncpy(mp->players[0].name, player_name ? player_name : "Host", 31);
    mp->players[0].connected = 1;
    mp->player_count = 1;
    mp->local_player_id = 0;
    mp->active = 1;
    printf("MP: hosting on port %d\n", MP_PORT);
    return 0;
}

int fs_mp_client_join(FS_MP_State *mp, const char *host_ip, const char *player_name) {
    FS_MP_Packet pkt;
    struct sockaddr_in dest;

    if (!mp || !host_ip) return -1;
    memset(mp, 0, sizeof(*mp));
    mp->role = MP_ROLE_CLIENT;
    mp->socket_fd = create_udp_socket(0);
    if (mp->socket_fd < 0) return -1;

    mp->host_addr = inet_addr(host_ip);

    /* Send join packet */
    memset(&pkt, 0, sizeof(pkt));
    pkt.magic = MP_MAGIC;
    pkt.type = MP_PKT_JOIN;
    strncpy((char *)pkt.data, player_name ? player_name : "Player", 31);

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(MP_PORT);
    dest.sin_addr.s_addr = mp->host_addr;
    sendto(mp->socket_fd, (const char *)&pkt, sizeof(pkt), 0,
        (struct sockaddr *)&dest, sizeof(dest));

    mp->active = 1;
    printf("MP: joining %s:%d\n", host_ip, MP_PORT);
    return 0;
}

void fs_mp_disconnect(FS_MP_State *mp) {
    if (!mp || !mp->active) return;
    if (mp->socket_fd >= 0) closesocket(mp->socket_fd);
    mp->socket_fd = -1;
    mp->active = 0;
    printf("MP: disconnected\n");
}

int fs_mp_send_input(FS_MP_State *mp, int command) {
    (void)mp; (void)command;
    /* TODO: pack input into packet and send to host */
    return 0;
}

int fs_mp_receive(FS_MP_State *mp) {
    FS_MP_Packet pkt;
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    int n;

    if (!mp || !mp->active) return 0;
    n = (int)recvfrom(mp->socket_fd, (char *)&pkt, sizeof(pkt), 0,
        (struct sockaddr *)&from, &fromlen);
    if (n <= 0) return 0;
    if (pkt.magic != MP_MAGIC) return 0;

    switch (pkt.type) {
    case MP_PKT_JOIN:
        if (mp->role == MP_ROLE_HOST && mp->player_count < MP_MAX_PLAYERS) {
            int id = mp->player_count;
            strncpy(mp->players[id].name, (char *)pkt.data, 31);
            mp->players[id].connected = 1;
            mp->player_count++;
            printf("MP: %s joined (player %d)\n", mp->players[id].name, id);
        }
        break;
    case MP_PKT_STATE:
        /* Client: apply game state from host */
        break;
    case MP_PKT_INPUT:
        /* Host: apply input from client */
        break;
    case MP_PKT_LEAVE:
        if (pkt.player_id < MP_MAX_PLAYERS)
            mp->players[pkt.player_id].connected = 0;
        break;
    default: break;
    }
    return 1;
}

void fs_mp_broadcast_state(FS_MP_State *mp, const uint8_t *state_data, int size) {
    (void)mp; (void)state_data; (void)size;
    /* TODO: serialize game state and broadcast to all clients */
}

