#pragma once
#include <stdint.h>
#include <stdalign.h>
#include <screen.h>

#include <opcode.h>
#include <dbg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <file_utility.h>


typedef uint16_t u16;
#include <inlined/lifo/lifo_u16.h>

enum { REG_V0, REG_V1, REG_V2, REG_V3, REG_V4, REG_V5, REG_V6, REG_V7, REG_V8, REG_V9, REG_VA, REG_VB, REG_VC, REG_VD, REG_VE, REG_VF, REG_LEN };

typedef struct {

    /* CHIP-8 programs should be loaded into memory starting at address 0x200. The memory addresses 0x000 to 0x1FF are reserved for the CHIP-8 interpreter. */
    union {
        alignas(uint16_t) uint8_t  reserved[0x200];   // 512 byte usually untouched by the rom
        alignas(uint16_t) uint8_t  memory[0xfff + 1]; // 4096 bytes of memory
    };

    union {
        uint8_t V[REG_LEN]; // 16 data registers

        struct {
            uint8_t V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;
        };
    };

    union {
        uint16_t I; // address register (it can only be loaded with a 12-bit memory address due to the range of memory accessible to CHIP-8)
    };

    // TODO: forward list for the stack?
    lifo_u16 *stack;

    // per la grafica probabilmente è meglio usare direttamente vector_bit[64 * 32 * 8]
    // visto che si ragiona in termini di grafica monochrome a 2 bit

    alignas(32) uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];

    uint8_t delay_timer;
    uint8_t sound_timer;

    union {
        uint16_t PC; // program counter
    };


    // use(ful?) metadata
    struct {
        uint8_t *prog_beg; // the program start: memory + 0x200
        uint8_t *prog_end; // TODO: questo potrebbe non essere allineato
        uint16_t rom_size; // maximum value is 3584 bytes (the rom will be loaded at 0x200 address)
    };


    bool is_running;

} chip8_t;


chip8_t * chip_new() {

    chip8_t *self = malloc(sizeof(chip8_t));
    if (!self) return NULL;

    memset(self->memory, 0, sizeof(self->memory));
    memset(self->screen, 0, sizeof(self->screen)); // clear the screen

    self->rom_size = 0;
    self->prog_beg = __builtin_assume_aligned(self->memory + 0x200, sizeof(uint16_t));
    self->prog_end = self->prog_beg; // a default value
    self->PC       = 0x200;

    self->is_running = false;
    self->stack = lifo_u16_new();

    return self;
}

void chip_free(chip8_t *self) {
    lifo_u16_free(self->stack);
    free(self);
}


bool load_rom(chip8_t *chip, const char *fpath) {

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

    //const size_t bytes_read = fread(chip->memory + 0x200, sizeof(uint8_t), rom_size, file);
    const size_t bytes_read = fread(chip->prog_beg, sizeof(uint8_t), rom_size, file);
    if (bytes_read != rom_size) {
        dbg("I/O error bytes read: \"%zu\" expected: \"%zu\" \n", bytes_read, rom_size);
        fclose(file);
        return false;
    }

    printf("bytes read %zu\n", bytes_read);
    chip->rom_size = rom_size;
    chip->prog_end = chip->prog_beg + rom_size; // set to: chip->memory + 0x200 + rom_size
    assert(chip->prog_end <= chip->memory + 4096);

#if 0
    // !! DO-NOT: swap the rom endianess! since contain raw bytes like sprites etc
    //for (uint16_t *word = (uint16_t *)chip->prog_beg; word != (uint16_t *)chip->prog_end; ++word)
    //    *word = be16toh(*word);
#endif

    fclose(file);
    return true;
}

/*
void dump_ram(const chip8_t *chip) {

    for (const uint8_t *p = chip->memory + 0x200; p != chip->memory + 0xfff + 1; ++p)
        printf( "%s", byte_dump(p, sizeof(uint8_t)) );

    puts("");
}
*/

/*
 reg_load(Vx, &I)
 Fills from V0 to VX (including VX) with values from memory, starting at address I.

 The offset from I is increased by 1 for each value read, but I itself is left unmodified.
*/

