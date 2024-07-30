#include "GTKHandler.h"

GTKHandler::GTKHandler()
			: _app(Gtk::Application::create("org.hikari03.blink")),
			_builder(Gtk::Builder::create_from_file("blink.ui")),
			_window(_builder->get_widget<Gtk::ApplicationWindow>("blink"))
			{}

void GTKHandler::initIntro() {
	_app->signal_startup().connect(sigc::mem_fun(*this, &GTKHandler::_onStartup));
	_app->signal_activate().connect(sigc::mem_fun(*this, &GTKHandler::_onActivate));
}

void GTKHandler::_onStartup() {
	_app->add_window(*_window);
}

void GTKHandler::_onActivate() {
	for (const auto & widgetName : _widgetNames) {
		auto [it, widget] = _widgets.insert({widgetName, _builder->get_widget<Gtk::Widget>(widgetName)});
		it->second->show();
	}

	dynamic_cast<Gtk::Button*>(_widgets.at("confirmButton"))->signal_clicked().connect(sigc::mem_fun(*this, &GTKHandler::_onIntroButtonClicked));

	_setWidgetText("enterName", "Enter name:");
	_setWidgetText("enterServer", "Enter server address:");

	_window->set_size_request();
	_window->present();
	_window->set_visible(true);
	_window->show();
}

void GTKHandler::show() {
	_app->run();
}

void GTKHandler::_setWidgetText(const std::string & name, const std::string & text) {
	if(_widgets.find(name) == _widgets.end())
		throw std::runtime_error("Widget not found");
	auto widget = _widgets[name];
	auto textBuffer = dynamic_cast<Gtk::TextView*>(widget)->get_buffer();
	textBuffer->set_text(text);
}

void GTKHandler::_onIntroButtonClicked() {
	name = dynamic_cast<Gtk::Entry*>(_widgets.at("enterNameDialog"))->get_buffer()->get_text();
	serverAddr = dynamic_cast<Gtk::Entry*>(_widgets.at("enterServerDialog"))->get_buffer()->get_text();
	_window->close();

	//debug
	std::ofstream file("debug.txt");
	file << "name: " << name << std::endl;
	file << "serverAddr: " << serverAddr << std::endl;
}