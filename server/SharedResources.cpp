#include "SharedResources.h"

SharedResources::SharedResources(std::mutex &messagesMutex)  : messagesMutex(messagesMutex) {}

SharedResources::~SharedResources() {
	callBackOnResourceChange.notify_all();
}

void SharedResources::addMessage(const Message & message) {
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        _messages.insert(message);
        _changeSinceLastSerialization = true;
    }
    callBackOnResourceChange.notify_all();
}

void SharedResources::removeMessage(const Message & message) {
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


