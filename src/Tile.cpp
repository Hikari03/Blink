#include <iostream>
#include "Tile.h"

using namespace std;


void Tile::setChar(wchar_t c) {
    ch = c;
    _updated = true;
}

const wchar_t & Tile::getChar() const {
    return ch;
}

void Tile::print(unsigned x, unsigned y) {
    mvprintw(static_cast<int>(x), static_cast<int>(y), "%lc", ch);
    _updated = false;
}

Tile::Type Tile::getType() const {
    return tileType;
}

bool Tile::updated() const {
    return _updated;
}
