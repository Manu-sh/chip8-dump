#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <bit_utility.h>
#include <chip8.h>
#include <font.h>
#include <opcode.h>
#include <dbg.h>


size_t file_size(FILE *f) {
    size_t cur_p, fsize;
    cur_p = ftell(f); fseek(f, 0L, SEEK_END);
    fsize = ftell(f); fseek(f, cur_p, SEEK_SET);
    return fsize;
}

// const uint64_t rounded_up = (index / align + 1) * align;
uint64_t round_up(uint64_t size, uint64_t align) {
    uint64_t index = size - 1; // TODO: cambiare in size - !!size
    return (index / align + 1) * align;
}

// WARN: non chiamare realloc() sul blocco visto che è usato aligned_alloc()
uint8_t * rom_map(const char *fpath, size_t *file_sz, size_t *mem_sz) {

    FILE *f;

    if (!(f = fopen(fpath, "rb")))
        return NULL;

    *file_sz = file_size(f);
    *mem_sz = round_up(*file_sz, sizeof(uint16_t)); // es. if you need 133 byte you will get 132 byte, one of padding

    uint8_t *memory;

    if (*file_sz == 0 || !(memory = (uint8_t *)aligned_alloc(sizeof(uint16_t), *mem_sz))) {
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

#if 0
#include <gui.h>
#include <raylib.h>
#include <emulated_instructions.h>

int main(int argc, char *argv[]) {

    assert(argc > 1);
    printf("argv[1] = \"%s\"\n", argv[1]);

    init_gui();

    Texture2D tex = {
        .id      = rlLoadTexture(NULL, GetScreenWidth(),  GetScreenHeight(), PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1),
        .width   = GetScreenWidth(),
        .height  = GetScreenHeight(),
        .format  = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        .mipmaps = 1
    };


    chip8_t chip;

    // Main game loop
    while (!WindowShouldClose()) { // Detect window close button or ESC key
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        BeginDrawing();
        //ClearBackground(BLACK);


        i00E0(&chip);

        // draw a line of pixel
        memset(chip.screen + 2 * 64, 0xff, 64);

        UpdateTexture(tex, chip.screen);
        DrawTexture(tex, 0, 0, WHITE);

        //DrawText("ciao!", 0, 0, 20, WHITE);
        EndDrawing();

    }

    CloseWindow(); // Close window and OpenGL context
    return 0;
}

#else
int main(int argc, char *argv[]) {

    assert(argc > 1);

    //dump_instruction(0x6a00);
    //return 0;

    //dump_instruction(0x3230);
    //dump_instruction(0x4230);
    //dump_instruction(0x5230);
    //dump_instruction((opcode_t){.data = 0X80FFu});

    //return 0;

    printf("argv[1] = \"%s\"\n", argv[1]);

#if 1
    FILE *file = fopen(argv[1], "rb");

    for (opcode_t instr; fread(&instr.data, sizeof(uint16_t), 1, file) == 1; ) {
        instr.data = be16toh(instr.data); // for my mental health
        dump_instruction(instr);
    }

    if (ferror(file))
        perror("fread()");

    fclose(file);
	return 0;
#endif

    size_t file_sz, mem_sz;
    uint8_t *rom = rom_map(argv[1], &file_sz, &mem_sz);
    assert(rom);
    assert( ((uintptr_t)rom) % sizeof(uint16_t) == 0);

    printf("file size in bytes: %zu\n", file_sz);
    printf("mem size in bytes: %zu\n", mem_sz);


    //

    free(rom);
}
#endif