#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include "Client.h"

void cleaner(std::list<Client> & clients, const bool & turnOff, std::mutex & clientsMutex);