all: loader
loader: task2.o startup.o start.o
	ld -o loader task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2
task2.o: task2.c
	gcc -m32 -c task2.c -o task2.o
.PHONY: clean
clean:
	rm task2.o loader
