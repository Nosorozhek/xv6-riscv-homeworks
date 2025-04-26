CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2

FIFO_FILE = fifo
DEFINES = -DFIFO_FILE="\"$(FIFO_FILE)\""

TARGET = echoserver
SRC = echoserver.c sighandler.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^

clean:
	rm -f $(TARGET)
	rm -f $(FIFO_FILE)
