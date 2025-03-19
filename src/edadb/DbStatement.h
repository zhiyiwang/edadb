/***
 * @file DbStatement.h
 * @brief DbStatement.h provides a way to manage the Sqlite3 statement.
 */

 #pragma once

 #include <sqlite3.h>

/**
 * @struct DbStatement
 * @brief This struct holds the sqlite3 statement and error message.
 */
struct DbStatement {
    // statement handler
    sqlite3_stmt *stmt = nullptr;

    // sqlite manager error message internal, no need to free
    // @see https://www.sqlite.org/c3ref/errcode.html
    const char *zErrMsg = nullptr;
    
public:
    DbStatement () = default;
    ~DbStatement() = default;
};

