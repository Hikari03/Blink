#include "accepter.h"

bool SetSocketBlockingEnabled(int fd, bool blocking)
{
    if (fd < 0) return false;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0);
#endif
}

void accepter(std::condition_variable & callBack, const int & serverSocket, int & acceptedSocket, bool & newClientAccepted, const bool & turnOff) {

    //set to non-blocking

    bool blockSuccess = SetSocketBlockingEnabled(serverSocket, false);

    std::cout << "accepter: serverSocket is: " << (blockSuccess ? "non-blocking" : "blocking") << std::endl;

    while(true) {
        if(turnOff)
            return;

        if(!newClientAccepted) {

            if(blockSuccess) std::this_thread::sleep_for(std::chrono::seconds(1));

            acceptedSocket = accept(serverSocket, nullptr, nullptr);

            if(acceptedSocket < 0) {
                continue;
            }

            newClientAccepted = true;
            callBack.notify_one();
            std::cout << "main: accepted client number " << acceptedSocket << std::endl;
        }
    }
}