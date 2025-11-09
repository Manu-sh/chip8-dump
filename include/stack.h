#pragma once
#include <stdint.h>

typedef struct {
    uint16_t stack[256];
    uint8_t idx;
} stack_t;

void stack_init(stack_t *self) {
    self->idx = 0;
}

// let idx silently overflow a normal chip8 stack is just 32 / 48 bytes
void stack_push(stack_t *self, uint16_t val) {
    self->stack[self->idx++] = val;
}

uint16_t stack_pop(stack_t *self) {
    return self->stack[--self->idx];
}
