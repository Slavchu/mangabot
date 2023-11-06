#pragma once

#include <string>
#include <vector>
#include <memory>
class IKeyboard{
    public:
    virtual ~IKeyboard() = default;
    virtual std::string reply_markup() = 0;
};
class ReplyKeyboard : public IKeyboard{
    std::vector<std::vector<std::string>> keyboard;
    bool one_time_keyboard;
    virtual std::string reply_markup() override;

    public:

    ReplyKeyboard(std::vector<std::vector<std::string>> keyboard, bool one_time_keyboard = true){
        this->keyboard = keyboard;
        this->one_time_keyboard = one_time_keyboard;
    }
};
struct InlineButton{
    InlineButton(const std::string& text, const std::string&  url, const std::string& callback_data){
        this->text = text; this->url = url; this->callback_data = callback_data;
    }
    std::string text;
    std::string url;
    std::string callback_data;
    InlineButton() = default;
  
};
class InlineKeyboard: public IKeyboard{
    std::vector<std::vector<InlineButton>> keyboard;
    
    public:
    virtual std::string reply_markup() override;
    InlineKeyboard(std::vector<std::vector<InlineButton>> keyboard){
        this->keyboard = keyboard;
    }
};
struct File{

    std::string file_id;
    std::string filename;
    
    std::string  binary;
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
    bool is_document;
    InlineKeyboard * keyboard;
    size_t date;
    std::shared_ptr<File> file;

};
struct CallbackQuery{

    std::string query_data;
    size_t from;
    std::shared_ptr <Message> message;


};
struct Update{
    size_t update_id;
    User from;
    std::shared_ptr<struct Message> message;
    std::shared_ptr<CallbackQuery> callback_query;
    ~Update(){
        message.reset();
        callback_query.reset();
    }
};

class BotApi{
    std::string BotToken;
    size_t update_offset = 0;
    private:
    std::string file_downloader(std::string file_path, size_t size);
    public:
    BotApi(std::string token);
    std::vector<std::shared_ptr<Update>> get_updates();

    void send_message(const std::string& message, 
                      size_t chat_id, 
                      size_t reply_to_message_id = 0, 
                      std::shared_ptr<IKeyboard> keyboard = 0, 
                      bool disable_notification = 0, 
                      const std::string&parse_mode = "");

    void send_remove_keyboard(const std::string& message, 
                              size_t chat_id, 
                              size_t reply_to_message_id = 0,
                              bool disable_notification = 0, 
                              const std::string& parse_mode = "" );
    void forward_message(size_t chat_id, size_t from_chat_id, size_t message_id);

    void edit_message_text(const std::string& message, 
                            size_t chat_id, 
                            size_t message_id, 
                            std::shared_ptr<InlineKeyboard> keyboard = 0, 
                            const std::string& parse_mode = "");

    void download_document(File &file);

};