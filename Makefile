CC=gcc
OPTIONS=-std=gnu99 -Wall -Wextra -Werror -pedantic -pthread 
MAIN_FILE=proj2

make: $(MAIN_FILE).c
	$(CC) $(OPTIONS) $(MAIN_FILE).c -o $(MAIN_FILE)

zip:
	 zip proj2.zip *.c *.h Makefile

