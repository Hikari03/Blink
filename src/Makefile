# stolen from https://gist.github.com/freelsn/fe5f160cf564e209dc9c5673296ee813

# - inc/
#     - *.h
# - src/
#     - *.c
#     - *.cpp
# - obj/
#     - *.o
# - main

TARGET := blink
SOURCES := $(wildcard *.c *.cpp)
OBJECTS := $(patsubst src%,obj%, $(patsubst %.c,%.o, $(patsubst %.cpp,%.o,$(SOURCES))))

INCLUDE := -I.
LIBPATH :=
LIBS := -lncursesw

STATICFLAG :=
FLAGS := -Wall -Wextra -Wpedantic -Werror -O2
CXXFLAGS := $(FLAGS) -std=c++23 $(STATICFLAG)

CC := gcc
CXX := g++

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) -o $(TARGET) $(LIBPATH) $(LIBS)

static:
	echo "Building static binary..."
	@$(MAKE) all STATICFLAG=-static

%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

.PHONY: clean_o clean_e static all

clean_o:
	rm -rf *.o

clean_e:
	rm -f $(TARGET)
