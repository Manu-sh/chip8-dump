#pragma once
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

static inline double now_msec(struct timespec *t) {
    clock_gettime(CLOCK_MONOTONIC, t);
    return t->tv_sec * 1000 + (t->tv_nsec / 1.0e6);
}

typedef struct {
    struct timespec t;
    double start;
} chronos_t;

// start / reset a timer
static double chronos_start(chronos_t *self) {
    return self->start = now_msec(&self->t);
}

// get elapsed milliseconds
static double chronos_elapsed(chronos_t *self) {
    return now_msec(&self->t) - self->start;
}

#define chronos_restart chronos_start
