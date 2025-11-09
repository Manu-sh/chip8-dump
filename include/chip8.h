#pragma once
#include <stdint.h>
#include <stdalign.h>
#include <screen.h>

#include <instruction.h>
#include <dbg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <file_utility.h>
#include <font.h>


typedef uint16_t u16;
#include <inlined/lifo/lifo_u16.h>

enum { REG_V0, REG_V1, REG_V2, REG_V3, REG_V4, REG_V5, REG_V6, REG_V7, REG_V8, REG_V9, REG_VA, REG_VB, REG_VC, REG_VD, REG_VE, REG_VF, REG_LEN };

/*
 Input is done with a hex keyboard that has 16 keys ranging 0 to F.
 The "8", "4", "6", and "2" keys are typically used for directional input.
*/

typedef uint8_t keystate_t;
typedef enum { HKEY_0, HKEY_1, HKEY_2, HKEY_3, HKEY_4, HKEY_5, HKEY_6, HKEY_7, HKEY_8, HKEY_9, HKEY_A, HKEY_B, HKEY_C, HKEY_D, HKEY_E, HKEY_F, HKEY_LEN } keycodes_t;

enum { NOT_PRESSED, PRESSED };

typedef struct {

    /* CHIP-8 programs should be loaded into memory starting at address 0x200. The memory addresses 0x000 to 0x1FF are reserved for the CHIP-8 interpreter. */
    union {
        alignas(uint16_t) uint8_t  reserved[0x200];   // 512 byte usually untouched by the rom
        alignas(uint16_t) uint8_t  memory[0xfff + 1]; // 4096 bytes of memory
    };

    alignas(32) uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];

    union {
        uint8_t V[REG_LEN]; // 16 data registers

        struct {
            uint8_t V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;
        };
    };


    // 2**12 -> 4096 possible values from 0 to 2**12-1
    uint16_t I  : 12; // address register (it can only be loaded with a 12-bit memory address due to the range of memory accessible to CHIP-8)
    uint16_t    : 4;

    uint16_t PC : 12; // program counter
    uint16_t    : 4;

    volatile uint8_t delay_timer;
    volatile uint8_t sound_timer;

    keystate_t keypad[HKEY_LEN];
    struct {
        bool is_awaiting;        // iFX0A: when awaiting for keypress every instruction is halted
        uint8_t await_dreg  : 4; // in which data register store the awaited key (0, 0xf);
        uint8_t             : 4;
    };

    lifo_u16 *stack;

    // use(ful?) metadata
    struct {
        uint16_t rom_size; // maximum value is 3584 bytes (the rom will be loaded at 0x200 address)
    };

} chip8_t;


chip8_t * chip_new() {

    chip8_t *self;

    if (!(self = calloc(1, sizeof(chip8_t))))
        return NULL;

    // copy front sprites at the beginning of the memory (0-512)
    assert(sizeof(font_sprites) < sizeof(self->reserved));
    memcpy(self->reserved, font_sprites, sizeof(font_sprites));

    self->PC = 0x200;
    self->I  = 0x200;

    if (!(self->stack = lifo_u16_new()))
        return free(self), NULL;

    return self;
}


void chip_free(chip8_t *self) {
    lifo_u16_free(self->stack);
    free(self);
}

// tick down if non zero.
void chip_tick(chip8_t *self) {
    self->delay_timer -= !!self->delay_timer;
    self->sound_timer -= !!self->sound_timer;
}

/*
 A key press is awaited, and then stored in VX
 (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing).
*/
void chip_press_key(chip8_t *self, keycodes_t key_code) {

    key_code &= 0xf; // same of key_code %= HKEY_LEN

    if (LIKELY(!self->is_awaiting)) {
        self->keypad[key_code] = PRESSED;
        return;
    }

    // store directly in the register and resume the status of machine
    self->V[self->await_dreg] = key_code;
    self->is_awaiting         = false;
}

