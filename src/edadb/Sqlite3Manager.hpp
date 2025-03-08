/**
 * @file Sqlite3Manager.hpp
 * @brief Sqlite3Manager.hpp provides a way to manage the Sqlite3.
 */

#pragma once

#include <iostream>
#include <string>
#include <sqlite3.h>

#include "CppTypeToDbType.hpp"

namespace edadb {

template<class T>
class Singleton : public boost::noncopyable {
public:
    static T &i() {
        static T _me;
        return _me;
    }
}; 



class Sqlite3Manager : public Singleton<Sqlite3Manager> {
protected:
    // connect to sqlite3 Sqlite3
    std::string connect_param;
    std::string table_name;

    char *zErrMsg = nullptr;
    sqlite3 *db = nullptr;
    sqlite3_stmt *stmt = nullptr;


public:
    Sqlite3Manager(void) = default;
    ~Sqlite3Manager() {
        close();
    }

public:
    bool connect(const std::string &c = "edadb.sqlite3.db") {
        connect_param = c;
        bool connected = (sqlite3_open(connect_param.c_str(), &db) == SQLITE_OK);
        if (!connected) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        }
        return connected;
    }

    bool exec(const std::string &sql) {
        bool executed = (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) == SQLITE_OK);
        if (!executed) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return executed;
    }

    bool close() {
        bool closed = (sqlite3_close(db) == SQLITE_OK);
        if (!closed) {
            std::cerr << "Can't close database: " << sqlite3_errmsg(db) << std::endl;
        }
        return closed;
    }

public:
    bool prepareSQL(const std::string &sql) {
        bool prepared = (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK);
        if (!prepared) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }   
        return prepared;
    }

    bool stepSQL() {
        bool stepped = (sqlite3_step(stmt) == SQLITE_DONE);
        if (!stepped) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return stepped;
    }

    int changesSQL() {
        return sqlite3_changes(db);
    }


    bool resetSQL() {
        bool reseted = (sqlite3_reset(stmt) == SQLITE_OK);
        if (!reseted) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return reseted;
    }

    bool finalizeSQL() {
        bool finalized = (sqlite3_finalize(stmt) == SQLITE_OK);
        if (!finalized) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return finalized;
    }


    // sqlite bind
    bool bind2SQL(int index, DbTypes type, void **value) {
        bool binded = false;
        switch (type) {
            case DbTypes::kInteger:
                binded = (sqlite3_bind_int(stmt, index, *(int*)value) == SQLITE_OK);
                break;
            case DbTypes::kReal:
                binded = (sqlite3_bind_double(stmt, index, *(double*)value) == SQLITE_OK);
                break;
            case DbTypes::kText:
                binded = (sqlite3_bind_text(stmt, index, (char*)value, -1, SQLITE_STATIC) == SQLITE_OK);
                break;
            default:
                break;
        }
        return binded;
    }


    int getColumnCount() {
        return sqlite3_column_count(stmt);
    }
    
    int getColumnType(int index) {
        return sqlite3_column_type(stmt, index);
    }

    const char *getColumnName(int index) {
        return sqlite3_column_name(stmt, index);
    }

    bool columnInSQL(int index, DbTypes type, void **value) {
        bool columned = false;
        switch (type) {
            case DbTypes::kInteger:
                *(int*)value = sqlite3_column_int(stmt, index);
                columned = true;
                break;
            case DbTypes::kReal:
                *(double*)value = sqlite3_column_double(stmt, index);
                columned = true;
                break;
            case DbTypes::kText:
                *(char*)value = (const char*)sqlite3_column_text(stmt, index);
                columned = true;
                break;
            default:
                break;
        }
        return columned;
    }


};



} // namespace edadb



