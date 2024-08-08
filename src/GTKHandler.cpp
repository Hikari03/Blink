#include "GTKHandler.h"

GTKHandler::GTKHandler() {

	_gtkData.init();

#ifdef _WIN32
	auto settings = Gtk::Settings::get_default();

	if (settings) {
		// Enable dark mode
		settings->property_gtk_application_prefer_dark_theme() = true;
	}
#endif


	for (const auto & widgetName : _gtkData._widgetNamesInit) {
		auto [it, widget] = _gtkData._widgetsIntro.insert({widgetName, _gtkData._builder->get_widget<Gtk::Widget>(widgetName)});
		it->second->set_visible();
	}

	for (const auto & widgetName : _gtkData._widgetNamesChat) {
		auto [it, widget] = _gtkData._widgetsChat.insert({widgetName, _gtkData._builder->get_widget<Gtk::Widget>(widgetName)});
		it->second->set_visible();
	}

	std::string cssPath;

#ifdef __linux__
	cssPath = "/usr/share/blink/style.css";

#elif _WIN32
	#ifdef BLINK_WIN_RELEASE
	cssPath = "style.css";
	#else
	char* appdata = getenv("APPDATA");
	cssPath = std::string(appdata) + "\\Blink\\style.css";
	#endif

#endif

	_cssProvider->load_from_path(cssPath);

	auto display = Gdk::Display::get_default();
#ifdef __linux__
	Gtk::StyleProvider::add_provider_for_display(display, _cssProvider, GTK_STYLE_PROVIDER_PRIORITY_THEME);
#elif _WIN32
	Gtk::StyleProvider::add_provider_for_display(display, _cssProvider, GTK_STYLE_PROVIDER_PRIORITY_SETTINGS);
#endif
}

void GTKHandler::init() {
	_gtkData._app->signal_startup().connect(sigc::mem_fun(*this, &GTKHandler::_onStartup));
	_gtkData._app->signal_activate().connect(sigc::mem_fun(*this, &GTKHandler::_onActivate));
	_gtkData._windowIntro->signal_close_request().connect(sigc::bind_return(sigc::mem_fun(*this, &GTKHandler::exit), false), false);
	_gtkData._windowChat->signal_close_request().connect(sigc::bind_return(sigc::mem_fun(*this, &GTKHandler::exit), false), false);
}

void GTKHandler::_initChat() {
	setWidgetText(_gtkData._widgetsChat, "onlineListTitle", "Online users:");
}

void GTKHandler::_onStartup() {
	_gtkData._app->add_window(*_gtkData._windowIntro);
	_gtkData._app->add_window(*_gtkData._windowChat);
}

void GTKHandler::_onActivate() {

	dynamic_cast<Gtk::Button*>(_gtkData._widgetsIntro.at("confirmButton"))
			->signal_clicked().connect(sigc::mem_fun(*this, &GTKHandler::_onIntroButtonClicked));
	dynamic_cast<Gtk::Button*>(_gtkData._widgetsIntro.at("exitButton"))->signal_clicked().connect(sigc::mem_fun(*this,
																												&GTKHandler::exit));
	dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat["messagesInput"])->set_wrap_mode(Gtk::WrapMode::WORD);
	dynamic_cast<Gtk::TextView*>(_gtkData._widgetsIntro["exceptionDisplay"])->set_wrap_mode(Gtk::WrapMode::WORD);
	dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat["messagesField"])->set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
	dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat.at("messagesInput"))->add_controller(_gtkData._key_controller);
	dynamic_cast<Gtk::ListBox*>(_gtkData._widgetsChat.at("onlineList"))->set_sort_func(sigc::ptr_fun(&_listBoxSort));
	setWidgetText(_gtkData._widgetsIntro, "enterName", "Enter name:");
	setWidgetText(_gtkData._widgetsIntro, "enterServer", "Enter server address:");

	_gtkData._windowIntro->set_visible(true);
	_gtkData._windowIntro->present();
	_initChat();
}

void GTKHandler::show() {
	_gtkData._app->run();
}

 void GTKHandler::setWidgetText(std::map<std::string, Gtk::Widget *> & widgets, const std::string & name, const std::string & text) {
	if(widgets.find(name) == widgets.end())
		throw std::runtime_error("Widget not found");
	dynamic_cast<Gtk::TextView*>(widgets[name])->get_buffer()->set_text(text);
}

void GTKHandler::_onIntroButtonClicked() {
	userName = dynamic_cast<Gtk::Entry*>(_gtkData._widgetsIntro.at("enterNameDialog"))->get_buffer()->get_text();
	serverAddr = dynamic_cast<Gtk::Entry*>(_gtkData._widgetsIntro.at("enterServerDialog"))->get_buffer()->get_text();

	try {
		_postIntroFunc();
	}
	catch(std::exception & e){
		setWidgetText(_gtkData._widgetsIntro, "exceptionDisplay", e.what());
		return;
	}

	auto onlineListTitle = "Connected to\n" + serverAddr + "\nas " + userName + "\nOnline users:";

	setWidgetText(_gtkData._widgetsChat, "onlineListTitle", onlineListTitle);

	_gtkData._windowIntro->set_visible(false);
	_gtkData._windowChat->set_visible(true);
	_gtkData._windowChat->present();
}

std::pair<std::string, std::string> GTKHandler::getIntroData() const {
	return {userName, serverAddr};
}

void GTKHandler::setPostIntroFunc(std::function<void()> func) {
	_postIntroFunc = std::move(func);
}

GTKHandler::GtkData &GTKHandler::getGtkData() {
	return _gtkData;
}

void GTKHandler::exit() {
	_gtkData._app->quit();
}

void GTKHandler::wipeMessages() {
	setWidgetText(_gtkData._widgetsChat, "messagesField", "");
}

int GTKHandler::_listBoxSort(Gtk::ListBoxRow *row1, Gtk::ListBoxRow *row2) {
	auto label1 = dynamic_cast<Gtk::Label*>(row1->get_child());
	auto label2 = dynamic_cast<Gtk::Label*>(row2->get_child());

	if (label1 && label2) {
		return label1->get_text().compare(label2->get_text());
	}
	return 0;
}

void GTKHandler::removeOnlineUserFromList(Gtk::ListBoxRow *row) {
	auto listBox = dynamic_cast<Gtk::ListBox*>(_gtkData._widgetsChat.at("onlineList"));
	listBox->remove(*row);
}

void GTKHandler::addOnlineUserToList(const std::string &name) {
	auto listBox = dynamic_cast<Gtk::ListBox*>(_gtkData._widgetsChat.at("onlineList"));
	auto label = Gtk::make_managed<Gtk::Label>(name);
	label->set_visible();
	listBox->append(*label);
}

void GTKHandler::addMessage(const std::vector<std::string> & messages) {

	auto buffer = dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat.at("messagesField"))->get_buffer();
	for(const auto & msg : messages) {
		buffer->insert_at_cursor(msg + "\n");
	}
	auto end = buffer->end();
	dynamic_cast<Gtk::TextView*>(_gtkData._widgetsChat.at("messagesField"))->scroll_to(end);
}