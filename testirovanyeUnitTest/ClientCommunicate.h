#ifndef CLIENT_COMMUNICATE_H
#define CLIENT_COMMUNICATE_H

#include <string>

class ClientCommunicate {
public:
    void communicate(int socket, const std::string& userDbFileName, const std::string& logFileName);
};

#endif // CLIENT_COMMUNICATE_H

