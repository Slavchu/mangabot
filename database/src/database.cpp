#include <database.hpp>
#include <fstream>

using namespace std;
void MangaDatabase::add_manga(string name, MangaLocation location){
    ofstream database;
    database.open(database_location, ios::app);
        if(!database.is_open()) throw MangaDatabaseException_NotFound();
    database << "\"" << name << "\": " << "\"" << location.chat_id << " " << location.message_id << endl;
    
}
int SettingsImporter::import_settings(){
    ifstream in(file);
    if(!in.is_open()){ 
        throw SettingsImporterCriticalException("File not found"); 
    }
    bool has_bot_id = false, has_delay = false;
    while(!in.eof()){
        string parameter;
        in >> parameter;
        if(!in.eof())
            if(parameter == "bot_id"){
                has_bot_id = true;
                in >> bot_id;
            }
            else if(parameter == "delay"){
                has_delay = true;
                in >> delay;
        }
    }

    if(!has_bot_id) throw SettingsImporterCriticalException("Bot id field empty");

}