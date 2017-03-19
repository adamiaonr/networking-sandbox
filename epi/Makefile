# special directories
SRCDIR := $(shell pwd)/src
BUILDDIR := $(shell pwd)/build

# extension of code
SRCEXT := cpp

# common objects and sources
COMMON_SOURCES := $(shell find $(SRCDIR)/common -maxdepth 1 -type f -name '*.$(SRCEXT)')
COMMON_OBJECTS := $(patsubst $(SRCDIR)/common/%,$(BUILDDIR)/%,$(COMMON_SOURCES:.$(SRCEXT)=.o))

# export common objects to other makefiles
export COMMON_OBJECTS

# use -ggdb for GNU debugger
CFLAGS := -g -ggdb -Wall -std=c++11

# search for libs 
#LDFLAGS += -Llib/ldns-1.6.17
# add these libs for linking
LIB := -pthread
# special include dirs to add
INC := -Iinclude

# add each individual exercise to TARGETS
TARGETS := 5-0-bit-count 5-4-closest-int 5-5-multiply 5-6-divide 5-7-power 5-8-reverse 6-1-dutch 6-2-increment 6-3-multiply 6-4-advance 7-8-look-n-say quicksort

all: $(TARGETS)

5-0-bit-count: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-0-bit-count

5-4-closest-int: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-4-closest-int

5-5-multiply: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-5-multiply

5-6-divide: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-6-divide

5-7-power: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-7-power

5-8-reverse: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-8-reverse

6-1-dutch: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-1-dutch

6-2-increment: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-2-increment

6-3-multiply: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-3-multiply

6-4-advance: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-4-advance

6-5-duplicates: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-5-duplicates

7-8-look-n-say: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-8-look-n-say

quicksort: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/quicksort

$(BUILDDIR)/%.o: $(SRCDIR)/common/%.$(SRCEXT)
	@echo " compiling common code..."
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " cleaning..."; 
	make -C $(SRCDIR)/5-0-bit-count clean
	make -C $(SRCDIR)/5-4-closest-int clean
	make -C $(SRCDIR)/5-5-multiply clean
	make -C $(SRCDIR)/5-6-divide clean
	make -C $(SRCDIR)/5-7-power clean
	make -C $(SRCDIR)/5-8-reverse clean
	make -C $(SRCDIR)/6-1-dutch clean
	make -C $(SRCDIR)/6-2-increment clean
	make -C $(SRCDIR)/6-3-multiply clean
	make -C $(SRCDIR)/6-4-advance clean
	make -C $(SRCDIR)/6-5-duplicates clean
	make -C $(SRCDIR)/7-8-look-n-say clean
	make -C $(SRCDIR)/quicksort clean
	make -C $(SRCDIR)/common clean
	$(RM) -r $(BUILDDIR) *~

.PHONY: clean
