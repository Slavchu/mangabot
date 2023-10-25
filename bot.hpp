#pragma once
#define LOG_TO_CONSOLE
#include <bot_api.hpp>
#include <thread_manager.hpp>
#include <logger.hpp>
#include <database.hpp>
#include <map>
#include <telegraph.hpp>
namespace mangabot{
    using TelegraphApi = telegraph::TelegraphApi;
    class MangaFunct : public IFunct{
    protected:
        
        std::shared_ptr<Update>  update;
        std::shared_ptr<MangaFunct> next = 0;
    public:
        MangaFunct() = default;
        MangaFunct(std::shared_ptr<Update> update){this->update = update;}
        virtual ~MangaFunct();
        void onTaskEnd() override; 
        void set_update(std::shared_ptr<Update> update){
            if(!this->update)
                this->update = update;
        }
    };
    class CallbackQueryAnalyzer : public MangaFunct{
        
        public:
        CallbackQueryAnalyzer(std::shared_ptr<Update> update):MangaFunct(update){}
        void callFunction() override;
        void onTaskEnd() override{};

    };

    class FirstMessage: public MangaFunct{
        public:
        FirstMessage() = default;
        FirstMessage(std::shared_ptr<Update> update) {
            set_update(update);
        };
        virtual void callFunction() override;
        
        
        ~FirstMessage() = default;
    };
    class FirstMessageAnalyzer : public MangaFunct{
        public:
        FirstMessageAnalyzer() = default;
        FirstMessageAnalyzer(std::shared_ptr<Update> update):MangaFunct(update){};
        virtual void callFunction();
        ~FirstMessageAnalyzer() = default;

    };

    

    class Mangabot{
    
    private:
        static TelegraphApi * telegraph_api;
        static size_t update_delay;

        static BotApi * bot_api;
        static Logger * logger;
        static Mangabot * mangabot;
        static ThreadManager * thread_manager;
        std::map<User, std::shared_ptr<MangaFunct>> next_funct;
    private:


    public:
        void tick();
        void start_bot();
        static ThreadManager * get_thread_manager();
        static Mangabot* get_mangabot() {
            return mangabot;        
        }
        Mangabot();
        static TelegraphApi* get_telegraph_api(){
            return telegraph_api;
        }
        static BotApi* get_bot_api() {
            return bot_api;
        }
        ~Mangabot(){
            delete bot_api;
            delete logger;
        }
        

        friend MangaFunct;
    };
    class MangaBotStartupException: public std::exception{
        std::string excep;
        public:
        virtual const char* what() const noexcept override{
            return excep.c_str();
        }
        
    };

}