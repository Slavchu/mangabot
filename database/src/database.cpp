#include <database.hpp>
#include <fstream>

using namespace std;

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
    return 0;
}

std::array<size_t, MAX_ARR_SIZE> CategoryUser::get_manga(size_t page, ECategory category) {
    //Local data
    Database myDB("172.20.10.3", "root", "123qwe", "manga_data"); 
    myDB.SetConnection(); 
    std::string sqlQuery = "SELECT MangaId FROM UserMangaCategories WHERE UserMangaCategories.UserID = " + std::to_string(user_id) + 
        " AND UserMangaCategories.Category = " + std::to_string(static_cast<int>(category) + 1) + 
        " Limit 5 OFFSET " + std::to_string(page * 5); 
    sql::ResultSet* res = myDB.getStatement()->executeQuery(sqlQuery); 
    
    int i = 0; 
    std::array<size_t, MAX_ARR_SIZE> resultArr; 
    while (res->next()) { 
        resultArr[i++] = res->getInt("MangaId"); 
    } 
    delete res; 
    return resultArr; 
} 
