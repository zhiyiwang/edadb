/**
 * @file DbManager.hpp
 * @brief DbManager.hpp provides a way to manage the Sqlite3.
 *      This class is a child class of DatabaseManager.
 */

#pragma once

#include <iostream>
#include <string>
#include <stdint.h>
#include <sqlite3.h>
#include <boost/core/noncopyable.hpp>

#include "CppTypeToDbType.hpp"
#include "DbStatement.h"

namespace edadb {


/**
 * @class Singleton
 * @brief This class is a singleton pattern.
 */
template<class T>
class Singleton : public boost::noncopyable {
public:
    static T &i() {
        static T _me;
        return _me;
    }
}; 

/**
 * @class DbManager
 * @brief This class manages the Sqlite3 database.
 *    Since Sqlite3 use R/W lock, we define Singleton pattern for this class.
 */
class DbManager : public Singleton<DbManager> {
protected:
    std::string connect_param; // database connection parameter
    sqlite3     *db = nullptr; // database handler

//    // TODO: move to DB operation class 
//    sqlite3_stmt *stmt = nullptr;
//    char      *zErrMsg = nullptr;

public:
    static const uint32_t s_bind_column_begin_index  = 1; // sqlite3 bind column index starts from 1
    static const uint32_t s_fetch_column_begin_index = 0; // sqlite3 fetch column index starts from 0

public:
    DbManager(void) = default;
    ~DbManager() {
        close();
    }

public: // database operation
    /**
     * @brief Connect to the database using the connection parameter.
     * @param c The connection parameter.
     * @return true if connected; otherwise, false.
     */
    bool connect(const std::string &c = "edadb.sqlite3.db") {
        if (!connect_param.empty()) {
            std::cerr << "Sqlite3 Error: already connected: " << connect_param << std::endl;
            return false;
        }

        connect_param = c;
        bool connected = (sqlite3_open(connect_param.c_str(), &db) == SQLITE_OK);
        if (!connected) {
            std::cerr << "Sqlite3 Error: can't open database: " << connect_param << std::endl;
            std::cerr << "Sqlite3 Error: " << sqlite3_errmsg(db) << std::endl;
        }
        return connected;
    }

    /**
     * @brief Execute the SQL statement directly.
     * @param sql The SQL statement.
     * @return true if executed; otherwise, false.
     */
    bool exec(const std::string &sql) {
        char *zErrMsg = nullptr;
        bool executed = (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) == SQLITE_OK);
        if (!executed) {
            std::cerr << "Sqlite3 Error: can't execute SQL: " << sql << std::endl;
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return executed;
    }

    /**
     * @brief Close the database.
     * @return true if closed; otherwise, false.
    */
    bool close() {
        if (connect_param.empty()) {
            std::cerr << "Sqlite3 Error: not connected: " << connect_param << std::endl;
            return false;
        }

        bool closed = (sqlite3_close(db) == SQLITE_OK);
        if (!closed) {
            std::cerr << "Sqlite3 Error: can't close database: " << connect_param << std::endl;
            std::cerr << "Sqlite3 Error: " << sqlite3_errmsg(db) << std::endl;
        }
        connect_param.clear();
        return closed;
    }

public: // sqlite3 statement operation 
    /**
     * @brief Prepare the SQL statement
     * @param stmt The sqlite3 statement
     * @param sql The SQL statement
     * @return true if prepared; otherwise, false
     */
    bool prepare(DbStatement &dbstmt, const std::string &sql) {
        #if DEBUG_SQLITE3_API
            std::cout << "DbManager::prepareSQL: " << sql << std::endl;
        #endif

        bool prepared =
            (sqlite3_prepare_v2(db, sql.c_str(), -1, &(dbstmt.stmt), 0) == SQLITE_OK);
        if (!prepared) {
            std::cerr << "Sqlite3 Error: can't prepare SQL: " << sql << std::endl;

            dbstmt.zErrMsg = sqlite3_errmsg(db);
            std::cerr << "Sqlite3 Error: " << dbstmt.zErrMsg << std::endl;
        }   
        return prepared;
    }

