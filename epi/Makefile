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

6-6-stock: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-6-stock

6-9-permutations: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-9-permutations

6-11-offline: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-11-offline

6-13-random-permutation: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-13-random-permutation

6-17-spiral: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-17-spiral

7-1-stringint: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-1-stringint

7-2-base-conversion: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-2-base-conversion

7-4-replace-remove: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-4-replace-remove

7-5-palindrome: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-5-palindrome

7-7-phone-numbers: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-7-phone-numbers

7-8-look-n-say: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-8-look-n-say

7-9-roman-numbers: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-9-roman-numbers

7-11-snakestring: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-11-snakestring

7-13-substr: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-13-substr

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
	make -C $(SRCDIR)/6-6-stock clean
	make -C $(SRCDIR)/6-9-permutations clean
	make -C $(SRCDIR)/6-11-offline clean
	make -C $(SRCDIR)/6-13-random-permutation clean
	make -C $(SRCDIR)/6-17-spiral clean
	make -C $(SRCDIR)/7-1-stringint clean
	make -C $(SRCDIR)/7-2-base-conversion clean
	make -C $(SRCDIR)/7-4-replace-remove clean
	make -C $(SRCDIR)/7-5-palindrome clean
	make -C $(SRCDIR)/7-7-phone-numbers clean
	make -C $(SRCDIR)/7-8-look-n-say clean
	make -C $(SRCDIR)/7-9-roman-numbers clean
	make -C $(SRCDIR)/7-11-snakestring clean
	make -C $(SRCDIR)/7-13-substr	 clean
	make -C $(SRCDIR)/quicksort clean
	make -C $(SRCDIR)/common clean
	$(RM) -r $(BUILDDIR) *~

.PHONY: clean
