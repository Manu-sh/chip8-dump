#pragma once

#include <endian.h>
#include <stdint.h>
#include <string.h>

#include <asm/byteorder.h>
#include <bit_utility.h>

#define X(_INSTR_) (nibble_slice(_INSTR_, 1, 2))
#define Y(_INSTR_) (nibble_slice(_INSTR_, 2, 3))

#define NN(_INSTR_)  (nibble_slice(_INSTR_, 2, 4))
#define NNN(_INSTR_) (nibble_slice(_INSTR_, 1, 4))
#define N(_INSTR_)   (nibble_slice(_INSTR_, 3, 4))

typedef union __attribute__((__packed__)) {

    uint16_t data;
    uint8_t byte[sizeof(uint16_t)];

	struct {
        #ifdef __LITTLE_ENDIAN_BITFIELD
            uint16_t N    : 4;
            uint16_t Y    : 4;
            uint16_t X    : 4;
            uint16_t type : 4;
        #elif defined(__BIG_ENDIAN_BITFIELD)
            uint16_t type : 4;
            uint16_t X    : 4;
            uint16_t Y    : 4;
            uint16_t N    : 4;
        #else
            #error "ooops"
        #endif
	};

    struct {
        #ifdef __LITTLE_ENDIAN_BITFIELD
            uint16_t NNN : 12;
            uint16_t     : 4;
        #elif defined(__BIG_ENDIAN_BITFIELD)
            uint16_t     : 4;
            uint16_t NNN : 12;
        #endif
    };

    struct {
        #ifdef __LITTLE_ENDIAN_BITFIELD
            uint16_t NN : 8;
            uint16_t    : 4;
            uint16_t    : 4;
        #elif defined(__BIG_ENDIAN_BITFIELD)
            uint16_t    : 4;
            uint16_t    : 4;
            uint16_t NN : 8;
        #endif
    };

} opcode_t;
