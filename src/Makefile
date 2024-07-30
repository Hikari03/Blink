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

INCLUDE :=-I. -Iinclude $(shell pkg-config -cflags gtkmm-4.0)
LIBPATH :=-Linclude/librender
LIBS :=-lncursesw -lhikrender -lsodium $(shell pkg-config -libs gtkmm-4.0)

STATICLIBS :=-lncursesw -ltinfow

STATICFLAG :=
FLAGS := -Wall -Wextra -Wpedantic -Werror -g
CXXFLAGS := $(FLAGS) -std=c++23 $(STATICFLAG)

CXX := g++

build: build_librender_dynamic make_obj
	@$(MAKE) all -j$(nproc)

install_dynamic:
	sudo install $(TARGET) /usr/bin/
	$(MAKE) -C include/librender install

install_static:
	cp $(TARGET) /usr/bin/

build_w_custom_ncurses:
	@if [ ! -d "include/librender/ncurses-build" ]; then $(MAKE) -C include/librender custom_ncurses; fi
	@$(MAKE) all -j$(nproc) INCLUDE="-I$(PWD)/include/librender/ncurses-build" LIBPATH="-L$(PWD)/include/librender/ncurses-build/lib"

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(addprefix obj/,$(OBJECTS)) -o $(TARGET) $(LIBPATH) $(LIBS)

build_librender_dynamic:
	@git submodule init
	@git submodule update include/librender
	@$(MAKE) -C include/librender dynamic -j$(nproc)

build_libsodium:
	@if [ -d "include/libsodium-build" ]; then \
	echo "ERROR: libsodium-build directory already exists. If you really want to build again, remove it."; \
	exit 1; \
	fi
	@git submodule init
	@git submodule update include/libsodium
	@mkdir -p include/libsodium-build
	@cd include/libsodium && \
		./configure --prefix=$(PWD)/include/libsodium-build --enable-static  && \
		make -j$(nproc) && \
		make check && \
		make install

static: make_obj
	echo "Building static binary..."
	@$(MAKE) clean_o
	@$(MAKE) clean_e
	@if [ ! -d "include/libsodium-build" ]; then $(MAKE) build_libsodium; fi
	@git submodule update include/librender
	@$(MAKE) -C include/librender static
	@$(MAKE) all -j$(nproc) INCLUDE="-Iinclude/librender/ncurses-build/include -Iinclude/librender/ncurses-build/include/ncursesw -Iinclude/libsodium-build/include -static -static-libgcc" \
 		LIBPATH="-Linclude/librender -Linclude/librender/ncurses-build/lib -Linclude/libsodium-build/lib" \
 		LIBS="-lhikrender -lsodium $(STATICLIBS)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o obj/$@

.PHONY: clean_o clean_e static all custom_ncurses build clean

clean_o:
	if [ -e "obj/*.o" ]; then rm obj/*.o; fi

clean_e:
	rm -f $(TARGET)

make_obj:
	mkdir -p obj

clean: clean_o clean_e
	@$(MAKE) -C include/librender clean
	@$(MAKE) -C include/libsodium clean
	@rm -rf include/libsodium-build
