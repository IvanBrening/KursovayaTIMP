#include "ClientCommunicate.h"
#include "Error.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <user_db_file> <log_file> [port]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string userDbFile = argv[1];
    std::string logFile = argv[2];
    int port = (argc == 4) ? std::atoi(argv[3]) : 22852;

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        Error::logError("Failed to create socket", true);
        return EXIT_FAILURE;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        Error::logError("Failed to bind to port", true);
        close(serverFd);
        return EXIT_FAILURE;
    }

    if (listen(serverFd, 3) < 0) {
        Error::logError("Failed to listen on socket", true);
        close(serverFd);
        return EXIT_FAILURE;
    }

    ClientCommunicate clientComm;
    std::cout << "Server is running and waiting for connections on port " << port << "..." << std::endl;

    while (true) {
        int clientSocket = accept(serverFd, nullptr, nullptr);
        if (clientSocket < 0) {
            Error::logError("Failed to accept client connection", false);
            continue;
        }
        clientComm.communicate(clientSocket, userDbFile, logFile);
        close(clientSocket);
    }

    close(serverFd);
    return EXIT_SUCCESS;
}

