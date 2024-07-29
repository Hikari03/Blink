#include "GTKHandler.h"

GTKHandler::GTKHandler()
			: _app(Gtk::Application::create("org.hikari03.blink")),
			_builder(Gtk::Builder::create_from_file("blink.ui")),
			_window(_builder->get_widget<Gtk::ApplicationWindow>("blink"))
			{

	for (const auto & name : _widgetNames) {
		_widgets.insert({name, _builder->get_widget<Gtk::Widget>(name)});
	}

	_app->run();
	_app->add_window(*_window);

	_window->present();
}

void GTKHandler::show() {
	_window->set_visible(true);
}