bool chip_load_rom(chip8_t *chip, const char *fpath) {

    assert(chip->rom_size == 0); // rom already loaded, crash the program

    FILE *file;
    if (!(file = fopen(fpath, "rb"))) {
        dbg("cannot open the path=\"%s\"\n", fpath);
        return false;
    }

    const size_t rom_size = file_size(file);
    if (rom_size < sizeof(uint16_t) || rom_size >= 0xfff - 0x200 + 1) { // 3584 bytes (the rom will be loaded at 0x200 address)
        dbg("Error invalid rom size=\"%zu\"\n", rom_size);
        fclose(file);
        return false;
    }

    const size_t bytes_read = fread(chip->memory + 0x200, sizeof(uint8_t), rom_size, file);
    if (bytes_read != rom_size) {
        dbg("I/O error bytes read: \"%zu\" expected: \"%zu\" \n", bytes_read, rom_size);
        fclose(file);
        return false;
    }

    //dbg("bytes read %zu\n", bytes_read);
    assert(chip->memory + 0x200 + rom_size <= chip->memory + 4096);

    // WARNING: !! DO-NOT: swap the rom endianess! since contain raw bytes like sprites etc. Not just instructions
    fclose(file);
    return true;
}

// 0X00E0 disp_clear() - Clears the screen
void i00E0(chip8_t *chip) {
    memset(__builtin_assume_aligned(chip->screen, 32), 0x00, sizeof(chip->screen)); // In Chip-8 By default, the screen is set to all black pixels.
}

// es. 0X600C V0 = 0XC - Sets VX to NN
void i6XNN(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] = instr.NN;
}

// 0XA22A I = 0X22A;
void iANNN(chip8_t *chip, instr_t instr) {
    chip->I = instr.NNN;
}

//PC = V0 + %#03X - Jumps to the address NNN plus V0.
void iBNNN(chip8_t *chip, instr_t instr) {
    chip->PC = chip->V0 + instr.NNN; // CHIP-8 compliant
}

// TODO: inizializzare il seed?
// CXNN: Vx = rand() & NN - Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
void iCXNN(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] = rand() & instr.NN;
}

// 0XD01F draw(V0, V1, f)
/*

    Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.

    Each row of 8 pixels is read as bit-coded starting from memory location I;
    I value does not change after the execution of this instruction.
    As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.

    CHIP-8 sprites are always eight pixels wide and between one to fifteen pixels high.
    If the sprite is to be visible on the screen,
    the VX register must contain a value between 00 and 3F (63: width index),
    and the VY register must contain a value between 00 and 1F (31: height index).

    For sprite data, a bit set to one corresponds to a white pixel.
    Contrastingly, a bit set to zero corresponds to a transparent pixel. (it's leave untouched)


    The two registers passed to this instruction determine the x and y location of the sprite on the screen.


    When this instruction is processed by the interpreter, N bytes of data are read from memory starting from the address stored in register I.
    These bytes then represent the sprite data that will be used to draw the sprite on the screen.
    Therefore, the value of the I register determines which sprite is drawn

    and should always point to the memory address where the sprite data for the desired graphic is stored.


    The corresponding graphic on the screen will be eight pixels wide and N pixels high.

    If the program attempts to draw a sprite at an x coordinate greater than 0x3F, the x value will be reduced modulo 64.
    Similarly, if the program attempts to draw at a y coordinate greater than 0x1F, the y value will be reduced modulo 32.

    Sprites that are drawn partially off-screen will be clipped.
 */

// TODO: più check buffer overflow in particolare il bound sul chip->I e sugli altri data register
void iDXYN(chip8_t *chip, instr_t instr) {

    // legge n byte consecutivi da memoria a partire da I, ciascun byte rappresenta una riga di 8 pixel.
    const uint8_t *const beg_sprite = chip->memory + chip->I;
    const uint8_t *const end_sprite = chip->memory + chip->I + instr.N; // n bytes of memory
    assert(end_sprite <= chip->memory + 4096);

    // The two registers passed to this instruction determine the x and y location of the sprite on the screen.
    const uint16_t x = chip->V[instr.X]; // x-offset (col_offset)
    const uint16_t y = chip->V[instr.Y]; // y-offset (row_offset)

    // instr.N * 8 -> bit
    const uint8_t sprite_len        = end_sprite - beg_sprite;
    const uint8_t sprite_bit_len    = sprite_len * 8;
    const uint8_t sprite_bit_height = instr.N; // in bits
    const uint8_t sprite_bit_width  = 8;       // in bits

    // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
    // The corresponding graphic on the screen will be eight pixels wide and N pixels high.

    chip->VF = 0;

    // The corresponding graphic on the screen will be eight pixels wide and N pixels high.
    for (uint8_t sprite_h = 0; sprite_h < sprite_bit_height; ++sprite_h) {
        for (uint8_t sprite_w = 0; sprite_w < sprite_bit_width; ++sprite_w) {

            const uint8_t sprite_bit_idx = sprite_h * sprite_bit_width + sprite_w; // a bit matrix in row-major-order
            const uint8_t pixel = access_bit(beg_sprite, sprite_bit_idx) ? 0xff : 0x00;

#ifdef CHIP_DEBUG
            printf("%d ", pixel ? 1 : 0);
#endif
            // questo fa il wrap around, tecnicamente è una roba di super-chip in chip8 originale viene clippato e basta se esce dallo schermo.
            uint16_t pixel_index = SC(
        (y + sprite_h) % SCREEN_HEIGHT,
        (x + sprite_w) % SCREEN_WIDTH
            );

            const uint8_t pixel_tmp = chip->screen[pixel_index];
            chip->screen[pixel_index] ^= pixel;
            chip->VF |= pixel_tmp && pixel; // disegna in XOR qua c'è il carry chip->VF = chip->VF || (old_pixel == pixel); spenge il pixel se entrambi sono on
            //chip->VF |= !!pixel_tmp & pixel;
        }
#ifdef CHIP_DEBUG
        printf("%c", '\n');
#endif
    }
}


