#ifndef CLIENT_COMMUNICATE_H
#define CLIENT_COMMUNICATE_H

#include <string>

class ClientCommunicate {
public:
    bool parseMessage(const std::string& message, std::string& login, std::string& salt, std::string& hash);
};

#endif // CLIENT_COMMUNICATE_H

