#ifndef CONNECTTOBASE_H
#define CONNECTTOBASE_H

#include <string>
#include <unordered_map>

class ConnectToBase {
public:
    explicit ConnectToBase(const std::string& dbFileName);
    bool authenticate(const std::string& login, const std::string& salt, const std::string& clientHash);
    std::string hashPassword(const std::string& password, const std::string& salt);
private:
    std::unordered_map<std::string, std::string> users;
    void loadDatabase(const std::string& dbFileName);
    
};

#endif // CONNECTTOBASE_H

