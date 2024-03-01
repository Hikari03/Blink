#include "App.h"

App::App() : _tiles({100,25}), _renderer(_tiles), _lightblue(_renderer.initColor(69, 0)) {}

void App::run() {
    _init();
    _renderer.print();
    _tiles.insertText(1,1, strToWStr(_connection.receive()));
    _renderer.print();
    getch();
}

/**
 * @brief Gets the name of the user and greets them
 */
void App::_init() {
    _tiles.insertBox(0, 0, 99, 24, _lightblue);
    _tiles.insertText(43, 5, L"Insert Name:", _lightblue);
    _renderer.print();
    move(6, 43);
    std::string name = getString(MAGENTA);
    _tiles.insertText(43, 8, L"Hello, " + std::wstring(name.begin(), name.end()) + L"!", _lightblue);
    // get user to insert ip address of server
    _tiles.insertText(43, 10, L"Insert IP Address:", _lightblue);
    _renderer.print();
    move(11, 43);
    std::string ip = getString(RED);
    _tiles.insertText(43, 13, L"Connecting to " + std::wstring(ip.begin(), ip.end()) + L" ...", _lightblue);
    _renderer.print();
    _connectToServer(ip, 6999);
}

/**
 * @brief Connects to the server
 */
void App::_connectToServer(std::string ip, int port) {
    _connection.connectToServer(ip, port);
}

std::string App::getString(const std::string & cursorColor) {
    std::string input;
    std::cout << cursorColor << std::flush;
    // let the terminal do the line editing
    nocbreak();
    echo();

    // this reads from buffer after <ENTER>, not "raw"
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

std::wstring App::strToWStr(const std::string & text) {

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(text);
}
