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
LIBPATH :=
LIBS :=-lncursesw -lsodium $(shell pkg-config -libs gtkmm-4.0)

STATICLIBS :=-lncursesw -ltinfow

STATICFLAG :=
FLAGS := -Wall -Wextra -Wpedantic -Werror -g
CXXFLAGS := $(FLAGS) -std=c++23 $(STATICFLAG)

CXX := g++

build: make_obj
	@$(MAKE) all -j$(nproc)

install:
	sudo install $(TARGET) /usr/bin/

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(addprefix obj/,$(OBJECTS)) -o $(TARGET) $(LIBPATH) $(LIBS)

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
	@$(MAKE) all -j$(nproc) INCLUDE="-Iinclude/libsodium-build/include -static -static-libgcc" \
 		LIBPATH="-Linclude/libsodium-build/lib" \
 		LIBS="-lsodium $(STATICLIBS)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o obj/$@

.PHONY: clean_o clean_e static all build clean

clean_o:
	if [ -e "obj/*.o" ]; then rm obj/*.o; fi

clean_e:
	rm -f $(TARGET)

make_obj:
	mkdir -p obj

clean: clean_o clean_e
	@$(MAKE) -C include/libsodium clean
	@rm -rf include/libsodium-build
