INC += -I.

SRC += Si468x.c
SRC += Si468x_ext.c
SRC += Si468x_platform.c
SRC += Si468x_shell_cmds.c
SRC += main.c
SRC += utils.c
		
FW += ./external_firmwares/dab_radio_5_0_5.bin
FW += ./external_firmwares/fmhd_radio_5_0_4.bin
FW += ./external_firmwares/rom00_patch.016.bin

OBJ = $(SRC:.c=.o)
FW_OBJ = $(FW:.bin=.o)

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
	
$(FW_OBJ) : %.o : %.bin
	@echo "Processing"   $<
#	@$(OBJCOPY)	-I binary -O elf64-littleaarch64 $< $@
	$(LD) -r -b binary -o $@ $< 
