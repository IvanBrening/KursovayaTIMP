#ifndef CONNECT_TO_BASE_H
#define CONNECT_TO_BASE_H

#include <string>

class ConnectToBase {
public:
    explicit ConnectToBase(const std::string& dbFileName);
    bool authenticate(const std::string& login, const std::string& salt, const std::string& hash);
    std::string hashPassword(const std::string& password, const std::string& salt);
private:
    std::string dbFileName;
};

#endif // CONNECT_TO_BASE_H

