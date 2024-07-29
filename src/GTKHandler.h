#pragma once
#ifdef OK
#undef OK
#endif
#include <gtkmm.h>

class GTKHandler {

public:
	GTKHandler();

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
			"enterServerDialog"
	};

	void on_startup();
	void on_activate();


};