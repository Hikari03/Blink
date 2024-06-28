
#include "terminal.h"

#define _terminal "terminal: "

void printT(const std::string& message){
	std::cout << _terminal << message << std::endl;
}

void printHelp(){
	printT("'q' - quit server");
	printT("'help' - list all commands");
	printT("'list' to list all users");
	printT("'kick <name>' to kick a user");
}

void terminal(std::condition_variable & callBack, std::list<Client> & clients, bool & turnOff){
	printHelp();
    while(true) {
        std::string input;
        std::cin >> input;
        if(input == "q") {
            turnOff = true;
            printT("turning off server, please wait up to 1 minute");
            callBack.notify_one();
            return;
        }

		if(input == "help") {
			printHelp();
		}

		if(input == "list") {
			printT("Users:");
			for (auto &client: clients) {
				printT(client.getName());
			}
			printT("-----------------");
		}

		if(input == "kick") {
			std::string name;

			std::cin >> name;

			bool found = false;

			for (auto &client: clients) {
				if (client.getName() == name) {
					client.exit();
					found = true;
					break;
				}
			}

			if(!found) {
				printT("User not found");
			}
		}
    }
}