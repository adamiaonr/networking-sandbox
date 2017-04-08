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
TARGETS := 5-00-bit-count 5-04-closest-int 5-05-multiply 5-06-divide 5-07-power 5-08-reverse\
6-01-dutch 6-02-increment 6-03-multiply 6-04-advance 6-05-duplicates 6-06-stock 6-09-permutations 6-11-offline 6-13-random-permutation 6-17-spiral\
7-01-stringint 7-02-base-conversion 7-04-replace-remove 7-05-palindrome 7-07-phone-numbers 7-08-look-n-say 7-09-roman-numbers 7-11-snakestring 7-13-substr\
8-01-merge-lists 8-02-reverse-sublist 8-03-cyclicity 8-04-overlap 8-05-overlap-cycle 8-06-delete-node\
quicksort

all: $(TARGETS)

5-00-bit-count: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-00-bit-count

5-04-closest-int: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-04-closest-int

5-05-multiply: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-05-multiply

5-06-divide: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-06-divide

5-07-power: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-07-power

5-08-reverse: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/5-08-reverse

6-01-dutch: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-01-dutch

6-02-increment: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-02-increment

6-03-multiply: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-03-multiply

6-04-advance: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-04-advance

6-05-duplicates: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-05-duplicates

6-06-stock: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-06-stock

6-09-permutations: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-09-permutations

6-11-offline: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-11-offline

6-13-random-permutation: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-13-random-permutation

6-17-spiral: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/6-17-spiral

7-01-stringint: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-01-stringint

7-02-base-conversion: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-02-base-conversion

7-04-replace-remove: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-04-replace-remove

7-05-palindrome: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-05-palindrome

7-07-phone-numbers: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-07-phone-numbers

7-08-look-n-say: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-08-look-n-say

7-09-roman-numbers: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-09-roman-numbers

7-11-snakestring: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-11-snakestring

7-13-substr: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/7-13-substr

8-01-merge-lists: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/8-01-merge-lists

8-02-reverse-sublist: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/8-02-reverse-sublist

8-03-cyclicity: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/8-03-cyclicity

8-04-overlap: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/8-04-overlap

8-05-overlap-cycle: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/8-05-overlap-cycle

8-06-delete-node: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/8-06-delete-node

quicksort: $(COMMON_OBJECTS)
	make -C $(SRCDIR)/quicksort

$(BUILDDIR)/%.o: $(SRCDIR)/common/%.$(SRCEXT)
	@echo " compiling common code..."
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " cleaning..."; 
	make -C $(SRCDIR)/5-00-bit-count clean
	make -C $(SRCDIR)/5-04-closest-int clean
	make -C $(SRCDIR)/5-05-multiply clean
	make -C $(SRCDIR)/5-06-divide clean
	make -C $(SRCDIR)/5-07-power clean
	make -C $(SRCDIR)/5-08-reverse clean
	make -C $(SRCDIR)/6-01-dutch clean
	make -C $(SRCDIR)/6-02-increment clean
	make -C $(SRCDIR)/6-03-multiply clean
	make -C $(SRCDIR)/6-04-advance clean
	make -C $(SRCDIR)/6-05-duplicates clean
	make -C $(SRCDIR)/6-06-stock clean
	make -C $(SRCDIR)/6-09-permutations clean
	make -C $(SRCDIR)/6-11-offline clean
	make -C $(SRCDIR)/6-13-random-permutation clean
	make -C $(SRCDIR)/6-17-spiral clean
	make -C $(SRCDIR)/7-01-stringint clean
	make -C $(SRCDIR)/7-02-base-conversion clean
	make -C $(SRCDIR)/7-04-replace-remove clean
	make -C $(SRCDIR)/7-05-palindrome clean
	make -C $(SRCDIR)/7-07-phone-numbers clean
	make -C $(SRCDIR)/7-08-look-n-say clean
	make -C $(SRCDIR)/7-09-roman-numbers clean
	make -C $(SRCDIR)/7-11-snakestring clean
	make -C $(SRCDIR)/7-13-substr clean
	make -C $(SRCDIR)/8-01-merge-lists clean
	make -C $(SRCDIR)/8-02-reverse-sublist clean
	make -C $(SRCDIR)/8-03-cyclicity clean
	make -C $(SRCDIR)/8-04-overlap clean
	make -C $(SRCDIR)/8-05-overlap-cycle clean
	make -C $(SRCDIR)/8-06-delete-node clean
	make -C $(SRCDIR)/quicksort clean
	make -C $(SRCDIR)/common clean
	$(RM) -r $(BUILDDIR) *~

.PHONY: clean
