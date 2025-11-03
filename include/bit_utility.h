#pragma once

#include <stdint.h>
#include <assert.h>


// helper to cut away the padding bits: data >> (16 - to_extract));
// this is like the ''.substr(0, bit_length) method but for binary,
// valid ranges for bit_length is 1-16
// take_few_bits(0b11110101, 4) -> 1111
// take_few_bits(0b10110101, 4) -> 1011
uint16_t take_few_bits(uint16_t data, uint8_t bit_length) {
    assert(bit_length <= 16);
    return data >> (16 - bit_length);
}

uint16_t bit_slice(uint16_t data, uint8_t from, uint8_t to) {
    return take_few_bits(data << from, to) >> from;
}

uint16_t nibble_slice(uint16_t data, uint8_t from, uint8_t to) {
    return bit_slice(data, from * 4, to * 4);
}
