/**
 * @file Macro4Sqlite.h
 * @brief Macro4Sqlite.h provides a macro for SQLite
 */

#pragma once

#include <iostream>
#include <assert.h>
#include <sqlite3.h>

/**
 * @macro SQLITE_LOG_ERROR
 * @brief Macro to log SQLite errors with error code and message
 * @param rc The return code from SQLite operation
 * @param db_handle The SQLite database handle
 * @param msg The custom message to log
 * @note This macro will print the error code, error string, and custom message to std::cerr
 */
#define SQLITE_LOG_ERROR(rc, db_handle, msg)                        \
    do {                                                            \
        std::cerr << "SQLite Error (" << (rc) << " "                \
                  << sqlite3_errstr(rc) << "): "                    \
                  << (msg) << ". Errmsg: "                          \
                  << sqlite3_errmsg(db_handle) << std::endl;        \
        assert(false);                                              \
    } while (0)


