CC := gcc
CFLAGS := -O2 -g -std=c11 -Wall
LDFLAGS := -lm 
TARGET = main


SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS))


t: 
	gcc ./src/scanner.c ./src/token.c ./tests/test.c ./tests/test_common.c ./tests/scanner_tests.c ./src/files.c ./src/common.c -o test

g: $(TARGET)
$(TARGET): $(OBJS)
	$(CC)  -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/ $(TARGET)

.PHONY: all clean

len:
	find . -name '*.c' | xargs wc -l
