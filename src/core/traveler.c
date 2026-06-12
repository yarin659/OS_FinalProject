#include "traveler.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dijkstra.h"
#include "vertex_lock.h"

pid_t traveler_create_subprocess(struct traveler_t *traveler, int index, int write_fd) {
    traveler->traveler_index = index;
    const pid_t pid = fork();

    if (pid < 0) {
        printf("traveler_create_subprocess: fork failed.\n");
        return pid;
    }

    if (pid == 0) {
        const int return_value = traveler_main(traveler, write_fd);
        exit(return_value);
    }

    traveler->subprocess_pid = pid;
    traveler->has_subprocess = TRUE;
    return pid;
}

void traveler_destroy_subprocess(struct traveler_t *traveler) {
    if (!traveler->has_subprocess) return;

    traveler->has_subprocess = FALSE;

    if (traveler->subprocess_pid == 0) return;

    errno = 0;
    if (kill(traveler->subprocess_pid, 0) == 0) {
        kill(traveler->subprocess_pid, SIGTERM);
    }
    traveler->subprocess_pid = 0;
}

void traveler_free(struct traveler_t *traveler) {
    if (traveler->has_subprocess) {
        traveler_destroy_subprocess(traveler);
    }
    if (traveler->dijkstra_result) {
        free_dijkstra_result(traveler->dijkstra_result);
        free(traveler->dijkstra_result);
        traveler->dijkstra_result = NULL;
    }
}

int traveler_main(const struct traveler_t *traveler, int write_fd) {
    if (!dijkstra_compute(traveler->graph, traveler->traveler_index)) {
        return 1;
    }

    struct dijkstra_result_t *res = traveler->dijkstra_result;
    if (!res || res->path_len == 0) return 0;

    const pid_t my_pid = getpid();
    struct vertex_locks_t *vl = traveler->vertex_locks;

    for (int i = 0; i < res->path_len; i++) {
        const int node = res->path[i];
        const int next = (i < res->path_len - 1) ? res->path[i + 1] : -1;

        struct ipc_msg_t msg;
        msg.pid = my_pid;
        msg.traveler_index = traveler->traveler_index;
        msg.current_node = node;
        msg.next_node = next;

        ssize_t _wr;

        // we don't want to wait on the source node so we leave right away
        if (i == 0) {
            if (next != -1) {
                msg.type = MSG_LEAVING;
                _wr = write(write_fd, &msg, sizeof(msg)); (void)_wr;
                const float weight = (float)traveler->graph->graph[node][next];
                usleep((useconds_t)(weight * JUMP_DURATION_MS * 1000));
            }
            continue;
        }

        // arrive at the outside of the node and wait until the node is empty
        msg.type = MSG_WAITING;
        _wr = write(write_fd, &msg, sizeof(msg)); (void)_wr;
        vertex_lock_acquire(vl, node);

        // once the node is empty we enter and wait 1 second then free the lock
        msg.type = MSG_ARRIVED;
        _wr = write(write_fd, &msg, sizeof(msg)); (void)_wr;
        usleep((useconds_t)(VERTEX_WAIT_MS * 1000));
        vertex_lock_release(vl, node);

        // and off we go
        if (next != -1) {
            msg.type = MSG_LEAVING;
            _wr = write(write_fd, &msg, sizeof(msg)); (void)_wr;
            const float weight = (float)traveler->graph->graph[node][next];
            usleep((useconds_t)(weight * JUMP_DURATION_MS * 1000));
        }
    }

    // and we're done
    struct ipc_msg_t fin;
    fin.pid = my_pid;
    fin.traveler_index = traveler->traveler_index;
    fin.type = MSG_FINISHED;
    fin.current_node = res->path[res->path_len - 1];
    fin.next_node = -1;
    const ssize_t _wr2 = write(write_fd, &fin, sizeof(fin)); (void)_wr2;

    close(write_fd);
    return 0;
}
