# Generic Makefile

CC= gcc

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

EXEC = rnse

CC_OPTS = -lm -fopenmp -lhdf5

# enable debug mode
ifeq ($(debug), 1)
	CC_OPTIMIZE = -O0 -g -Wall -DDEBUG -std=c99
else
	CC_OPTIMIZE = -O2 -std=c99
endif

# Require all object files and then link
all: $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(CC_OPTS) $(CC_OPTIMIZE)

# Just compile every .c file
%.o: %.c
	$(CC) -c $< $(CC_OPTIMIZE)

clean:
	rm $(EXEC) *.o
