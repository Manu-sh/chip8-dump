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


#if 1
#include <gui.h>
#include <raylib.h>


int main(int argc, char *argv[]) {

    assert(argc > 1);
    printf("argv[1] = \"%s\"\n", argv[1]);

    SetTraceLogLevel(LOG_ERROR);
    init_gui();

    Texture2D tex = {
        .id      = rlLoadTexture(NULL, GetScreenWidth(),  GetScreenHeight(), PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1),
        .width   = GetScreenWidth(),
        .height  = GetScreenHeight(),
        .format  = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        .mipmaps = 1
    };


    assert(argc > 1);

    chip8_t *chip = chip_new();

    if (!load_rom(chip, argv[1]))
        goto die;

    //exec(chip, *(opcode_t *)(chip->memory + chip->PC));
    //exec(chip, *(opcode_t *)(chip->prog_beg));

    chip->is_running = true;

    // Main game loop
    while (!WindowShouldClose()) { // Detect window close button or ESC key

        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        chip_exec(chip, chip_fetch(chip, chip->PC));

        // Draw
        BeginDrawing();
        //ClearBackground(BLACK);




        //i00E0(chip);

        // draw a line of pixel
        //memset(chip->screen + 2 * 64, 0xff, 64);


        //UpdateTexture(tex, scale_chip_screen(chip) );
        UpdateTexture(tex, scale_screen(chip));

        // disegna la texture con scaling automatico
        DrawTexture(tex, 0, 0, WHITE);

        //DrawText("ciao!", 0, 0, 20, WHITE);
        EndDrawing();

        sleep(1);

    }

die:
    CloseWindow(); // Close window and OpenGL context
    chip_free(chip);
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