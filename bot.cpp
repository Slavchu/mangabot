#include "bot.hpp"
#include <database.hpp>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
#include <memory>
using namespace mangabot;

Logger * Mangabot::logger = 0;
BotApi * Mangabot::bot_api = 0;
Mangabot * Mangabot::mangabot = 0;
size_t Mangabot::update_delay = 1;
ThreadManager * Mangabot::thread_manager = 0;

telegraph::TelegraphApi * Mangabot::telegraph_api = 0;
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
        telegraph_api = new telegraph::TelegraphApi(telegraph::TelegraphApi::create_account("fuck you"));
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

        for(auto it :updates){
            auto tm = time(0);
            logger->add_log(std::string("New update: ") + std::to_string(it->update_id) +" "+ asctime(localtime(&tm)));
            if (it->message){
                if (next_funct.find(it->from) == next_funct.end()){
                    next_funct[it->from] = std::make_shared<FirstMessage>(it);
                }
                else{
                    next_funct[it->from]->set_update(it);
                }

                functions.insert(next_funct[it->from]);
            }
            else if(it->callback_query){
                functions.insert(std::make_shared<CallbackQueryAnalyzer>(it));
            }
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
    update.reset();
    next.reset();
}

void mangabot::MangaFunct::onTaskEnd(){
    fnmap_mtx.lock();
    
    Mangabot::get_mangabot()->next_funct.erase(update->from);

    if(next){
        Mangabot::get_mangabot()->next_funct[update->from] = next;
    }
    fnmap_mtx.unlock();
    
}

void mangabot::FirstMessage::callFunction(){
    if(update->message){
        if(update->message->file){
            Mangabot::get_bot_api()->download_document(*update->message->file);
            std::string url = Mangabot::get_telegraph_api()->create_page("some_title", std::vector<std::string>{Mangabot::get_telegraph_api()->upload_image(update->message->file->binary)}, "засуджую твої фото!");
            Mangabot::get_bot_api()->send_message(url, update->message->chat_id);
        }
        InlineButton button;
        button.text = "Just a text";
        button.callback_data = "fuck";
        std::vector<std::vector<InlineButton>> but = {{button}};

        Mangabot::get_bot_api()->send_message("Привіт! Я бот для читання та публікації манги! Виберіть дію:", update->message->chat_id, 0, std::make_shared<InlineKeyboard>(but));
    }
    else{
        InlineButton button;
        button.text = "edited text";
        button.callback_data = "fuck2";
        std::vector<std::vector<InlineButton>> but = {{button}};
        Mangabot::get_bot_api()->edit_message_text("Опа, відредаговано", update->callback_query->message->chat_id, update->callback_query->message->message_id, std::make_shared<InlineKeyboard>(but));    
    }
    next = std::make_shared<FirstMessageAnalyzer>();
}
void mangabot::FirstMessageAnalyzer::callFunction(){
    if(this->update->message){
        std::string message = this->update->message->text;

        if(message == "Пошук"){
            Mangabot::get_bot_api()->send_message("Ще не реалізована функція пошуку!", update->message->chat_id);
            Mangabot::get_thread_manager()->add_function( std::make_shared<FirstMessage>(update));

        }
        else{
            Mangabot::get_bot_api()->send_message("Ще не реалізована функція пошуку!", update->message->chat_id);
            Mangabot::get_thread_manager()->add_function( std::make_shared<FirstMessage>(update));

        }
    }
}

void mangabot::CallbackQueryAnalyzer::callFunction(){
    if(!update || !update->callback_query){
        return;
    }
    if(update->callback_query->query_data == "fuck"){
        InlineButton in1; in1.text = "stfu"; in1.callback_data = "fr";
        std::vector<std::vector<InlineButton>> buttons {{in1}};

        Mangabot::get_bot_api()->edit_message_text(update->callback_query->message->text,update->callback_query->message->chat_id, update->callback_query->message->message_id, std::make_shared<InlineKeyboard>(buttons));
        return;
    }
    InlineButton in1; in1.text = "fuck"; in1.callback_data = "fuck";
    std::vector<std::vector<InlineButton>> buttons {{in1}};

    Mangabot::get_bot_api()->edit_message_text(update->callback_query->message->text,update->callback_query->message->chat_id, update->callback_query->message->message_id, std::make_shared<InlineKeyboard>(buttons));
}
