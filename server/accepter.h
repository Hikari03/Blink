#pragma once

#include <fcntl.h>
#include <thread>
#include <sys/socket.h>
#include <iostream>
#include <condition_variable>
#include <netinet/in.h>
#include "ClientInfo.h"

void accepter(std::condition_variable & callBack, const int & serverSocket, ClientInfo & acceptedClient, bool & newClientAccepted, const bool & turnOff);