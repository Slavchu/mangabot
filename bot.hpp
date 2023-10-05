#pragma once
#define LOG_TO_CONSOLE
#include <bot_api.hpp>
#include <thread_manager.hpp>
#include <logger.hpp>
#include <database.hpp>
#include <map>
namespace mangabot{
    
    class MangaFunct : public IFunct{
    protected:
        
        Update * update;
        std::shared_ptr<MangaFunct> next = 0;
    public:
        MangaFunct() = default;
        MangaFunct(Update update){this->update = new Update(update);}
        virtual ~MangaFunct();
        void onTaskEnd() override; 
        void set_update(Update update){
            if(!this->update)
                this->update = new Update(update);
        }
    };


    class FirstMessage: public MangaFunct{
        public:
        FirstMessage() = default;
        FirstMessage(Update update) : MangaFunct(update){};
        virtual void callFunction() override;
        
        
        ~FirstMessage() = default;
    };
    class FirstMessageAnalyzer : public MangaFunct{
        public:
        FirstMessageAnalyzer() = default;
        FirstMessageAnalyzer(Update update):MangaFunct(update){};
        virtual void callFunction();
        ~FirstMessageAnalyzer() = default;

    };

    

    class Mangabot{
    
    private:
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