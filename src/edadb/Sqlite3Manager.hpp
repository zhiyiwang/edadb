/**
 * @file Sqlite3Manager.hpp
 * @brief Sqlite3Manager.hpp provides a way to manage the Sqlite3.
 *      This class is a child class of DatabaseManager.
 */

#pragma once

#include <iostream>
#include <string>
#include <stdint.h>
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
    // connect to sqlite3 
    std::string connect_param;

    sqlite3      *db   = nullptr;
    sqlite3_stmt *stmt = nullptr;
    char         *zErrMsg = nullptr;

public:
    static const uint32_t s_bind_column_begin_index  = 1;
    static const uint32_t s_fetch_column_begin_index = 0;


public:
    Sqlite3Manager(void) = default;
    ~Sqlite3Manager() {
        close();
    }

public: // database 
    bool connect(const std::string &c = "edadb.sqlite3.db") {
        connect_param = c;
        bool connected = (sqlite3_open(connect_param.c_str(), &db) == SQLITE_OK);
        if (!connected) {
            std::cerr << "Sqlite3 Error: can't open database: " << connect_param << std::endl;
            std::cerr << "Sqlite3 Error: " << sqlite3_errmsg(db) << std::endl;
        }
        return connected;
    }

    bool exec(const std::string &sql) {
        bool executed = (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) == SQLITE_OK);
        if (!executed) {
            std::cerr << "Sqlite3 Error: can't execute SQL: " << sql << std::endl;
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return executed;
    }

    bool close() {
        bool closed = (sqlite3_close(db) == SQLITE_OK);
        if (!closed) {
            std::cerr << "Sqlite3 Error: can't close database: " << connect_param << std::endl;
            std::cerr << "Sqlite3 Error: " << sqlite3_errmsg(db) << std::endl;
        }
        return closed;
    }

public: // statement
    bool prepare(const std::string &sql) {
        #if DEBUG_SQLITE3_API
            std::cout << "Sqlite3Manager::prepareSQL: " << sql << std::endl;
        #endif

        bool prepared = (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK);
        if (!prepared) {
            std::cerr << "Sqlite3 Error: can't prepare SQL: " << sql << std::endl;
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }   
        return prepared;
    }

    int changes() {
        return sqlite3_changes(db);
    }


    bool reset() {
        bool reseted = (sqlite3_reset(stmt) == SQLITE_OK);
        if (!reseted) {
            std::cerr << "Sqlite3 Error: can't reset SQL" << std::endl;
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return reseted;
    }

    bool finalize() {
        bool finalized = (sqlite3_finalize(stmt) == SQLITE_OK);
        if (!finalized) {
            std::cerr << "Sqlite3 Error: can't finalize SQL" << std::endl;
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        return finalized;
    }


public: // insert
    template <typename T>
    bool bindValue(int index, const T &value) {
        bool binded = false;
        auto dbtype = CppTypeToDbType<T>::dbType;
        switch (dbtype) {
            case DbTypes::kInteger: {
                int *bin = (int*)value;
                binded = (sqlite3_bind_int(stmt, index, *bin) == SQLITE_OK);
                break;
            }
            case DbTypes::kReal: {
                double *bin = (double*)value;
                binded = (sqlite3_bind_double(stmt, index, *bin) == SQLITE_OK);
                break;
            }
            case DbTypes::kText: {
                std::string *str = (std::string*)value;
                const char  *bin = str->c_str();
                binded = (sqlite3_bind_text(stmt, index, bin, -1, SQLITE_STATIC) == SQLITE_OK);
                break;
            }
            default:
                std::cerr << "Sqlite3 Error: unknown type " << index << std::endl;
                break;
        }
        
        return binded;
    }

    bool bindStep() {
        bool stepped = (sqlite3_step(stmt) == SQLITE_DONE);
        if (!stepped) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            std::cerr << "sqlite3_step failed" << std::endl;
            sqlite3_free(zErrMsg);
        }
        return stepped;
    }


public: // schema info 
    int getColumnCount() {
        return sqlite3_column_count(stmt);
    }
    
    int getColumnType(int index) {
        return sqlite3_column_type(stmt, index);
    }

    const char *getColumnName(int index) {
        return sqlite3_column_name(stmt, index);
    }

public: // scan
    /**
     * call sqlite3_step to get the next row.
     * Since return may not be SQLITE_ROW, we need to check the return value.
     * @return true if the next row exists; otherwise, false.
     */
    bool fetchStep() {
        #if DEBUG_SQLITE3_SCAN
            std::cout << "Sqlite3Manager::fetchStep" << std::endl;
        #endif

        return (sqlite3_step(stmt) == SQLITE_ROW);
    }

    template <typename T>
    bool fetchValue(int index, T &value) {
        bool columned = false;
        auto dbtype = CppTypeToDbType<T>::dbType;
        switch (dbtype) {
            case DbTypes::kInteger: {
                *(int*)value = sqlite3_column_int(stmt, index);
                columned = true;
                break;
            }
            case DbTypes::kReal: {
                *(double*)value = sqlite3_column_double(stmt, index);
                columned = true;
                break;

            }
            case DbTypes::kText: {
                uint32_t size = sqlite3_column_bytes(stmt, index);
                const char *bin = (const char*)sqlite3_column_text(stmt, index);
                std::string *str = (std::string*)value;
                str->assign(bin, size);
                columned = true;
                break;
            }
            default:
                std::cerr << "Sqlite3 Error: unknown type " << index << std::endl;
                break;
        }
        return columned;
    }
};


} // namespace edadb