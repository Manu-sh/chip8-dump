#pragma once
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <immintrin.h>

#ifdef DEBUG
    #include <stdio.h>
#endif


#ifdef FORCED
    #warning "FORCED() macro already defined, inline may not performed"
#endif


/*
 NOTE: to use inline you need to compile at least with c99, you can disable inline defining FORCED(_) as a macro that do nothing
 __STDC_VERSION__ is a macro defined with c95 (199409L)
*/

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    #define FORCED(_)
#endif


/* just hope that is a real inline */
#ifndef FORCED
    #define FORCED(_UNUSED_) inline __attribute__((always_inline))
#endif

#define LIKELY(_EXP_)       __builtin_expect(_EXP_, 1)
#define UNLIKELY(_EXP_)     __builtin_expect(_EXP_, 0)


// helper to cut away the padding bits: data >> (16 - to_extract));
// this is like the ''.substr(0, bit_length) method but for binary,
// valid ranges for bit_length is 1-16
// take_few_bits16(0b11110101, 4) -> 1111
// take_few_bits16(0b10110101, 4) -> 1011
uint16_t take_few_bits16(uint16_t data, uint8_t bit_length) {
    assert(bit_length <= 16);
    return data >> (16 - bit_length);
}

uint16_t bit_slice16(uint16_t data, uint8_t from, uint8_t to) {
    return take_few_bits16(data << from, to) >> from;
}

uint16_t nibble_slice16(uint16_t data, uint8_t from, uint8_t to) {
    return bit_slice16(data, from * 4, to * 4);
}


static FORCED(inline) void set_bit(uint8_t *restrict byte, uint8_t i) {
    assert(i < 8);
    *byte |= (1 << (7-i));
}

static FORCED(inline) void clear_bit(uint8_t *restrict byte, uint8_t i) {
    assert(i < 8);
    *byte &= ~(1 << (7-i));
}

static FORCED(inline) void assign_bit(uint8_t *restrict v, uint64_t bit_index, bool value) {
    __builtin_prefetch(&bit_index,  1, 3);
    const uint64_t byte_idx = bit_index >> 3; // (i/8)
    (void)(value ? set_bit(v + byte_idx, bit_index & 7) : clear_bit(v + byte_idx, bit_index & 7));  // i&7 -> i%8
}

// https://github.com/Manu-sh/huffman/blob/main/include/bitarray/BitArray.hpp#L166
static FORCED(inline) bool access_bit(const uint8_t *const restrict v, uint64_t bit_index) {
    const uint64_t byte_idx = bit_index >> 3; // (i/8)
    return (v[byte_idx] >> (7 - (bit_index&7))) & 1; // i&7 -> i%8
}

