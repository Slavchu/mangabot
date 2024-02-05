#include "database.hpp"

Database *Database::instancePtr = nullptr;

Database::Database(const std::string &host,
                   const std::string &user,
                   const std::string &password,
                   const std::string &database)
    : driver(sql::mysql::get_mysql_driver_instance()),
      con(driver->connect("tcp://" + host, user, password))
{
    if (con->isValid())
    {
        std::cout << "Connection to the database is successful." << std::endl;
    }
    else
    {
        std::cerr << "Connection to the database failed." << std::endl;
    }
    con->setSchema(database);
}

Database::Database()
{
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://localhost", "root", "123qwe");
    if (!con->isValid())
        std::cerr << "Connection to the database failed." << std::endl;
    con->setSchema("manga_data");
}

std::unique_ptr<sql::PreparedStatement> Database::FactPreparedStatement(const std::string& query)
{
    sql::PreparedStatement* pstmt = con->prepareStatement(query);  
    return std::unique_ptr<sql::PreparedStatement>(pstmt); 
}

std::unique_ptr<sql::Statement> Database::FactStatement(const std::string& query)
{
    sql::Statement* stmt = con->createStatement();
    return std::unique_ptr<sql::Statement>(stmt);
}

void Database::fillParams(const std::unique_ptr<sql::PreparedStatement>& stmt, const std::vector<MyVariant> &params)
{
    for (size_t i = 0; i < params.size(); ++i) {
        std::visit([&](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>) {
                stmt->setInt(i + 1, arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                stmt->setString(i + 1, arg);
            } else if constexpr (std::is_same_v<T, size_t>) {
                stmt->setInt(i + 1, static_cast<int>(arg));
            } else if constexpr (std::is_same_v<T, double>) {
                stmt->setDouble(i + 1, arg);
            } else {
               throw std::runtime_error("Unsupported type.");
            }
        }, params[i]);
    }
}

std::unique_ptr<sql::ResultSet> Database::executeQueryWithParams(const std::unique_ptr<sql::PreparedStatement>& stmt, const std::vector<MyVariant> &params)
{
    fillParams(stmt, params);
    if (stmt->execute()) {
        return std::unique_ptr<sql::ResultSet>(stmt->getResultSet());
    }
    return nullptr;
}

std::unique_ptr<sql::ResultSet> Database::executeQuery(const std::unique_ptr<sql::Statement>& stmt, const std::string& query)
{
    if (stmt->execute(query)) {
        return std::unique_ptr<sql::ResultSet>(stmt->getResultSet());
    }
    return nullptr;
}

int Database::updateValues(const std::unique_ptr<sql::PreparedStatement>& stmt, const std::vector<MyVariant> &params)
{
    fillParams(stmt, params);
    return stmt->executeUpdate();
}

Database::~Database()
{
    if (con)
    {
        con->close(); 
        delete con;
    }
    delete driver;
}

std::ostream& operator<<(std::ostream& os, const CategoryUser& user)
{
    os << "User ID: " << user.user_id << "\n";
    os << "User Name: " << user.user_name << "\n";
    os << "Team ID: " << user.team_id << "\n";
    os << "Manga ID Size: " << user.manga_id_size << "\n";
    return os;
}

CategoryUser::CategoryUser(std::unique_ptr<sql::ResultSet>& res)
{
    user_id = res->getInt("UserID");
    team_id = res->getInt("TeamID");
    user_name = res->getString("Username");
    manga_id_size = 0;
}

