CC = gcc
CFLAGS = -Wall -g -pthread

TARGET = app
SRCS = main.c ring_buffer.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c ring_buffer.h
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean test

clean:
	rm -f $(OBJS) $(TARGET) log.txt

test: $(TARGET)
	./$(TARGET)
