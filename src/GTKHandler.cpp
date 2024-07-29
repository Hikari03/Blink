#include "GTKHandler.h"

GTKHandler::GTKHandler()
			: _app(Gtk::Application::create("org.hikari03.blink")),
			_builder(Gtk::Builder::create_from_file("blink.ui")),
			_window(_builder->get_widget<Gtk::ApplicationWindow>("blink"))
			{

	_app->signal_startup().connect(sigc::mem_fun(*this, &GTKHandler::on_startup));
	_app->signal_activate().connect(sigc::mem_fun(*this, &GTKHandler::on_activate));

}

void GTKHandler::on_startup() {
	_app->add_window(*_window);
}

void GTKHandler::on_activate() {
	for (const auto & name : _widgetNames) {
		auto [nam, widget] = _widgets.insert({name, _builder->get_widget<Gtk::Widget>(name)});
		nam->second->show();
	}


	dynamic_cast<Gtk::TextView*>(_widgets["enterName"])->set_buffer([]{
		auto text_buffer = Gtk::TextBuffer::create();
		text_buffer->set_text("Enter your name");
		return text_buffer; }());

	_window->present();
	_window->set_visible(true);
	_window->show();
}

void GTKHandler::show() {
	_app->run();

}