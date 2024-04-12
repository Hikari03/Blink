#include "MessageHolder.h"

MessageHolder::MessageHolder(std::mutex &messagesMutex)  : messagesMutex(messagesMutex) {}

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

std::string MessageHolder::serializeMessages() {
    if(!_changeSinceLastSerialization) {
        return _serializedMessagesCache;
    }
    std::string serializedMessages;
    for(const auto & message : _messages) {
        serializedMessages += message.serialize() + "\n";
    }
    _serializedMessagesCache = serializedMessages;
    _changeSinceLastSerialization = false;
    return serializedMessages;
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
