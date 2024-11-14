#include "ClientCommunicate.h"

bool ClientCommunicate::parseMessage(const std::string& message, std::string& login, std::string& salt, std::string& hash) {
    if (message.size() < 60) return false;
    login = message.substr(0, 4);
    salt = message.substr(4, 16);
    hash = message.substr(20);
    return true;
}
