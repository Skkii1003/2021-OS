Main: main.cpp my_print.asm
	nasm -f elf32 my_print.asm
	g++ -m32 -std=c++11 main.cpp my_print.o -o main
	rm -rf my_print.o
clean:
	rm -rf main