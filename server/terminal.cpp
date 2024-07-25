
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
	printT("'ipban <name>' to ban a user by ip");
	printT("'ipunban <name>' to unban a user by ip");
	printT("'listipban' to list all banned ips");
}

void terminal(std::condition_variable & callBack, std::list<Client> & clients, std::map<std::string,std::string> & bannedIps, bool & turnOff){
	printHelp();
    while(true) {
        std::string input;
        std::cin >> input;
        if(input == "q") {
            turnOff = true;
            printT("turning off server, please wait up to 15 seconds");
            callBack.notify_one();
            return;
        }

		if(input == "help") {
			printHelp();
		}

		if(input == "list") {
			printT("Users:");
			for (auto &client: clients) {
				printT(client.info().name);
			}
			printT("-----------------");
		}

		if(input == "kick") {
			std::string name;

			std::cin >> name;

			bool found = false;

			for (auto &client: clients) {
				if (client.info().name == name) {
					client.exit();
					found = true;
					break;
				}
			}

			if(!found) {
				printT("User not found");
			}
		}

		if(input == "ipban") {
			std::string name;
			std::cin >> name;

			bool found = false;

			for (auto &client: clients) {
				if (client.info().name == name) {
					bannedIps[name] = client.info().ip;
					client.exit();
					found = true;
					break;
				}
			}

			if(!found) {
				printT("User not found");
			}
		}

		if(input == "ipunban") {
			std::string name;
			std::cin >> name;

			auto it = bannedIps.find(name);

			if(it != bannedIps.end()) {
				bannedIps.erase(it);
			} else {
				printT("User not found");
			}
		}

		if(input == "listipban") {
			printT("Banned IPs:");
			for (auto &ip: bannedIps) {
				printT(ip.first + " - " + ip.second);
			}
			printT("-----------------");
		}
    }
}