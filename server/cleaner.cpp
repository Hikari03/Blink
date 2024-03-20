
#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include "Client.h"


void cleaner(std::list<Client> & clients, std::vector<std::thread> & clientRunners, const bool & turnOff, std::mutex & clientsMutex) {
    while(!turnOff) {

        std::this_thread::sleep_for(std::chrono::seconds(30));


        // critical section - clients
        {
            std::lock_guard<std::mutex> lock(clientsMutex);

            auto it = clients.begin();

            for (long unsigned int i = 0; i < clients.size(); i++, it++) {

                if (!(*it).isActive()) {
                    clientRunners[i].join();
                    clientRunners.erase(clientRunners.begin() + i);

                    std::cout << "cleaner: client number " << (*it).getSocket() << " removed" << std::endl;

                    clients.erase(it);
                }
            }
        }

    }
}