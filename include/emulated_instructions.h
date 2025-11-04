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

    assert(reg_index + 1 <= chip->d_register + REG_LEN);
    memcpy(chip->d_register, chip->memory + address, reg_index + 1);
}

// 0X00E0 disp_clear() - Clears the screen
void i00E0(chip8_t *chip8) {

}