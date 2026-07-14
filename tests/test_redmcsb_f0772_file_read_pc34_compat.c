#include "redmcsb_f0772_file_read_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

struct ReadScript {
    size_t returned[3];
    uint16_t requested[3];
    int16_t handles[3];
    unsigned char *buffers[3];
    size_t calls;
};

static size_t ScriptedRead(
    void *context,
    int16_t file_handle,
    unsigned char *buffer,
    uint16_t byte_count)
{
    struct ReadScript *script = context;
    size_t call = script->calls++;

    script->requested[call] = byte_count;
    script->handles[call] = file_handle;
    script->buffers[call] = buffer;
    return script->returned[call];
}

int main(void)
{
    unsigned char buffer[65537];
    struct ReadScript script = {{32768U, 32768U, 1U}, {0U}, {0}, {0}, 0U};

    if (!RedmcsbF0772FileReadPc34Compat(
            &script, ScriptedRead, -7, 65537UL, buffer)) {
        return 1;
    }
    if (script.calls != 3U || script.requested[0] != 32768U ||
        script.requested[1] != 32768U || script.requested[2] != 1U ||
        script.handles[0] != -7 || script.handles[1] != -7 ||
        script.handles[2] != -7 || script.buffers[0] != buffer ||
        script.buffers[1] != buffer + 32768U ||
        script.buffers[2] != buffer + 65536U) {
        return 2;
    }

    script = (struct ReadScript){{8U, 0U, 0U}, {0U}, {0}, {0}, 0U};
    if (RedmcsbF0772FileReadPc34Compat(
            &script, ScriptedRead, 3, 9UL, buffer) || script.calls != 1U ||
        script.requested[0] != 9U) {
        return 3;
    }

    script = (struct ReadScript){{0U, 0U, 0U}, {0U}, {0}, {0}, 0U};
    if (!RedmcsbF0772FileReadPc34Compat(
            &script, ScriptedRead, 4, 0UL, NULL) || script.calls != 0U) {
        return 4;
    }

    if (RedmcsbF0772FileReadPc34Compat(NULL, NULL, 1, 1UL, buffer) ||
        RedmcsbF0772FileReadPc34Compat(NULL, ScriptedRead, 1, 1UL, NULL)) {
        return 5;
    }

    return 0;
}
