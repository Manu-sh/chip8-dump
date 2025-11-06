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
#include <scale.h>

#if 1
#include <raylib.h>

#include <unistd.h>
#include <signal.h>



int main(int argc, char *argv[]) {

    assert(argc > 1);
    printf("argv[1] = \"%s\"\n", argv[1]);

    SetTraceLogLevel(LOG_ERROR);

    InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "chip8 emulator");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //SetTargetFPS(60 * 12); // Set our game to run at 60 frames-per-second


    Texture2D tex = {
        .id      = rlLoadTexture(NULL, GetScreenWidth(),  GetScreenHeight(), PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1),
        .width   = GetScreenWidth(),
        .height  = GetScreenHeight(),
        .format  = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        .mipmaps = 1
    };


    assert(argc > 1);

    chip8_t *chip = chip_new();

    if (!chip_load_rom(chip, argv[1]))
        goto die;

    //exec(chip, *(opcode_t *)(chip->memory + chip->PC));
    //exec(chip, *(opcode_t *)(chip->prog_beg));

    // Main game loop
    while (!WindowShouldClose()) { // Detect window close button or ESC key

        PollInputEvents();

        /* for (int k; (k = GetKeyPressed()) != 0; ) {
            switch (k) {
                case KEY_ZERO:  chip->keypad[HKEY_0] = PRESS; break;
                case KEY_ONE:   chip->keypad[HKEY_1] = PRESS; break;
                case KEY_TWO:   chip->keypad[HKEY_2] = PRESS; break;
                case KEY_THREE: chip->keypad[HKEY_3] = PRESS; break;
                case KEY_FOUR:  chip->keypad[HKEY_4] = PRESS; break;
                case KEY_FIVE:  chip->keypad[HKEY_5] = PRESS; break;
                case KEY_SIX:   chip->keypad[HKEY_6] = PRESS; break;
                case KEY_SEVEN: chip->keypad[HKEY_7] = PRESS; break;
                case KEY_EIGHT: chip->keypad[HKEY_8] = PRESS; break;
                case KEY_NINE:  chip->keypad[HKEY_9] = PRESS; break;
                case KEY_A:     chip->keypad[HKEY_A] = PRESS; break;
                case KEY_B:     chip->keypad[HKEY_B] = PRESS; break;
                case KEY_C:     chip->keypad[HKEY_C] = PRESS; break;
                case KEY_D:     chip->keypad[HKEY_D] = PRESS; break;
                case KEY_E:     chip->keypad[HKEY_E] = PRESS; break;
                case KEY_F:     chip->keypad[HKEY_F] = PRESS; break;
            }
        }*/


        if (IsKeyDown(KEY_ZERO))  chip->keypad[HKEY_0] = PRESS;
        if (IsKeyDown(KEY_ONE))   chip->keypad[HKEY_1] = PRESS;
        if (IsKeyDown(KEY_TWO))   chip->keypad[HKEY_2] = PRESS;
        if (IsKeyDown(KEY_THREE)) chip->keypad[HKEY_3] = PRESS;
        if (IsKeyDown(KEY_FOUR))  chip->keypad[HKEY_4] = PRESS;
        if (IsKeyDown(KEY_FIVE))  chip->keypad[HKEY_6] = PRESS;
        if (IsKeyDown(KEY_SIX))   chip->keypad[HKEY_7] = PRESS;
        if (IsKeyDown(KEY_SEVEN)) chip->keypad[HKEY_8] = PRESS;
        if (IsKeyDown(KEY_EIGHT)) chip->keypad[HKEY_9] = PRESS;
        if (IsKeyDown(KEY_A))     chip->keypad[HKEY_A] = PRESS;
        if (IsKeyDown(KEY_B))     chip->keypad[HKEY_B] = PRESS;
        if (IsKeyDown(KEY_C))     chip->keypad[HKEY_C] = PRESS;
        if (IsKeyDown(KEY_D))     chip->keypad[HKEY_D] = PRESS;
        if (IsKeyDown(KEY_E))     chip->keypad[HKEY_E] = PRESS;
        if (IsKeyDown(KEY_F))     chip->keypad[HKEY_F] = PRESS;


        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        //dbg("PC: %#04x ", chip->PC);
        chip_exec(chip, chip_fetch(chip, chip->PC));

        // Draw
        BeginDrawing();

        //ClearBackground(BLACK);

        // draw a line of pixel
        //memset(chip->screen + 2 * 64, 0xff, 64);

        //UpdateTexture(tex, scale_chip_screen(chip) );
        UpdateTexture(tex, scale_screen(chip));

        // disegna la texture con scaling automatico
        DrawTexture(tex, 0, 0, WHITE);

        //DrawText("ciao!", 0, 0, 20, WHITE);
        EndDrawing();

        printf("%s\n", byte_dump(chip->keypad, sizeof(chip->keypad)));

        //usleep(200 * 1000);
        chip_tick(chip);

        memset(chip->keypad, NOT_PRESS, sizeof(chip->keypad));
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