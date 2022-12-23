CXXFLAGS=-I$(INCDIR) -std=c++17 -O2
LDFLAGS=-lfmt -lglfw
INCDIR=src/common
SRCDIR=src
OBJDIR=obj
BINDIR=bin
TARGETS=$(BINDIR)/01-triangle \
        $(BINDIR)/02-cube

all: $(TARGETS)

# Link object files to produce executables
$(BINDIR)/01-triangle: $(OBJDIR)/01-triangle.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/02-cube: $(OBJDIR)/02-cube.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)

# Compile main files
$(OBJDIR)/01-triangle.o: $(SRCDIR)/01-triangle/triangle.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/02-cube.o: $(SRCDIR)/02-cube/cube.cpp
	g++ -c $< -o $@ $(CXXFLAGS)

# Compile common files
$(OBJDIR)/shader.o: $(SRCDIR)/common/shader.cpp $(SRCDIR)/common/shader.h
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/utils.o: $(SRCDIR)/common/utils.cpp $(SRCDIR)/common/utils.h
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/glad.o: $(SRCDIR)/common/glad.c $(SRCDIR)/common/glad.h $(SRCDIR)/common/khrplatform.h
	g++ -c $< -o $@ $(CXXFLAGS)

# Create $(OBJDIR) and $(BINDIR) after the Makefile is parsed
$(shell mkdir -p $(OBJDIR) $(BINDIR))

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)
