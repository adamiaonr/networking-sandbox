# Set the main compiler here. Options e.g.: 'gcc', 'g++'
CC := g++

# Special directories
SRCDIR := src
BUILDDIR := build
TARGET := pingy

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

# use -ggdb for GNU debugger
CFLAGS := -g -ggdb -Wall -std=c++11

# search for libs here
#LDFLAGS += -Llib/ldns-1.6.17
# add these libs for linking
LIB := -pthread
# special include dirs to add
INC := -Iinclude 

all: $(TARGET)
	@echo " Doing nothing..."

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	$(RM) -r $(BUILDDIR) $(TARGET) *~

.PHONY: clean