void createUser(CategoryUser& user, Database *db)
{
    if (!db)
    {
        throw std::invalid_argument("Database instance is null");
    }

    std::string query = "SELECT * FROM User WHERE Username = ?";
    std::vector<MyVariant> params = {user.user_name};
    std::unique_ptr<sql::PreparedStatement> prep_stmt(db->FactPreparedStatement(query));
    std::unique_ptr<sql::ResultSet> res(db->executeQueryWithParams(prep_stmt, params));

    if (res->next())
    {
        throw std::runtime_error("User with the given name already exists");
    }

    query = "INSERT INTO User (TeamID, Username) VALUES (?, ?)";
    params = {std::to_string(user.team_id), user.user_name};

    prep_stmt = db->FactPreparedStatement(query);
    if (!db->updateValues(prep_stmt, params))
        throw std::runtime_error("Failed to insert the user.");

    prep_stmt->close(); 
    query = "SELECT LAST_INSERT_ID()";
    std::unique_ptr<sql::Statement> stmt = db->FactStatement(query);
    std::unique_ptr<sql::ResultSet> res_last_insert_id(db->executeQuery(stmt, query));

    if (res_last_insert_id->next()) {
        user.user_id = res_last_insert_id->getInt(1);
    } else {
        throw std::runtime_error("Failed to retrieve the inserted user ID.");
    }

    return;
}

CategoryUser findUser(const std::string& Column, MyVariant param, Database*db)
{
    std::string query = "SELECT * FROM User WHERE + " + Column + " = ?";
    std::vector <MyVariant> params = {param};
    std::unique_ptr<sql::PreparedStatement> prep_stmt = db->FactPreparedStatement(query);
    std::unique_ptr<sql::ResultSet> res(db->executeQueryWithParams(prep_stmt, params));
    if (res->next()) {
        return CategoryUser(res);
    }

    throw std::runtime_error("Couldn't find the user.");
}

bool updateUser(const CategoryUser& user, Database* db)
{
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    std::string checkUserQuery = "SELECT * FROM User WHERE UserID = ?";
    std::vector<MyVariant> checkUserParams = {user.user_id};
    std::unique_ptr<sql::PreparedStatement> checkUserStmt(db->FactPreparedStatement(checkUserQuery));
    std::unique_ptr<sql::ResultSet> checkUserRes(db->executeQueryWithParams(checkUserStmt, checkUserParams));

    if (!checkUserRes->next()) {
        throw std::runtime_error("User not found");
    }

    std::string updateUserQuery = "UPDATE User SET TeamID = ?, Username = ? WHERE UserID = ?";
    std::vector<MyVariant> updateUserParams = {user.team_id, user.user_name, user.user_id};
    std::unique_ptr<sql::PreparedStatement> updateUserStmt(db->FactPreparedStatement(updateUserQuery));

    if (db->updateValues(updateUserStmt, updateUserParams) > 0) {
        return true; 
    }
    return false;
}

