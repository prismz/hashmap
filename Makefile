all:
	cc *.c -g -Wall -Wextra -pedantic -o main

run:all
	./main
