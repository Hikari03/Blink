#include "Tiles.h"

Tiles::Tiles(unsigned int width_, unsigned int height_) : width(width_), height(height_) {

    resize(width_, height_);
}

void Tiles::resize(unsigned int width_, unsigned int height_) {

    tiles.resize(height_);
        for(unsigned int i = 0; i < height_; i++) {
            tiles[i].resize(width_);
        }

    for(unsigned int i = 0; i < height_; i++) {
        for(unsigned int j = 0; j < width_; j++) {
            std::shared_ptr<Tile> tile = std::make_shared<Tile>();
            tile->setChar(L' ');
            tiles[i][j] = tile;
        }
    }
}

void Tiles::insertTile(unsigned int x, unsigned int y, wchar_t c) {

    if(x < width && y < height) {
        tiles[y][x] = std::make_shared<Tile>();
        tiles[y][x]->setChar(c);
    }

    throw std::out_of_range("setTile: Tile index out of range: " + std::to_string(x) + ", " + std::to_string(y));
}

std::shared_ptr<Tile> Tiles::getTile(unsigned int x, unsigned int y) {

    if(x < width && y < height) {
        return tiles[y][x];
    }

    throw std::out_of_range("getTile: Tile index out of range: " + std::to_string(x) + ", " + std::to_string(y));
}

std::shared_ptr<Tile> & Tiles::setColor(unsigned int x, unsigned int y, int _color) {

    if(x >= width || y >= height)
        throw std::out_of_range("insertColorTile: Tile index out of range: " + std::to_string(x) + ", " + std::to_string(y));

    if(_color > 255 || _color < 0)
        throw std::out_of_range("insertColorTile: Color index out of range: " + std::to_string(_color));


    if(tiles[y][x]->getType() == Tile::Type::ColorTile) {
        tiles[y][x]->setColor(_color);
    } else {
        std::shared_ptr<ColorTile> tile = std::make_shared<ColorTile>();
        tile->setChar(tiles[y][x]->getChar());
        tile->setColor(_color);
        tiles[y][x] = tile;
    }

    return tiles[y][x];

}

void Tiles::clear() {

    for(unsigned int i = 0; i < height; i++) {
        for(unsigned int j = 0; j < width; j++) {
            tiles[i][j] = std::make_shared<Tile>();
        }
    }

}

void Tiles::insertRect(unsigned int x_s, unsigned int y_s, unsigned int x_e, unsigned int y_e, wchar_t c, std::optional<color> _color) {

    if(x_s > x_e || y_s > y_e)
        throw std::invalid_argument("insertRect: Invalid rectangle coordinates");

    if(x_e >= width || y_e >= height)
        throw std::out_of_range("insertRect: Rectangle coordinates out of range");

    if(!_color.has_value()) {
        for(unsigned int i = y_s; i <= y_e; i++) {
            for(unsigned int j = x_s; j <= x_e; j++) {
                tiles[i][j] = std::make_shared<Tile>();
                tiles[i][j]->setChar(c);
            }
        }
    }
    else {
        for(unsigned int i = y_s; i <= y_e; i++) {
            for(unsigned int j = x_s; j <= x_e; j++) {
                std::shared_ptr<ColorTile> tile = std::make_shared<ColorTile>();
                tile->setChar(c);
                tile->setColor(_color.value());
                tiles[i][j] = tile;
            }
        }
    }
}

/**
 * @brief prints a box on the screen
 * @details ┏━┓\n
 *          ┃ \ ┃\n
 *          ┗━┛
 * @param x_s
 * @param y_s
 * @param x_e
 * @param y_e
 * @param _color
 */
void Tiles::insertBox(unsigned int x_s, unsigned int y_s, unsigned int x_e, unsigned int y_e, bool clear, std::optional<short> _color) {

    if(x_s > x_e || y_s > y_e)
        throw std::invalid_argument("insertBox: Invalid box coordinates");

    if(x_e >= width || y_e >= height)
        throw std::out_of_range("insertBox: Box coordinates out of range");

    if(!_color.has_value()) {
        for(unsigned int i = y_s; i <= y_e; i++) {
            for(unsigned int j = x_s; j <= x_e; j++) {
                if(i == y_s && j == x_s) {
                    tiles[i][j] = std::make_shared<Tile>();
                    tiles[i][j]->setChar(L'┏');
                }
                else if(i == y_s && j == x_e) {
                    tiles[i][j] = std::make_shared<Tile>();
                    tiles[i][j]->setChar(L'┓');
                }
                else if(i == y_e && j == x_s) {
                    tiles[i][j] = std::make_shared<Tile>();
                    tiles[i][j]->setChar(L'┗');
                }
                else if(i == y_e && j == x_e) {
                    tiles[i][j] = std::make_shared<Tile>();
                    tiles[i][j]->setChar(L'┛');
                }
                else if(i == y_s || i == y_e) {
                    tiles[i][j] = std::make_shared<Tile>();
                    tiles[i][j]->setChar(L'━');
                }
                else if(j == x_s || j == x_e) {
                    tiles[i][j] = std::make_shared<Tile>();
                    tiles[i][j]->setChar(L'┃');
                }
				else if(clear) {
					tiles[i][j] = std::make_shared<Tile>();
					tiles[i][j]->setChar(L' ');
				}
            }
        }
    }
    else {
        for (unsigned int i = y_s; i <= y_e; i++) {
            for (unsigned int j = x_s; j <= x_e; j++) {
                std::shared_ptr<ColorTile> tile = std::make_shared<ColorTile>();
                tile->setColor(_color.value());
                if (i == y_s && j == x_s) {
                    tile->setChar(L'┏');
                    tiles[i][j] = tile;
                } else if (i == y_s && j == x_e) {
                    tile->setChar(L'┓');
                    tiles[i][j] = tile;
                } else if (i == y_e && j == x_s) {
                    tile->setChar(L'┗');
                    tiles[i][j] = tile;
                } else if (i == y_e && j == x_e) {
                    tile->setChar(L'┛');
                    tiles[i][j] = tile;
                } else if (i == y_s || i == y_e) {
                    tile->setChar(L'━');
                    tiles[i][j] = tile;
                } else if (j == x_s || j == x_e) {
                    tile->setChar(L'┃');
                    tiles[i][j] = tile;
                }
				else if(clear) {
					tile->setChar(L' ');
					tiles[i][j] = tile;
				}
            }
        }
    }
}

void Tiles::insertText(unsigned int x, unsigned int y, std::wstring text, std::optional<short> _color) {

        if(x + text.length() > width-1 || y > height-1)
            throw std::out_of_range("insertText: Text coordinates out of range");

        if(!_color.has_value()) {
            for(unsigned int i = 0; i < text.length(); i++) {
                tiles[y][x + i] = std::make_shared<Tile>();
                tiles[y][x + i]->setChar(text[i]);
            }
        }
        else {
            for(unsigned int i = 0; i < text.length(); i++) {
                std::shared_ptr<ColorTile> tile = std::make_shared<ColorTile>();
                tile->setChar(text[i]);
                tile->setColor(_color.value());
                tiles[y][x + i] = tile;
            }
        }

}