// TODO: test me
/*
void iFX65(chip8_t *chip, int reg_index) {

    // TODO: controllare eventuali problemi di endianess con address
    uint16_t address = take_few_bits16(chip->I, 12); // take 12 bit from I

    printf("iFX65 endianess check: %s\n",
       byte_dump(&address, sizeof(address))
    );

    assert(chip->V + reg_index + 1 <= chip->V + REG_LEN);
    memcpy(chip->V, chip->memory + address, reg_index + 1);
}
*/

// 0X00E0 disp_clear() - Clears the screen
void i00E0(chip8_t *chip) {
    memset(__builtin_assume_aligned(chip->screen, 32), 0x00, sizeof(chip->screen)); // In Chip-8 By default, the screen is set to all black pixels.
}

// es. 0X600C V0 = 0XC - Sets VX to NN
void i6XNN(chip8_t *chip, opcode_t instr) {
    chip->V[instr.X] = instr.NN;
}

// 0XA22A I = 0X22A;
void iANNN(chip8_t *chip, opcode_t instr) {
    chip->I = instr.NNN;
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
void iDXYN(chip8_t *chip, opcode_t instr) {

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

            uint8_t sprite_bit_idx = sprite_h * sprite_bit_width + sprite_w; // a bit matrix in row-major-order
            uint8_t pixel = access_bit(beg_sprite, sprite_bit_idx) ? 0xff : 0x00;

            printf("%d ", pixel ? 1 : 0);

            // questo fa il wrap around, tecnicamente è una roba di super-chip in chip8 originale viene clippato e basta se esce dallo schermo.
            uint16_t pixel_index = SC(
        (y + sprite_h) % SCREEN_HEIGHT,
        (x + sprite_w) % SCREEN_WIDTH
            );

            uint8_t pixel_tmp = chip->screen[pixel_index];
            chip->screen[pixel_index] ^= pixel;
            chip->VF |= pixel_tmp & pixel; // disegna in XOR qua c'è il carry chip->VF = chip->VF || (old_pixel == pixel); spenge il pixel se entrambi sono on
        }

        printf("%c", '\n');
    }
}

// es. 0X7009 V0 += 0X9 - Adds NN to VX (carry flag is not changed)
void i7XNN(chip8_t *chip, opcode_t instr) {
    chip->V[instr.X] += instr.NN;
}

//  es. 0X1228 goto 0X228; - Jumps to address NNN.
void i1NNN(chip8_t *chip, opcode_t instr) {
    chip->PC = instr.NNN;
}

// es. 0X362B if (V6 == 0x2b) - Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block).
void i3XNN(chip8_t *chip, opcode_t instr) {
    chip->PC += (chip->V[instr.X] == instr.NN) << 1; // same of: (chip->V[instr.X] == instr.NN) ? sizeof(opcode_t) : 0
}

// es. 0X452A if (V5 != 0x2a) - Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).
void i4XNN(chip8_t *chip, opcode_t instr) {
    chip->PC += (chip->V[instr.X] != instr.NN) << 1;
}

// .. - Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block).
void i5XY0(chip8_t *chip, opcode_t instr) {
    chip->PC += (chip->V[instr.X] == chip->V[instr.Y]) << 1;
}

// es. 0X9560 if (V5 != V6) - Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block).
void i9XY0(chip8_t *chip, opcode_t instr) {
    chip->PC += (chip->V[instr.X] != chip->V[instr.Y]) << 1;
}

// es.  0X2812 *(0X812)() - Calls subroutine at NNN.
void i2NNN(chip8_t *chip, opcode_t instr) {

    // 0x2NNN: Call subroutine at NNN
    // Store current address to return to on subroutine stack ("push" it on the stack)
    //   and set program counter to subroutine address so that the next opcode
    //   is gotten from there.

    lifo_u16_push(chip->stack, chip->PC);
    chip->PC = instr.NNN;

    //puts( byte_dump(chip->memory + chip->PC, sizeof(opcode_t)) );
    //dbg("pc=%d\n", chip->PC);
    dbg("pc=%u\n", instr.NNN);
    dbg("pc=%u\n", chip->PC);
    dbg("end_prog=%p pc_addr=%p\n", chip->prog_end, chip->memory + chip->PC);
    //exit(0);
}


// 0X00EE return; - Returns from a subroutine.
void i00EE(chip8_t *chip) {
    assert( !lifo_u16_isEmpty(chip->stack) );
    //chip->PC = lifo_u16_pop(chip->stack) + sizeof(opcode_t);
    chip->PC = lifo_u16_pop(chip->stack);
}


