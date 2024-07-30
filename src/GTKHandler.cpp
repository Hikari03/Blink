#include "GTKHandler.h"

GTKHandler::GTKHandler()
			: _app(Gtk::Application::create("org.hikari03.blink")),
			  _builder(Gtk::Builder::create_from_file("blink.ui")),
			  _windowIntro(_builder->get_widget<Gtk::ApplicationWindow>("blink")),
			  _windowChat(_builder->get_widget<Gtk::ApplicationWindow>("mainAppWin"))
			{
				for (const auto & widgetName : _widgetNamesInit) {
					auto [it, widget] = _widgetsIntro.insert({widgetName, _builder->get_widget<Gtk::Widget>(widgetName)});
					it->second->set_visible();
				}

				for (const auto & widgetName : _widgetNamesChat) {
					auto [it, widget] = _widgetsChat.insert({widgetName, _builder->get_widget<Gtk::Widget>(widgetName)});
					it->second->set_visible();
				}
				//cssProvider->load_from_path("style.css");
				//auto display = Gdk::Display::get_default();
				//Gtk::StyleProvider::add_provider_for_display(display, cssProvider, GTK_STYLE_PROVIDER_PRIORITY_USER);
			}

void GTKHandler::init() {
	_app->signal_startup().connect(sigc::mem_fun(*this, &GTKHandler::_onStartup));
	_app->signal_activate().connect(sigc::mem_fun(*this, &GTKHandler::_onActivate));
}

void GTKHandler::initChat() {
	_setWidgetText(_widgetsChat, "onlineListTitle", "Online users:");
}

void GTKHandler::_onStartup() {
	_app->add_window(*_windowIntro);
	_app->add_window(*_windowChat);
}

void GTKHandler::_onActivate() {

	dynamic_cast<Gtk::Button*>(_widgetsIntro.at("confirmButton"))->signal_clicked().connect(sigc::mem_fun(*this, &GTKHandler::_onIntroButtonClicked));

	_setWidgetText(_widgetsIntro, "enterName", "Enter name:");
	_setWidgetText(_widgetsIntro, "enterServer", "Enter server address:");

	_windowIntro->set_visible(true);
	_windowIntro->present();
	initChat();
}

void GTKHandler::show() {
	_app->run();

}

void GTKHandler::_setWidgetText(std::map<std::string, Gtk::Widget *> & widgets, const std::string & name, const std::string & text) {
	if(widgets.find(name) == widgets.end())
		throw std::runtime_error("Widget not found");
	auto widget = widgets[name];
	auto textBuffer = dynamic_cast<Gtk::TextView*>(widget)->get_buffer();
	textBuffer->set_text(text);
}

void GTKHandler::_onIntroButtonClicked() {
	userName = dynamic_cast<Gtk::Entry*>(_widgetsIntro.at("enterNameDialog"))->get_buffer()->get_text();
	serverAddr = dynamic_cast<Gtk::Entry*>(_widgetsIntro.at("enterServerDialog"))->get_buffer()->get_text();
	_windowIntro->set_visible(false);
	_windowChat->set_visible(true);
	_windowChat->present();
}

std::pair<std::string, std::string> GTKHandler::getIntroData() {
	return {userName, serverAddr};
}