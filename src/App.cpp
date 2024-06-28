#include "App.h"


App::App() : _tiles({100,25}), _renderer(_tiles), _lightblue(_renderer.initColor(69, 0)),
                _red(_renderer.initColor(198, 0)) {}

void App::run() {
    try {
		_init();
	}
	catch (std::exception & e) {
		_tiles.insertText(43, 8, _strToWStr(e.what()), _red);
		_renderer.print();
		getch();
		return;
	}

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

    _chat();

    _debug("exited connection");

    getch();
}

/**
 * @brief Gets the name of the user and greets them
 */
void App::_init() {
    _tiles.insertBox(0, 0, 99, 24, false,_lightblue);
    _tiles.insertText(43, 5, L"Insert Name:", _lightblue);
    _renderer.print();
    _userName = _getUserInput(43,6, App::CursorColor::Magenta);

	if(_userName.empty() || _userName.contains(" ")) {
		throw std::runtime_error("Invalid name!");
	}
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
    _tiles.insertBox(0, 0, 99, 24, false,_lightblue);
    _tiles.insertBox(1, 2, 50, 20, false, _lightblue);
    _tiles.insertBox(1, 21, 50, 23, false, _lightblue);
    _tiles.insertText(1,1, L"Connected as: " + _strToWStr(_userName), _red);
    _renderer.print();
}

/**
 * @brief Connects to the server
 */
void App::_connectToServer(std::string ip, int port) {
    _debug("connecting to server");
    _connection.connectToServer(std::move(ip), port);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

std::string App::_getUserInput(int x, int y, App::CursorColor cursorColor) {

    std::string input;

    move(y, x);
    _cursorPos = {y, x};

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
    _currentCursorColor = s_cursorColor;
    // let the terminal do the line editing
    nocbreak();
    echo();

    // this reads from _buffer after <ENTER>, not "raw"
    // so any backspacing etc. has already been taken care of
    _returnCursor();
    int ch = getch();

    while ( ch != '\n' && input.size() < 40 ) {
        input.push_back( ch );
        _returnCursor();
        ch = getch();
    }

    // restore your cbreak / echo settings here

    std::cout << WHITE << std::flush;
    move(0, 0);

    return input;

}

std::wstring App::_strToWStr(const std::string & text) const {

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(text);
}

void App::_debug(const std::string & text) {
    if constexpr(DEBUG) {
        _tiles.insertBox(0, 0, 99, 24, false, _lightblue);
        _tiles.insertText(1, 24, _strToWStr(text), _red);
        _renderer.print();
    }

}

void App::_chat() {

    // initialize send and receive threads
    std::thread sendThread(&App::_sendThread, this);
    std::thread receiveThread(&App::_receiveThread, this);

    sendThread.join();
    receiveThread.join();

    _tiles.insertText(43, 15, L"Chat ended!", _lightblue);
}

void App::_sendThread() {
    std::string message;
    while(_running) {
        message = _getUserInput(2, 22, App::CursorColor::Green);
		// clear the chat box
		_tiles.insertBox(1, 21, 50, 23, true, _lightblue);
		if (message.empty())
			continue;
        if(message == "/exit") {
            _connection.send(_internal"exit");
            _running = false;
			return;
        }

		// we may have been kicked
		if(_running)
        	_connection.sendMessage(message);
    }
}

void App::_receiveThread() {
    std::string message;
    std::vector<std::string> messages;
    while(_running) {
        message = _connection.receive();
        if(message == _internal"exit") {
			_running = false;
            _tiles.insertText(43, 15, L"Server closed connection", _red);
			_debug("Server closed connection");
            _renderer.print();
            return;
        }
        if(message == _internal"ping") {
            _connection.send(_internal"pong");
            continue;
        }

        if(message == _internal"pong") {
            continue;
        }

        if(message == _internal"exitAck") {
            break;
        }

		_tiles.insertBox(1, 2, 50, 20, true, _lightblue);

        messages = _split(message, '\n');

        for(unsigned i = 0; i < messages.size(); i++) {
            _tiles.insertText(3, 3 + i, _strToWStr(messages[i]), _red);
            _debug(messages[i]);
        }

        {
            std::lock_guard<std::mutex> lock(_ioMtx);
            _renderer.print();
            _returnCursor();
        }
    }
}

std::vector<std::string> App::_split(const string &text, char delimiter) const {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(text);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void App::_returnCursor() {
    move(_cursorPos.first, _cursorPos.second);
    std::cout << _currentCursorColor << std::flush;
}


