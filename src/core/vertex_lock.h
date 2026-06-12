#ifndef OSFINALPROJECT_VERTEX_LOCK_H
#define OSFINALPROJECT_VERTEX_LOCK_H

#include <semaphore.h>
#include "config.h"

struct vertex_locks_t {
    sem_t locks[MAX_VERTICES];
    int   count;
};

struct vertex_locks_t *vertex_locks_create(int vertex_count);
void vertex_locks_destroy(struct vertex_locks_t *vl);

void vertex_lock_acquire(struct vertex_locks_t *vl, int node);
void vertex_lock_release(struct vertex_locks_t *vl, int node);

#endif /* OSFINALPROJECT_VERTEX_LOCK_H */
