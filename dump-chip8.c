#define _DEFAULT_SOURCE
#include <endian.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
            uint16_t N    : 4;
            uint16_t Y    : 4;
            uint16_t X    : 4;
            uint16_t type : 4;
        #elif defined(__BIG_ENDIAN_BITFIELD)
            uint16_t type : 4;
            uint16_t X    : 4;
            uint16_t Y    : 4;
            uint16_t N    : 4;
        #else
            #error "ooops"
        #endif
	};

    struct {
        #ifdef __LITTLE_ENDIAN_BITFIELD
            uint16_t NNN : 12;
            uint16_t     : 4;
        #elif defined(__BIG_ENDIAN_BITFIELD)
            uint16_t     : 4;
            uint16_t NNN : 12;
        #endif
    };

    struct {
        #ifdef __LITTLE_ENDIAN_BITFIELD
            uint16_t NN : 8;
            uint16_t    : 4;
            uint16_t    : 4;
        #elif defined(__BIG_ENDIAN_BITFIELD)
            uint16_t    : 4;
            uint16_t    : 4;
            uint16_t NN : 8;
        #endif
    };

} opcode_t;


#define X(_INSTR_) (nibble_slice(_INSTR_, 1, 2))
#define Y(_INSTR_) (nibble_slice(_INSTR_, 2, 3))

#define NN(_INSTR_) (nibble_slice(_INSTR_, 2, 4))
#define NNN(_INSTR_) (nibble_slice(_INSTR_, 1, 4))
#define N(_INSTR_) (nibble_slice(_INSTR_, 3, 4))

