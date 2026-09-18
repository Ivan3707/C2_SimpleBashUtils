CC = gcc

CFLAGS = -std=c11 -Wall -Wextra -Werror
PCRE2_CFLAGS = -I/ucrt64/include
PCRE2_LIBS = -L/ucrt64/lib -lpcre2-8

TARGET = s21_grep
SRC = s21_grep.c

.PHONY: all clean rebuild

all: $(TARGET)

$(TARGET): $(SRC) s21_grep.h
	$(CC) $(CFLAGS) $(PCRE2_CFLAGS) $(SRC) -o $(TARGET) $(PCRE2_LIBS)

clean:
	rm -f $(TARGET) *.o

rebuild: clean all