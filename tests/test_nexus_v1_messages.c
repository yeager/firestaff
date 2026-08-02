
#include <stdio.h>
#include <string.h>
#include "nexus_v1_messages.h"

int main(void) {
    int fail = 0;

    /* Test 1: init — no current message */
    {
        Nexus_MessageQueue mq;
        nexus_v1_messages_init(&mq);
        if (nexus_v1_message_current(&mq) != NULL) {
            fprintf(stderr, "FAIL: init current not NULL\n"); fail++;
        } else {
            printf("  Init: no current message OK\n");
        }
    }

    /* Test 2: push and tick pops message */
    {
        Nexus_MessageQueue mq;
        nexus_v1_messages_init(&mq);
        nexus_v1_message_push(&mq, "Hello world");
        nexus_v1_messages_tick(&mq);
        if (!nexus_v1_message_current(&mq) ||
            strcmp(nexus_v1_message_current(&mq), "Hello world") != 0) {
            fprintf(stderr, "FAIL: push/tick\n"); fail++;
        } else {
            printf("  Push and tick: '%s' OK\n", nexus_v1_message_current(&mq));
        }
    }

    /* Test 3: message expires after duration */
    {
        Nexus_MessageQueue mq;
        int i;
        nexus_v1_messages_init(&mq);
        nexus_v1_message_push_ex(&mq, "Short", 3, 0);
        nexus_v1_messages_tick(&mq);
        if (!nexus_v1_message_current(&mq)) {
            fprintf(stderr, "FAIL: not displayed\n"); fail++;
        } else {
            for (i = 0; i < 3; i++)
                nexus_v1_messages_tick(&mq);
            if (nexus_v1_message_current(&mq) != NULL) {
                fprintf(stderr, "FAIL: message not expired\n"); fail++;
            } else {
                printf("  Expiry after 3 ticks OK\n");
            }
        }
    }

    /* Test 4: queue ordering — FIFO */
    {
        Nexus_MessageQueue mq;
        int i;
        nexus_v1_messages_init(&mq);
        nexus_v1_message_push_ex(&mq, "First", 2, 0);
        nexus_v1_message_push_ex(&mq, "Second", 2, 0);
        nexus_v1_messages_tick(&mq);
        if (!nexus_v1_message_current(&mq) ||
            strcmp(nexus_v1_message_current(&mq), "First") != 0) {
            fprintf(stderr, "FAIL: not First\n"); fail++;
        } else {
            for (i = 0; i < 2; i++) nexus_v1_messages_tick(&mq);
            if (!nexus_v1_message_current(&mq) ||
                strcmp(nexus_v1_message_current(&mq), "Second") != 0) {
                fprintf(stderr, "FAIL: not Second, got '%s'\n",
                        nexus_v1_message_current(&mq) ? nexus_v1_message_current(&mq) : "NULL");
                fail++;
            } else {
                printf("  FIFO ordering OK\n");
            }
        }
    }

    /* Test 5: clear removes everything */
    {
        Nexus_MessageQueue mq;
        nexus_v1_messages_init(&mq);
        nexus_v1_message_push(&mq, "A");
        nexus_v1_message_push(&mq, "B");
        nexus_v1_messages_tick(&mq);
        nexus_v1_messages_clear(&mq);
        if (nexus_v1_message_current(&mq) != NULL || nexus_v1_messages_pending(&mq) != 0) {
            fprintf(stderr, "FAIL: clear\n"); fail++;
        } else {
            printf("  Clear OK\n");
        }
    }

    /* Test 6: overflow drops oldest */
    {
        Nexus_MessageQueue mq;
        int i;
        char buf[16];
        nexus_v1_messages_init(&mq);
        for (i = 0; i < NEXUS_MSG_QUEUE_SIZE + 2; i++) {
            sprintf(buf, "msg%d", i);
            nexus_v1_message_push(&mq, buf);
        }
        nexus_v1_messages_tick(&mq);
        if (!nexus_v1_message_current(&mq) ||
            strcmp(nexus_v1_message_current(&mq), "msg2") != 0) {
            fprintf(stderr, "FAIL: overflow, got '%s'\n",
                    nexus_v1_message_current(&mq) ? nexus_v1_message_current(&mq) : "NULL");
            fail++;
        } else {
            printf("  Overflow drops oldest OK\n");
        }
    }

    /* Test 7: pending count */
    {
        Nexus_MessageQueue mq;
        nexus_v1_messages_init(&mq);
        if (nexus_v1_messages_pending(&mq) != 0) {
            fprintf(stderr, "FAIL: pending init\n"); fail++;
        } else {
            nexus_v1_message_push(&mq, "A");
            nexus_v1_message_push(&mq, "B");
            if (nexus_v1_messages_pending(&mq) != 2) {
                fprintf(stderr, "FAIL: pending=%d\n", nexus_v1_messages_pending(&mq));
                fail++;
            } else {
                nexus_v1_messages_tick(&mq);
                if (nexus_v1_messages_pending(&mq) != 2) {
                    fprintf(stderr, "FAIL: pending after tick=%d\n", nexus_v1_messages_pending(&mq));
                    fail++;
                } else {
                    printf("  Pending count OK\n");
                }
            }
        }
    }

    /* Test 8: NULL safety */
    {
        nexus_v1_messages_init(NULL);
        nexus_v1_message_push(NULL, "x");
        nexus_v1_messages_tick(NULL);
        nexus_v1_messages_clear(NULL);
        if (nexus_v1_message_current(NULL) != NULL || nexus_v1_messages_pending(NULL) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus message system verified\n");
    return 0;
}
