all: task2

start.o: start.s
	nasm -f elf32 start.s -o start.o

util.o: util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

task2: start.o task2.o util.o 
	gcc -m32 -nostartfiles start.o task2.o util.o -o task2

task2.o: main.c
	gcc -m32 -c main.c -o task2.o


clean:
	rm -f start.o util.o task2 task2.o