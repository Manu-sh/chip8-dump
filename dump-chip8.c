#define _DEFAULT_SOURCE
#include <endian.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <asm/byteorder.h>

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

typedef union  __attribute__((__packed__)) {

    uint16_t data;
    uint8_t byte[sizeof(uint16_t)];

	struct {
        #ifdef __LITTLE_ENDIAN_BITFIELD
            uint16_t : 4;
            uint16_t : 4;
            uint16_t : 4;
            uint16_t type : 4;
        #elif defined(__BIG_ENDIAN_BITFIELD)
            uint16_t type : 4;
            uint16_t : 4;
            uint16_t : 4;
            uint16_t : 4;
        #else
            #error "ooops"
        #endif
	};

} opcode_t;


#define X(_INSTR_) (nibble_slice(_INSTR_, 1, 2))
#define Y(_INSTR_) (nibble_slice(_INSTR_, 2, 3))

#define NN(_INSTR_) (nibble_slice(_INSTR_, 2, 4))
#define NNN(_INSTR_) (nibble_slice(_INSTR_, 1, 4))
#define N(_INSTR_) (nibble_slice(_INSTR_, 3, 4))


void dump_instruction(opcode_t instr) {

    static size_t instruction_counter;
    assert(instr.type == nibble_slice(instr.data, 0, 1));

    printf("[%zu] ", ++instruction_counter);

    if (instr.data == 0x00E0) {
        printf("%#06X disp_clear() - Clears the screen\n", instr.data);
        return;
    } else if (instr.data == 0x00EE) {
        printf("%#06X return; - Returns from a subroutine.\n", instr.data);
        return;
    }

    switch (instr.type) {
        case 0:
            printf("%#06X call( %#03X ); - Calls machine code routine at address NNN.\n",
               instr.data,
               NNN(instr.data)
            );
            return;
        case 1:
            printf("%#06X goto %#03X; - Jumps to address NNN.\n",
               instr.data,
               NNN(instr.data)
            );
            return;
        case 2:
            printf("%#06X *(%#03X)() - Calls subroutine at NNN.\n",
               instr.data,
               NNN(instr.data)
            );
            return;
        case 3:
            printf("%#06X if (V%x == %#02x) - Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block).\n",
               instr.data,
               X(instr.data),
               NN(instr.data)
            );
            return;
        case 4:
            printf("%#06X if (V%x != %#02x) - Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).\n",
                instr.data,
                X(instr.data),
                NN(instr.data)
            );
            return;
        case 5:
            printf("%#06X if (V%x == V%x) - Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).\n",
                instr.data,
                X(instr.data),
                Y(instr.data)
            );
            return;
        case 6:
            printf("%#06X V%x = %X - Sets VX to NN.\n",
               instr.data,
               X(instr.data),
               NN(instr.data)
            );
            return;
        case 7:
            printf("%#06X V%x += %X - Adds NN to VX (carry flag is not changed).\n",
               instr.data,
               X(instr.data),
               NN(instr.data)
            );
            return;

        case 8:

            // check last nibble
            switch (N(instr.data)) {
                case 0:
                    printf("%#06X V%x = V%x - Sets VX to the value of VY.\n",
                        instr.data,
                        X(instr.data),
                        Y(instr.data)
                    );
                    return;
                case 1:
                    printf("%#06X V%x |= V%x - Sets VX to VX or VY. (bitwise OR operation).\n",
                        instr.data,
                        X(instr.data),
                        Y(instr.data)
                    );
                    return;
                case 2:
                    printf("%#06X V%x &= V%x - Sets VX to VX and VY. (bitwise AND operation).\n",
                        instr.data,
                        X(instr.data),
                        Y(instr.data)
                    );
                    return;
                case 3:
                    printf("%#06X V%x ^= V%x - Sets VX to VX xor VY.\n",
                        instr.data,
                        X(instr.data),
                        Y(instr.data)
                    );
                    return;
                case 4:
                    printf("%#06X V%x += V%x - Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.\n",
                        instr.data,
                        X(instr.data),
                        Y(instr.data)
                    );
                    return;
                case 5:
                    printf("%#06X V%x -= V%x - VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not).\n",
                        instr.data,
                        X(instr.data),
                        Y(instr.data)
                    );
                    return;
                case 6:
                    printf("%#06X V%x >>= 1 - Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 7: {
                    uint8_t vx = X(instr.data); return;
                     printf(
                            "%#06X V%x = V%x - V%x - Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX).\n",
                            instr.data,
                            vx,
                            Y(instr.data), // Vy
                            vx
                    ); return;
                }
                case 0xE:
                    printf("%#06X V%x <<= 1 - Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;

                default:
                    goto not_an_opcode;
            }

        case 9:
            printf("%#06X if (V%x != V%x) - Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block).\n",
                instr.data,
                X(instr.data),
                Y(instr.data)
            );
            return;

        case 0xA:
            printf("%#06X I = %#03X; - Sets I to the address NNN.\n",
                instr.data,
                NNN(instr.data)
            );
            return;
        case 0xB:
            printf("%#06X PC = V0 + %#03X - Jumps to the address NNN plus V0.\n",
                instr.data,
                NNN(instr.data)
            );
            return;

        case 0xC:
            printf("%#06X V%x = rand() & %#02X; - Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.\n",
                instr.data,
                X(instr.data),
                NN(instr.data)
            );
            return;

        case 0xD:
            printf("%#06X draw(V%x, V%x, %x) - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.\n",
                instr.data,
                X(instr.data),
                Y(instr.data),
                N(instr.data)
            );
            return;

        case 0xE:
            switch (NN(instr.data)) {
                case 0x9E:
                    printf("%#06X if (key() == V%x) - Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed (usually the next instruction is a jump to skip a code block).\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0xA1:
                    printf("%#06X if (key() != V%x) - Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed (usually the next instruction is a jump to skip a code block).\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;

                default:
                    goto not_an_opcode;
            }
        case 0xF:
            switch (NN(instr.data)) {
                case 0x07:
                    printf("%#06X V%x = get_delay() - Sets VX to the value of the delay timer.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x0A:
                    printf("%#06X V%x = get_key() - A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing).\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x15:
                    printf("%#06X delay_timer(V%x) - Sets the delay timer to VX.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x18:
                    printf("%#06X sound_timer(V%x) - Sets the sound timer to VX.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x1E:
                    printf("%#06X I += V%x - Adds VX to I. VF is not affected.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x29:
                    printf("%#06X I = sprite_addr[V%x] - Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). Characters 0-F (in hexadecimal) are represented by a 4x5 font.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x33:
                    printf(
                            "%#06X set_BCD(V%x) "
                        "*(I+0) = BCD(3);"
                        "*(I+1) = BCD(2);"
                        "*(I+2) = BCD(1); - Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x55:
                    printf("%#06X reg_dump(V%x, &I)  - Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 0x65:
                    printf("%#06X reg_load(V%x, &I) - Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified.\n",
                        instr.data,
                        X(instr.data)
                    );
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
    FILE *file = fopen(argv[1], "rb");

    for (opcode_t instr; fread(&instr.data, sizeof(uint16_t), 1, file) == 1; ) {
        instr.data = be16toh(instr.data); // for my mental health
        dump_instruction(instr);
    }

    if (ferror(file))
        perror("fread()");

    fclose(file);
}