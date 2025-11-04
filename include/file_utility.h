#pragma once
#include <stdio.h>
#include <stddef.h>

size_t file_size(FILE *f) {
    size_t cur_p, fsize;
    cur_p = ftell(f); fseek(f, 0L, SEEK_END);
    fsize = ftell(f); fseek(f, cur_p, SEEK_SET);
    return fsize;
}
