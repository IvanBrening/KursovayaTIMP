#ifndef CONNECTTOBASE_H
#define CONNECTTOBASE_H

#include <string>

class ConnectToBase {
public:
    bool authenticateUser(const std::string& login, const std::string& salt, const std::string& clientHash, const std::string& dbFileName);

private:
    std::string hashPassword(const std::string& password, const std::string& salt);
    bool compareHashes(const std::string& serverHash, const std::string& clientHash);
};

#endif
