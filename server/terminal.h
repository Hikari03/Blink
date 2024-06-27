#pragma once

#include <iostream>
#include <condition_variable>
#include <list>
#include "Client.h"

void terminal(std::condition_variable & callBack, std::list<Client> & clients, bool & turnOff);