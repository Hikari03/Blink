#pragma once

#define color short

#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>
#include "Tile.h"
#include "ColorTile.h"

/**
 * @brief Holds and manages tiles matrix
 */
class Tiles {
public:

    Tiles(unsigned width_, unsigned height_);

    void resize(unsigned width_, unsigned height_);

    void insertTile(unsigned x, unsigned y, wchar_t c = L' ');

    std::shared_ptr<Tile> getTile(unsigned x, unsigned y);

    std::shared_ptr<Tile> & setColor(unsigned x, unsigned y, int _color);

    void insertRect(unsigned x_s, unsigned y_s, unsigned x_e, unsigned y_e, wchar_t c = L' ', std::optional<color> _color = std::nullopt);

    void insertBox(unsigned x_s, unsigned y_s, unsigned x_e, unsigned y_e, bool clear = false, std::optional<color> _color = std::nullopt);

    void insertText(unsigned x, unsigned y, std::wstring text, std::optional<color> _color = std::nullopt);

    void clear();

    friend class Renderer;

private:
    std::vector<std::vector<std::shared_ptr<Tile>>> tiles;

    unsigned int width = 0;
    unsigned int height = 0;
};