// es. 0X8750 V7 = V5 - Sets VX to the value of VY.
void i8XY0(chip8_t *chip, opcode_t instr) {
    chip->V[instr.X] = chip->V[instr.Y];
}

// es. 0X87B1 V7 |= Vb - Sets VX to VX or VY. (bitwise OR operation).
void i8XY1(chip8_t *chip, opcode_t instr) {
    chip->V[instr.X] |= chip->V[instr.Y];
}

// es. 0X87B2 V7 &= Vb - Sets VX to VX and VY. (bitwise AND operation).
void i8XY2(chip8_t *chip, opcode_t instr) {
    chip->V[instr.X] &= chip->V[instr.Y];
}

// es. 0X87B3 V7 ^= Vb - Sets VX to VX xor VY.
void i8XY3(chip8_t *chip, opcode_t instr) {
    chip->V[instr.X] ^= chip->V[instr.Y];
}

// es. 0X8764 V7 += V6 - Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.
void i8XY4(chip8_t *chip, opcode_t instr) {

    // dopotutto perché... perché non dovrei?! **rigira avidamente l'anello tra le mani**
    chip->VF = !!(__builtin_add_overflow(
        chip->V[instr.X],
        chip->V[instr.Y],
        chip->V + instr.X
    ));
}

// es. 0X8765 V7 -= V6 - VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not).
void i8XY5(chip8_t *chip, opcode_t instr) {

    chip->VF = !!(__builtin_sub_overflow(
        chip->V[instr.X],
        chip->V[instr.Y],
        chip->V + instr.X
    ));
}

// Vx = Vy - Vx Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX).
void i8XY7(chip8_t *chip, opcode_t instr) {

    chip->VF = !!(__builtin_sub_overflow(
        chip->V[instr.Y],
        chip->V[instr.X],
        chip->V + instr.X
    ));
}

// es. 0X866E V6 <<= 1 - Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.
void i8XYE(chip8_t *chip, opcode_t instr) {
    chip->VF = access_bit(chip->V + instr.X, 0); // take the msb
    chip->V[instr.X] <<= 1;
}

// es. 0X8666 V6 >>= 1 - Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.
void i8XY6(chip8_t *chip, opcode_t instr) {
    chip->VF = access_bit(chip->V + instr.X, sizeof(uint8_t) - 1); // take the lsb
    chip->V[instr.X] >>= 1;
}

// es.  0XF155 reg_dump(V1, &I)  - Stores from V0 to VX (including VX) in memory,
// starting at address I. The offset from I is increased by 1 for each value written,
// but I itself is left unmodified.
void iFX55(chip8_t *chip, opcode_t instr) {

    const size_t sz = instr.X + 1;
    assert(chip->I + sz <= 4096);

    memcpy(chip->memory + chip->I, chip->V, sz);
}

// es. 0XF065 reg_load(V0, &I) - Fills from V0 to VX (including VX) with values from memory,
// starting at address I. The offset from I is increased by 1 for each value read,
// but I itself is left unmodified.
void iFX65(chip8_t *chip, opcode_t instr) {

    const size_t sz = instr.X + 1;
    assert(chip->I + sz <= 4096);

    memcpy(chip->V, chip->memory + chip->I, sz);
}


//  es. 0XF633 set_BCD(V6) *(I+0) = BCD(3);*(I+1) = BCD(2);*(I+2) = BCD(1); - Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
// Store the binary-coded decimal equivalent of the value stored in register VX
// at addresses I, I + 1, and I + 2
// The interpreter takes the decimal value of Vx,
// and places the hundreds digit in memory at location in I,
// the tens digit at location I+1,
// and the ones digit at location I+2.
void iFX33(chip8_t *chip, opcode_t instr) {

    assert(chip->I + 2 < 4096); // mem[I+2] writeable

    uint8_t value = chip->V[instr.X]; // es. 123
    chip->memory[chip->I + 2] = value % 10, value /= 10; // store 3
    chip->memory[chip->I + 1] = value % 10, value /= 10; // store 2
    chip->memory[chip->I + 0] = value % 10;              // store 1
}

// TODO:  0XFC29 I = sprite_addr[Vc] - Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). Characters 0-F (in hexadecimal) are represented by a 4x5 font.

