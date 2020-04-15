OBJS = udpbcaster.o

CFLAGS = -static
CC = gcc

PROS = udpbcaster

all: $(PROS)

.c:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(PROS)