    /**
     * @brief check the number of rows changed by the last statement.
     * @return The number of rows changed.
     */
    int changes() {
        return sqlite3_changes(db);
    }

    bool reset(DbStatement &dbstmt) {
        bool reseted = (sqlite3_reset(dbstmt.stmt) == SQLITE_OK);
        if (!reseted) {
            std::cerr << "Sqlite3 Error: can't reset SQL" << std::endl;

            dbstmt.zErrMsg = sqlite3_errmsg(db);
            std::cerr << "Sqlite3 Error: " << dbstmt.zErrMsg << std::endl;
        }
        return reseted;
    }

    bool finalize(DbStatement &dbstmt) {
        bool finalized = (sqlite3_finalize(dbstmt.stmt) == SQLITE_OK);
        if (!finalized) {
            std::cerr << "Sqlite3 Error: can't finalize SQL" << std::endl;

            dbstmt.zErrMsg = sqlite3_errmsg(db);
            std::cerr << "Sqlite3 Error: " << dbstmt.zErrMsg << std::endl;
        }
        return finalized;
    }


public: // insert
    bool bindColumn(DbStatement &dbstmt, int index, int *value) {
        return (sqlite3_bind_int(dbstmt.stmt, index, *value) == SQLITE_OK);
    }

    bool bindColumn(DbStatement &dbstmt, int index, double *value) {
        return (sqlite3_bind_double(dbstmt.stmt, index, *value) == SQLITE_OK);
    }

    bool bindColumn(DbStatement &dbstmt, int index, std::string *value) {
        return (sqlite3_bind_text(dbstmt.stmt, index, value->c_str(), -1, SQLITE_STATIC) == SQLITE_OK);
    }

    bool bindStep(DbStatement &dbstmt) {
        bool stepped = (sqlite3_step(dbstmt.stmt) == SQLITE_DONE);
        if (!stepped) {
            std::cerr << "sqlite3_step failed" << std::endl;

            dbstmt.zErrMsg = sqlite3_errmsg(db);
            std::cerr << "Sqlite3 Error: " << dbstmt.zErrMsg << std::endl;
        }
        return stepped;
    }


public: // schema info 
    int getColumnCount(DbStatement &dbstmt) {
        return sqlite3_column_count(dbstmt.stmt);
    }
    
    int getColumnType(DbStatement &dbstmt, int index) {
        return sqlite3_column_type(dbstmt.stmt, index);
    }

    const char *getColumnName(DbStatement &dbstmt, int index) {
        return sqlite3_column_name(dbstmt.stmt, index);
    }

public: // scan
    /**
     * call sqlite3_step to get the next row.
     * Since return may not be SQLITE_ROW, we need to check the return value.
     * @return true if the next row exists; otherwise, false.
     */
    bool fetchStep(DbStatement &dbstmt) {
        #if DEBUG_SQLITE3_SCAN
            std::cout << "DbManager::fetchStep" << std::endl;
        #endif

        return (sqlite3_step(dbstmt.stmt) == SQLITE_ROW);
    }

    bool fetchColumn(DbStatement &dbstmt, int index, int *value) {
        *value = sqlite3_column_int(dbstmt.stmt, index);
        return true;
    }

    bool fetchColumn(DbStatement &dbstmt, int index, double *value) {
        *value = sqlite3_column_double(dbstmt.stmt, index);
        return true;
    }

    bool fetchColumn(DbStatement &dbstmt, int index, std::string *value) {
        uint32_t size = sqlite3_column_bytes(dbstmt.stmt, index);
        const char  *bin = (const char*)sqlite3_column_text(dbstmt.stmt, index);
        value->assign(bin, size);
        return true;
    }
};


} // namespace edadb