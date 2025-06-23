/***
 * @file DbStatement4Sqlite.h
 * @brief DbStatement4Sqlite.h provides a way to manage the Sqlite3 statement.
 */

#pragma once

#include <string>
#include <iostream>

#include <sqlite3.h>

#include "DbBackendType.h"
#include "DbStatement.h"
#include "Macro4Sqlite.h"


namespace edadb {

/**
 * @struct DbStatement
 * @brief This struct holds the sqlite3 statement and error message.
 */
template <>
struct DbStatementImpl<DbBackendType::SQLITE> {
    // sqlite3 database connection handler
    sqlite3 *db = nullptr;

    // statement handler
    sqlite3_stmt *stmt = nullptr;

    // sqlite manager error message internal, no need to free
    // @see https://www.sqlite.org/c3ref/errcode.html
    const char *zErrMsg = nullptr;
    
public:
    DbStatementImpl () = default;
    ~DbStatementImpl() = default;

public:
    inline bool invalidDb     () { return (db   == nullptr); }
    inline bool stmtIsNull    () { return (stmt == nullptr); }
    inline bool stmtIsPrepared() { return !stmtIsNull(); }

    /**
     * @brief prepare the SQL statement
     */
    bool prepare(const std::string &sql) {
        if (invalidDb()) {
            std::cerr << "DbStatementImpl::prepare: invalid database" << std::endl;
            return false;
        }

        if (stmtIsPrepared()) {
            std::cerr << "DbStatementImpl::prepare: statement is already prepared" << std::endl;
            return false;
        }

        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &(stmt), 0);
        bool prepared = (rc == SQLITE_OK);
        if (!prepared) {
            std::cerr << "DbStatementImpl::prepare: sqlite3_prepare_v2 failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to prepare SQL: " + sql);
        }
        return prepared;
    }

    /**
     * @brief reset the SQL statement
     * @return true if reseted; otherwise, false.
     */
    bool reset() {
        int rc = sqlite3_reset(stmt);
        bool reseted = (rc == SQLITE_OK);
        if (!reseted) {
            std::cerr << "DbStatementImpl::reset: sqlite3_reset failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to reset SQL statement");
        }
        return reseted;
    }

    /**
     * @brief finalize the SQL statement
     * @return true if finalized; otherwise, false.
     */
    bool finalize() {
        int rc = sqlite3_finalize(stmt);
        bool finalized = (rc == SQLITE_OK);
        if (!finalized) {
            std::cerr << "DbStatementImpl::finalize: sqlite3_finalize failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to finalize SQL statement");
        }

        stmt = nullptr;
        return finalized;
    }


public: // insert column
    /**
     * @brief bind null to the column
     * @return true if binded; otherwise, false.
     */
    bool bindNull(int index) {
        int rc = sqlite3_bind_null(stmt, index);
        if (rc != SQLITE_OK) {
            std::cerr << "DbStatementImpl::bindNull: sqlite3_bind_null failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to bind null at index " + std::to_string(index));
        }
        return (rc == SQLITE_OK);
    }

    /**
     * @brief bind to 32B integer type
     *   the size of T should be less than or equal to int
     *      bool, (unsigned) char, (unsigned) short, (unsigned) int
     *   use enable_if_t for SFINAE（Substitution Failure Is Not An Error)
     * @return true if binded; otherwise, false.
     */
    template <typename T>
    std::enable_if_t<std::is_integral_v<T> && (sizeof(T) <= sizeof(int)), bool>
        bindColumn(int index, T *value) {
        int rc = sqlite3_bind_int(stmt, index, *value);
        if (rc != SQLITE_OK) {
            std::cerr << "DbStatementImpl::bindColumn: sqlite3_bind_int failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to bind column at index " + std::to_string(index));
        }
        return (rc == SQLITE_OK);
    }

    /**
     * @brief bind to 64B integer type
     *   the size of T should be greater than int: enable_if
     *      (unsigned) long, (unsigned) long long
     * @return true if binded; otherwise, false.
     */
    template <typename T>
    std::enable_if_t<std::is_integral_v<T> && (sizeof(T) > sizeof(int)), bool>
        bindColumn(int index, T *value) {
        int rc = sqlite3_bind_int64(stmt, index, *value);
        if (rc != SQLITE_OK) {
            std::cerr << "DbStatementImpl::bindColumn: sqlite3_bind_int64 failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to bind column at index " + std::to_string(index));
        }
        return (rc == SQLITE_OK);
    }

    /**
     * @brief bind double type
     *     float, double, long double
     * @return true if binded; otherwise, false.
     */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, bool>
        bindColumn(int index, T *value) {
        int rc = sqlite3_bind_double(stmt, index, *value);
        if (rc != SQLITE_OK) {
            std::cerr << "DbStatementImpl::bindColumn: sqlite3_bind_double failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to bind column at index " + std::to_string(index));
        }
        return (rc == SQLITE_OK);
    }

    /**
     * @brief bind string type
     * @return true if binded; otherwise, false.
     */
    bool bindColumn(int index, std::string *value) {
        return bindColumn(index, value->c_str());
    }
    bool bindColumn(int index, const char *value) {
        int rc = sqlite3_bind_text(stmt, index, value, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            std::cerr << "DbStatementImpl::bindColumn: sqlite3_bind_text failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to bind column at index " + std::to_string(index));
        }
        return (rc == SQLITE_OK);
    }

    /**
     * @brief bind wstring type
     * @return true if binded; otherwise, false.
     */
    bool bindColumn(int index, std::wstring *value) {
        return bindColumn(index, value->c_str());
    }
    bool bindColumn(int index, const wchar_t *value) {
        int rc = sqlite3_bind_text16(stmt, index, value, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            std::cerr << "DbStatementImpl::bindColumn: sqlite3_bind_text16 failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to bind column at index " + std::to_string(index));
        }
        return (rc == SQLITE_OK);
    }


    /**
     * @brief bind the column and execute the SQL statement.
     * @return true if inserted; otherwise, false.
     */
    bool bindStep() {
        int rc = sqlite3_step(stmt);
        bool stepped = (rc == SQLITE_DONE);
        if (!stepped) {
            std::cerr << "DbStatementImpl::bindStep: sqlite3_step failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to step SQL statement");
        }

        #if _DEADB_DEBUG_TRACE_SQL_STMT_
            // use sqlite3_changes to check the number of rows changed
            int changes = sqlite3_changes(db);
            std::cout << "## [Debug EDADB] DbStatement::bindStep sqlite3_changes: "
                    << changes << std::endl;
        #endif

        return stepped;
    }


