
#include "nexus_v1_messages.h"
#include <string.h>

void nexus_v1_messages_init(Nexus_MessageQueue *mq) {
    if (!mq) return;
    memset(mq, 0, sizeof(*mq));
}

void nexus_v1_message_push(Nexus_MessageQueue *mq, const char *text) {
    nexus_v1_message_push_ex(mq, text, NEXUS_MSG_DEFAULT_TICKS, 0);
}

void nexus_v1_message_push_ex(Nexus_MessageQueue *mq, const char *text,
                               int duration_ticks, int priority) {
    int idx;
    if (!mq || !text) return;
    if (mq->count >= NEXUS_MSG_QUEUE_SIZE) {
        /* Drop oldest if full */
        mq->head = (mq->head + 1) % NEXUS_MSG_QUEUE_SIZE;
        mq->count--;
    }
    idx = (mq->head + mq->count) % NEXUS_MSG_QUEUE_SIZE;
    strncpy(mq->queue[idx].text, text, NEXUS_MSG_MAX_LEN - 1);
    mq->queue[idx].text[NEXUS_MSG_MAX_LEN - 1] = '\0';
    mq->queue[idx].ticks_remaining = duration_ticks;
    mq->queue[idx].priority = priority;
    mq->count++;
}

int nexus_v1_messages_tick(Nexus_MessageQueue *mq) {
    if (!mq) return 0;

    if (mq->current_ticks > 0) {
        mq->current_ticks--;
        if (mq->current_ticks <= 0) {
            mq->current[0] = '\0';
            /* Pop next message if available */
            if (mq->count > 0) {
                Nexus_Message *m = &mq->queue[mq->head];
                strncpy(mq->current, m->text, NEXUS_MSG_MAX_LEN - 1);
                mq->current[NEXUS_MSG_MAX_LEN - 1] = '\0';
                mq->current_ticks = m->ticks_remaining;
                mq->head = (mq->head + 1) % NEXUS_MSG_QUEUE_SIZE;
                mq->count--;
                return 1;
            }
            return 1;
        }
        return 0;
    }

    /* No current message — pop from queue */
    if (mq->count > 0) {
        Nexus_Message *m = &mq->queue[mq->head];
        strncpy(mq->current, m->text, NEXUS_MSG_MAX_LEN - 1);
        mq->current[NEXUS_MSG_MAX_LEN - 1] = '\0';
        mq->current_ticks = m->ticks_remaining;
        mq->head = (mq->head + 1) % NEXUS_MSG_QUEUE_SIZE;
        mq->count--;
        return 1;
    }

    return 0;
}

const char *nexus_v1_message_current(const Nexus_MessageQueue *mq) {
    if (!mq || mq->current[0] == '\0') return NULL;
    return mq->current;
}

void nexus_v1_messages_clear(Nexus_MessageQueue *mq) {
    if (!mq) return;
    mq->head = 0;
    mq->count = 0;
    mq->current[0] = '\0';
    mq->current_ticks = 0;
}

int nexus_v1_messages_pending(const Nexus_MessageQueue *mq) {
    if (!mq) return 0;
    return mq->count + (mq->current_ticks > 0 ? 1 : 0);
}
