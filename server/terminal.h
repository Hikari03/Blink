#pragma once

#include <iostream>
#include <condition_variable>
#include <list>
#include <map>
#include "Client.h"

void terminal(std::condition_variable & callBack, std::list<Client> & clients, std::map<std::string,std::string> & bannedIps, bool & turnOff);