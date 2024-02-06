/*
    Hey! Don't go here
    Only God and me know what's happening here. 
    Now I forgot it, so only God knows
*/
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


       /*   TODO
            1.Авторизація в базі даних
            2.Створення та авторизація в telegraph api
       */
        
        
    }
    catch(SettingsImporterCriticalException &e){
        if(logger){
            logger->add_log("--------------CRITICAL EXCEPTION!--------------");
            logger->add_log(e.what());
        }
        exit(-1);
    }

    
}

//Тут не трогати, тут стращно і може йобнути абсолютно все
//Але на майбутнє - чистити із пам'яті тих користувачів, що очікують на повідомлення овер n часу. Оперативку шкода
void Mangabot::start_bot(){
    
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
                    next_funct[it->from] = std::make_shared<FirstMessage>(it);
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
//Це також не трогати
void mangabot::MangaFunct::onTaskEnd(){
    fnmap_mtx.lock();
    
    Mangabot::get_instance()->next_funct.erase(update->from);

    if(next){
        Mangabot::get_instance()->next_funct[update->from] = next;
    }
    fnmap_mtx.unlock();
    
}

//В майбутньому було б добре пам'ятати стан з останнього запуску бота
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

//В стані активного розвитку!
void mangabot::FirstMessageAnalyzer::callFunction(){
    if(this->update->message){
        std::string message = this->update->message->text;

        if(message == "Пошук"){
            Mangabot::get_bot_api_instance()->send_message("Ще не реалізована функція пошуку!", update->message->chat_id);
            Mangabot::get_thread_manager()->add_function( std::make_shared<FirstMessage>(this->update));

        }
        else if(message == "Профіль"){
            std::stringstream ss;
            
            
            CategoryUser usr((this->update->from.user_id));
            

            if(!this->update->from.username.empty())
                ss << '@'<< this->update->from.username << std::endl << std::endl;
            else if(!this->update->from.first_name.empty())
                ss << this->update->from.first_name << std::endl << std::endl;
            else if(!this->update->from.last_name.empty())
                ss << this->update->from.last_name << std::endl << std::endl;
            
            ss << "В процесі:"; 
            ss << "undef" << std::endl; //переписати після реалізації функцій в CategoryUser
            ss << "Прочитано:";
            ss << 0 << std::endl;
            ss << "В планах:";
            ss << "0" << std::endl;
            
            InlineButton b1("В процесі", "" , "READING_" + std::to_string(this->update->from.user_id));
            InlineButton b2("Прочитані", "" , "ALREADY_READ_" + std::to_string(this->update->from.user_id));
            InlineButton b3("В планах", "" , "PLANING_" + std::to_string(this->update->from.user_id));
            std::vector<std::vector<InlineButton>> buttons  = {{b1}, {b2}, {b3}};
            Mangabot::get_bot_api_instance()->send_message(ss.str(), this->update->message->chat_id, 0, std::make_shared<InlineKeyboard>(buttons));
            next = std::make_shared<FirstMessageAnalyzer>();
        }
        else if(message == "Додати мангу"){
            
            /*
                Реалізувати пошук юзера, перевірити його team_id, якщо 0, то виводити, що він не є членом команди
            */
            CategoryUser usr (this->update->from.user_id);
            if(usr.team_id)
                Mangabot::get_thread_manager()->add_function(std::make_shared<CreatePage>(this->update, "Page name"));
            else{
                Mangabot::get_bot_api_instance()->send_message("Ви не є членом комуністичної партії", this->update->message->chat_id);

            }
        }
        else{
            Mangabot::get_bot_api_instance()->send_message("404", update->message->chat_id);
            Mangabot::get_thread_manager()->add_function( std::make_shared<FirstMessage>(update));
        }
    }
}

void mangabot::CallbackQueryAnalyzer::callFunction(){
    if(!update || !update->callback_query){
        return;
    }
    if(update->callback_query->query_data.find("ALREADY_READ") != std::string::npos){
        Mangabot::get_bot_api_instance()->edit_message_text("Не реалізовано! Вибачте:(",update->callback_query->message->chat_id, update->callback_query->message->message_id);
        return;
    }
   Mangabot::get_bot_api_instance()->edit_message_text("Не реалізовано! Вибачте:(",update->callback_query->message->chat_id, update->callback_query->message->message_id);
}

void mangabot::CreatePage::callFunction(){
    if(update){
        if(update->message->text == "Скасувати"){
            Mangabot::get_thread_manager()->add_function(std::make_shared<FirstMessage>(this->update));
            return;
        }
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
        if (!update->message->file) {
          next = std::make_shared<CreatePage>(images, title);
          return;
        }

        Mangabot::get_bot_api_instance()->download_document(*update->message->file);
        images->push_back(Mangabot::get_telegraph_api_instance()->upload_image(update->message->file->binary));
        
        next = std::make_shared<CreatePage>(images, title);
        images.reset();
    }
}
