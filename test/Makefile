# I know this makefile sucks but I don't care

debug:
	cc *.c ../*.c -DHM_DEBUG -g -Wall -Wextra -pedantic -o test

prod:
	cc *.c ../*.c -O3 -Wall -Wextra -pedantic -o test

test:debug
	./test

run:prod
	./test
