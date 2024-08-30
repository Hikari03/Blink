#pragma once

#include <set>
#include <condition_variable>
#include <ranges>
#include <fstream>
#include "Message.h"

class SharedResources {
public:

    explicit SharedResources(std::mutex & messagesMutex);

	~SharedResources();

    void addMessage(const Message & message);

    void removeMessage(const Message & message);

	[[nodiscard]] unsigned long getMessagesCount() const;

    [[nodiscard]] std::set<Message> getMessages() const;

    std::string serializeMessages(unsigned long maxMessages);

    void clearMessages();

    [[nodiscard]] std::condition_variable & getCallback();

    [[nodiscard]] std::mutex & getMessagesMutex();

	void setUserAsOnline(const std::string & name);
	void setUserAsOffline(const std::string & name);
	[[nodiscard]] const std::vector<std::string> & getOnlineUsers() const;


private:
    std::set<Message> _messages;
	std::vector<std::string> _onlineUsers;
    std::condition_variable callBackOnResourceChange;
    std::mutex & messagesMutex;
	std::mutex _onlineUsersMutex;
	std::fstream _messagesFile;
	std::string _messagesFileName = "persistent/messagesFile";

    std::string _serializedMessagesCache;
    bool _changeSinceLastSerialization = false;
	bool inited = false;
	unsigned long _lastSerializedMessagesCount = 0;

	void parseMessagesFile();
};
