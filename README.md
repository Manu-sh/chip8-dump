# chip8
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

###### Copyright © 2025, [Manu-sh](https://github.com/Manu-sh), s3gmentationfault@gmail.com. Released under the [MIT license](LICENSE).
