#define _DEFAULT_SOURCE
#include "traveler.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dijkstra.h"

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
    if (!traveler->has_subprocess) {
        return;
    }

    traveler->has_subprocess = FALSE;

    if (traveler->subprocess_pid == 0) {
        return;
    }

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

int traveler_main(struct traveler_t *traveler, int write_fd) {
    if (!dijkstra_compute(traveler->graph, traveler->traveler_index)) {
        return 1;
    }

    struct dijkstra_result_t* res = traveler->dijkstra_result;
    if (!res || res->path_len == 0) {
        return 0;
    }

    const pid_t my_pid = getpid();

    for (int i = 0; i < res->path_len; i++) {
        struct ipc_msg_t msg;
        msg.pid = my_pid;
        msg.traveler_index = traveler->traveler_index;
        msg.type = MSG_ARRIVED;
        msg.current_node = res->path[i];
        msg.next_node = (i < res->path_len - 1) ? res->path[i + 1] : -1;

        // Checking write return value to suppress compiler warnings
        if (write(write_fd, &msg, sizeof(struct ipc_msg_t)) < 0) {
            // Silently ignore or handle write error if needed
        }

        if (msg.next_node != -1) {
            const int weight = traveler->graph->graph[msg.current_node][msg.next_node];
            usleep(weight * JUMP_DURATION_MS * 1000);
            if (i < res->path_len - 2) {
                usleep(VERTEX_WAIT_MS * 1000);
            }
        }
    }

    struct ipc_msg_t fin_msg;
    fin_msg.pid = my_pid;
    fin_msg.traveler_index = traveler->traveler_index;
    fin_msg.type = MSG_FINISHED;
    fin_msg.current_node = res->path[res->path_len - 1];
    fin_msg.next_node = -1;
    
    if (write(write_fd, &fin_msg, sizeof(struct ipc_msg_t)) < 0) {
        // Silently ignore or handle write error if needed
    }

    close(write_fd);
    return 0;
}
