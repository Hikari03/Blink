
#include "cleaner.h"

void cleaner(std::list<Client> & clients, const bool & turnOff, std::mutex & clientsMutex) {
    while(!turnOff) {

        std::this_thread::sleep_for(std::chrono::seconds(30));

        printf("cleaner: cleaning clients\n");

        // critical section - clients
        {
            std::lock_guard<std::mutex> lock(clientsMutex);

            auto it = clients.begin();

            while (it != clients.end()) {
                printf("cleaner: checking client number %d, that is %s\n", (*it).getSocket(), ((*it).isActive() ? "active" : "inactive"));

                if (!(*it).isActive()) {
					std::cout << "cleaner: client number " << (*it).getSocket() << " removed" << std::endl;
                    it = clients.erase(it);
                } else ++it;
            }
        }

    }
}