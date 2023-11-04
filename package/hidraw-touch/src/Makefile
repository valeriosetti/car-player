INC += -I.

SRC += main.c

PROJ_NAME = hidraw-touch

#LIBS +=

#############################

OBJ = $(SRC:.c=.o)

all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o *~

