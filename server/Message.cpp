#include "Message.h"

#include <utility>

Message::Message(std::string username, std::string message,
                 const std::chrono::time_point<std::chrono::system_clock> &time):
        _username(std::move(username)), _message(std::move(message)), _time(time) {}

bool Message::operator<(const Message &other) const {
    return _time < other._time;
}
