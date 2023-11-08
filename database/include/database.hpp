#pragma once
#include <exception>
#include <string>
#include <array>

#include <mysql/mysql.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h> 
#include <cppconn/prepared_statement.h>

using namespace sql;
using namespace sql::mysql;

#define MAX_ARR_SIZE 5

enum ECategory {
    READING,
    ALREADY_READ,
    PLANNING
};

struct CategoryUser {
    CategoryUser(size_t user_id, size_t team_id = 0, size_t manga_id_size = 0) : 
        user_id(user_id), 
        team_id(team_id), 
        manga_id_size(manga_id_size) { 
    } 
    size_t user_id;
    size_t team_id;
    size_t manga_id_size;

    std::array <size_t, MAX_ARR_SIZE> get_manga(size_t page, ECategory category); //перезаписує size_t {catogery}.manga_id [];
    //void set_var();//кидає змінну на сервер
    ~CategoryUser() = default;
};

class Database {
public:
    //Constructor
    Database(const std::string& host, const std::string& user, const std::string& password, const std::string& database)
        : driver(sql::mysql::get_mysql_driver_instance()), con(driver->connect("tcp://" + host + ":3306", user, password)) {
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
    
    void SetConnection()
    {
        stmt = con->createStatement();
    }

    ~Database() {
        if (con) {
            delete con;
        }
        delete stmt;
        delete driver;
    }
    //Getters
    sql::Statement* getStatement() const
    {
        return stmt;
    }


private:
    sql::Statement *stmt;
    sql::mysql::MySQL_Driver *driver; 
    sql::Connection *con; 
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