#pragma once
#include <bot_api.hpp>
#include <thread_manager.hpp>
#include <logger.hpp>
namespace mangabot{
    class Mangabot{

        static BotApi * bot_api;
        static Logger * logger;
        static Mangabot * mangabot;
        public:
        static Mangabot* get_mangabot() {
            return mangabot;        
        }
        Mangabot(){
            if(!mangabot){
                mangabot = this;
                logger = new Logger("mangabot.log");
                
            }
        }
        void set_bot_api(BotApi * bot_api);
        BotApi* get_bot_api() const{
            return bot_api;
        }
        ~Mangabot(){
            delete logger;
        }
    };

}