// es. 0X7009 V0 += 0X9 - Adds NN to VX (carry flag is not changed)
void i7XNN(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] += instr.NN;
}

//  es. 0X1228 goto 0X228; - Jumps to address NNN.
void i1NNN(chip8_t *chip, instr_t instr) {
    chip->PC = instr.NNN;
}

// es. 0X362B if (V6 == 0x2b) - Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block).
void i3XNN(chip8_t *chip, instr_t instr) {
    chip->PC += (chip->V[instr.X] == instr.NN) << 1; // same of: (chip->V[instr.X] == instr.NN) ? sizeof(instr_t) : 0
}

// es. 0X452A if (V5 != 0x2a) - Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).
void i4XNN(chip8_t *chip, instr_t instr) {
    chip->PC += (chip->V[instr.X] != instr.NN) << 1;
}

// .. - Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block).
void i5XY0(chip8_t *chip, instr_t instr) {
    chip->PC += (chip->V[instr.X] == chip->V[instr.Y]) << 1;
}

// es. 0X9560 if (V5 != V6) - Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block).
void i9XY0(chip8_t *chip, instr_t instr) {
    chip->PC += (chip->V[instr.X] != chip->V[instr.Y]) << 1;
}

// es.  0X2812 *(0X812)() - Calls subroutine at NNN.
void i2NNN(chip8_t *chip, instr_t instr) {

    // 0x2NNN: Call subroutine at NNN
    // Store current address to return to on subroutine stack ("push" it on the stack)
    //   and set program counter to subroutine address so that the next instruction
    //   is gotten from there.

    lifo_u16_push(chip->stack, chip->PC);
    chip->PC = instr.NNN;
}

// 0X00EE return; - Returns from a subroutine.
void i00EE(chip8_t *chip) {
    assert( !lifo_u16_isEmpty(chip->stack) );
    chip->PC = lifo_u16_pop(chip->stack);
}

// es. 0X8750 V7 = V5 - Sets VX to the value of VY.
void i8XY0(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] = chip->V[instr.Y];
}

// es. 0X87B1 V7 |= Vb - Sets VX to VX or VY. (bitwise OR operation).
void i8XY1(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] |= chip->V[instr.Y];
    chip->VF = 0; // CHIP-8 compliant: add VF reset as described here: https://github.com/Timendus/chip8-test-suite/blob/main/bin/
}

// es. 0X87B2 V7 &= Vb - Sets VX to VX and VY. (bitwise AND operation).
void i8XY2(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] &= chip->V[instr.Y];
    chip->VF = 0; // CHIP-8 compliant
}

// es. 0X87B3 V7 ^= Vb - Sets VX to VX xor VY.
void i8XY3(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] ^= chip->V[instr.Y];
    chip->VF = 0; // CHIP-8 compliant
}

// es. 0X8764 V7 += V6 - Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.
void i8XY4(chip8_t *chip, instr_t instr) {

    // dopotutto perché... perché non dovrei?! **rigira avidamente l'anello tra le mani**
    chip->VF = !!(__builtin_add_overflow(
        chip->V[instr.X],
        chip->V[instr.Y],
        chip->V + instr.X
    ));
}

