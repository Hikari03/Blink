#include "App.h"

void App::run() {
    try {
		_gtkHandler.init();
		_gtkHandler.setPostIntroFunc(std::bind(&App::_postInitCall, this));
		_gtkData = _gtkHandler.getGtkData();

		_gtkData._key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &App::_onKeyPressed), false);

		_gtkHandler.show();
		_connection.sendInternal("exit");
		_receiveThr.join();
	}
	catch (std::exception & e) {
		g_message(e.what());
	}
}



void App::_postInitCall() {
	_userName = _gtkHandler.getIntroData().first;
	_ip = _gtkHandler.getIntroData().second;

	if(_userName.empty() || _userName.contains(" ")) {
		throw std::runtime_error("Invalid name!");
	}
	_connectToServer(_ip, 6999);

	_connection.sendInternal("getHistory");
	_debug("sent history request");
	_receiveThr = std::thread(&App::_receiveThread, this);
}

/**
 * @brief Connects to the server
 */
void App::_connectToServer(std::string ip, int port) {
    _debug("connecting to server");
    _connection.connectToServer(std::move(ip), port);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
	_connection.receive(); // for encryption initialization
	_debug("connected to server");
    std::string message = _connection.receive();
    _debug("server message: " + message);
	if(message == _internal"ban") {
		throw std::runtime_error("Server banned you!");
	}
    if(message == _internal"name")
        _connection.send(_userName);
    else
        throw std::runtime_error("Server did not ask for name");

    _debug("sent name to server");

    message = _connection.receive();
    _debug("server message: " + message);
    if(message == _internal"nameAck")
		g_message("Connected!");
    else
		g_message("Connection failed!");
}

void App::_debug(const std::string & text) {
    if constexpr(DEBUG) {

		g_message(text.c_str());
    }

}

void App::_receiveThread() {
    std::string message;
    std::vector<std::string> messages;
    while(_running) {
        message = _connection.receive();
        if(message == _internal"exit") {
			_running = false;
			g_message("Server closed connection");
			_gtkHandler.exit();
			_connection.close();
            return;
        }
        if(message == _internal"ping") {
            _connection.sendInternal("pong");
            continue;
        }

        if(message == _internal"pong") {
            continue;
        }

        if(message == _internal"exitAck") {
            break;
        }

		if(message.contains(_text)) {
			message = message.substr(sizeof(_text) - 1, message.length());

			auto buffer = dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat.at("messagesField"))->get_buffer();
			buffer->insert_at_cursor(message);
			auto end = buffer->end();
			dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat.at("messagesField"))->scroll_to(end);
		}
    }
}


bool App::_onKeyPressed(guint keyval, guint, Gdk::ModifierType) {
	_debug("key pressed");
	if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
		_debug("enter pressed");
		std::string message = dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat.at("messagesInput"))->get_buffer()->get_text();
		GTKHandler::setWidgetText(_gtkData._widgetsChat, "messagesInput", "");
		if (message.empty())
			return true;
		if(message == "/e" || message == "/q") {
			_running = false;
			_gtkHandler.exit();
			return true;
		}

		// debug commands
		if constexpr (DEBUG) {
			if(message == "/getmessages") {
				_gtkHandler.wipeMessages();
				_connection.sendInternal("getMessages");
				return true;
			}
			if(message == "/gethistory") {
				_gtkHandler.wipeMessages();
				_connection.sendInternal("getHistory");
				return true;
			}
		}

		if(_running)
			_connection.sendMessage(message);

		return true;
	}
	return false;
}


