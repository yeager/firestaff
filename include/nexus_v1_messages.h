
#ifndef NEXUS_V1_MESSAGES_H
#define NEXUS_V1_MESSAGES_H

/* Nexus V1 game message system — HUD text messages and wall inscriptions.
 * Messages display for a duration then fade. Multiple messages queue.
 * Source: DM1 DUNGEON.C text display, MOVESENS.C sensor text messages,
 *         ReDMCSB COMMAND.C action feedback text. */

#define NEXUS_MSG_MAX_LEN     80
#define NEXUS_MSG_QUEUE_SIZE  8
#define NEXUS_MSG_DEFAULT_TICKS 120

typedef struct {
    char text[NEXUS_MSG_MAX_LEN];
    int ticks_remaining;
    int priority;        /* higher priority messages replace lower */
} Nexus_Message;

typedef struct {
    Nexus_Message queue[NEXUS_MSG_QUEUE_SIZE];
    int head;
    int count;
    char current[NEXUS_MSG_MAX_LEN];
    int current_ticks;
} Nexus_MessageQueue;

void nexus_v1_messages_init(Nexus_MessageQueue *mq);

/* Push a message with default duration. */
void nexus_v1_message_push(Nexus_MessageQueue *mq, const char *text);

/* Push a message with custom duration and priority. */
void nexus_v1_message_push_ex(Nexus_MessageQueue *mq, const char *text,
                               int duration_ticks, int priority);

/* Tick the message queue. Returns 1 if current message changed. */
int nexus_v1_messages_tick(Nexus_MessageQueue *mq);

/* Get the currently displayed message, or NULL if none. */
const char *nexus_v1_message_current(const Nexus_MessageQueue *mq);

/* Clear all messages. */
void nexus_v1_messages_clear(Nexus_MessageQueue *mq);

int nexus_v1_messages_pending(const Nexus_MessageQueue *mq);

#endif
