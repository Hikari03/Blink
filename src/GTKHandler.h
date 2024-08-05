#pragma once

#include <gtkmm.h>
#include <thread>

class GTKHandler {

public:

	GTKHandler();
	void init();
	void setPostIntroFunc(std::function<void()> func);
	static void setWidgetText(std::map<std::string, Gtk::Widget *> & widgets, const std::string & name, const std::string & text);

	[[nodiscard]] std::pair<std::string, std::string> getIntroData() const;
	void wipeMessages();
	void removeOnlineUserFromList(Gtk::ListBoxRow * row);
	void addOnlineUserToList(const std::string & name);
	void addMessage(const std::vector<std::string> & messages);

	void show();
	void exit();
	struct GtkData {

		void init() {
			_app = Gtk::Application::create("blink");
			_builder = Gtk::Builder::create_from_file("/usr/share/blink/blink.ui");
			_windowIntro = _builder->get_widget<Gtk::ApplicationWindow>("blink");
			_windowChat = _builder->get_widget<Gtk::ApplicationWindow>("mainAppWin");
			_key_controller = Gtk::EventControllerKey::create();

		}

		GtkData & operator = (const GtkData & other){
			if(this == &other)
				return *this;

			_app = other._app;
			_builder = other._builder;
			_windowIntro = other._windowIntro;
			_windowChat = other._windowChat;
			_widgetsIntro = other._widgetsIntro;
			_widgetsChat = other._widgetsChat;
			_widgetNamesInit = other._widgetNamesInit;
			_widgetNamesChat = other._widgetNamesChat;
			_key_controller = other._key_controller;
			return *this;
		}

		std::shared_ptr<Gtk::Application> _app;
		std::shared_ptr<Gtk::Builder> _builder;
		Gtk::ApplicationWindow * _windowIntro;
		Gtk::ApplicationWindow * _windowChat;
		std::map<std::string, Gtk::Widget *> _widgetsIntro;
		std::map<std::string, Gtk::Widget *> _widgetsChat;
		std::shared_ptr<Gtk::EventControllerKey> _key_controller;
		std::vector<std::string> _widgetNamesInit ={
				"IntroBox",
				"enterName",
				"enterNameDialog",
				"enterServer",
				"enterServerDialog",
				"confirmButton",
				"exitButton",
				"exceptionDisplay"
		};
		std::vector<std::string> _widgetNamesChat = {
				"mainApp",
				"messagesBox",
				"messagesScroll",
				"messagesField",
				"messagesInput",
				"onlineListBox",
				"onlineListTitle",
				"onlineList"
		};
	};

	GtkData & getGtkData();
private:

	GtkData _gtkData;
	std::function<void()> _postIntroFunc;
	std::string userName;
	std::string serverAddr;

	void _initChat();
	void _onStartup();
	void _onActivate();
	void _onIntroButtonClicked();
	static int _listBoxSort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);

};