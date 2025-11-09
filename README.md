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

###### Copyright © 2025, [Manu-sh](https://github.com/Manu-sh), s3gmentationfault@gmail.com. Released under the [MIT license](LICENSE).
