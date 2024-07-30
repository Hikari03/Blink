#pragma once
#ifdef OK
#undef OK
#endif
#include <gtkmm.h>
#include <fstream>

class GTKHandler {

public:
	GTKHandler();
	void initIntro();

	void show();
private:
	std::shared_ptr<Gtk::Application> _app;
	std::shared_ptr<Gtk::Builder> _builder;
	Gtk::ApplicationWindow * _window;
	std::map<std::string, Gtk::Widget *> _widgets;
	std::vector<std::string> _widgetNames ={
			"IntroBox",
			"enterName",
			"enterNameDialog",
			"enterServer",
			"enterServerDialog",
			"confirmButton"
	};
	std::string name;
	std::string serverAddr;

	void _onStartup();
	void _onActivate();
	void _onIntroButtonClicked();
	void _setWidgetText(const std::string & name, const std::string & text);


};