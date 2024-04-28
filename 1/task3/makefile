all: menu_map
menu_map: menu_map.o
	gcc -g -m32 -Wall -o menu_map menu_map.o -lm
menu_map.o: menu_map.c
	gcc -m32 -g -Wall -c -o menu_map.o menu_map.c 
.PHONY: clean
clean:
	rm -f *.o menu_map