public: // schema info
    /**
     * @brief Get the column count of the statement.
     * @return The column count.
     */
    int getColumnCount() {
        return sqlite3_column_count(stmt);
    }
    
    /**
     * @brief Get the column type of the statement.
     * @param index The column index
     * @return The column type.
     */
    int getColumnType(int index) {
        return sqlite3_column_type(stmt, index);
    }

    /**
     * @brief Get the column name of the statement.
     * @param index The column index
     * @return The column name.
     */
    const char *getColumnName(int index) {
        return sqlite3_column_name(stmt, index);
    }


public: // fetch column
    /**
     * call sqlite3_step to get the next row.
     * Since return may not be SQLITE_ROW, we need to check the return value.
     * @return true if the next row exists; otherwise, false.
     */
    bool fetchStep() {
        #if DEBUG_SQLITE3_SCAN
            std::cout << "DbManager::fetchStep" << std::endl;
        #endif

        int rc = sqlite3_step(stmt);
        // get one row or read done
        if ((rc != SQLITE_ROW) && (rc != SQLITE_DONE)) {
            std::cerr << "DbStatementImpl::fetchStep: sqlite3_step failed!" << std::endl;
            EDADB_SQLITE_LOG_ERROR(rc, db, "Failed to fetch step SQL statement");
        }
        return (rc == SQLITE_ROW);
    }

    /**
     * @brief try to fetch null from the column using the column index.
     * @param index The column index.
     * @return true if fetched; otherwise, false.
     */
    bool fetchNull(int index) {
        return  (sqlite3_column_type(stmt, index) == SQLITE_NULL);
    } // fetchNull

    /**
     * @brief fetch from 32B integer type
     *   the size of T should be less than or equal to int: enable_if
     *      bool, (unsigned) char, (unsigned) short, (unsigned) int
     * @return true if fetched; otherwise, false.
     */
    template <typename T>
    std::enable_if_t<std::is_integral_v<T> && (sizeof(T) <= sizeof(int)), bool>
        fetchColumn(int index, T *value) {
        *value = sqlite3_column_int(stmt, index);
        return true;
    }

    /**
     * @brief fetch from 64B integer type
     *   the size of T should be greater than int: enable_if
     *   (unsigned) long, (unsigned) long long
     * @return true if fetched; otherwise, false.
     */
    template <typename T>
    std::enable_if_t<std::is_integral_v<T> && (sizeof(T) > sizeof(int)), bool>
        fetchColumn(int index, T *value) {
        *value = sqlite3_column_int64(stmt, index);
        return true;
    }

    /**
     * @brief fetch double type
     *  float, double, long double
     * @return true if fetched; otherwise, false.
     */
    template <typename T>
    std::enable_if_t<std::is_floating_point<T>::value, bool>
        fetchColumn(int index, T *value) {
        *value = sqlite3_column_double(stmt, index);
        return true;
    }

    /**
     * @brief fetch string type
     * @return true if fetched; otherwise, false.
     */
    bool fetchColumn(int index, std::string *value) {
        uint32_t size = sqlite3_column_bytes(stmt, index);
        const char  *bin = (const char*)sqlite3_column_text(stmt, index);
        value->assign(bin, size);
        return true;
    }
    bool fetchColumn(int index, const char **value) {
        *value = (const char*)sqlite3_column_text(stmt, index);
        return true;
    }

    /**
     * @brief fetch wstring type
     * @return true if fetched; otherwise, false.
     */
    bool fetchColumn(int index, std::wstring *value) {
        uint32_t size = sqlite3_column_bytes16(stmt, index);
        const wchar_t  *bin = (const wchar_t*)sqlite3_column_text16(stmt, index);
        value->assign(bin, size);
        return true;
    }
    bool fetchColumn(int index, const wchar_t **value) {
        *value = (const wchar_t*)sqlite3_column_text16(stmt, index);
        return true;
    }
}; // DbStatementImpl



// if Config::backend_type is SQLITE, use DbStatement4Sqlite as DbStatement
using DbStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE, 
        DbStatementImpl<DbBackendType::SQLITE>>;

} // namespace edadb