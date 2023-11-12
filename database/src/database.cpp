#include <database.hpp>
#include <fstream>

using namespace std;

Database* Database::instancePtr = nullptr;

Database::Database(
    const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& database)
    :
    driver(sql::mysql::get_mysql_driver_instance()),
    con(driver->connect("tcp://" + host, user, password)) 
{
    if (con->isValid()) {
        // The connection is valid
        std::cout << "Connection to the database is successful." << std::endl;
    } else {
        // The connection is not valid
        std::cerr << "Connection to the database failed." << std::endl;
    }
    con->setSchema(database);   
    const std::string intendedSchema = "manga_data";
    std::string currentSchema = con->getSchema();
}

Database::Database()
{
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://localhost", "root", "123qwe"); 
    if (!con->isValid()) 
            std::cerr << "Connection to the database failed." << std::endl;
    con->setSchema("manga_data"); 
}

Database::~Database()
{
    if (con) {
            delete con;
        }
    delete stmt;
    delete driver;
}

void Database::SetConnection()
{
    stmt = con->createStatement();
}

sql::Statement* Database::getStatement() const
{
    return stmt;
}

std::array <size_t, MAX_ARR_SIZE> CategoryUser::get_manga(size_t page, ECategory category)
{
        Database *myDB = Database::GetInstance(); 
        myDB->SetConnection(); 
        std::string sqlQuery = "SELECT MangaId FROM UserMangaCategories WHERE UserMangaCategories.UserID = " + std::to_string(user_id) + 
            " AND UserMangaCategories.Category = " + std::to_string(static_cast<int>(category) + 1) + 
            " Limit 5 OFFSET " + std::to_string(page * 5); 
        sql::ResultSet* res = myDB->getStatement()->executeQuery(sqlQuery); 
        
        int i = 0; 
        std::array<size_t, MAX_ARR_SIZE> resultArr; 
        while (res->next()) { 
            resultArr[i++] = res->getInt("MangaId"); 
        } 
        delete res; 
        return resultArr; 
}

int SettingsImporter::import_settings() {
    ifstream in(file);
    if (!in.is_open()) {
        throw SettingsImporterCriticalException("File not found");
    }
    bool has_bot_id = false, has_delay = false, has_sql_ip = false, has_sql_username = false, has_sql_password = false;
    
    while (!in.eof()) {
        string parameter;
        in >> parameter;
        if (!in.eof()) {
            if (parameter == "bot_id") {
                has_bot_id = true;
                in >> bot_id;
            } else if (parameter == "delay") {
                has_delay = true;
                in >> delay;
            } else if (parameter == "sql_ip") {
                has_sql_ip = true;
                in >> sql_ip;
            } else if (parameter == "sql_username") {
                has_sql_username = true;
                in >> sql_username;
            } else if (parameter == "sql_password") {
                has_sql_password = true;
                in >> sql_password;
            }
        }
    }

    if (!has_bot_id) throw SettingsImporterCriticalException("Bot id field empty");
    if (!has_sql_ip) throw SettingsImporterCriticalException("SQL IP field empty");
    if (!has_sql_username) throw SettingsImporterCriticalException("SQL username field empty");
    if (!has_sql_password) throw SettingsImporterCriticalException("SQL password field empty");
    
    return 0;
}



