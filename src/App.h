#pragma once

#include <fstream>

#include "Connection.h"
#include "GTKHandler.h"

#ifdef BLINK_DEBUG
#define DEBUG 1
#else 
#define DEBUG 0
#endif


class App {
public:

    void run();

private:
	GTKHandler _gtkHandler;
	std::vector<std::string> _onlineUsers;
	std::thread _receiveThr;

    std::string _userName;
    std::string _ip;

#ifdef __linux__
    std::string _fileName = "/usr/share/blink/userData";
#elif _WIN32
	#ifdef BLINK_WIN_RELEASE
	std::string _fileName = "userData";
	#else
	std::string _fileName = std::string(getenv("APPDATA")) + "\\Blink\\userData";
	#endif
#endif

    Connection _connection = Connection();

    bool _running = true;

    void _connectToServer(std::string ip, int port);
    void _receiveThread();
	void _postInitCall();
	GTKHandler::GtkData _gtkData;

	bool _onKeyPressed(guint keyval, guint, Gdk::ModifierType);

    void _debug(const std::string & text);
};

