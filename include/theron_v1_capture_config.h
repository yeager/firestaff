#ifndef THERON_V1_CAPTURE_CONFIG_H
#define THERON_V1_CAPTURE_CONFIG_H

typedef struct {
    unsigned int version;
    const char *track02_hash;
    const char *system_card_hash;
    const char *status;
    int valid;
    int runtime_blocked;
} Theron_V1CaptureConfig;

int theron_v1_capture_config_validate(
    const Theron_V1CaptureConfig *stored,
    const Theron_V1CaptureConfig *current);

#endif
