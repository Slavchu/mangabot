#include <iostream>
#include <string>
#include <curl/curl.h>
#include <thread_manager.hpp>
#include <bot_api.hpp>
#include <database.hpp>
#include <fstream>
#include <ctime>
#include <iomanip>
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
int main(int argc, char **argv){
    
    //Importing params
    //loger
    std::ofstream log("log.txt", std::ios::app);
    auto t = std::time(0);
    auto tm = *std::localtime(&t);
    BotApi * bot;
    try{
        SettingsImporter settings("settings.txt");
        settings.import_settings();

        bot = new BotApi(settings.get_bot_id());
        thread_manager = new ThreadManager;
    } catch(SettingsImporterException_NotFound &e){
        std::cout << "Critical error:";
         log << "-----------------------------\n"
        << "BOT FAILED "
        << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl
        << e.what() << std::endl
        <<"-----------------------------\n";
    }
    

    


    log << "-----------------------------\n"
        << "BOT STARTED "
        << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl
        <<"-----------------------------\n";
        
    //Getting updates   
    for (;;){
        auto updates = bot->GetUpdates();
        std::cout << "Updates : " << updates.size() << std::endl;
        if (updates.size()){
            std::cout << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;
            for (auto &it : updates){
                auto msg_anal = new MessageAnalyzer(bot, it);
                msg_anal->callFunction();
                thread_manager->add_function(msg_anal);

                std::cout << "---------Update :" << it.update_id << "-----------------";
                std::cout << "New message: \nchat_id:" << it.message.chat_id << " message_id: " << it.message.message_id << " user_id:" << it.message.from.user_id << std::endl;
                log << "New message: \nchat_id:" << it.message.chat_id << " message_id: " << it.message.message_id << " user_id:" << it.message.from.user_id << std::endl;
            }
        }
    }
}