#ifndef CLIENTCOMMUNICATE_H
#define CLIENTCOMMUNICATE_H

#include <string>

class ClientCommunicate {
public:
    bool parseMessage(const std::string& msg, std::string& login, std::string& salt, std::string& hash);
};

#endif // CLIENTCOMMUNICATE_H

