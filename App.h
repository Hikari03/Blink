#pragma once

#include "include/Tiles.h"
#include "include/Renderer.h"

class App {
public:
    App();

    void run();

private:
    Tiles _tiles;
    Renderer _renderer;
    color _lightblue;

    void _init();
};