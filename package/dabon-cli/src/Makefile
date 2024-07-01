INC += -I.

SRC += Si468x.c
SRC += Si468x_ext.c
SRC += Si468x_platform.c
SRC += Si468x_shell_cmds.c
SRC += main.c
SRC += utils.c

OBJ = $(SRC:.c=.o)

PROJ_NAME = dabon-cli

CFLAGS = -g
LIBRARIES = -lncurses -lgpiod

VPATH = $(dir $(SRC)) \
		$(dir $(FW))

OBJCOPY = $(dir $(CC))aarch64-linux-gnu-objcopy

####################################################################

all: $(PROJ_NAME)
	
$(PROJ_NAME) : $(FW_OBJ) $(OBJ)
	@echo "Assembling objects"
	@$(CC) $^ $(LIBRARIES) $(LINKER_FLAGS) -o $@ 

$(OBJ) : %.o : %.c
	@echo "Compiling"   $<
	@$(CC) -c $< $(CFLAGS) $(INC) -o $@