// es. 0X8765 V7 -= V6 - VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not).
void i8XY5(chip8_t *chip, instr_t instr) {

    chip->VF = !!(__builtin_sub_overflow(
        chip->V[instr.X],
        chip->V[instr.Y],
        chip->V + instr.X
    ));
}

// Vx = Vy - Vx Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX).
void i8XY7(chip8_t *chip, instr_t instr) {

    chip->VF = !!(__builtin_sub_overflow(
        chip->V[instr.Y],
        chip->V[instr.X],
        chip->V + instr.X
    ));
}

// es. 0X866E V6 <<= 1 - Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.
void i8XYE(chip8_t *chip, instr_t instr) {
    /*
    chip->VF = access_bit(chip->V + instr.X, 0); // take the msb
    chip->V[instr.X] <<= 1;
     */

    // CHIP-8 compliant
    chip->VF = access_bit(chip->V + instr.Y, 0); // take the lsb
    chip->V[instr.X] = chip->V[instr.Y] << 1;
}

// es. 0X8666 V6 >>= 1 - Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.
void i8XY6(chip8_t *chip, instr_t instr) {
    /*
    chip->VF = access_bit(chip->V + instr.X, sizeof(uint8_t) - 1); // take the lsb
    chip->V[instr.X] >>= 1;
     */

    // CHIP-8 compliant
    chip->VF = access_bit(chip->V + instr.Y, sizeof(uint8_t) - 1); // take the lsb
    chip->V[instr.X] = chip->V[instr.Y] >> 1;
}

// es.  0XF155 reg_dump(V1, &I)  - Stores from V0 to VX (including VX) in memory,
// starting at address I. The offset from I is increased by 1 for each value written,
// but I itself is left unmodified.
void iFX55(chip8_t *chip, instr_t instr) {

    const size_t sz = instr.X + 1;
    assert(chip->I + sz <= 4096);

    memcpy(chip->memory + chip->I, chip->V, sz);
    chip->I += sz; // CHIP-8 compliant

}

// es. 0XF065 reg_load(V0, &I) - Fills from V0 to VX (including VX) with values from memory,
// starting at address I. The offset from I is increased by 1 for each value read,
// but I itself is left unmodified.
void iFX65(chip8_t *chip, instr_t instr) {

    const size_t sz = instr.X + 1;
    assert(chip->I + sz <= 4096);

    memcpy(chip->V, chip->memory + chip->I, sz);
    chip->I += sz; // CHIP-8 compliant
}


//  es. 0XF633 set_BCD(V6) *(I+0) = BCD(3);*(I+1) = BCD(2);*(I+2) = BCD(1); - Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
// Store the binary-coded decimal equivalent of the value stored in register VX
// at addresses I, I + 1, and I + 2
// The interpreter takes the decimal value of Vx,
// and places the hundreds digit in memory at location in I,
// the tens digit at location I+1,
// and the ones digit at location I+2.
void iFX33(chip8_t *chip, instr_t instr) {

    assert(chip->I + 2 < 4096); // mem[I+2] writeable

    uint8_t value = chip->V[instr.X]; // es. 123
    chip->memory[chip->I + 2] = value % 10, value /= 10; // store 3
    chip->memory[chip->I + 1] = value % 10, value /= 10; // store 2
    chip->memory[chip->I + 0] = value % 10;              // store 1
}

// es. 0XFC29 I = sprite_addr[Vc] -
//  Sets I to the location of the sprite for the character in VX(only consider the lowest nibble).
//  Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void iFX29(chip8_t *chip, instr_t instr) {
    //chip->I = chip->V[instr.X] & 0x0f;

    //dbg("%d\n", N(chip->V[instr.X]));
    //dbg("%d\n", N(0xfc));

    chip->I = N(chip->V[instr.X]) * sizeof(font_sprites[0]);
}

// es. 0XF015 delay_timer(V0) - Sets the delay timer to VX.
void iFX15(chip8_t *chip, instr_t instr) {
    chip->delay_timer = chip->V[instr.X];
}

// es. 0XF007 V0 = get_delay() - Sets VX to the value of the delay timer.
void iFX07(chip8_t *chip, instr_t instr) {
    chip->V[instr.X] = chip->delay_timer;
}

// es. 0XF118 sound_timer(V1) - Sets the sound timer to VX.
void iFX18(chip8_t *chip, instr_t instr) {
    chip->sound_timer = chip->V[instr.X];
}

