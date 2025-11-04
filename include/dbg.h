#pragma once

#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>


// can handle at most 255 byte dump and is not thread safe
const char * byte_dump(const void *data, uint8_t size) {

    static char buffer[256 * 3 + 1]; // +1 for '\0' of sprintf

    int from = 0;
    for (size_t i = 0; i < size; ++i) {
        int retval = sprintf(buffer + from, "%02x ", ((uint8_t *)data)[i]);
        assert(retval == 3);
        from += retval;
    }

    return buffer;
}
