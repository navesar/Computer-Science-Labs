all: antiVirus
antiVirus: antiVirus.o
	gcc -g -m32 -Wall -o antiVirus antiVirus.c
antiVirus.o: antiVirus.c
	gcc -g -m32 -Wall -o antiVirus.o antiVirus.c
.PHONEY: clean
clean:
	rm -f antiVirus.o antiVirus
