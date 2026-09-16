CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic

TARGET = s21_cat
SRC = s21_cat.c

.PHONY: all clean rebuild

all: $(TARGET)

$(TARGET): $(SRC) s21_cat.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

rebuild: clean all
