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
	make -C 5-0-bit-count
	make -C 5-4-closest-int
	make -C 5-5-multiply
	make -C 7-8-look-n-say

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo " Compiling common code..."
	make -C common

clean:
	@echo " Cleaning..."; 
	make -C 5-0-bit-count clean
	make -C 5-4-closest-int clean
	make -C 5-5-multiply clean
	make -C 7-8-look-n-say clean
	$(RM) -r *~

.PHONY: clean
