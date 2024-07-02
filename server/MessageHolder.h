#pragma once

#include <set>
#include <condition_variable>
#include "Message.h"

class MessageHolder {
public:

    explicit MessageHolder(std::mutex & messagesMutex);

	~MessageHolder();

    void addMessage(const Message & message);

    void removeMessage(const Message & message);

    [[nodiscard]] std::set<Message> getMessages() const;

    std::string serializeMessages(unsigned long maxMessages);

    void clearMessages();

    [[nodiscard]] std::condition_variable & getCallback();

    [[nodiscard]] std::mutex & getMessagesMutex();


private:
    std::set<Message> _messages;
    std::condition_variable callBackOnMessagesChange;
    std::mutex & messagesMutex;

    std::string _serializedMessagesCache;
    bool _changeSinceLastSerialization = false;
};
