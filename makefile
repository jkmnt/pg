# Target file name (without extension).

#TARGET := pg_test
TARGET := pg_server

# Compiler optimization
OPT := 2

# Build type
CFG := release

# Defines to pass to compiler
CDEFS :=

# Print commands, executed during build
#QUIET:= false
QUIET:= true

# Build directory - place for intermediate objects, listings etc.
BUILD_DIR := $(CFG)/build
# Target directory. Final binary will be placed there.
TARG_DIR := $(CFG)/out

# Extra libraries to link against
LIBS := wsock32

# Directories with include files
CINCS := . fonts drivers widgets test

SRC :=

# List C source files here
SRC += pg.c
SRC += pg_theme.c
SRC += pg_tagitems.c
SRC += pg_primitives.c
SRC += pg_mempool.c
SRC += pg_res.c

SRC += widgets/pg_abitmap.c
SRC += widgets/pg_bitmap.c
SRC += widgets/pg_box.c
SRC += widgets/pg_group.c
SRC += widgets/pg_listbox.c
SRC += widgets/pg_matrix.c
SRC += widgets/pg_pbar.c
SRC += widgets/pg_scroll.c
SRC += widgets/pg_txtedit.c
SRC += widgets/pg_txtlabel.c
SRC += widgets/pg_txtview.c

SRC += drivers/drv_fb_mono_32.c

ifeq ($(TARGET), pg_test)
# For visual tests
  SRC += test/socket.c
  SRC += test/test.c
else
## For driver tests
  SRC += test/test_bw_driver.c
endif

#---------------- Common compiler options ----------------
# General
COMMON_CFLAGS = -std=gnu99 -gdwarf-2 -pipe -O$(OPT) -g
COMMON_CFLAGS += -Wa,-adhlmns=$(@:.o=.lst)
COMMON_CFLAGS += -Wall -Wcast-align -Wimplicit -Wstrict-prototypes -Wuninitialized
COMMON_CFLAGS += -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Winline
# Various
COMMON_CFLAGS += -fverbose-asm
# Auto-generation of dependencies
COMMON_CFLAGS += -MMD -MP
#---------------- PC compiler options ----------------
# CFLAGS may be used for overriding some options
ALL_CFLAGS = $(COMMON_CFLAGS) $(CDEFS) $(addprefix -I,$(CINCS)) $(CFLAGS)

#---------------- Linker options ----------------
LDFLAGS := -mwindows,-Wl,-Map=$(TARG_DIR)/$(TARGET).map,--cref,--gc-sections

#---------------- Commands ----------------
ifeq ($(QUIET), true)
  CMDP = @
endif

SHELL := sh
WINSHELL := cmd

CC := gcc
OBJCOPY := objcopy
OBJDUMP := objdump
SIZE := size
NM := nm
STRIP := strip
GREP := grep
TR := tr
PY := python
REMOVE := rm -f
COPY := cp
MD := mkdir -p

#---------------- Objects ----------------
# Define all object files.
OBJS := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
# Dependencies
DEPS = $(OBJS:.o=.d)

#---------------- Directories ----------------
ALL_DIRS := $(addprefix $(BUILD_DIR)/, $(sort $(dir $(SRC))))
ALL_DIRS += $(TARG_DIR) $(BUILD_DIR)
ALL_DIRS +=
# Remove trailing slashes
ALL_DIRS := $(subst / , , $(ALL_DIRS))

#---------------- Chain rules ----------------
# Default target
all: begin build end
#gccversion

# Clean project
clean: begin clean_list end

# Build target
build: $(ALL_DIRS) exe sym

exe: $(TARG_DIR)/$(TARGET).exe
sym: $(TARG_DIR)/$(TARGET).sym

#---------------- Recipes ----------------
$(ALL_DIRS):
	@echo 'Creating dir' $@
	$(CMDP) $(MD) $@

begin:
	@echo
	@echo '-------- begin --------'
	@echo 'Target directory is' $(TARG_DIR)

end:
	@echo '--------  end  --------'
	@echo

gccversion :
	$(CMDP) $(CC) --version

# Create a symbol table from ELF output file.
$(TARG_DIR)/$(TARGET).sym: $(TARG_DIR)/$(TARGET).exe
	@echo 'Creating Symbol Table' $@
	$(CMDP) $(NM) -n $< > $@

# Link: create exe output file from object files.
$(TARG_DIR)/$(TARGET).exe: $(OBJS)
	@echo 'Linking' $@
	$(CMDP) $(CC) $(LDFLAGS) --output $@ $^ $(addprefix -l,$(LIBS))
	$(CMDP) $(STRIP) $@ --strip-debug

# Compile: create object files from C source files.
$(BUILD_DIR)/%.o : %.c
	@echo 'Compiling' $<
	$(CMDP) $(CC) -c $(ALL_CFLAGS) $< -o $@

clean_list:
	@echo
	@echo 'Cleaning project'
	$(CMDP) $(REMOVE) $(addprefix $(TARG_DIR)/$(TARGET), .exe .map .sym)
	$(CMDP) $(REMOVE) $(OBJS)
	$(CMDP) $(REMOVE) $(OBJS:.o=.lst)
	$(CMDP) $(REMOVE) $(DEPS)

-include $(DEPS)

# Listing of phony targets.
.PHONY : all burn begin end gccversion build exe sym clean clean_list
