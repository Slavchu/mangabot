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
    bool operator<(const User& usr) const{
        return user_id < usr.user_id;
    }
    bool operator==(const User &usr) const{
        return user_id == usr.user_id;
    }
};

struct Message{
    std::string text;
    size_t message_id, chat_id;
    User from;
    bool is_document;
    size_t date;
};
struct Update{
    size_t update_id;
    struct Message message;
};
class BotApi{
    std::string BotToken;
    size_t update_offset = 0;

    public:
    BotApi(std::string token);
    std::vector<Update> get_updates();
    void send_message(std::string message, size_t chat_id, size_t reply_to_message_id = 0, std::vector<std::vector<std::string>> buttons = {}, bool one_time_keyboard = true, bool disable_notification = 0, std::string parse_mode = "");
    void send_remove_keyboard(std::string message, size_t chat_id, size_t reply_to_message_id = 0,bool disable_notification = 0, std::string parse_mode = "" );
    void forward_message(size_t chat_id, size_t from_chat_id, size_t message_id);
};