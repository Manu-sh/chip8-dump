#pragma once
#include <stdio.h>
#include <stddef.h>



size_t file_size(FILE *f) {
    size_t cur_p, fsize;
    cur_p = ftell(f); fseek(f, 0L, SEEK_END);
    fsize = ftell(f); fseek(f, cur_p, SEEK_SET);
    return fsize;
}

#if 0
#include <stdint.h>
#include <stdlib.h>

// const uint64_t rounded_up = (index / align + 1) * align;
uint64_t round_up(uint64_t size, uint64_t align) {
    uint64_t index = size - 1; // TODO: cambiare in size - !!size
    return (index / align + 1) * align;
}

// WARN: non chiamare realloc() sul blocco visto che è usato aligned_alloc()
void * rom_map(const char *fpath, size_t *file_sz, size_t *mem_sz) {

    FILE *f;

    if (!(f = fopen(fpath, "rb")))
        return NULL;

    *file_sz = file_size(f);
    *mem_sz = round_up(*file_sz, sizeof(uint16_t)); // es. if you need 133 byte you will get 132 byte, one of padding

    void *memory;

    if (*file_sz == 0 || !(memory = aligned_alloc(sizeof(uint16_t), *mem_sz))) {
        fclose(f);
        return NULL;
    }

    // set padding byte(s) to zero
    if (*mem_sz > *file_sz)
        memset(memory + *file_sz - 1, 0x00, *mem_sz - *file_sz);

    const size_t readed = fread(memory, sizeof(uint8_t), *file_sz, f);

    if (readed != *file_sz) {
        free(memory), fclose(f);
        return NULL;
    }

    fclose(f);
    return memory;
}
#endif