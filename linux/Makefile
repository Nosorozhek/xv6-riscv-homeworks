CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2

TARGET = inode
TARGET_DEBUG = inoded
SRC = main.c ext2.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(SRC)
	$(CC) $(CFLAGS) -DDEBUG -o $@ $^

clean:
	rm -f $(TARGET)
	rm -f $(TARGET_DEBUG)