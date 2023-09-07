#include <iostream>
#include <string>
#include <curl/curl.h>
#include <thread_manager.hpp>
#include <bot_api.hpp>
ThreadManager *th = NULL;
class MessageAnalyzer: public IFunct{
    BotApi * bot;
    struct Update update;
    public:
    MessageAnalyzer(BotApi * bot, Update update){
        this->bot = bot;
        this->update = update;
    }
    virtual void callFunction() override{
        bot->SendMassage(update.message.text, update.message.chat_id);
    }
    virtual void onTaskEnd() override{

    }
};
int main(){
    BotApi * bot = new BotApi("6458804621:AAHJCmmUNXPQIyQaASR8LFsabxJgVkpC2wA");
    
    for (;;){
        auto updates = bot->GetUpdates();
        std::cout << "Updates : " << updates.size() << std::endl;

        for (auto &it : updates){
            std::cout << "id " << it.update_id << std::endl;
            std::cout << "from " << it.message.from.first_name << std::endl;
            std::cout << "text " << it.message.text << std::endl;
            std::cout << "is document " << std::boolalpha << it.message.is_document << std::endl; 
            std::cout << "\n\n";
            
            auto msg_anal = new MessageAnalyzer(bot, it);
            msg_anal->callFunction();
        }
    }
}