#!/bin/bash
xxd -i maze_char.bin > ../src/maze_char.c
xxd -i maze_colors.bin > ../src/maze_colors.c
xxd -i title_char.bin > ../src/title_char.c
xxd -i title_colors.bin > ../src/title_colors.c
xxd -i maze_tiles.chr > ../src/maze_tiles.c
xxd -i pacman_sprites.spr > ../src/pacman_sprites.c
