#pragma once

#include <string>
#include <vector>

enum UpdateType{
    Message,
    File,
    EditedMessage
};
struct User{
    size_t user_id;
    bool is_bot;
    std::string first_name;
    std::string last_name;
    std::string username;

};

struct Message{
    std::string text;
    size_t message_id, chat_id;
    User from;
    bool is_document;

};
struct Update{
    size_t update_id;
    struct Message message;
};
class BotApi{
    std::string BotToken;
    size_t update_offset = 0;

    public:
    BotApi(std::string token): BotToken(token){

    }
    std::vector<Update> GetUpdates();
    Update GetLastUpdate();
    void SendMassage(std::string message, size_t chat_id, size_t reply_to_message_id = 0, bool disable_notification = 0, std::string parse_mode = "");
    int ForwardMessage(size_t chat_id, size_t from_chat_id, size_t message_id);
};