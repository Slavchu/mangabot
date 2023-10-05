#include "bot.hpp"
#include <database.hpp>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
using namespace mangabot;

Logger * Mangabot::logger = 0;
BotApi * Mangabot::bot_api = 0;
Mangabot * Mangabot::mangabot = 0;
size_t Mangabot::update_delay = 1;
ThreadManager * Mangabot::thread_manager = 0;
std::mutex fnmap_mtx;
Mangabot::Mangabot(){
    if(mangabot) return;
    try{
        try
        {
            auto t = std::time(0);
            auto tm = *std::localtime(&t);
            std::stringstream logname;
            logname << "mangabot " << std::put_time(&tm, "%d-%m-%Y") << ".log";
            Mangabot::logger = new Logger(logname.str());
        }
        catch (LogException &e)
        {
            std::cout << e.what() << std::endl;
        }
        
        SettingsImporter setting("settings.txt");
        setting.import_settings();
        update_delay = setting.get_delay();
        
        bot_api = new BotApi(setting.get_bot_id());
        mangabot = this;
       
                
    }
    catch(SettingsImporterCriticalException &e){
        if(logger){
            logger->add_log("--------------CRITICAL EXCEPTION!--------------");
            logger->add_log(e.what());
        }
        exit(-1);
    }

    
}

void Mangabot::start_bot()
{
    auto t = std::time(0);
    auto tm = *std::localtime(&t);
    std::stringstream ss;
    thread_manager = new ThreadManager();
    thread_manager->start_work();

    ss << "------------------BOT STARTED------------------" << std::endl 
    << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl 
    << "-----------------------------------------------";
    logger->add_log(ss.str());
    ss.clear();
    
    std::set<std::shared_ptr<MangaFunct>> functions;
    while(1){
        auto updates = bot_api->get_updates();
        
        fnmap_mtx.lock();

        for(auto &it :updates){
            auto tm = time(0);
            logger->add_log(std::string("New update: ") + std::to_string(it.update_id) +" "+ asctime(localtime(&tm)));
            if(next_funct.find(it.message.from) == next_funct.end()){
                next_funct[it.message.from] = std::make_shared<FirstMessage>(it);

                
            }
            else{
                next_funct[it.message.from]->set_update(it);
            }
            
            functions.insert(next_funct[it.message.from]);

        }
        
        fnmap_mtx.unlock();

        for(auto &it : functions){
            thread_manager->add_function(it);
        }

        functions.clear();

        std::this_thread::sleep_for(std::chrono::nanoseconds(update_delay));
    }
    delete thread_manager;
}

ThreadManager *mangabot::Mangabot::get_thread_manager(){
    
    return thread_manager;
}

MangaFunct::~MangaFunct(){
    delete update;
    next.reset();
}

void mangabot::MangaFunct::onTaskEnd(){
    fnmap_mtx.lock();
    
    Mangabot::get_mangabot()->next_funct.erase(update->message.from);

    if(next){
        Mangabot::get_mangabot()->next_funct[update->message.from] = next;
    }
    fnmap_mtx.unlock();
    
}

void mangabot::FirstMessage::callFunction(){
    Mangabot::get_bot_api()->send_message("Привіт! Я бот для читання та публікації манги! Виберіть дію:", update->message.chat_id, 0,{{"Публікація", "Пошук"}});
    
    next = std::make_shared<FirstMessageAnalyzer>();
}
void mangabot::FirstMessageAnalyzer::callFunction(){
    std::string message = this->update->message.text;

    if(message == "Пошук"){
        Mangabot::get_bot_api()->send_message("Ще не реалізована функція пошуку!", update->message.chat_id);
        Mangabot::get_thread_manager()->add_function( std::make_shared<FirstMessage>(*update));

    }
    else{
        Mangabot::get_bot_api()->send_message("Ще не реалізована функція пошуку!", update->message.chat_id);
        Mangabot::get_thread_manager()->add_function( std::make_shared<FirstMessage>(*update));

    }
}