#ifndef THERON_V1_TRACK02_DOOR_H
#define THERON_V1_TRACK02_DOOR_H

#include <stdint.h>

typedef struct {
    uint16_t next_ref;
    uint8_t  type;
    uint8_t  ornate;
    uint8_t  opens_up;
    uint8_t  button;
    uint8_t  destroyable;
    uint8_t  bashable;
} Theron_Door;

typedef struct {
    uint16_t next_ref;
    uint8_t  x_dest;
    uint8_t  y_dest;
    uint8_t  rotation;
    uint8_t  absolute;
    uint8_t  scope;
    uint8_t  sound;
    uint8_t  level_dest;
} Theron_Teleporter;

int theron_v1_track02_door_decode(const uint8_t *raw4, Theron_Door *out);
int theron_v1_track02_teleporter_decode(const uint8_t *raw6, Theron_Teleporter *out);

#endif
