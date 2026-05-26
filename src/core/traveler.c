#include "traveler.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dijkstra.h"


pid_t traveler_create_subprocess(struct traveler_t *traveler) {
    const pid_t pid = fork();
    if (pid < 0) {
        printf("traveler_create_subprocess: fork failed.\n");
        return pid;
    }

    if (pid != 0) {
        const int return_value = traveler_main(pid);
        exit(return_value); // we don't want to return to main program execution here EVER.
    }

    return pid;
}

void traveler_destroy_subprocess(struct traveler_t *traveler) {
    if (!traveler->has_subprocess) {
        return;
    }

    traveler->has_subprocess = FALSE;

    // this happens after we reset has_subprocess just in case we got into some weird state where
    // has_subprocess is true but subprocess_pid is 0, so we let this function just reset has_subprocess.
    if (traveler->subprocess_pid == 0) {
        return;
    }

    errno = 0;
    if (kill(traveler->subprocess_pid, 0) == 0) {
        kill(traveler->subprocess_pid, SIGTERM);
    } else if (errno == EPERM) {
        traveler->has_subprocess = TRUE; // want to reset this just in case we want to try again in the future.
        printf("traveler_destroy_subprocess: cannot kill traveler with pid %d due to permission error.\n",
            traveler->subprocess_pid);
    }

    traveler->subprocess_pid = 0;
}

void traveler_free(struct traveler_t *traveler) {
    if (traveler->has_subprocess) {
        traveler_destroy_subprocess(traveler);
    }

    free_dijkstra_result(traveler->dijkstra_result);
}

int traveler_main(const pid_t pid) {
    printf("[%d] started\n", pid);
    return 0;
}
