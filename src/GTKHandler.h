#pragma once
#ifdef OK
#undef OK
#endif
#include <gtkmm.h>

class GTKHandler {

public:
	GTKHandler();
	void init();
	void initChat();

	std::pair<std::string, std::string> getIntroData();

	void show();
private:
	std::shared_ptr<Gtk::Application> _app;
	std::shared_ptr<Gtk::Builder> _builder;
	Gtk::ApplicationWindow * _windowIntro;
	Gtk::ApplicationWindow * _windowChat;
	std::map<std::string, Gtk::Widget *> _widgetsIntro;
	std::map<std::string, Gtk::Widget *> _widgetsChat;
	std::vector<std::string> _widgetNamesInit ={
		"IntroBox",
		"enterName",
		"enterNameDialog",
		"enterServer",
		"enterServerDialog",
		"confirmButton"
	};
	std::vector<std::string> _widgetNamesChat = {
		"mainApp",
		"messagesBox",
		"messagesField",
		"messagesInput",
		"onlineListBox",
		"onlineListTitle",
		"onlineList"
	};
	//std::shared_ptr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
	std::string userName;
	std::string serverAddr;

	void _onStartup();
	void _onActivate();
	void _onIntroButtonClicked();
	void _setWidgetText(std::map<std::string, Gtk::Widget *> & widgets, const std::string & name, const std::string & text);


};