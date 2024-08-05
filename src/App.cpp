#include "App.h"

void App::run() {
    try {
		_gtkHandler.init();
		_gtkHandler.setPostIntroFunc(std::bind(&App::_postInitCall, this));
		_gtkData = _gtkHandler.getGtkData();

		_gtkData._key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &App::_onKeyPressed), false);

		_gtkHandler.show();
		try {
			_connection.sendInternal("exit");
		}
		catch (std::exception & e) {
			std::string message = e.what();
			if constexpr(DEBUG) {
				g_log("App", G_LOG_LEVEL_CRITICAL,"Exception %s", message.c_str());
			}
		}
		_receiveThr.join();
	}
	catch (std::exception & e) {
		std::string message = e.what();
		if constexpr(DEBUG) {
			g_log("App", G_LOG_LEVEL_CRITICAL,"Exception %s", message.c_str());
		}

	}
}



void App::_postInitCall() {
	_userName = _gtkHandler.getIntroData().first;
	_ip = _gtkHandler.getIntroData().second;

	if(_userName.empty() || _userName.contains(" ")) {
		throw std::runtime_error("Invalid name!");
	}
	_connectToServer(_ip, 6999);

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
	if constexpr(DEBUG) {
		if (message == _internal"nameAck")
			g_message("Connected!");
		else
			g_message("Connection failed!");
	}
}

void App::_debug(const std::string & text) {
    if constexpr(DEBUG) {
		std::string message = text;
		g_log("App", G_LOG_LEVEL_DEBUG,"Debug: %s",  message.c_str());
    }

}

void App::_receiveThread() {
    std::string message;
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

		if(message.contains(_internal"onlineUsers:")) {
			message = message.substr(sizeof(_internal"onlineUsers:") - 1);

			auto onlineUsers = [&]() {
				std::vector<std::string> users;
				std::string user;
				for(const auto & c : message) {
					if(c == ',') {
						users.push_back(user);
						user.clear();
					} else {
						user += c;
					}
				}
				return users;
			}();

			if constexpr(DEBUG) {
				for(const auto & user : onlineUsers) {
					g_message("User: %s", user.c_str());
				}
			}

			auto listBox = dynamic_cast<Gtk::ListBox*>(_gtkData._widgetsChat.at("onlineList"));

			// find online users that are not in the _onlineUsers list
			for(const auto & user : onlineUsers) {
				if(std::find(_onlineUsers.begin(), _onlineUsers.end(), user) == _onlineUsers.end()) {
					_onlineUsers.push_back(user);
					Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(_gtkHandler, &GTKHandler::addOnlineUserToList), user));
				}
			}

			// find users that are not online anymore
			for(auto it = _onlineUsers.begin(); it != _onlineUsers.end(); ) {
				if(std::find(onlineUsers.begin(), onlineUsers.end(), *it) == onlineUsers.end()) {
					auto row = listBox->get_row_at_index(static_cast<int>(std::distance(_onlineUsers.begin(), it)));
					Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(_gtkHandler, &GTKHandler::removeOnlineUserFromList), row));
					it = _onlineUsers.erase(it);
				} else {
					++it;
				}
			}

			continue;
		}

		if(message.contains(_text)) {
			message = message.substr(sizeof(_text) - 1, message.length());

			std::vector<std::string> messages;
			std::string delimiter = "\n";
			size_t pos = 0;
			std::string token;
			while ((pos = message.find(delimiter)) != std::string::npos) {
				token = message.substr(0, pos);
				messages.push_back(token);
				message.erase(0, pos + delimiter.length());
			}

			Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(_gtkHandler, &GTKHandler::addMessage), messages));
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
			if(message == "/getallmessages") {
				_gtkHandler.wipeMessages();
				_connection.sendInternal("getAllMessages");
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


