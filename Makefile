CXX=g++
CFLAGS=-std=c++14 -Wall -O3 $(shell pkg-config --cflags libpng freetype2)
LFLAGS=-O3 $(shell pkg-config --libs libpng freetype2)
app := fb-tetris
objects := $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

all: $(objects)
	$(CXX) $(objects) $(LFLAGS) -o $(app)
$(objects): %.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@
clean:
	-$(RM) $(app) $(objects)