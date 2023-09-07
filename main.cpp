#include <iostream>
#include <string>
#include <curl/curl.h>
#include <thread_manager.hpp>
#include <bot_api.hpp>
ThreadManager *thread_manager = NULL;
class MessageAnalyzer: public IFunct{
    BotApi * bot;
    struct Update update;
    public:
    MessageAnalyzer(BotApi * bot, Update update){
        this->bot = bot;
        this->update = update;
    }
    virtual void callFunction() override{
        if(!update.message.is_document){
            if(update.message.text == "hentai")
                bot->SendMassage("Влад, заєбав", update.message.chat_id);
            else
                bot->SendMassage(update.message.text, update.message.chat_id);
        }
        else{
            bot->ForwardMessage(1240898707, update.message.chat_id, update.message.message_id);
        }
    }
    virtual void onTaskEnd() override{

    }
};
int main(){
    BotApi * bot = new BotApi("6458804621:AAHJCmmUNXPQIyQaASR8LFsabxJgVkpC2wA");
    thread_manager = new ThreadManager;
    for (;;){
        auto updates = bot->GetUpdates();
        std::cout << "Updates : " << updates.size() << std::endl;
        
        for (auto &it : updates){
            auto msg_anal = new MessageAnalyzer(bot, it);
            msg_anal->callFunction();
            thread_manager->add_function(msg_anal);
            std::cout << "id " << it.update_id << std::endl;
            std::cout << "from " << it.message.from.first_name << std::endl;
            std::cout << "text " << it.message.text << std::endl;
            std::cout << "is document " << std::boolalpha << it.message.is_document << std::endl; 
            std::cout << "\n\n";
            
            
        }
    }
}