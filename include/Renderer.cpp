#include "Renderer.h"

Renderer::Renderer(Tiles & tiles_) : tiles(tiles_.tiles), width(tiles_.width), height(tiles_.height) {
    setlocale(LC_CTYPE,"");

/*    prevTiles.resize(height);
    for(unsigned int i = 0; i < height; i++) {
        prevTiles[i].resize(width);
    }

    for(unsigned int i = 0; i < height; i++) {
        for(unsigned int j = 0; j < width; j++) {
            std::shared_ptr<Tile> tile = std::make_shared<Tile>();
            tile->setChar(L' ');
            prevTiles[i][j] = tile;
        }
    }*/

    //copyTiles();

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
        //copyTiles();
    }
}

/*void Renderer::copyTiles() {

    prevTiles.resize(height);
    for(unsigned i = 0; i < height; i++) {
        prevTiles[i].resize(width);
    }

    for(unsigned int i = 0; i < height; i++) {
        for(unsigned int j = 0; j < width; j++) {
            auto prevTile = prevTiles[i][j]->getChar();
            auto tile = tiles[i][j]->getChar();
            if(prevTile != tile) {
                std::shared_ptr<Tile> newTile = std::make_shared<Tile>();
                newTile->setChar(tile);
                prevTiles[i][j] = newTile;
            }
        }
    }

}*/

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


