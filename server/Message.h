#pragma once

#include <string>
#include <chrono>

class Message {

public:
    Message(std::string username, std::string message, const std::chrono::time_point<std::chrono::system_clock> & time);

    bool operator < (const Message & other) const;

private:
    std::string _username;
    std::string _message;
    std::chrono::time_point<std::chrono::system_clock> _time;
};
