#pragma once

#include <locale>
#include <codecvt>

#include "include/Tiles.h"
#include "include/Renderer.h"
#include "Connection.h"

//foreground colors
#define GREEN "\033[32m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"

class App {
public:
    App();

    void run();

private:
    Tiles _tiles;
    Renderer _renderer;
    color _lightblue;

    Connection _connection = Connection();

    void _init();
    void _connectToServer(std::string ip, int port);

    std::string getString(const std::string & cursorColor = "");

    std::wstring strToWStr(const std::string & text);
};