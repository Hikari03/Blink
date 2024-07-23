#pragma once

#include <locale>
#include <codecvt>
#include <string>
#include <thread>
#include <utility>
#include <sstream>

#include "include/librender/Renderer.h"
#include "Connection.h"

//foreground colors
#define GREEN "\033[32m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"

#define DEBUG 0

class App {
public:
    App();

    void run();

private:
    Tiles _tiles;
    Renderer _renderer;
    std::mutex _ioMtx;
    color _lightblue;
    color _red;

    std::string _userName;
    std::string _ip;

    Connection _connection = Connection();

    bool _running = true;

    enum class CursorColor {
        Green,
        Red,
        Blue,
        Yellow,
        Cyan,
        Magenta,
        White
    };

    void _init();
    void _prepareUI();
    void _connectToServer(std::string ip, int port);
    void _chat();
    void _sendThread();
    void _receiveThread();
    void _returnCursor();
    std::string _currentCursorColor = WHITE;
    std::pair<int, int> _cursorPos = {0, 0};

    [[nodiscard]] std::vector<std::string> _split(const std::string & text, char delimiter) const;

    std::string _getUserInput(int x, int y, App::CursorColor = CursorColor::White);

    std::wstring _strToWStr(const std::string & text) const;

    void _debug(const std::string & text);
};
