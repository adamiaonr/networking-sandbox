# Special directories
SRCDIR := src
BUILDDIR := build

all:
	@echo " Seting up..."
	@mkdir -p $(BUILDDIR)
	@echo " Compiling sources..."
	make -C $(SRCDIR)

clean:
	@echo " Cleaning..."; 
	make -C $(SRCDIR) clean
	$(RM) -r $(BUILDDIR) *~

.PHONY: clean
