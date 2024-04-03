
#include "terminal.h"

void terminal(std::condition_variable & callBack, bool & turnOff){
    while(true) {
        std::string input;
        std::cout << "Type 'q' to turn off server" << std::endl;
        std::cin >> input;
        if(input == "q") {
            turnOff = true;
            std::cout << "turning off server, please wait up to 1 minute" << std::endl;
            callBack.notify_one();
            return;
        }
    }
}