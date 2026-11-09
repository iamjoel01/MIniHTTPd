CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Iinclude
SRC = src/main.c src/mime.c src/files.c
TARGET = minihttpd

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
