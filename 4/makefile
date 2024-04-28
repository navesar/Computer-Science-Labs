all :hexeditplus task4

hexeditplus: task1.o
	gcc -m32 -g -Wall -o hexeditplus task1.o
	
task1.o: task1.c
	gcc -m32 -g -Wall -c -o task1.o task1.c

task4: task4.o
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -o task4 task4.o

task4.o: task4.c
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -c -o task4.o task4.c

.PHONY: clean

clean:
	rm hexeditplus task1.o task4 task4.o