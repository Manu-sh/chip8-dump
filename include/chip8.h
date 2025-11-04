#pragma once
#include <stdint.h>
#include <stdalign.h>

enum { REG_V0, REG_V1, REG_V2, REG_V3, REG_V4, REG_V5, REG_V6, REG_V7, REG_V8, REG_V9, REG_VA, REG_VB, REG_VC, REG_VD, REG_VE, REG_VF, REG_LEN };

typedef struct {

    /* CHIP-8 programs should be loaded into memory starting at address 0x200. The memory addresses 0x000 to 0x1FF are reserved for the CHIP-8 interpreter. */
    union {
        alignas(uint16_t) uint8_t reserved[0x200];   // 512 byte usually untouched by the rom
        alignas(uint16_t) uint8_t memory[0xfff + 1]; // 4096 bytes of memory
    };

    union {
        uint8_t d_register[REG_LEN]; // 16 data registers
        uint8_t V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;
    };


    union {
        uint16_t I; // address register (it can only be loaded with a 12-bit memory address due to the range of memory accessible to CHIP-8)
    };

    // TODO: forward list for the stack?

    // per la grafica probabilmente è meglio usare direttamente vector_bit[64 * 32 * 8]
    // visto che si ragiona in termini di grafica monochrome a 2 bit
    uint8_t delay_timer;
    uint8_t sound_timer;

} chip8_t;