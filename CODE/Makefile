CC = gcc
CFLAGS = -Wall -std=c99
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS =
else
    LDFLAGS = -static-libgcc
endif

TARGET = memory_allocator
SRC = static_memory_allocation.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o