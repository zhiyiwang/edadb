/***
 * @file DbStatement.h
 * @brief DbStatement.h provides a way to manage the Sqlite3 statement.
 */

#pragma once

#include <string>
#include <iostream>

#include <sqlite3.h>

/**
 * @struct DbStatement
 * @brief This struct holds the sqlite3 statement and error message.
 */
struct DbStatement {
    // sqlite3 database connection handler
    sqlite3 *db = nullptr;

    // statement handler
    sqlite3_stmt *stmt = nullptr;

    // sqlite manager error message internal, no need to free
    // @see https://www.sqlite.org/c3ref/errcode.html
    const char *zErrMsg = nullptr;
    
public:
    DbStatement () = default;
    ~DbStatement() = default;

public:
    inline bool invalidDb     () { return (db   == nullptr); }
    inline bool stmtIsNull    () { return (stmt == nullptr); }
    inline bool stmtIsPrepared() { return (stmt != nullptr); }

    /**
     * @brief prepare the SQL statement
     */
    bool prepare(const std::string &sql) {
        if (invalidDb()) {
            std::cerr << "DbStatement::prepare: invalid database" << std::endl;
            return false;
        }

        if (stmtIsPrepared()) {
            std::cerr << "DbStatement::prepare: statement is already prepared" << std::endl;
            return false;
        }

        bool prepared =
            (sqlite3_prepare_v2(db, sql.c_str(), -1, &(stmt), 0) == SQLITE_OK);
        if (!prepared) {
            std::cerr << "DbStatement::prepare: can't prepare SQL: " << sql << std::endl;
            zErrMsg = sqlite3_errmsg(db);
            std::cerr << "DbStatement::prepare: " << zErrMsg << std::endl;
        }
        return prepared;
    }

    /**
     * @brief reset the SQL statement
     * @return true if reseted; otherwise, false.
     */
    bool reset() {
        bool reseted = (sqlite3_reset(stmt) == SQLITE_OK);
        if (!reseted) {
            std::cerr << "Sqlite3 Error: can't reset SQL" << std::endl;

            zErrMsg = sqlite3_errmsg(db);
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
        }
        return reseted;
    }

    /**
     * @brief finalize the SQL statement
     * @return true if finalized; otherwise, false.
     */
    bool finalize() {
        bool finalized = (sqlite3_finalize(stmt) == SQLITE_OK);
        if (!finalized) {
            std::cerr << "Sqlite3 Error: can't finalize SQL" << std::endl;

            zErrMsg = sqlite3_errmsg(db);
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
        }

        stmt = nullptr;
        return finalized;
    }


public: // insert column
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
        return (sqlite3_bind_int(stmt, index, *value) == SQLITE_OK);
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
        return (sqlite3_bind_int64(stmt, index, *value) == SQLITE_OK);
    }

    /**
     * @brief bind double type
     *     float, double, long double
     * @return true if binded; otherwise, false.
     */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, bool>
        bindColumn(int index, T *value) {
        return (sqlite3_bind_double(stmt, index, *value) == SQLITE_OK);
    }

    /**
     * @brief bind string type
     * @return true if binded; otherwise, false.
     */
    bool bindColumn(int index, std::string *value) {
        return (sqlite3_bind_text(stmt, index, value->c_str(), -1, SQLITE_STATIC) == SQLITE_OK);
    }
    bool bindColumn(int index, const char *value) {
        return (sqlite3_bind_text(stmt, index, value, -1, SQLITE_STATIC) == SQLITE_OK);
    }

    /**
     * @brief bind wstring type
     * @return true if binded; otherwise, false.
     */
    bool bindColumn(int index, std::wstring *value) {
        return (sqlite3_bind_text16(stmt, index, value->c_str(), -1, SQLITE_STATIC) == SQLITE_OK);
    }
    bool bindColumn(int index, const wchar_t *value) {
        return (sqlite3_bind_text16(stmt, index, value, -1, SQLITE_STATIC) == SQLITE_OK);
    }


    /**
     * @brief bind the column and execute the SQL statement.
     * @return true if inserted; otherwise, false.
     */
    bool bindStep() {
        bool stepped = (sqlite3_step(stmt) == SQLITE_DONE);
        if (!stepped) {
            std::cerr << "sqlite3_step failed" << std::endl;

            zErrMsg = sqlite3_errmsg(db);
            std::cerr << "Sqlite3 Error: " << zErrMsg << std::endl;
        }
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


public: // fetch 
    /**
     * call sqlite3_step to get the next row.
     * Since return may not be SQLITE_ROW, we need to check the return value.
     * @return true if the next row exists; otherwise, false.
     */
    bool fetchStep() {
        #if DEBUG_SQLITE3_SCAN
            std::cout << "DbManager::fetchStep" << std::endl;
        #endif

        return (sqlite3_step(stmt) == SQLITE_ROW);
    }

public: // fetch column
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
};