#include "ColorTile.h"

void ColorTile::setColor(int color_) {
    this->_color = color_;
    _updated = true;
}

void ColorTile::print(unsigned x, unsigned y) {
    attron(COLOR_PAIR(_color));
    Tile::print(x, y);
    attroff(COLOR_PAIR(_color));
}

int ColorTile::getColor() const {
    return _color;
}

ColorTile::ColorTile() {
    tileType = Tile::Type::ColorTile;
}
