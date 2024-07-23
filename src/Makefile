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

build: build_librender_dynamic make_obj
	@$(MAKE) all -j$(nproc)

install_dynamic:
	cp $(TARGET) ~/.local/bin/
	$(MAKE) -C include/librender install

install_static:
	cp $(TARGET) ~/.local/bin/

build_w_custom_ncurses:
	@if [ ! -d "include/librender/ncurses-build" ]; then $(MAKE) -C include/librender custom_ncurses; fi
	@$(MAKE) all -j$(nproc) INCLUDE="-I$(PWD)/include/librender/ncurses-build" LIBPATH="-L$(PWD)/include/librender/ncurses-build/lib"

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(addprefix obj/,$(OBJECTS)) -o $(TARGET) $(LIBPATH) $(LIBS)

build_librender_dynamic:
	@git submodule init
	@git submodule update include/librender
	@$(MAKE) -C include/librender dynamic -j$(nproc)

static: make_obj # not working
	echo "Building static binary..."
	@$(MAKE) clean_o
	@$(MAKE) clean_e
	@$(MAKE) -C include/librender static
	@$(MAKE) all -j$(nproc) INCLUDE="-static -static-libgcc" LIBPATH="-Linclude/librender -Linclude/librender/ncurses-build/lib" LIBS="-lhikrender $(STATICLIBS)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o obj/$@

.PHONY: clean_o clean_e static all custom_ncurses build clean

clean_o:
	if [ -e obj/*.o ]; then rm obj/*.o; fi


clean_e:
	rm -f $(TARGET)

make_obj:
	mkdir -p obj

clean: clean_o clean_e

