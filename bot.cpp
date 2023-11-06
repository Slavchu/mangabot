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
                
                std::string message_type = (it->message->file) ? "FILE" : "TEXT";

                logger->add_log(std::string("Message. Type: ") + message_type + "\nFrom:" + std::to_string(it->from.user_id));
                if (next_funct.find(it->from) == next_funct.end()){
                    next_funct[it->from] = std::make_shared<CreatePage>(it, it->from.username);
                }
                else{
                    next_funct[it->from]->set_update(it);
                }

                functions.insert(next_funct[it->from]);
            }
            else if(it->callback_query){
                logger->add_log("Callback query:" + it->callback_query->query_data);
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
    
    Mangabot::get_instance()->next_funct.erase(update->from);

    if(next){
        Mangabot::get_instance()->next_funct[update->from] = next;
    }
    fnmap_mtx.unlock();
    
}

void mangabot::FirstMessage::callFunction(){
    if(update->message){
        if(update->message->file){
            Mangabot::get_bot_api_instance()->download_document(*update->message->file);
            std::string url = Mangabot::get_telegraph_api_instance()->create_page("some_title", std::vector<std::string>{Mangabot::get_telegraph_api_instance()->upload_image(update->message->file->binary)}, "засуджую твої фото!");
            Mangabot::get_bot_api_instance()->send_message(url, update->message->chat_id);
        }
        
        
        std::vector<std::vector<std::string>> buttons {{"Профіль", "Додати мангу"},{"Пошук"}};
        Mangabot::get_bot_api_instance()->send_message("Привіт! Я бот для читання та публікації манги! Виберіть дію:", update->message->chat_id, 0, std::make_shared<ReplyKeyboard>(buttons));
    }
    
    next = std::make_shared<FirstMessageAnalyzer>();
}
void mangabot::FirstMessageAnalyzer::callFunction(){
    if(this->update->message){
        std::string message = this->update->message->text;

        if(message == "Пошук"){
            Mangabot::get_bot_api_instance()->send_message("Ще не реалізована функція пошуку!", update->message->chat_id);
            Mangabot::get_thread_manager()->add_function( std::make_shared<FirstMessage>(update));

        }
        else{
            Mangabot::get_bot_api_instance()->send_message("Ще не реалізована функція пошуку!", update->message->chat_id);
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

        Mangabot::get_bot_api_instance()->edit_message_text(update->callback_query->message->text,update->callback_query->message->chat_id, update->callback_query->message->message_id, std::make_shared<InlineKeyboard>(buttons));
        return;
    }
    InlineButton in1; in1.text = "fuck"; in1.callback_data = "fuck";
    std::vector<std::vector<InlineButton>> buttons {{in1}};

    Mangabot::get_bot_api_instance()->edit_message_text(update->callback_query->message->text,update->callback_query->message->chat_id, update->callback_query->message->message_id, std::make_shared<InlineKeyboard>(buttons));
}

void mangabot::CreatePage::callFunction(){
    if(update){
        if(update->message->text == "Скасувати") return;
        if(update->message->text == "Завершити"){
            if(!images || !images->size()) return;
            auto page = Mangabot::get_telegraph_api_instance()->create_page(title, *images, "text" );
            Mangabot::get_bot_api_instance()->send_message(page,update->message->chat_id);
            images.reset();
            return;
        }
        if(!images){
            
            images = std::make_shared<std::vector<std::string>>();
            std::vector<std::vector<std::string>>buttons;
            buttons = {{"Завершити", "Скасувати"}};
            std::shared_ptr<IKeyboard> keyboard = std::make_shared<ReplyKeyboard>(buttons);
            Mangabot::get_bot_api_instance()->send_message("Відправляйте сторінки по порядку", update->message->chat_id, 0, keyboard);
            next = std::make_shared<CreatePage>(images, title);
            return;

        }
        if(!update->message->file) {
            next = std::make_shared<CreatePage>(images, title);    
            return;
        }
        
        Mangabot::get_bot_api_instance()->download_document(*update->message->file);
        images->push_back(Mangabot::get_telegraph_api_instance()->upload_image(update->message->file->binary));
        next = std::make_shared<CreatePage>(images, title);
        images.reset();
    }
}
