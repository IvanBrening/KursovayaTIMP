#ifndef CLIENTCOMMUNICATE_H
#define CLIENTCOMMUNICATE_H

#include <string>
#include <vector>

// Структура для хранения разобранного сообщения
struct ParsedMessage {
    std::string login;
    std::string salt;
    std::string hash;
};

class ClientCommunicate {
public:
    void communicate(int socket, const std::string& userDbFileName, const std::string& logFileName);
    static ParsedMessage parseMessage(const std::string& message);

private:

};

#endif