// TODO:
//  0XA22A I = 0X22A
//  ADDR = 0X22A
//  I = be16toh( two_byte[ADDR - 0x200] )
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
            assert(NNN(instr.data) == instr.NNN);
            printf("%#06X call( %#03X ); - Calls machine code routine at address NNN.\n",
               instr.data,
               instr.NNN
            );
            return;
        case 1:
            assert(NNN(instr.data) == instr.NNN);
            printf("%#06X goto %#03X; - Jumps to address NNN.\n",
               instr.data,
               instr.NNN
            );
            return;
        case 2:
            assert(NNN(instr.data) == instr.NNN);
            printf("%#06X *(%#03X)() - Calls subroutine at NNN.\n",
               instr.data,
               instr.NNN
            );
            return;
        case 3:
            assert(X(instr.data) == instr.X);
            assert(NN(instr.data) == instr.NN);
            printf("%#06X if (V%x == %#02x) - Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block).\n",
               instr.data,
               instr.X,
               instr.NN
            );
            return;
        case 4:
            assert(X(instr.data) == instr.X);
            assert(NN(instr.data) == instr.NN);
            printf("%#06X if (V%x != %#02x) - Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).\n",
                instr.data,
               instr.X,
               instr.NN
            );
            return;
        case 5:
            assert(X(instr.data) == instr.X);
            assert(Y(instr.data) == instr.Y);
            printf("%#06X if (V%x == V%x) - Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).\n",
                instr.data,
               instr.X,
               instr.NN
            );
            return;
        case 6:
            assert(X(instr.data) == instr.X);
            assert(NN(instr.data) == instr.NN);
            printf("%#06X V%x = %#02X - Sets VX to NN.\n",
               instr.data,
               instr.X,
               instr.NN
            );
            return;
        case 7:
            assert(X(instr.data) == instr.X);
            assert(NN(instr.data) == instr.NN);
            printf("%#06X V%x += %#02X - Adds NN to VX (carry flag is not changed).\n",
               instr.data,
               instr.X,
               instr.NN
            );
            return;

        case 8:
            assert(N(instr.data) == instr.N);
            // check last nibble
            switch (instr.N) {
                case 0:
                    assert(X(instr.data) == instr.X);
                    assert(Y(instr.data) == instr.Y);
                    printf("%#06X V%x = V%x - Sets VX to the value of VY.\n",
                        instr.data,
                        instr.X,
                        instr.Y
                    );
                    return;
                case 1:
                    assert(X(instr.data) == instr.X);
                    assert(Y(instr.data) == instr.Y);
                    printf("%#06X V%x |= V%x - Sets VX to VX or VY. (bitwise OR operation).\n",
                       instr.data,
                       instr.X,
                       instr.Y
                    );
                    return;
                case 2:
                    assert(X(instr.data) == instr.X);
                    assert(Y(instr.data) == instr.Y);
                    printf("%#06X V%x &= V%x - Sets VX to VX and VY. (bitwise AND operation).\n",
                        instr.data,
                        instr.X,
                        instr.Y
                    );
                    return;
                case 3:
                    assert(X(instr.data) == instr.X);
                    assert(Y(instr.data) == instr.Y);
                    printf("%#06X V%x ^= V%x - Sets VX to VX xor VY.\n",
                       instr.data,
                       instr.X,
                       instr.Y
                    );
                    return;
                case 4:
                    assert(X(instr.data) == instr.X);
                    assert(Y(instr.data) == instr.Y);
                    printf("%#06X V%x += V%x - Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.\n",
                       instr.data,
                       instr.X,
                       instr.Y
                    );
                    return;
                case 5:
                    assert(X(instr.data) == instr.X);
                    assert(Y(instr.data) == instr.Y);
                    printf("%#06X V%x -= V%x - VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not).\n",
                       instr.data,
                       instr.X,
                       instr.Y
                    );
                    return;
                case 6:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X V%x >>= 1 - Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.\n",
                        instr.data,
                        X(instr.data)
                    );
                    return;
                case 7: {
                    assert(X(instr.data) == instr.X);
                    assert(Y(instr.data) == instr.Y);
                    uint8_t vx = instr.X;
                     printf(
                        "%#06X V%x = V%x - V%x - Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX).\n",
                        instr.data,
                        vx,
                        instr.Y, // Vy
                        vx
                    ); return;
                }
                case 0xE:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X V%x <<= 1 - Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.\n",
                        instr.data,
                        instr.X
                    );
                    return;

                default:
                    goto not_an_opcode;
            }

        case 9:
            assert(X(instr.data) == instr.X);
            assert(Y(instr.data) == instr.Y);
            printf("%#06X if (V%x != V%x) - Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block).\n",
                instr.data,
                instr.X,
                instr.Y
            );
            return;

        case 0xA:
            assert(NNN(instr.data) == instr.NNN);
            printf("%#06X I = %#03X; - Sets I to the address NNN.\n",
                instr.data,
                instr.NNN
            );
            return;
        case 0xB:
            assert(NNN(instr.data) == instr.NNN);
            printf("%#06X PC = V0 + %#03X - Jumps to the address NNN plus V0.\n",
                instr.data,
                instr.NNN
            );
            return;

        case 0xC:
            assert(X(instr.data) == instr.X);
            assert(NN(instr.data) == instr.NN);
            printf("%#06X V%x = rand() & %#02X; - Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.\n",
                instr.data,
                instr.X,
                instr.NN
            );
            return;

        case 0xD:
            assert(X(instr.data) == instr.X);
            assert(Y(instr.data) == instr.Y);
            assert(N(instr.data) == instr.N);
            printf("%#06X draw(V%x, V%x, %x) - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.\n",
                instr.data,
                instr.X,
                instr.Y,
                instr.N
            );
            return;

        case 0xE:
            switch (NN(instr.data)) {
                case 0x9E:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X if (key() == V%x) - Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed (usually the next instruction is a jump to skip a code block).\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0xA1:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X if (key() != V%x) - Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed (usually the next instruction is a jump to skip a code block).\n",
                        instr.data,
                        instr.X
                    );
                    return;

                default:
                    goto not_an_opcode;
            }
        case 0xF:
            assert(NN(instr.data) == instr.NN);
            switch (instr.NN) {
                case 0x07:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X V%x = get_delay() - Sets VX to the value of the delay timer.\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x0A:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X V%x = get_key() - A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing).\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x15:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X delay_timer(V%x) - Sets the delay timer to VX.\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x18:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X sound_timer(V%x) - Sets the sound timer to VX.\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x1E:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X I += V%x - Adds VX to I. VF is not affected.\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x29:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X I = sprite_addr[V%x] - Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). Characters 0-F (in hexadecimal) are represented by a 4x5 font.\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x33:
                    assert(X(instr.data) == instr.X);
                    printf(
                            "%#06X set_BCD(V%x) "
                        "*(I+0) = BCD(3);"
                        "*(I+1) = BCD(2);"
                        "*(I+2) = BCD(1); - Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x55:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X reg_dump(V%x, &I)  - Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.\n",
                        instr.data,
                        instr.X
                    );
                    return;
                case 0x65:
                    assert(X(instr.data) == instr.X);
                    printf("%#06X reg_load(V%x, &I) - Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified.\n",
                        instr.data,
                        instr.X
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
