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
LIBS :=-lncursesw

ifeq ($(shell ldconfig -p | grep tinfo > /dev/null; echo $$? ),0)
STATICLIBS := -lncursesw -ltinfo
else
STATICLIBS := -lncursesw -ltinfow
endif

STATICFLAG :=
FLAGS := -Wall -Wextra -Wpedantic -Werror -O2
CXXFLAGS := $(FLAGS) -std=c++23 $(STATICFLAG)

CC := gcc
CXX := g++

build:
	@$(MAKE) all -j$(nproc)

build_w_custom_ncurses:
	@$(MAKE) custom_ncurses
	@$(MAKE) all -j$(nproc) INCLUDE="-I$(PWD)/ncurses-build" LIBPATH="-L$(PWD)/ncurses-build/lib"

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) -o $(TARGET) $(LIBPATH) $(LIBS)

static:
	echo "Building static binary..."
	@$(MAKE) clean_o
	@$(MAKE) clean_e
	@if [ ! -d "ncurses-build" ]; then $(MAKE) custom_ncurses; fi
	@$(MAKE) all -j$(nproc) STATICFLAG="-static -static-libgcc" INCLUDE="-I$(PWD)/ncurses-build" LIBPATH="-L$(PWD)/ncurses-build/lib" LIBS="$(STATICLIBS)"

custom_ncurses:
	@if [ -d "ncurses-build" ]; then \
  	echo "ERROR: ncurses-build directory already exists. If you really want to build again, remove it."; \
 	exit 1; \
 	fi
	@mkdir -p ncurses-build
	@cd ncurses && ./configure --prefix=$(PWD)/ncurses-build --enable-widec --with-termlib && make -j$(nproc) && make -j$(nproc) install

%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

.PHONY: clean_o clean_e static all custom_ncurses build clean

clean_o:
	rm -rf *.o

clean_e:
	rm -f $(TARGET)

clean: clean_o clean_e
	rm -rf ncurses-build
