/**
 * @file DbManager
 * @brief DbManager.hpp provides a way to manage the Sqlite3.
 *      This class is a child class of DatabaseManager.
 */

#pragma once

#include <type_traits>
#include <iostream>
#include <string>
#include <stdint.h>
#include <sqlite3.h>

#include "Config.h"
#include "Singleton.h"
#include "Cpp2SqlType.h"

#include "DbBackendType.h"
#include "DbStatement.h"
#include "DbStatement4Sqlite.h"
#include "DbManager.h"

namespace edadb {

/**
 * @class DbManager
 * @brief This class manages the Sqlite3 database.
 *    Since Sqlite3 use R/W lock, we define Singleton pattern for this class.
 */
template<>
class DbManagerImpl<DbBackendType::SQLITE> :
        public Singleton< DbManagerImpl<DbBackendType::SQLITE> > {
private:
    /**
     * @brief friend class for Singleton pattern.
     */
    friend class Singleton< DbManagerImpl<DbBackendType::SQLITE> >;

protected:
    std::string connect_param; // database connection parameter
    sqlite3     *db = nullptr; // database handler

public:
    static const uint32_t s_bind_column_begin_index = 1; // sqlite3 bind column index starts from 1
    static const uint32_t s_read_column_begin_index = 0; // sqlite3 fetch column index starts from 0

protected:
    /**
     * @brief protected ctor to avoid direct instantiation, use Singleton instead.
     */
    DbManagerImpl(void) = default;

    /**
     * @brief protected dtor to avoid direct instantiation, use Singleton instead.
     */
    ~DbManagerImpl() {
        close();
    }

    DbManagerImpl(const DbManagerImpl &) = delete;
    DbManagerImpl &operator=(const DbManagerImpl &) = delete;

public: // database operation
    /**
     * @brief Get the database connection parameter.
     * @return connected status.
     */
    bool isConnected() const {
        return !connect_param.empty();
    }

    /**
     * @brief Connect to the database using the connection parameter.
     * @param c The connection parameter.
     * @return true if connected; otherwise, false.
     */
    bool connect(const std::string &c = "edadb.sqlite3.db") {
        if (isConnected()) {
            return true;
        }

        // connect to the database
        connect_param = c;
        if (!(sqlite3_open(connect_param.c_str(), &db) == SQLITE_OK)) {
            std::cerr << "Sqlite3 Error: can't open database: " << connect_param << std::endl;
            std::cerr << "Sqlite3 Error: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        // enable foreign key constraint
        if (!exec("PRAGMA foreign_keys = ON;")) {
            std::cerr << "Sqlite3 Error: can't enable foreign key constraint" << std::endl;
            return false;
        }

        // enable sqlite trace if needed
        #if _EDADB_DEBUG_TRACE_SQL_STMT_
            // register the trace callback to output the SQL statement
            registerTrace();
        #endif

        return true;
    } // connect

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
    } // exec

    /**
     * @brief Close the database.
     * @return true if closed; otherwise, false.
    */
    bool close() {
        // success close if not connected 
        if (!isConnected()) {
            return true;
        }

        bool closed = (sqlite3_close(db) == SQLITE_OK);
        if (!closed) {
            std::cerr << "Sqlite3 Error: can't close database: " << connect_param << std::endl;
            std::cerr << "Sqlite3 Error: " << sqlite3_errmsg(db) << std::endl;
        }
        connect_param.clear();
        db = nullptr;
        return closed;
    } // close


public: // sqlite3 statement operation 
    /**
     * @brief Initialize the SQL statement
     * @param stmt The sqlite3 statement
     * @return true if initialized; otherwise, false
     */
    bool initStatement(DbStatementImpl<DbBackendType::SQLITE> &dbstmt) {
        dbstmt.db = db;
        dbstmt.stmt = nullptr;
        dbstmt.zErrMsg = nullptr;

        return true;
    } // initStatement

    /**
     * @brief check the number of rows changed by the last statement.
     * @return The number of rows changed.
     */
    int changes() {
        return sqlite3_changes(db);
    }


private: // sqlite3 trace API
    void registerTrace() {
        // only register once per db handle; you can guard with a flag if you like
        sqlite3_trace_v2(
            db,
            SQLITE_TRACE_STMT,
            &DbManagerImpl::traceCallback,
            nullptr
        );
    }

    static int traceCallback(
        unsigned   type,
        void*      /*ctx*/,
        void*      pStmt,
        void*      /*unused*/
    ) {
        if(type != SQLITE_TRACE_STMT) 
            return 0;

        auto *stmt = reinterpret_cast<sqlite3_stmt*>(pStmt);

        // prepared statement, with placeholders 
        std::string raw_sql = sqlite3_sql(stmt);

        std::string exp_sql;
        #if SQLITE_VERSION_NUMBER >= 3014000
            // exepanded SQL, with bound values
            if(char *exp = sqlite3_expanded_sql(stmt)) {
                exp_sql.assign(exp);
                sqlite3_free(exp);
            }
        #endif

        if (!exp_sql.empty())  {
            if (raw_sql == exp_sql) {
                std::cout << "[EDADB DEBUG TRACE] << SQL STATEMENT: " << std::endl;
                std::cout << "    " << raw_sql << std::endl;
            } else {
                std::cout << "[EDADB DEBUG TRACE] << RAW SQL STATEMENT: " << std::endl;
                std::cout << "    " << raw_sql << std::endl;

                std::cout << "[EDADB DEBUG TRACE] << EXPANDED SQL STATEMENT: " << std::endl;
                std::cout << "    " << exp_sql << std::endl;
            } // if
        } // if
        std::cout << std::endl;

        return 0;
    }
};



// if Config::backend_type is SQLITE, use DbStatement4Sqlite as DbStatement
using DbManager = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE, 
        DbManagerImpl<DbBackendType::SQLITE> >;

} // namespace edadb