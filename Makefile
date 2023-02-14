CXXFLAGS=-I$(INCDIR) -std=c++17 -O2
LDFLAGS=-lfmt -lglfw
INCDIR=src/common
SRCDIR=src
OBJDIR=obj
BINDIR=bin
TARGETS=$(BINDIR)/01-triangle \
        $(BINDIR)/02-triangle-interleaved \
        $(BINDIR)/03-triangle-dsa \
        $(BINDIR)/04-triangle-transforms \
        $(BINDIR)/05-rectangle-dsa \
        $(BINDIR)/06-cube \
        $(BINDIR)/07-tumbling-cube \
        $(BINDIR)/08-cubes-instancing \
        $(BINDIR)/09-circle \
        $(BINDIR)/10-pentagon-web \
        $(BINDIR)/11-pyramid \
        $(BINDIR)/12-google-photos-logo

all: $(TARGETS)

# Link object files to produce executables
$(BINDIR)/01-triangle: $(OBJDIR)/01-triangle.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/02-triangle-interleaved: $(OBJDIR)/02-triangle-interleaved.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/03-triangle-dsa: $(OBJDIR)/03-triangle-dsa.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/04-triangle-transforms: $(OBJDIR)/04-triangle-transforms.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/05-rectangle-dsa: $(OBJDIR)/05-rectangle-dsa.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/06-cube: $(OBJDIR)/06-cube.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/07-tumbling-cube: $(OBJDIR)/07-tumbling-cube.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/08-cubes-instancing: $(OBJDIR)/08-cubes-instancing.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/09-circle: $(OBJDIR)/09-circle.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/10-pentagon-web: $(OBJDIR)/10-pentagon-web.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/11-pyramid: $(OBJDIR)/11-pyramid.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)
$(BINDIR)/12-google-photos-logo: $(OBJDIR)/12-google-photos-logo.o $(OBJDIR)/shader.o $(OBJDIR)/utils.o $(OBJDIR)/glad.o
	g++ $^ -o $@ $(LDFLAGS)

# Compile main files
$(OBJDIR)/01-triangle.o: $(SRCDIR)/01-triangle/triangle.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/02-triangle-interleaved.o: $(SRCDIR)/02-triangle-interleaved/triangle-interleaved.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/03-triangle-dsa.o: $(SRCDIR)/03-triangle-dsa/triangle-dsa.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/04-triangle-transforms.o: $(SRCDIR)/04-triangle-transforms/triangle-transforms.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/05-rectangle-dsa.o: $(SRCDIR)/05-rectangle-dsa/rectangle-dsa.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/06-cube.o: $(SRCDIR)/06-cube/cube.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/07-tumbling-cube.o: $(SRCDIR)/07-tumbling-cube/tumbling-cube.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/08-cubes-instancing.o: $(SRCDIR)/08-cubes-instancing/cubes-instancing.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/09-circle.o: $(SRCDIR)/09-circle/circle.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/10-pentagon-web.o: $(SRCDIR)/10-pentagon-web/pentagon-web.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/11-pyramid.o: $(SRCDIR)/11-pyramid/pyramid.cpp
	g++ -c $< -o $@ $(CXXFLAGS)
$(OBJDIR)/12-google-photos-logo.o: $(SRCDIR)/12-google-photos-logo/google-photos-logo.cpp
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
