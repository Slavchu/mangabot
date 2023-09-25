#pragma once
#include <exception>
#include <string>

class MangaDatabaseException_NotFound : public std::exception{
    public:
    virtual const char* what() const throw(){
        return "Manga database file not found";
    }
};
struct MangaLocation{
    size_t chat_id;
    size_t message_id;
};
class MangaDatabase{
    std::string database_location;
    public:
    MangaDatabase(std::string filelocation): database_location(filelocation){
        

    }
    MangaLocation find_manga (std::string name) const;
    void add_manga(std::string name, MangaLocation msg);
};
class SettingsImporterCriticalException: public std::exception{
    std::string exception;
    public:
    SettingsImporterCriticalException(std::string text): exception(text){

    } 
    virtual const char* what() const throw(){
        return exception.c_str();
    }
};

class SettingsImporter{
    std::string bot_id;
    unsigned int delay;
    std::string file;
    public:
    SettingsImporter(std::string filelocation) :file(filelocation) {
        
    };
    int import_settings();
    std::string get_bot_id() const{
        return bot_id;
    }
    unsigned int get_delay() const{
        return delay;
    }
    
    
};