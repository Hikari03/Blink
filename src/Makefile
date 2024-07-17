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

INCLUDE :=-I. -Iinclude
LIBPATH :=-Linclude/librender
LIBS :=-lncursesw -lhikrender

THISDIR := $(shell pwd)

ifeq ($(shell ldconfig -p | grep tinfo > /dev/null; echo $$? ),0)
STATICLIBS :=-lncursesw -ltinfo
else
STATICLIBS :=-lncursesw -ltinfow
endif

STATICFLAG :=
FLAGS := -Wall -Wextra -Wpedantic -Werror -O2
CXXFLAGS := $(FLAGS) -std=c++23 $(STATICFLAG)

CC := gcc
CXX := g++

build:
	@$(MAKE) all -j$(nproc)

install:
	cp $(TARGET) ~/.local/bin/
	$(MAKE) -C include/librender install

build_w_custom_ncurses:
	@if [ ! -d "include/ncurses-build" ]; then $(MAKE) custom_ncurses; fi
	@$(MAKE) all -j$(nproc) INCLUDE="-I$(PWD)/include/ncurses-build" LIBPATH="-L$(PWD)/include/ncurses-build/lib"

all: $(OBJECTS) build_librender
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(addprefix obj/,$(OBJECTS)) -o $(TARGET) $(LIBPATH) $(LIBS)

build_librender:
	@git submodule init
	@git submodule update include/librender
	@$(MAKE) -C include/librender all -j$(nproc)

static: build_librender
	echo "Building static binary..."
	@$(MAKE) clean_o
	@$(MAKE) clean_e
	@if [ ! -d "include/ncurses-build" ]; then $(MAKE) custom_ncurses; fi
	@$(MAKE) all -j$(nproc) STATICFLAG="-static -static-libgcc" INCLUDE="-I$(PWD)/include/ncurses-build" LIBPATH="-L$(PWD)/include/ncurses-build/lib" LIBS="$(STATICLIBS)"

custom_ncurses:
	@if [ -d "include/ncurses-build" ]; then \
  	echo "ERROR: include/ncurses-build directory already exists. If you really want to build again, remove it."; \
 	exit 1; \
 	fi
	@git submodule init
	@git submodule update
	@mkdir -p include/ncurses-build
	@cd include/ncurses && \
 		./configure --prefix=$(PWD)/include/ncurses-build --enable-widec --with-termlib \
 		&& make -j$(nproc) \
 		&& make -j$(nproc) install


%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o obj/$@

.PHONY: clean_o clean_e static all custom_ncurses build clean

clean_o:
	rm obj/*.o

clean_e:
	rm -f $(TARGET)

make_obj:
	mkdir -p obj

clean: clean_o clean_e
	rm -rf include/ncurses-build
