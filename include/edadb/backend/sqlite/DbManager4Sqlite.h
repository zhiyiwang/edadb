/**
 * @file DbManager4Sqlite.h
 * @brief DbManager.hpp provides a way to manage the Sqlite3.
 *      This class is a child class of DatabaseManager.
 */

#pragma once

#include <type_traits>
#include <iostream>
#include <string>
#include <stdint.h>
#include <sqlite3.h>

#include "Macro4Sqlite.h"

#include "edadb/Config.h"
#include "edadb/Singleton.h"

#include "edadb/DbBackendType.h"
#include "edadb/DbStatement.h"
#include "edadb/backend/sqlite/DbStatement4Sqlite.h"
#include "edadb/DbManager.h"

#define _EDADB_DEBUG_TRACE_SQL_STMT_ 1

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
    // sqlite3 bind column index starts from 1
    static const uint32_t s_bind_column_begin_index = 1; 

    // sqlite3 fetch column index starts from 0
    static const uint32_t s_read_column_begin_index = 0; 

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
        int rc = sqlite3_open(connect_param.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "DbManager4Sqlite::connect[sqlite3_open] failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to open database using param: " + c);
            return false;
        }

        // enable foreign key constraint
        if (!exec("PRAGMA foreign_keys = ON;")) {
            std::cerr << "DbManager4Sqlite::connect[PRAGMA foreign_keys] failed!" << std::endl;
            return false;
        }

        // enable sqlite trace if needed
        #if _EDADB_DEBUG_TRACE_SQL_STMT_
            if (!checkForeignKeyEnabled()) {
                std::cerr << "DbManager4Sqlite::connect: foreign key constraint is not enabled!" << std::endl;
                return false;
            } 
            else {
                std::cout << "DbManager4Sqlite::connect: foreign key constraint is enabled." << std::endl;
            }

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
        int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
        bool executed = (rc == SQLITE_OK);
        if (!executed) {
            std::cerr << "DbManager4Sqlite::exec[sqlite3_exec] failed!" << std::endl;
            std::cerr << "Error: " << (zErrMsg ? zErrMsg : "Unknown error") << std::endl;
            sqlite3_free(zErrMsg); // free the error message
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to execute SQL: " + sql);
        } // if 
        return executed;
    } // exec


    /**
     * @brief close the database.
     * @return true if closed; otherwise, false.
    */
    bool close() {
        // success close if not connected 
        if ((!isConnected()) || (db == nullptr)) {
            connect_param.clear();
            return true;
        }

        // finalize all the prepared statements
        finalize_all_stmt();

        int rc = SQLITE_OK;
        for (int attempt = 0; attempt < 2; ++attempt) {
            rc = sqlite3_close_v2(db);
            if (rc != SQLITE_BUSY) break; // busy: retry to finalize; otherwise, break
            finalize_all_stmt();
        }

        // close success: reset db pointer
        if (rc == SQLITE_OK) {
            db = nullptr;  
            return true;
        }

        std::cerr << "DbManager4Sqlite::close[sqlite3_close_v2] failed!" << std::endl;
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;

        return false;
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


private:
    /**
     * @brief check if foreign key constraint is enabled.
     * @return true if enabled; otherwise, false.
     */
    bool checkForeignKeyEnabled(void) {
        // check if foreign key constraint is enabled
        sqlite3_stmt *stmt = nullptr;
        const char *sql = "PRAGMA foreign_keys;";
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "DbManager4Sqlite::checkForeignKeyEnabled: sqlite3_prepare_v2 failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to prepare PRAGMA foreign_keys statement");
            return false;
        }

        rc = sqlite3_step(stmt);
        bool enabled = (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) == 1);
        sqlite3_finalize(stmt);
        return enabled;
    } // checkForeignKeyEnabled


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
    } // traceCallback


private: // utility functions
    /**
     * @brief finalize all prepared statements for the database connection
     * @param db The sqlite3 database connection
    */
    void finalize_all_stmt(void) {
        // returns the next prepared statement for the database connection
        sqlite3_stmt* s = sqlite3_next_stmt(db, nullptr);
        while (s) {
            sqlite3_finalize(s);
            s = sqlite3_next_stmt(db, s);
        } // while
    } // finalize_all_stmt

}; // DbManagerImpl<DbBackendType::SQLITE>



// if Config::backend_type is SQLITE, use DbStatement4Sqlite as DbStatement
using DbManager = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE, 
        DbManagerImpl<DbBackendType::SQLITE> >;

} // namespace edadb