#include <iostream>
#include <netinet/in.h>
#include <csignal>

#include "Client.h"
#include "accepter.h"
#include "cleaner.h"
#include "terminal.h"
#include "SharedResources.h"

sig_atomic_t stopRequested = 0;
std::condition_variable callBack;

// used for graceful shutdown in docker
void signalHandler(int signal) {
	std::cout << "Interrupt signal (" << signal << ") received." << std::endl;
	if (signal == SIGINT || signal == SIGTERM) {
		stopRequested = 1;
	}

	callBack.notify_one();

}


int main() {

	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);

    std::mutex clientsMutex;
    std::mutex messagesMutex;

    std::cout << "main: starting server" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress = {AF_INET,
                                 htons(6999),
                                 INADDR_ANY,
								 {0}};

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    std::vector<std::thread> clientRunners;
    // holds chat
    SharedResources sharedResources(messagesMutex);
    std::list<Client> clients;
	std::map<std::string,std::string> bannedIps;

    bool turnOff = false;

    std::thread terminalThread(terminal, std::ref(callBack), std::ref(clients), std::ref(bannedIps), std::ref(turnOff));
    std::thread cleanerThread(cleaner, std::ref(clients), std::ref(turnOff), std::ref(clientsMutex));

    listen(serverSocket, 10);

    bool newClientAccepted = false;
    ClientInfo acceptedClient;


    std::thread accepterThread(accepter, std::ref(callBack), std::ref(serverSocket), std::ref(acceptedClient), std::ref(newClientAccepted), std::ref(turnOff));

    std::cout << "main: entering main loop, server started" << std::endl;

    /*
     * ########################################################################################
     * ################################## MAIN LOOP ###########################################
     * ########################################################################################
     */

    while(true) {

        // critical section - clients

        std::unique_lock<std::mutex> lock(clientsMutex);
        callBack.wait(lock);

        if(newClientAccepted) {

			// check if they aren't ip banned
			for(const auto & it : bannedIps) {
				if(it.second == acceptedClient.ip) {
					std::cout << "main: client number " << acceptedClient.socket_ << " with ip " << acceptedClient.ip << " is banned" << std::endl;
					std::string message = _internal"ban";
					message += _end;
					send(acceptedClient.socket_, message.c_str(), message.length(), 0);
					close(acceptedClient.socket_);
					newClientAccepted = false;
					continue;
				}
			}
			if(!newClientAccepted)
				continue;

            // create client
            clients.emplace_back(std::move(acceptedClient), sharedResources);

            // run client
            clientRunners.emplace_back(&Client::run, &clients.back());

            newClientAccepted = false;
        }

		if(stopRequested)
			turnOff = true;


        if(turnOff) {
			// cleanup
			lock.unlock();
			std::cout << "main: cleaning up threads" << std::endl;
			if(stopRequested) {
				std::cout << "main: stop requested" << std::endl;
				pthread_cancel(terminalThread.native_handle());
			}
			else
				terminalThread.join();

			std::cout << "main: terminal closed" << std::endl;
			shutdown(serverSocket, SHUT_RDWR);
			close(serverSocket);
			accepterThread.join();
			std::cout << "main: accepter closed" << std::endl;
			std::cout << "main: waiting for clients to close" << std::endl;
			for (auto &client: clients) {
				client.exit();
			}
			for (auto &clientRunner: clientRunners) {
				clientRunner.join();
			}
			try {
			cleanerThread.join();
			} catch (std::exception & e) {
				std::cout << "main: cleaner exception | " << e.what() << std::endl;
			}
			std::cout << "main: cleaner closed" << std::endl;
            break;
        }

    }

    std::cout << "main: closing server" << std::endl;
	return 0;
}
