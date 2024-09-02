#pragma once

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

    std::string _fileName = "filler";

    Connection _connection = Connection();

    bool _running = true;

    void _connectToServer(std::string ip, int port);
    void _receiveThread();
	void _postInitCall();
	GTKHandler::GtkData _gtkData;

	bool _onKeyPressed(guint keyval, guint, Gdk::ModifierType);

    void _debug(const std::string & text);
};

