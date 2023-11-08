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

class Database {
public:
    static Database& GetInstance()
    {
        static Database mangaDB;
        return mangaDB;
    }

    
    void SetConnection();

    sql::Statement* getStatement() const;

private:
    sql::Statement *stmt;
    sql::mysql::MySQL_Driver *driver; 
    sql::Connection *con; 

    Database(
        const std::string& host,
        const std::string& user,
        const std::string& password,
        const std::string& database);
    Database();
    ~Database();
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

    std::array <size_t, MAX_ARR_SIZE> get_manga(size_t page, ECategory category);
  
    ~CategoryUser() = default;
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

class SettingsImporter {
private:
    std::string bot_id;
    unsigned int delay;
    std::string file;
    std::string sql_ip;
    std::string sql_username;
    std::string sql_password;

public:
    SettingsImporter(std::string filelocation) : file(filelocation) {}

    int import_settings();
    std::string get_bot_id() const {
        return bot_id;
    }
    unsigned int get_delay() const {
        return delay;
    }
    std::string get_sql_ip() {
        return sql_ip ;
    }
    std::string get_sql_username() {
        return sql_username ;
    }
    std::string get_sql_password() {
        return sql_password;
    }
};
