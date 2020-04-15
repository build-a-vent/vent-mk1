OBJS = udpbcaster.o

CFLAGS = -c
CC = gcc

PROS = udpbcaster

all: $(PROS)

.c.o:
	$(CC) $(CFLAGS) $<

b_client: $(OBJS_CLIENT)
	$(CC) -o $@ $^ $(LIBS_CLIENT)

b_server: $(OBJS_SERVER)
	$(CC) -o $@ $^ $(LIBS_SERVER)

clean:
	rm -rf $(PROS) $(OBJS_CLIENT) $(OBJS_SERVER)

