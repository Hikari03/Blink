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
FLAGS := -Wall -Wextra -Wpedantic -Werror -O2
CXXFLAGS := $(FLAGS) -std=c++23 $(STATICFLAG)

CXX := g++

build: make_obj
	@$(MAKE) all -j$(nproc)

install:
	sudo install $(TARGET) /usr/bin/
	sudo install -Dm644 $(TARGET).desktop /usr/share/applications/$(TARGET).desktop
	sudo mkdir -p /usr/share/blink
	sudo install $(TARGET).ui /usr/share/blink/$(TARGET).ui
	sudo install -Dm755 exec.sh /usr/share/blink/exec.
	sudo install -Dm644 blink.png /usr/share/pixmaps/blink.png

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(addprefix obj/,$(OBJECTS)) -o $(TARGET) $(LIBPATH) $(LIBS)

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
