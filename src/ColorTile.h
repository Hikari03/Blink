#pragma once

#include "Tile.h"

/**
 * @brief Extends Tile class with color support
 */
class ColorTile : public Tile {

public:
    ColorTile();
    ColorTile(const ColorTile & other) = default;

    void setColor(int color_) override;

    [[nodiscard]] int getColor() const override;

    void print(unsigned x, unsigned y) override;

private:
    int _color = 0;
    Type tileType = Type::ColorTile;
};