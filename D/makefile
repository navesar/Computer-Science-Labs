all: multi

multi: multi.o
	gcc -m32 -o multi multi.o 

multi.o: multi.s
	nasm -f elf32 -o multi.o multi.s

clean:
	rm -f multi.o multi