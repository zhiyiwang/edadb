/**
 * @file Sqlite3Manager.hpp
 * @brief Sqlite3Manager.hpp provides a way to manage the Sqlite3.
 *      This class is a child class of DatabaseManager.
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
        assert(connected);
        return connected;
    }

    bool exec(const std::string &sql) {
        bool executed = (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) == SQLITE_OK);
        if (!executed) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            std::cerr << sql << std::endl;
            sqlite3_free(zErrMsg);
        }
        assert(executed);
        return executed;
    }

    bool close() {
        bool closed = (sqlite3_close(db) == SQLITE_OK);
        if (!closed) {
            std::cerr << "Can't close database: " << sqlite3_errmsg(db) << std::endl;
        }
        assert(closed);
        return closed;
    }

public:
    bool prepareSQL(const std::string &sql) {
        #if DEBUG_SQLITE3_API
            std::cout << "Sqlite3Manager::prepareSQL: " << sql << std::endl;
        #endif

        bool prepared = (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK);
        if (!prepared) {
            std::cerr << sql << std::endl;
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }   
        assert(prepared);
        return prepared;
    }

    int changesSQL() {
        return sqlite3_changes(db);
    }


    bool resetSQL() {
        bool reseted = (sqlite3_reset(stmt) == SQLITE_OK);
        if (!reseted) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            std::cerr << "sqlite3_reset failed" << std::endl;
            sqlite3_free(zErrMsg);
        }
        return reseted;
    }

    bool finalizeSQL() {
        bool finalized = (sqlite3_finalize(stmt) == SQLITE_OK);
        if (!finalized) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            std::cerr << "sqlite3_finalize failed" << std::endl;
            sqlite3_free(zErrMsg);
        }
        return finalized;
    }


public: // insert
    bool bind2SQL(int index, DbTypes type, void *value) {
        bool binded = false;
        int bind_index = index + 1; // sqlite3 bind index starts from 1
        switch (type) {
            case DbTypes::kInteger:
                #if DEBUG_SQLITE3_INSERT
                    std::cout << "bind2SQL: " << bind_index << " type " << cppTypeEnumToDbTypeString<DbTypes::kInteger>() << " value: " << *(int*)value << std::endl;
                #endif
                binded = (sqlite3_bind_int(stmt, bind_index, *(int*)value) == SQLITE_OK);
                break;
            case DbTypes::kReal:
                #if DEBUG_SQLITE3_INSERT
                    std::cout << "bind2SQL: " << bind_index << " type " << cppTypeEnumToDbTypeString<DbTypes::kReal>() << " value: " << *(double*)value << std::endl;
                #endif
                binded = (sqlite3_bind_double(stmt, bind_index, *(double*)value) == SQLITE_OK);
                break;
            case DbTypes::kText:
                #if DEBUG_SQLITE3_INSERT
                    std::cout << "bind2SQL: " << bind_index << " type " << cppTypeEnumToDbTypeString<DbTypes::kText>() << " value: " << (char*)value << std::endl;
                #endif
                binded = (sqlite3_bind_text(stmt, bind_index, (char*)value, -1, SQLITE_STATIC) == SQLITE_OK);
                break;
            default:
                break;
        }
        assert(binded);
        return binded;
    }

    bool stepInsertSQL() {
        bool stepped = (sqlite3_step(stmt) == SQLITE_DONE);
        if (!stepped) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            std::cerr << "sqlite3_step failed" << std::endl;
            sqlite3_free(zErrMsg);
        }
        assert(stepped);
        return stepped;
    }


public: // scan
    /**
     * call sqlite3_step to get the next row.
     * Since return may not be SQLITE_ROW, we need to check the return value.
     * @return true if the next row exists; otherwise, false.
     */
    bool stepColumnSQL() {
        #if DEBUG_SQLITE3_SCAN
            std::cout << "stepColumnSQL" << std::endl;
        #endif

        return (sqlite3_step(stmt) == SQLITE_ROW);
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

    bool columnValueInSQL(int index, DbTypes type, void* value) {
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
            default:
                break;
        }
        return columned;
    }

    bool columnPointerInSQL(int index, DbTypes type, void* &value, int &size) {
        bool columned = false;
        switch (type) {
            case DbTypes::kText:
                value = (void*)sqlite3_column_text(stmt, index);
                size = sqlite3_column_bytes(stmt, index);
                columned = true;
                break;
            default:
                break;
        }
        return columned;
    }
};



} // namespace edadb



