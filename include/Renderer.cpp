#include "Renderer.h"

Renderer::Renderer(Tiles & tiles_) : tiles(tiles_.tiles), width(tiles_.width), height(tiles_.height) {
    setlocale(LC_CTYPE,"");

    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, true);
    keypad(stdscr, true);
    //resize_term(height, width);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);

    refresh();
}


void Renderer::print() {

    std::shared_ptr<Tile> tile;
    bool changed = false;

    for(unsigned int i = 0; i < height; i++) {
        for(unsigned int j = 0; j < width; j++) {
            tile = tiles[i][j];

            if(tiles[i][j]->updated()) {
                tile->print(i, j);
                changed = true;
            }
        }
    }

    if(changed) {
        refresh();
    }
}

Renderer::~Renderer() {
    endwin();
}

short Renderer::initColor(short foreground, short background, std::optional<short> index_) {
    if(foreground > 255 || background > 255)
        throw std::out_of_range("initColor: Color value out of range: " + std::to_string(foreground) + ", " + std::to_string(background));

    short index = 0;
    if(index_.has_value()) {
        index = index_.value();
    }
    else {
        index = static_cast<short>(colorPairs.size()+1);
    }

    colorPairs[index] = std::make_pair(foreground, background);
    init_pair(index, foreground, background);

    return index;
}