opcode_t chip_fetch(const chip8_t *chip, uint16_t chip_addr) {
    assert(chip_addr <= (4096 - sizeof(uint16_t))); // usually chip_addr is the program counter
    return (opcode_t) {
        .data = be16toh( *((uint16_t *)(chip->memory + chip_addr)) )
    };
}


// TODO: load the program
void chip_exec(chip8_t *chip, opcode_t instr) {

    dump_instruction(instr);

    if (instr.data == 0x00E0) {
        i00E0(chip);
        chip->PC += sizeof(opcode_t);
        return;
    } else if (instr.data == 0x00EE) {
        i00EE(chip);
        chip->PC += sizeof(opcode_t);
        return;
    }

    switch (instr.type) {
        case 0:
            printf("%#06X call( %#03X ); - Calls machine code routine at address NNN.\n",
               instr.data,
               instr.NNN
            );
            return;
        case 1:
            i1NNN(chip, instr);
            //chip->PC += sizeof(opcode_t); it's a jump, do not move the PC
            return;
        case 2:
            i2NNN(chip, instr);
            //chip->PC += sizeof(opcode_t); //it's a call-jump, do not move the PC
            return;
        case 3:
            i3XNN(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;
        case 4:
            i4XNN(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;
        case 5:
            i5XY0(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;
        case 6:
            i6XNN(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;
        case 7:
            i7XNN(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;

        case 8:
            // check last nibble
            switch (instr.N) {
                case 0:
                    i8XY0(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 1:
                    i8XY1(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 2:
                    i8XY2(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 3:
                    i8XY3(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 4:
                    i8XY4(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 5:
                    i8XY5(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 6:
                    i8XY6(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 7:
                    i8XY7(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 0xE:
                    i8XYE(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;

                default:
                    goto not_an_opcode;
            }

        case 9:
            i9XY0(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;
        case 0xA:
            iANNN(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;
        case 0xB:
            printf("%#06X PC = V0 + %#03X - Jumps to the address NNN plus V0.\n",
                   instr.data,
                   instr.NNN
            );
            return;

        case 0xC:
            printf("%#06X V%x = rand() & %#02X; - Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.\n",
                   instr.data,
                   instr.X,
                   instr.NN
            );
            return;

        case 0xD:
            iDXYN(chip, instr);
            chip->PC += sizeof(opcode_t);
            return;

        case 0xE:
            switch (NN(instr.data)) {
                case 0x9E:
                    printf("%#06X if (key() == V%x) - Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed (usually the next instruction is a jump to skip a code block).\n",
                           instr.data,
                           instr.X
                    );
                    return;
                case 0xA1:
                    printf("%#06X if (key() != V%x) - Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed (usually the next instruction is a jump to skip a code block).\n",
                           instr.data,
                           instr.X
                    );
                    return;

                default:
                    goto not_an_opcode;
            }
        case 0xF:
            switch (instr.NN) {
                case 0x07:
                    printf("%#06X V%x = get_delay() - Sets VX to the value of the delay timer.\n",
                           instr.data,
                           instr.X
                    );
                    return;
                case 0x0A:
                    printf("%#06X V%x = get_key() - A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing).\n",
                           instr.data,
                           instr.X
                    );
                    return;
                case 0x15:
                    printf("%#06X delay_timer(V%x) - Sets the delay timer to VX.\n",
                           instr.data,
                           instr.X
                    );
                    return;
                case 0x18:
                    printf("%#06X sound_timer(V%x) - Sets the sound timer to VX.\n",
                           instr.data,
                           instr.X
                    );
                    return;
                case 0x1E:
                    printf("%#06X I += V%x - Adds VX to I. VF is not affected.\n",
                           instr.data,
                           instr.X
                    );
                    return;
                case 0x29:
                    printf("%#06X I = sprite_addr[V%x] - Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). Characters 0-F (in hexadecimal) are represented by a 4x5 font.\n",
                           instr.data,
                           instr.X
                    );
                    return;
                case 0x33:
                    iFX33(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 0x55:
                    iFX55(chip, instr);
                    chip->PC += sizeof(opcode_t);
                    return;
                case 0x65:
                    iFX65(chip, instr);
                    chip->PC += sizeof(opcode_t);
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