// I += V%x - Adds VX to I. VF is not affected.
void iFX1E(chip8_t *chip, instr_t instr) {
    chip->I += chip->V[instr.X];
}

// EX9E. if (key() == Vx) Skips the next instruction if the key stored
// in VX(only consider the lowest nibble) is pressed
// (usually the next instruction is a jump to skip a code block).
void iEX9E(chip8_t *chip, instr_t instr) {
    const uint8_t expected_key = N(chip->V[instr.X]);
    chip->PC += (chip->keypad[ expected_key ] == PRESSED) << 1; // same of: (chip->keypad[ N(chip->V[instr.V]) ] == PRESSED) ? sizeof(instr_t) : 0

    chip->keypad[expected_key] = NOT_PRESSED;
}

// iEXA1 if (key() != V%x) - Skips the next instruction if the key stored
// in VX(only consider the lowest nibble) is not pressed
// (usually the next instruction is a jump to skip a code block).
void iEXA1(chip8_t *chip, instr_t instr) {
    const uint8_t expected_key = N(chip->V[instr.X]);
    chip->PC += (chip->keypad[ expected_key ] == NOT_PRESSED) << 1;
}


// iFX0A = get_key()
// A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing).
void iFX0A(chip8_t *chip, instr_t instr) {
    chip->await_dreg  = instr.X;
    chip->is_awaiting = true;
}

instr_t chip_fetch(const chip8_t *chip, uint16_t chip_addr) {
    assert(chip_addr <= (4096 - sizeof(uint16_t))); // usually chip_addr is the program counter
    return (instr_t) {
        .data = be16toh( *((uint16_t *)(chip->memory + chip_addr)) )
    };
}


void chip_exec(chip8_t *chip, instr_t instr) {

    // execution is halted by iFX0A, waiting for a key being pressed
    if (UNLIKELY(chip->is_awaiting)) // NOP
        return;

#ifdef CHIP_DEBUG
    dump_instruction(instr);
#endif

    if (instr.data == 0x00E0) {
        i00E0(chip);
        chip->PC += sizeof(instr_t);
        return;
    } else if (instr.data == 0x00EE) {
        i00EE(chip);
        chip->PC += sizeof(instr_t);
        return;
    }

    switch (instr.type) {
        case 0:
            printf("%#06X call( %#03X ); - Calls machine code routine at address NNN.\n",
               instr.data,
               instr.NNN
            );
            assert(0);
            return;
        case 1:
            i1NNN(chip, instr); // it's a jump, do not move the PC
            return;
        case 2:
            i2NNN(chip, instr); // it's a call-jump, do not move the PC
            return;
        case 3:
            i3XNN(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 4:
            i4XNN(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 5:
            i5XY0(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 6:
            i6XNN(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 7:
            i7XNN(chip, instr);
            chip->PC += sizeof(instr_t);
            return;

        case 8:
            // check last nibble
            switch (instr.N) {
                case 0:
                    i8XY0(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 1:
                    i8XY1(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 2:
                    i8XY2(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 3:
                    i8XY3(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 4:
                    i8XY4(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 5:
                    i8XY5(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 6:
                    i8XY6(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 7:
                    i8XY7(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0xE:
                    i8XYE(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;

                default:
                    goto not_an_opcode;
            }

        case 9:
            i9XY0(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 0xA:
            iANNN(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 0xB:
            iBNNN(chip, instr);
            return;
        case 0xC:
            iCXNN(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 0xD:
            iDXYN(chip, instr);
            chip->PC += sizeof(instr_t);
            return;
        case 0xE:
            switch (NN(instr.data)) {
                case 0x9E:
                    iEX9E(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0xA1:
                    iEXA1(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;

                default:
                    goto not_an_opcode;
            }
        case 0xF:
            switch (instr.NN) {
                case 0x07:
                    iFX07(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x0A:
                    iFX0A(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x15:
                    iFX15(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x18:
                    iFX18(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x1E:
                    iFX1E(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x29:
                    iFX29(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x33:
                    iFX33(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x55:
                    iFX55(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;
                case 0x65:
                    iFX65(chip, instr);
                    chip->PC += sizeof(instr_t);
                    return;

                default:
                    goto not_an_opcode;
            }

        default:
            not_an_opcode:
            printf("NOT AN OPCODE: %#06X - b:%x,%x\n",
               instr.data, instr.byte[0], instr.byte[1]
            );
            //assert(0);
            return;

    }


}