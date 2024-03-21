#include "App.h"

#include <utility>

App::App() : _tiles({100,25}), _renderer(_tiles), _lightblue(_renderer.initColor(69, 0)),
                _red(_renderer.initColor(198, 0)) {}

void App::run() {
    _init();
    _debug("initialized app");

    try {
        _connectToServer(_ip, 6999);
    }
    catch(std::exception & e) {
        _tiles.insertText(43, 15, L"Connection failed from exception!", _lightblue);
        _debug(e.what());
        _renderer.print();
        getch();
        return;
    }
    _debug("connected to server");

    _prepareUI();


    _debug("exited connection");

    getch();
}

/**
 * @brief Gets the name of the user and greets them
 */
void App::_init() {
    _tiles.insertBox(0, 0, 99, 24, _lightblue);
    _tiles.insertText(43, 5, L"Insert Name:", _lightblue);
    _renderer.print();
    _userName = _getUserInput(43,6, App::CursorColor::Magenta);
    _tiles.insertText(43, 8, L"Hello, " + std::wstring(_userName.begin(), _userName.end()) + L"!", _lightblue);
    // get user to insert ip address of server
    _tiles.insertText(43, 10, L"Insert IP Address:", _lightblue);
    _renderer.print();
    _debug("getting ip");
    _ip = _getUserInput(43, 11, App::CursorColor::Red);
    _debug("got ip");
    _tiles.insertText(43, 13, L"Connecting to " + std::wstring(_ip.begin(), _ip.end()) + L" ...", _lightblue);
    _renderer.print();
}

void App::_prepareUI() {
    _tiles.clear();
    _tiles.insertBox(0, 0, 99, 24, _lightblue);
    _tiles.insertBox(1, 2, 50, 20, _lightblue);
    _tiles.insertBox(1, 21, 50, 23, _lightblue);
    _tiles.insertText(1,1, L"Connected as: " + _strToWStr(_userName), _red);
    _renderer.print();
}

/**
 * @brief Connects to the server
 */
void App::_connectToServer(std::string ip, int port) {
    _debug("connecting to server");
    _connection.connectToServer(std::move(ip), port);
    std::string message = _connection.receive();
    _debug("server message: " + message);
    if(message == _internal"name")
        _connection.send(_userName);
    else
        throw std::runtime_error("Server did not ask for name");

    _debug("sent name to server");

    message = _connection.receive();
    _debug("server message: " + message);
    if(message == _internal"nameAck")
        _tiles.insertText(43, 15, L"Connected!", _lightblue);
    else
        _tiles.insertText(43, 15, L"Connection failed!", _lightblue);

    _debug("server message: " + message);
    _renderer.print();
}

std::string App::_getUserInput(int x, int y, App::CursorColor cursorColor) const {
    std::string input;

    move(y, x);

    std::string s_cursorColor;

    if(cursorColor == App::CursorColor::Green)
        s_cursorColor = GREEN;
    else if(cursorColor == App::CursorColor::Red)
        s_cursorColor = RED;
    else if(cursorColor == App::CursorColor::Blue)
        s_cursorColor = BLUE;
    else if(cursorColor == App::CursorColor::Yellow)
        s_cursorColor = YELLOW;
    else if(cursorColor == App::CursorColor::Cyan)
        s_cursorColor = CYAN;
    else if(cursorColor == App::CursorColor::Magenta)
        s_cursorColor = MAGENTA;
    else if(cursorColor == App::CursorColor::White)
        s_cursorColor = WHITE;

    std::cout << s_cursorColor << std::flush;
    // let the terminal do the line editing
    nocbreak();
    echo();

    // this reads from _buffer after <ENTER>, not "raw"
    // so any backspacing etc. has already been taken care of
    int ch = getch();

    while ( ch != '\n' )
    {
        input.push_back( ch );
        ch = getch();
    }

    // restore your cbreak / echo settings here

    std::cout << WHITE << std::flush;

    return input;

}

std::wstring App::_strToWStr(const std::string & text) const {

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(text);
}

void App::_debug(const std::string & text) {
    if constexpr(DEBUG) {
        _tiles.insertBox(0, 0, 99, 24, _lightblue);
        _tiles.insertText(1, 24, _strToWStr(text), _red);
        _renderer.print();
    }

}
