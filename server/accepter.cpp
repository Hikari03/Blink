#include "accepter.h"



void accepter(std::condition_variable & callBack, const int & serverSocket, int & acceptedSocket, bool & newClientAccepted, const bool & turnOff) {

	while(true) {
        if(turnOff)
            return;

        if(!newClientAccepted) {

            acceptedSocket = accept(serverSocket, nullptr, nullptr);
			if(turnOff)
				return;

            if(acceptedSocket < 0) {
                continue;
            }

            newClientAccepted = true;
            callBack.notify_one();
            std::cout << "main: accepted client number " << acceptedSocket << std::endl;
        }
    }
}