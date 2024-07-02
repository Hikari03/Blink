#include "MessageHolder.h"

MessageHolder::MessageHolder(std::mutex &messagesMutex)  : messagesMutex(messagesMutex) {}

MessageHolder::~MessageHolder() {
	callBackOnMessagesChange.notify_all();
}

void MessageHolder::addMessage(const Message & message) {
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        _messages.insert(message);
        _changeSinceLastSerialization = true;
    }
    callBackOnMessagesChange.notify_all();
}

void MessageHolder::removeMessage(const Message & message) {
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        _messages.erase(message);
        _changeSinceLastSerialization = true;
    }
    callBackOnMessagesChange.notify_all();
}

[[nodiscard]] std::set<Message> MessageHolder::getMessages() const {
    return _messages;
}

std::string MessageHolder::serializeMessages(unsigned long maxMessages) {
    if(!_changeSinceLastSerialization) {
        return _serializedMessagesCache;
    }

	if(_messages.size() < maxMessages)
		maxMessages = _messages.size();

	{
		//std::lock_guard<std::mutex> lock(messagesMutex);
		std::string serializedMessages;
		auto messPtr = _messages.end();
		for(unsigned long i = 0; i < maxMessages; i++) {
			messPtr--;
		}

		for(auto it = messPtr; it != _messages.end(); it++) {
			serializedMessages += it->serialize() + "\n";
		}

		_serializedMessagesCache = serializedMessages;
		_changeSinceLastSerialization = false;
		return serializedMessages;
	}
}

void MessageHolder::clearMessages() {
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        _messages.clear();
        _changeSinceLastSerialization = true;
    }
    callBackOnMessagesChange.notify_all();
}

std::condition_variable & MessageHolder::getCallback() {
    return callBackOnMessagesChange;
}

std::mutex & MessageHolder::getMessagesMutex() {
    return messagesMutex;
}
