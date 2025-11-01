#CFLAGS=-std=c11 -O0 -pipe -Wall -Wextra -Wno-unused-function -pedantic -ftrapv -fstack-protector-all -fstack-protector-strong -fno-builtin-memcpy -fsanitize=address,undefined,null,alignment,leak,pointer-compare,pointer-subtract,unreachable -g3 -fno-omit-frame-pointer #-fno-strict-aliasing

CFLAGS=-std=c11 -O0 -pipe -Wno-implicit-function-declaration -Wall -Wextra -Wno-unused-function -ftrapv -fstack-protector-all -fstack-protector-strong -pedantic
#-fno-builtin-memcpy -fsanitize=address,undefined,null,alignment,leak,pointer-compare,pointer-subtract,unreachable -g3 -fno-omit-frame-pointer #-fno-strict-aliasing
.PHONY: all clean

all:
	$(CC) $(CFLAGS) -o main dump-chip8.c

clean:
	rm -rf main
