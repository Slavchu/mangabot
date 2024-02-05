#pragma once
#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <memory>

#include <exception>
#include <string>
#include <array>
#include <variant>
#include <fstream>

#include <mysql/mysql.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using MyVariant = std::variant<size_t, int, std::string, double>;

using namespace std;
using namespace sql;
using namespace sql::mysql;

#define MAX_ARR_SIZE 5

enum ECategory
{
    READING,
    ALREADY_READ,
    PLANNING
};

class Database
{
public:
    static Database *GetInstance()
    {
        if (instancePtr == nullptr)
        {
            instancePtr = new Database();
            return instancePtr;
        }
        else
        {
            return instancePtr;
        }
    }

    Database(const Database &obj) = delete;

    std::unique_ptr<sql::PreparedStatement> FactPreparedStatement(const std::string& query);
    std::unique_ptr<sql::Statement>FactStatement(const std::string& query);

    std::unique_ptr<sql::ResultSet> executeQueryWithParams(const std::unique_ptr<sql::PreparedStatement>& stmt, const std::vector<MyVariant> &params);
    std::unique_ptr<sql::ResultSet> executeQuery(const std::unique_ptr<sql::Statement>& stmt, const std::string& query);

    int updateValues(const std::unique_ptr<sql::PreparedStatement>& stmt, const std::vector<MyVariant> &params);

private:
    static Database *instancePtr;
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;

    void fillParams(const std::unique_ptr<sql::PreparedStatement>& stmt, const std::vector<MyVariant> &params);
    Database(const std::string &host,
             const std::string &user,
             const std::string &password,
             const std::string &database);
    Database();
    ~Database();
};

struct CategoryUser
{
    CategoryUser(size_t user_id = 0, std::string user_name = "", size_t team_id = 0, size_t manga_id_size = 0) :
        user_id(user_id),
        user_name(user_name),
        team_id(team_id),
        manga_id_size(manga_id_size) {}
    size_t user_id;
    size_t team_id;
    size_t manga_id_size;
    std::string user_name;

    CategoryUser(std::unique_ptr<sql::ResultSet>& res);

    friend std::ostream& operator<<(std::ostream& os, const CategoryUser& user);
    ~CategoryUser() = default;
};

struct TranslationTeam {
    std::string Description;
    size_t TsTeamID;
    std::string TeamName;

    TranslationTeam() : TsTeamID(0) {}

    TranslationTeam(const std::string& description, size_t teamID, const std::string& teamName)
        : Description(description), TsTeamID(teamID), TeamName(teamName) {}

    TranslationTeam(std::unique_ptr<sql::ResultSet>& res);

    friend std::ostream& operator<<(std::ostream& os, const TranslationTeam& team);
};

struct Chapter {
    size_t ChapterID;
    size_t MangaID;
    double ChapterNumber;
    std::string Title;
    std::string URL;

    Chapter() : ChapterID(0), MangaID(0), ChapterNumber(0.0) {}

    Chapter(size_t chapterID, size_t mangaID, double chapterNumber, const std::string& title, const std::string& url)
        : ChapterID(chapterID), MangaID(mangaID), ChapterNumber(chapterNumber), Title(title), URL(url) {}

    Chapter(std::unique_ptr<sql::ResultSet>& res);

    friend std::ostream& operator<<(std::ostream& os, const Chapter& chapter);
};

struct Manga {
    size_t MangaID;
    size_t TsTeamId;
    std::string Title;
    std::string Description;
    std::string Author;
    std::string CoverImage;

    Manga() : MangaID(0), TsTeamId(0) {}

    Manga(size_t mangaID, size_t tsTeamId, const std::string& title, const std::string& desc,
          const std::string& author, const std::string& coverImage)
        : MangaID(mangaID), TsTeamId(tsTeamId), Title(title), Description(desc),
          Author(author), CoverImage(coverImage) {}

    Manga(std::unique_ptr<sql::ResultSet>& res);

    friend std::ostream& operator<<(std::ostream& os, const Manga& manga);
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
    std::string get_sql_ip() const{
        return sql_ip ;
    }
    std::string get_sql_username() const {
        return sql_username ;
    }
    std::string get_sql_password() const {
        return sql_password;
    }
};