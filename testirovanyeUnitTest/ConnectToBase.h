#ifndef CONNECT_TO_BASE_H
#define CONNECT_TO_BASE_H

#include <string>

class ConnectToBase {
public:
    bool authenticateUser(const std::string& login, const std::string& salt, const std::string& password, const std::string& dbFileName);
    std::string hashPassword(const std::string& password, const std::string& salt);
private:
    bool compareHashes(const std::string& serverHash, const std::string& clientHash);
};

#endif // CONNECT_TO_BASE_H