std::array<size_t, MAX_ARR_SIZE> get_manga(size_t user_id, ECategory category, Database *db)
{
    if (!db)
    {
        throw std::invalid_argument("Database instance is null");
    }

    try
    {
        std::string sqlQuery = "SELECT MangaId FROM UserMangaCategories WHERE UserMangaCategories.UserID = ? AND UserMangaCategories.Category = ? ORDER BY MangaId DESC LIMIT 5";
    
        std::vector<MyVariant> params = {user_id, category + 1};
        std::unique_ptr<sql::PreparedStatement> prep_stmt(db->FactPreparedStatement(sqlQuery));
        std::unique_ptr<sql::ResultSet> res(db->executeQueryWithParams(prep_stmt, params));

        int i = 0;
        std::array<size_t, MAX_ARR_SIZE> resultArr;
        while (res->next() && i < MAX_ARR_SIZE)
        {
            resultArr[i++] = res->getInt("MangaId");
            std::cout << "MangaId: " << resultArr[i - 1] << std::endl;
        }

        return resultArr;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQL Exception: " << e.what() << std::endl;
        std::cerr << "Error Code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQL State: " << e.getSQLState() << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const TranslationTeam& team) {
        os << "TsTeamID: " << team.TsTeamID << "\n";
        os << "Description: " << team.Description << "\n";
        os << "TeamName: " << team.TeamName << "\n";
        return os;
}

TranslationTeam::TranslationTeam(std::unique_ptr<sql::ResultSet>& res) {
    if (res) {
        Description = res->getString("Description");
        TsTeamID = res->getInt("TsTeamID");
        TeamName = res->getString("TeamName");
    } else {
        TsTeamID = 0;
    }
}

void createTranslationTeam(TranslationTeam& team, Database* db) {
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    std::string checkTeamQuery = "SELECT * FROM TranslationTeams WHERE TeamName = ?";
    std::vector<MyVariant> checkTeamParams = {team.TeamName};
    std::unique_ptr<sql::PreparedStatement> checkTeamStmt(db->FactPreparedStatement(checkTeamQuery));
    std::unique_ptr<sql::ResultSet> checkTeamRes(db->executeQueryWithParams(checkTeamStmt, checkTeamParams));

    if (checkTeamRes->next()) {
        throw std::runtime_error("Translation team with the given name already exists");
    }

    std::string insertTeamQuery = "INSERT INTO TranslationTeams (TeamName, Description, CreatedAt, UpdatedAt) VALUES (?, ?, NOW(), NOW())";
    std::vector<MyVariant> insertTeamParams = {team.TeamName, team.Description}; 

    std::unique_ptr<sql::PreparedStatement> insertTeamStmt(db->FactPreparedStatement(insertTeamQuery));

    if (db->updateValues(insertTeamStmt, insertTeamParams) <= 0) {
        throw std::runtime_error("Failed to insert the translation team");
    }

    std::string getInsertedID = "SELECT LAST_INSERT_ID()";
    std::unique_ptr<sql::Statement> stmtID(db->FactStatement(getInsertedID));
    std::unique_ptr<sql::ResultSet> resID(db->executeQuery(stmtID, getInsertedID));

    if (resID->next()) {
        team.TsTeamID = resID->getInt(1);
    } else {
        throw std::runtime_error("Failed to retrieve the inserted translation team");
    }
}

TranslationTeam findTeam(const std::string& columnName, const MyVariant& columnValue, Database* db) {
    std::string query = "SELECT * FROM TranslationTeams WHERE " + columnName + " = ?";
    std::vector<MyVariant> params = {columnValue};
    std::unique_ptr<sql::PreparedStatement> prep_stmt = db->FactPreparedStatement(query);
    std::unique_ptr<sql::ResultSet> res(db->executeQueryWithParams(prep_stmt, params));

    if (res->next()) {
        TranslationTeam foundTeam(res);
        return foundTeam;
    }

    throw std::runtime_error("Couldn't find the team.");
}


bool updateTranslationTeam(const TranslationTeam& team, Database* db) {
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    std::string checkTeamQuery = "SELECT * FROM TranslationTeams WHERE TsTeamID = ?";
    std::vector<MyVariant> checkTeamParams = {team.TsTeamID};
    std::unique_ptr<sql::PreparedStatement> checkTeamStmt(db->FactPreparedStatement(checkTeamQuery));
    std::unique_ptr<sql::ResultSet> checkTeamRes(db->executeQueryWithParams(checkTeamStmt, checkTeamParams));

    if (!checkTeamRes->next()) {
        throw std::runtime_error("Translation team not found");
    }

    std::string updateTeamQuery = "UPDATE TranslationTeams SET TeamName = ?, Description = ?, UpdatedAt = NOW() WHERE TsTeamID = ?";
    std::vector<MyVariant> updateTeamParams = {team.TeamName, team.Description, team.TsTeamID};
    std::unique_ptr<sql::PreparedStatement> updateTeamStmt(db->FactPreparedStatement(updateTeamQuery));

    if (db->updateValues(updateTeamStmt, updateTeamParams) > 0) {
        return true;
    }
    return false;
}

Chapter::Chapter(std::unique_ptr<sql::ResultSet>& res) {
    if (res) {
        ChapterID = res->getInt("ChapterID");
        MangaID = res->getInt("MangaID");
        ChapterNumber = res->getDouble("ChapterNumber");
        Title = res->getString("Title");
        URL = res->getString("URL");
    } else {
        ChapterID = 0;
        MangaID = 0;
        ChapterNumber = 0.0;
    }
}

std::ostream& operator<<(std::ostream& os, const Chapter& chapter) {
    os << "ChapterID: " << chapter.ChapterID << "\n";
    os << "MangaID: " << chapter.MangaID << "\n";
    os << "ChapterNumber: " << chapter.ChapterNumber << "\n";
    os << "Title: " << chapter.Title << "\n";
    os << "URL: " << chapter.URL << "\n";
    return os;
}

void insertChapter(Chapter& chapter, Database* db) {
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    std::string checkChapterQuery = "SELECT * FROM Chapter WHERE MangaID = ? AND ChapterNumber = ?";
    std::vector<MyVariant> checkChapterParams = {chapter.MangaID, chapter.ChapterNumber};
    std::unique_ptr<sql::PreparedStatement> checkChapterStmt(db->FactPreparedStatement(checkChapterQuery));
    std::unique_ptr<sql::ResultSet> checkChapterRes(db->executeQueryWithParams(checkChapterStmt, checkChapterParams));

    if (checkChapterRes->next()) {
        throw std::runtime_error("Chapter with the given chapter number already exists for the MangaID");
    }

    std::string insertChapterQuery = "INSERT INTO Chapter (MangaID, ChapterNumber, Title, URL, ReleaseDate) VALUES (?, ?, ?, ?, NOW())";
    std::vector<MyVariant> insertChapterParams = {chapter.MangaID, chapter.ChapterNumber, chapter.Title, chapter.URL};

    std::unique_ptr<sql::PreparedStatement> insertChapterStmt(db->FactPreparedStatement(insertChapterQuery));

    if (db->updateValues(insertChapterStmt, insertChapterParams) > 0) {
        std::string getInsertedID = "SELECT LAST_INSERT_ID()";
        std::unique_ptr<sql::Statement> stmtID(db->FactStatement(getInsertedID));
        std::unique_ptr<sql::ResultSet> resID(db->executeQuery(stmtID, getInsertedID));

        if (resID->next()) {
            chapter.ChapterID = resID->getInt(1);
        } else {
            throw std::runtime_error("Failed to retrieve the inserted ChapterID");
        }
    } else {
        throw std::runtime_error("Failed to insert the chapter");
    }
}


bool updateChapter(const Chapter& chapter, Database* db) {
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    // Check if the chapter with the given ChapterID exists
    std::string checkChapterQuery = "SELECT * FROM Chapter WHERE ChapterID = ?";
    std::vector<MyVariant> checkChapterParams = {chapter.ChapterID};
    std::unique_ptr<sql::PreparedStatement> checkChapterStmt(db->FactPreparedStatement(checkChapterQuery));
    std::unique_ptr<sql::ResultSet> checkChapterRes(db->executeQueryWithParams(checkChapterStmt, checkChapterParams));

    if (!checkChapterRes->next()) {
        throw std::runtime_error("Chapter not found");
    }

    // Update the chapter values
    std::string updateChapterQuery = "UPDATE Chapter SET MangaID = ?, ChapterNumber = ?, Title = ?, URL = ? WHERE ChapterID = ?";
    std::vector<MyVariant> updateChapterParams = {chapter.MangaID, chapter.ChapterNumber, chapter.Title, chapter.URL, chapter.ChapterID};
    std::unique_ptr<sql::PreparedStatement> updateChapterStmt(db->FactPreparedStatement(updateChapterQuery));

    if (db->updateValues(updateChapterStmt, updateChapterParams) > 0) {
        return true;
    }

    return false;
}


Manga::Manga(std::unique_ptr<sql::ResultSet>& res) {
    if (res) {
        MangaID = res->getInt("MangaID");
        TsTeamId = res->getInt("TsTeamId");
        Title = res->getString("Title");
        Description = res->getString("Description");
        Author = res->getString("Author");
        CoverImage = res->getString("CoverImage");
    } else {
        MangaID = 0;
        TsTeamId = 0;
    }
}

std::ostream& operator<<(std::ostream& os, const Manga& manga) {
    os << "MangaID: " << manga.MangaID << "\n";
    os << "TsTeamId: " << manga.TsTeamId << "\n";
    os << "Title: " << manga.Title << "\n";
    os << "Description: " << manga.Description << "\n";
    os << "Author: " << manga.Author << "\n";
    os << "CoverImage: " << manga.CoverImage << "\n";
    return os;
}

void createManga(Manga& manga, Database* db) {
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    std::string checkMangaQuery = "SELECT * FROM MangaTitles WHERE Title = ?";
    std::vector<MyVariant> checkMangaParams = {manga.Title};
    std::unique_ptr<sql::PreparedStatement> checkMangaStmt(db->FactPreparedStatement(checkMangaQuery));
    std::unique_ptr<sql::ResultSet> checkMangaRes(db->executeQueryWithParams(checkMangaStmt, checkMangaParams));

    if (checkMangaRes->next()) {
        throw std::runtime_error("Manga with the given title already exists");
    }

    std::string insertMangaQuery = "INSERT INTO MangaTitles (Title, Description, Author, CoverImage, TsTeamId, CreatedAt, UpdatedAt) VALUES (?, ?, ?, ?, ?, NOW(), NOW())";
    std::vector<MyVariant> insertMangaParams = {manga.Title, manga.Description, manga.Author, manga.CoverImage, manga.TsTeamId}; 

    std::unique_ptr<sql::PreparedStatement> insertMangaStmt(db->FactPreparedStatement(insertMangaQuery));

    if (db->updateValues(insertMangaStmt, insertMangaParams) > 0) {
        std::string getInsertedID = "SELECT LAST_INSERT_ID()";
        std::unique_ptr<sql::Statement> stmtID(db->FactStatement(getInsertedID));
        std::unique_ptr<sql::ResultSet> resID(db->executeQuery(stmtID, getInsertedID));

        if (resID->next()) {
            manga.MangaID = resID->getInt(1);
        } else {
            throw std::runtime_error("Failed to retrieve the inserted manga ID");
        }
    } else {
        throw std::runtime_error("Failed to insert the manga");
    }
}

Manga findManga(const std::string& column, MyVariant param, Database* db) {
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    std::string query = "SELECT * FROM MangaTitles WHERE " + column + " = ?";
    std::vector<MyVariant> params = {param};
    std::unique_ptr<sql::PreparedStatement> prepStmt(db->FactPreparedStatement(query));
    std::unique_ptr<sql::ResultSet> result(db->executeQueryWithParams(prepStmt, params));

    if (result->next()) {
        return Manga(result);
    }

    throw std::runtime_error("Manga not found with the given parameter");
}

bool updateManga(const Manga& manga, Database* db) {
    if (!db) {
        throw std::invalid_argument("Database instance is null");
    }

    std::string checkMangaQuery = "SELECT * FROM MangaTitles WHERE MangaID = ?";
    std::vector<MyVariant> checkMangaParams = {manga.MangaID};
    std::unique_ptr<sql::PreparedStatement> checkMangaStmt(db->FactPreparedStatement(checkMangaQuery));
    std::unique_ptr<sql::ResultSet> checkMangaRes(db->executeQueryWithParams(checkMangaStmt, checkMangaParams));

    if (!checkMangaRes->next()) {
        throw std::runtime_error("Manga not found");
    }

    std::string updateMangaQuery = "UPDATE MangaTitles SET TsTeamId = ?, Title = ?, Description = ?, Author = ?, CoverImage = ?, UpdatedAt = NOW() WHERE MangaID = ?";
    std::vector<MyVariant> updateMangaParams = {manga.TsTeamId, manga.Title, manga.Description, manga.Author, manga.CoverImage, manga.MangaID};
    std::unique_ptr<sql::PreparedStatement> updateMangaStmt(db->FactPreparedStatement(updateMangaQuery));

    if (db->updateValues(updateMangaStmt, updateMangaParams) > 0) {
        return true;
    }
    
    return false;
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
    // if (!has_sql_ip) throw SettingsImporterCriticalException("SQL IP field empty");
    // if (!has_sql_username) throw SettingsImporterCriticalException("SQL username field empty");
    // if (!has_sql_password) throw SettingsImporterCriticalException("SQL password field empty");
    
    return 0;
}