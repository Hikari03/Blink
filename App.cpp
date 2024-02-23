#include "App.h"

App::App() : _tiles({100,25}), _renderer(_tiles), _lightblue(_renderer.initColor(69, 0)) {}

void App::run() {
    _init();
    _renderer.print();
    getch();
}

/**
 * @brief Initializes the UI
 */
void App::_init() {
    _tiles.insertBox(0, 0, 99, 24, _lightblue);
}