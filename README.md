# chip8

<img width="340" height="160" alt="Schermata del 2025-11-09 17-12-16" src="https://github.com/user-attachments/assets/8d253740-360f-45f9-9363-52fb7b56d746" />

this emulator require `sdl3`

on archlinux:
`sudo pacman -S sdl3`

```bash
git clone https://github.com/Manu-sh/chip8
cmake -B build
make -j`nproc --all` -C build
```

launch the emulator

```bash
./build/main /path/to/your/rom.ch8
```

#### useful links
- https://en.wikipedia.org/wiki/CHIP-8
- https://github.com/mattmikolay/chip-8/wiki/Mastering-CHIP%E2%80%908 (best reference)
- https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Technical-Reference
- https://github.com/Timendus/chip8-test-suite (very useful test suite)
- https://github.com/corax89/chip8-test-rom (other useful tests)
- https://chip-8.vercel.app/ (an online emulator i used to test the behavior)
- https://github.com/queso-fuego/chip8_emulator_c a guy with nice project
- http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

#### other
- https://project-awesome.org/tobiasvl/awesome-chip-8#testing
- https://archive.org/details/bitsavers_rcacosmacCManual1978_6956559/page/n49/mode/2up

###### Copyright © 2025, [Manu-sh](https://github.com/Manu-sh), s3gmentationfault@gmail.com. Released under the [MIT license](LICENSE).
