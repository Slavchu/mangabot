#include <iostream>
#include <string>
#include <curl/curl.h>
#include "thread_manager.hpp"
#include <bot_api.hpp>
#include <database.hpp>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <logger.hpp>
#include <sstream>
#include "database.hpp"

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
                bot->send_message("Влад, заєбав", update.message.chat_id, 0, {{"hentai", "HENTAI"}});
            else{
                bot->send_message("Ok", update.message.chat_id, 0, {{"uhh", "uhhhhhhhhhh"}});
            }
        }
        else{
            bot->forward_message(1240898707, update.message.chat_id, update.message.message_id);
        }
    }
    virtual void onTaskEnd() override{

    }
};
int main(int argc, char **argv){
    unsigned int delay = 0;

    //loger
    Logger log;
    auto t = std::time(0);
    auto tm = *std::localtime(&t);
    BotApi * bot;
    
    
    std::stringstream ss;
    //Importing params

    try{
        SettingsImporter settings("settings.txt");
        settings.import_settings();

        bot = new BotApi(settings.get_bot_id());
    } catch(SettingsImporterCriticalException &e){
        std::cerr << "Critical error:" << e.what() << std::endl; 
        ss << "-----------------------------\n"
        << "BOT FAILED "
        << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl
        << e.what() << std::endl
        <<"-----------------------------\n";
        log.add_log(ss.str());
        ss.clear();
        return -1;
    } 
    
    auto thread_manager = new ThreadManager;
    


    ss << "-----------------------------\n"
        << "BOT STARTED "
        << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl
        <<"-----------------------------\n";

    log.add_log(ss.str());
    ss.clear();
    ss << "-----------------------------\n"
        << "BOT STARTED "
        << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl
        <<"-----------------------------\n";
    ss.clear();
    log.add_log(ss.str());

    //Getting updates   
    thread_manager->start_work();
    for (;;){
        auto updates = bot->get_updates();
        std::cout << "Updates : " << updates.size() << std::endl;
        if (updates.size()){
            std::cout << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;
            for (auto &it : updates){
                auto msg_anal = new MessageAnalyzer(bot, it);
                thread_manager->add_function(msg_anal);
                //msg_anal->callFunction();
                std::cout << "---------Update :" << it.update_id << "-----------------\n";
                std::cout << "New message: \nchat_id:" << it.message.chat_id << " message_id: " << it.message.message_id << " user_id:" << it.message.from.user_id << std::endl;
                ss << "New message: \nchat_id:" << it.message.chat_id << " message_id: " << it.message.message_id << " user_id:" << it.message.from.user_id << std::endl;
                log.add_log(ss.str());
                ss.clear();

            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        
    }
    delete thread_manager;
}