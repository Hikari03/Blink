#include <iostream>
#include <filesystem>
#include "SharedResources.h"

SharedResources::SharedResources(std::mutex & messagesMutex)  : messagesMutex(messagesMutex) {
	_messagesFile.open(_messagesFileName, std::ios::in);
	//debug!
	std::string pwd = std::filesystem::current_path();
	std::cout << "debug: opening file: " << _messagesFileName << " with pwd: " << pwd << std::endl;

	if(_messagesFile.bad() || !_messagesFile.is_open()) {

		_messagesFile.clear();  // Clear any error flags
		_messagesFile.open(_messagesFileName, std::ios::out);
		_messagesFile.close();  // Close immediately after creating

		// Reopen for reading and writing
		_messagesFile.open(_messagesFileName, std::ios::in);

		if(_messagesFile.bad() || !_messagesFile.is_open())
			throw std::runtime_error("unable to open file");
	}
	parseMessagesFile();

	_messagesFile.close();
	_messagesFile.open(_messagesFileName, std::ios::out | std::ios::app);

	inited = true;
}

SharedResources::~SharedResources() {
	callBackOnResourceChange.notify_all();
	_messagesFile.close();
}

void SharedResources::addMessage(const Message & message) { //todo: fix not writing to file
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        _messages.insert(message);
		if(inited) {
			std::string messageSerial = message.serialize() + '\n';
			std::cout << "debug: writing to file: " << messageSerial << " with size: " << messageSerial.size() << std::endl;
			_messagesFile.write(messageSerial.c_str(), static_cast<std::streamsize>(messageSerial.size()));
			if(_messagesFile.fail())
				std::cout << "error writing to file" << std::endl;
			_messagesFile.flush();
		}
        _changeSinceLastSerialization = true;
    }
    callBackOnResourceChange.notify_all();
}

void SharedResources::removeMessage(const Message & message) { // doesnt remove message in persistent storage
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        _messages.erase(message);
        _changeSinceLastSerialization = true;
    }
    callBackOnResourceChange.notify_all();
}

[[nodiscard]] std::set<Message> SharedResources::getMessages() const {
    return _messages;
}

std::string SharedResources::serializeMessages(unsigned long maxMessages) {

	if(_messages.size() < maxMessages)
		maxMessages = _messages.size();

    if(!_changeSinceLastSerialization && _lastSerializedMessagesCount == maxMessages) {
        return _serializedMessagesCache;
    }

	std::vector<Message> messages;
	messages.reserve(maxMessages);

	for(const auto & _message : std::ranges::reverse_view(_messages)) {
		messages.push_back(_message);
		if(messages.size() == maxMessages)
			break;
	}

	std::string serializedMessages;
	for(const auto & message : std::ranges::reverse_view(messages)) {
		serializedMessages += message.serialize() + '\n';
	}

	_serializedMessagesCache = serializedMessages;
	_changeSinceLastSerialization = false;
	_lastSerializedMessagesCount = maxMessages;
	return serializedMessages;
}

void SharedResources::clearMessages() {
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        _messages.clear();
        _changeSinceLastSerialization = true;
    }
    callBackOnResourceChange.notify_all();
}

std::condition_variable & SharedResources::getCallback() {
    return callBackOnResourceChange;
}

std::mutex & SharedResources::getMessagesMutex() {
    return messagesMutex;
}

unsigned long SharedResources::getMessagesCount() const {
	return _messages.size();
}

void SharedResources::setUserAsOnline(const std::string & name) {
	std::lock_guard<std::mutex> lock(_onlineUsersMutex);
	if(std::find(_onlineUsers.begin(), _onlineUsers.end(), name) == _onlineUsers.end())
		_onlineUsers.push_back(name);

	std::sort(_onlineUsers.begin(), _onlineUsers.end());
}

void SharedResources::setUserAsOffline(const std::string & name) {
	std::lock_guard<std::mutex> lock(_onlineUsersMutex);
	_onlineUsers.erase(std::find(_onlineUsers.begin(), _onlineUsers.end(), name));
}

const std::vector<std::string> & SharedResources::getOnlineUsers() const {
	return _onlineUsers;
}

void SharedResources::parseMessagesFile() { //TODO safe and retrieve time data
	std::string line, user, message;

	while(getline(_messagesFile, line)) {
		user = line.substr(0,line.find(':'));
		message = line.substr(line.find(':')+2);

		addMessage({user, message, std::chrono::system_clock::now()});
	}
}


