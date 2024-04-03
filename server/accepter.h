#pragma once

#include <fcntl.h>
#include <thread>
#include <sys/socket.h>
#include <iostream>
#include <condition_variable>

void accepter(std::condition_variable & callBack, const int & serverSocket, int & acceptedSocket, bool & newClientAccepted, const bool & turnOff);