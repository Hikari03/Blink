# stolen from https://gist.github.com/freelsn/fe5f160cf564e209dc9c5673296ee813

TARGET := blink
SOURCES := $(wildcard *.c *.cpp)
OBJECTS := $(patsubst src%,obj%, $(patsubst %.c,%.o, $(patsubst %.cpp,%.o,$(SOURCES))))

INCLUDE :=-I. -Iinclude $(shell pkg-config -cflags gtkmm-4.0)
LIBPATH :=
LIBS :=-lsodium $(shell pkg-config -libs gtkmm-4.0)

STATICLIBS :=

STATICFLAG :=
FLAGS := -Wall -Wextra -Wpedantic -Werror -O2
DEBUGFLAGS :=
CXXFLAGS := $(FLAGS) -std=c++23 $(STATICFLAG) $(DEBUGFLAGS)

CXX := g++

build: make_obj
	@$(MAKE) all -j$(nproc)

debug: make_obj
	@$(MAKE) all -j$(nproc) DEBUGFLAGS="-g -DBLINK_DEBUG"

install:
	sudo install $(TARGET) /usr/bin/
	sudo install -Dm644 $(TARGET).desktop /usr/share/applications/$(TARGET).desktop
	sudo mkdir -p /usr/share/blink
	sudo install $(TARGET).ui /usr/share/blink/$(TARGET).ui
	sudo install -Dm644 assets/blink.png /usr/share/pixmaps/blink.png
	sudo install -Dm644 assets/blink_32x32.png /usr/share/icons/hicolor/32x32/apps/blink.png
	sudo install -Dm644 assets/blink_64x64.png /usr/share/icons/hicolor/64x64/apps/blink.png
	sudo install -Dm644 assets/blink.png /usr/share/icons/hicolor/512x512/apps/blink.png

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(addprefix obj/,$(OBJECTS)) -o $(TARGET) $(LIBPATH) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o obj/$@

.PHONY: clean_o clean_e static all build clean install

clean_o:
	if [ -e "obj/*.o" ]; then rm obj/*.o; fi

clean_e:
	rm -f $(TARGET)

make_obj:
	mkdir -p obj

clean: clean_o clean_e
