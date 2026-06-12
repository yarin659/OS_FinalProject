#include "vertex_lock.h"

#include <sys/mman.h>
#include <stdio.h>

struct vertex_locks_t *vertex_locks_create(int vertex_count) {
    struct vertex_locks_t *vl = mmap(
        NULL, sizeof(struct vertex_locks_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0
    );
    if (vl == MAP_FAILED) {
        perror("vertex_locks_create: mmap");
        return NULL;
    }

    vl->count = vertex_count;
    for (int i = 0; i < vertex_count; i++) {
        if (sem_init(&vl->locks[i], 1, 1) != 0) {
            perror("vertex_locks_create: sem_init");
        }
    }
    return vl;
}

void vertex_locks_destroy(struct vertex_locks_t *vl) {
    if (!vl) return;
    for (int i = 0; i < vl->count; i++) {
        sem_destroy(&vl->locks[i]);
    }
    munmap(vl, sizeof(struct vertex_locks_t));
}

void vertex_lock_acquire(struct vertex_locks_t *vl, int node) {
    sem_wait(&vl->locks[node]);
}

void vertex_lock_release(struct vertex_locks_t *vl, int node) {
    sem_post(&vl->locks[node]);
}
