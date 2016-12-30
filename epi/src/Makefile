# Set the main compiler here. Options e.g.: 'gcc', 'g++'
CC := g++

# Special directories
SRCDIR := $(shell pwd)/common
BUILDDIR := $(shell pwd)/../build

SRCEXT := cpp
COMMON_SOURCES := $(shell find $(SRCDIR) -maxdepth 1 -type f -name '*.$(SRCEXT)')
COMMON_OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(COMMON_SOURCES:.$(SRCEXT)=.o))

# export common objects to other makefiles
export COMMON_OBJECTS

# use -ggdb for GNU debugger
CFLAGS := -g -ggdb -Wall -std=c++11

# search for libs here
#LDFLAGS += -Llib/ldns-1.6.17
# add these libs for linking
LIB := -pthread
# special include dirs to add
INC := -I../include 

all: $(COMMON_OBJECTS)
	@echo " Compiling EPI problems..."
	make -C bit-count
	make -C closest-int
	make -C look-n-say

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo " Compiling common code..."
	make -C common

clean:
	@echo " Cleaning..."; 
	make -C bit-count clean
	make -C closest-int clean
	make -C look-n-say clean
	$(RM) -r *~

.PHONY: clean
