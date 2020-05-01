OBJS = parson.o
CFLAGS = 
LDFLAGS = -static
CC = gcc

TGT = ventsimulator

all: $(TGT)

$(TGT) : $(OBJS)

%.o : %.c; $(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

% :: %.c; $(CC) $(CFLAGS) $(LDFLAGS)  $< -o $@ $(OBJS)

clean:
	rm -rf $(TGT) $(OBJS)

