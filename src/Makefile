# stolen from https://gist.github.com/freelsn/fe5f160cf564e209dc9c5673296ee813

# For Windows build i assume using MSYS2 ucrt64

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

## Windows specific
TARGET_PATH := "C:\Program Files\Blink\$(TARGET).exe"
SHORTCUT_PATH := "C:\Users\$(USER)\Desktop\$(TARGET).lnk"
DESCRIPTION := "Blink"

build: make_obj
	if [ ! -e "style.css" ] || [ ! -d "assets/gtkcss" ]; then $(MAKE) get_catppuccin; fi
	@$(MAKE) all -j$(nproc)

win: make_obj
	if [ ! -e "style.css" ] || [ ! -d "assets/gtkcss" ]; then $(MAKE) get_catppuccin; fi
	@$(MAKE) all -j$(nproc) LIBS="-lsodium -lws2_32 $(shell pkg-config -libs gtkmm-4.0)"

win_release: make_obj
	if [ ! -e "style.css" ] || [ ! -d "assets/gtkcss" ]; then $(MAKE) get_catppuccin; fi
	@$(MAKE) all -j$(nproc) DEBUGFLAGS="-DBLINK_WIN_RELEASE" LIBS="-lsodium -lws2_32 $(shell pkg-config -libs gtkmm-4.0)"
	mkdir -p "blink_win/assets"
	cp $(TARGET).exe "blink_win/$(TARGET).exe"
	cp -r assets/dlls/* "blink_win/"
	cp $(TARGET).ui "blink_win/$(TARGET).ui"
	cp style.css "blink_win/style.css"
	touch userData
	chmod a+rw userData
	cp -r assets/gtkcss "blink_win/assets"

win_debug: make_obj
	if [ ! -e "style.css" ] || [ ! -d "assets/gtkcss" ]; then $(MAKE) get_catppuccin; fi
	@$(MAKE) all -j$(nproc) DEBUGFLAGS="-g -DBLINK_DEBUG" LIBS="-lsodium -lws2_32 $(shell pkg-config -libs gtkmm-4.0)"

debug: make_obj
	if [ ! -e "style.css" ] || [ ! -d "assets/gtkcss" ]; then $(MAKE) get_catppuccin; fi
	@$(MAKE) all -j$(nproc) DEBUGFLAGS="-g -O1 -DBLINK_DEBUG"

install:
	sudo install $(TARGET) /usr/bin/blink
	sudo install -Dm644 $(TARGET).desktop /usr/share/applications/cz.hikari03.blink.desktop
	sudo mkdir -p /usr/share/blink/assets/gtkcss
	sudo touch /usr/share/blink/userData
	sudo chmod a+wr /usr/share/blink/userData
	sudo install $(TARGET).ui /usr/share/blink/$(TARGET).ui
	sudo install style.css /usr/share/blink/style.css
	sudo cp -r assets/gtkcss/* /usr/share/blink/assets/gtkcss
	sudo install -Dm644 assets/blink.png /usr/share/pixmaps/cz.hikari03.blink.png
	sudo install -Dm644 assets/blink_32x32.png /usr/share/icons/hicolor/32x32/apps/cz.hikari03.blink.png
	sudo install -Dm644 assets/blink_64x64.png /usr/share/icons/hicolor/64x64/apps/cz.hikari03.blink.png
	sudo install -Dm644 assets/blink.png /usr/share/icons/hicolor/512x512/apps/cz.hikari03.blink.png

install_win:
	mkdir -p /c/Program\ Files/blink
	mkdir -p $(APPDATA)/Blink/assets
	touch $(APPDATA)/Blink/userData
	chmod a+wr $(APPDATA)/Blink/userData
	cp $(TARGET).exe /c/Program\ Files/Blink/$(TARGET).exe
	cp -r assets/dlls/* /c/Program\ Files/Blink/
	cp $(TARGET).ui $(APPDATA)/Blink/$(TARGET).ui
	cp style.css $(APPDATA)/Blink/style.css
	cp -r assets/gtkcss $(APPDATA)/Blink/assets
	cp assets/blink.png /c/Program\ Files/Blink/blink.png
	cmd //c assets\\create_shortcut.bat $(TARGET_PATH) $(SHORTCUT_PATH) $(DESCRIPTION)

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

get_catppuccin:
	if [ -e style.css ] && [ -d assets/gtkcss ]; then \
  		echo "catppuccin already downloaded"; \
  		exit 1; \
  	fi

	wget https://github.com/catppuccin/gtk/releases/download/v1.0.3/catppuccin-mocha-mauve-standard+default.zip
	unzip catppuccin-mocha-mauve-standard+default.zip
	rm catppuccin-mocha-mauve-standard+default.zip
	rm -rf catppuccin-mocha-mauve-standard+default-hdpi
	rm -rf catppuccin-mocha-mauve-standard+default-xhdpi
	cp catppuccin-mocha-mauve-standard+default/gtk-4.0/gtk-dark.css style.css
	mkdir -p assets/gtkcss
	cp -r catppuccin-mocha-mauve-standard+default/gtk-4.0/assets/* assets/gtkcss
	rm -rf catppuccin-mocha-mauve-standard+default
	# remove all lines with @define-color
	sed -i '/@define-color/d' style.css
	# substitute all "assets" with "assets/gtkcss"
	sed -i 's/assets/assets\/gtkcss/g' style.css


clean: clean_o clean_e
	rm style.css
	rm -rf assets/gtkcss
