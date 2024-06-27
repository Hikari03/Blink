
#include "terminal.h"

void printHelp(){
	std::cout << "q - quit server" << std::endl;
	std::cout << "'help' to list all commands" << std::endl;
	std::cout << "'list' to list all users" << std::endl;
	std::cout << "'kick <name>' to kick a user" << std::endl;
}

void terminal(std::condition_variable & callBack, std::list<Client> & clients, bool & turnOff){
	printHelp();
    while(true) {
        std::string input;
        std::cin >> input;
        if(input == "q") {
            turnOff = true;
            std::cout << "turning off server, please wait up to 1 minute" << std::endl;
            callBack.notify_one();
            return;
        }

		if(input == "help") {
			printHelp();
		}

		if(input == "list") {
			std::cout << "Users:" << std::endl;
			for (auto &client: clients) {
				std::cout << client.getName() << std::endl;
			}
			std::cout << "-----------------" << std::endl;
		}

		if(input == "kick") {
			std::string name;

			std::cin >> name;

			for (auto &client: clients) {
				if (client.getName() == name) {
					client.exit();
					break;
				}
			}
		}
    }
}