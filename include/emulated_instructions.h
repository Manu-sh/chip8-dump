#pragma once
#include <chip8.h>
#include <dbg.h>
#include <bit_utility.h>
#include <stdint.h>

/*
 reg_load(Vx, &I)
 Fills from V0 to VX (including VX) with values from memory, starting at address I.

 The offset from I is increased by 1 for each value read, but I itself is left unmodified.
*/

// TODO: test me
void iFX65(chip8_t *chip, int reg_index) {

    // TODO: controllare eventuali problemi di endianess con address
    uint16_t address = take_few_bits(chip->I, 12); // take 12 bit from I

    printf("iFX65 endianess check: %s\n",
        byte_dump(&address, sizeof(address))
    );

    assert(chip->d_register + reg_index + 1 <= chip->d_register + REG_LEN);
    memcpy(chip->d_register, chip->memory + address, reg_index + 1);
}


// 0X00E0 disp_clear() - Clears the screen
void i00E0(chip8_t *chip8) {
    memset(__builtin_assume_aligned(chip8->screen, 32), 0x00, sizeof(chip8->screen)); // In Chip-8 By default, the screen is set to all black pixels.
}

// 0XA22A I = 0X22A;
void iANNN(chip8_t *chip8) {

}