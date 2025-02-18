/**
 * @file test_sqlite_performance.cpp
 * @brief test the soci sqlite3 interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
#include <assert.h>
#include <vector>
#include <string>
#include <iostream>
#include <sqlite3.h>

#include "test_funcs.h"

/**
 * @brief test the soci sqlite3 interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

int test_sqlite_performance(uint64_t recd_num, uint64_t query_num) 
{
    std::cout << "test_sqlite_performance:" << std::endl;

    sqlite3* db;
    sqlite3_stmt* stmt;
    const char* db_name = "sqlite3.db";

    // open database
    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // create table
    const char* create_table_sql = perf_create_table_sql.c_str();
    if (sqlite3_exec(db, create_table_sql, 0, 0, 0) != SQLITE_OK) {
        std::cerr << "Can't create table: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }


    auto start_insert = std::chrono::high_resolution_clock::now();

    //  insert data
    const char* insert_sql = perf_insert_table_sql.c_str();
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

    for (uint64_t i = 0; i < recd_num; ++i) {
        sqlite3_bind_int(stmt, 1, i);  // bind id
        sqlite3_bind_text(stmt, 2, "Alice", -1, SQLITE_STATIC);  // bind name
        sqlite3_bind_int(stmt, 3, i + 1);  // bind age

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        } 

        // number of rows modified by the last statement
        int inst_num = sqlite3_changes(db);  
        assert(inst_num == 1);

        sqlite3_reset(stmt);  // reset the prepared statement
    }

    sqlite3_finalize(stmt);  // finalize the prepared statement


    auto start_scan = std::chrono::high_resolution_clock::now();
    uint64_t id_sum = 0, age_sum = 0, name_len = 0; 
    for (uint64_t i = 0; i < query_num; ++i) {
        const char* select_sql = perf_scan_table_sql.c_str();
        if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return 1;
        }

        // get column number and type
        int col_num = sqlite3_column_count(stmt);  
        assert(col_num == 3); 
        assert(sqlite3_column_type(stmt, 0) == SQLITE_INTEGER);  
        assert(sqlite3_column_type(stmt, 1) == SQLITE_TEXT);
        assert(sqlite3_column_type(stmt, 2) == SQLITE_INTEGER);

        std::string id_str  (sqlite3_column_name(stmt, 0));
        std::string name_str(sqlite3_column_name(stmt, 1));
        std::string age_str (sqlite3_column_name(stmt, 2));
        assert(id_str == "id" && name_str == "name" && age_str == "age");

        // iterate over the result set
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // get values for each column
            int id = sqlite3_column_int(stmt, 0);  
            const char* name = (const char*)sqlite3_column_text(stmt, 1); 
            int age = sqlite3_column_int(stmt, 2);  

            id_sum += id, age_sum += age, name_len += *name - 'A' + 5; // "Alice" -> 5

            // number of rows modified by the last statement
            int inst_num = sqlite3_changes(db);  
            assert(inst_num == 0);
        }

        sqlite3_finalize(stmt);  // finalize the prepared statement
    }
    std::cout << ">> Scan all records result:" ;
    std::cout << "ID Sum: " << id_sum << ", Age Sum: " << age_sum 
        << ", Name Length: " << name_len << std::endl;


    auto start_lookup = std::chrono::high_resolution_clock::now();
    std::cout << ">> Lookup result: ";
    for (uint64_t i = 0; i < query_num; ++i) {
        const char* select_sql = perf_lookup_table_sql.c_str();
        if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return 1;
        }

        // get column number and type
        int col_num = sqlite3_column_count(stmt);  
        assert(col_num == 3); 
        assert(sqlite3_column_type(stmt, 0) == SQLITE_INTEGER);  
        assert(sqlite3_column_type(stmt, 1) == SQLITE_TEXT);
        assert(sqlite3_column_type(stmt, 2) == SQLITE_INTEGER);

        std::string id_str  (sqlite3_column_name(stmt, 0));
        std::string name_str(sqlite3_column_name(stmt, 1));
        std::string age_str (sqlite3_column_name(stmt, 2));
        assert(id_str == "id" && name_str == "name" && age_str == "age");

        // iterate over the result set
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // get values for each column
            int id = sqlite3_column_int(stmt, 0);  
            const char* name = (const char*)sqlite3_column_text(stmt, 1); 
            int age = sqlite3_column_int(stmt, 2);  

            if (i == 0) 
                std::cout << id << ", " << name << ", " << age << std::endl;

            // number of rows modified by the last statement
            int inst_num = sqlite3_changes(db);  
            assert(inst_num == 0);
        }

        sqlite3_finalize(stmt);  // finalize the prepared statement
    }


    auto end = std::chrono::high_resolution_clock::now();

    sqlite3_close(db);  // close the database

    std::cout << "Insert Time: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(start_scan - start_insert).count() << " ms" << std::endl;
    std::cout << "Scan Time: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(start_lookup - start_scan).count() << " ms" << std::endl;
    std::cout << "Lookup Time: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_lookup).count() << " ms" << std::endl;
    std::cout << std::endl;

    return 0;
} // test_sqlite_performance