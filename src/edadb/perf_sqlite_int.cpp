/**
 * @file perf_sqlite_str.cpp
 * @brief test the soci sqlite3 interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
#include <assert.h>
#include <vector>
#include <string>
#include <iostream>
#include <sqlite3.h>

#include "perf.h"

int test_sqlite_performance_int(uint64_t recd_num, uint64_t query_num)
{
#if ((PERF_SQLITE_VALUE_ONLY == 0) && (PERF_SQLITE_STEP_ONLY == 0))
    std::cout << "\033[1;31mPerformance: sqlite3 interface for int\033[0m" << std::endl;
#endif

    sqlite3* db;
    sqlite3_stmt* stmt;
    const char* db_name = "sqlite3.perf.int.db";

    // open database
    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }


    // create table
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, int1 INTEGER, int2 INTEGER, int3 INTEGER) STRICT;";
    if (sqlite3_exec(db, create_table_sql, 0, 0, 0) != SQLITE_OK) {
        std::cerr << "Can't create table: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }


    //  insert data
    auto start_insert = std::chrono::high_resolution_clock::now();
    if (sqlite3_exec(db, "BEGIN;", 0, 0, 0) != SQLITE_OK) {
        std::cerr << "Can't begin transaction: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

    const char* insert_sql = 
        "INSERT INTO person (id, int1, int2, int3) VALUES (?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

#if (PERF_SQLITE_VALUE_ONLY == 1)
    for (uint64_t i = 0; i < recd_num; ++i) {
        sqlite3_bind_int(stmt, 1, i);  // bind id
        sqlite3_bind_int(stmt, 2, i);  // bind int1
        sqlite3_bind_int(stmt, 3, i+1);  // bind int2
        sqlite3_bind_int(stmt, 4, i+2);  // bind int3

        if (0) {
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            } 

            // number of rows modified by the last statement
            int inst_num = sqlite3_changes(db);  
            assert(inst_num == 1);
        }

        sqlite3_reset(stmt);  // reset the prepared statement
    }
#elif (PERF_SQLITE_STEP_ONLY == 1)
    for (uint64_t i = 0; i < recd_num; ++i) {
        sqlite3_bind_int(stmt, 1, i);  // bind id
        if (i == 0) {
            sqlite3_bind_int(stmt, 2, i);  // bind int1
            sqlite3_bind_int(stmt, 3, i+1);  // bind int2
            sqlite3_bind_int(stmt, 4, i+2);  // bind int3
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        } 

        // number of rows modified by the last statement
        int inst_num = sqlite3_changes(db);  
        assert(inst_num == 1);

        sqlite3_reset(stmt);  // reset the prepared statement
    }
#else
    for (uint64_t i = 0; i < recd_num; ++i) {
        sqlite3_bind_int(stmt, 1, i);  // bind id
        sqlite3_bind_int(stmt, 2, i);  // bind int1
        sqlite3_bind_int(stmt, 3, i+1);  // bind int2
        sqlite3_bind_int(stmt, 4, i+2);  // bind int3

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        } 

        // number of rows modified by the last statement
        int inst_num = sqlite3_changes(db);  
        assert(inst_num == 1);

        sqlite3_reset(stmt);  // reset the prepared statement
    } 
#endif

    sqlite3_finalize(stmt); // finalize the prepared statement

    if (sqlite3_exec(db, "COMMIT;", 0, 0, 0) != SQLITE_OK) {
        std::cerr << "Can't commit transaction: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }


    // scan all records
    auto start_scan = std::chrono::high_resolution_clock::now();
    uint64_t id_sum = 0, int1_sum = 0, int2_sum = 0, int3_sum = 0;
    for (uint64_t i = 0; i < query_num; ++i) {
        const char* select_sql = "SELECT id, int1, int2, int3 FROM person;";
        if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return 1;
        }

        // get column number and type
        int col_num = sqlite3_column_count(stmt);  
        assert(col_num == 4);
        assert(sqlite3_column_type(stmt, 0) == SQLITE_INTEGER);
        assert(sqlite3_column_type(stmt, 1) == SQLITE_INTEGER);
        assert(sqlite3_column_type(stmt, 2) == SQLITE_INTEGER);
        assert(sqlite3_column_type(stmt, 3) == SQLITE_INTEGER);

        std::string id_str  (sqlite3_column_name(stmt, 0));
        std::string int1_str(sqlite3_column_name(stmt, 1));
        std::string int2_str(sqlite3_column_name(stmt, 2));
        std::string int3_str(sqlite3_column_name(stmt, 3));
        assert(id_str == "id" && int1_str == "int1" && int2_str == "int2" && int3_str == "int3");

#if (PERF_SQLITE_VALUE_ONLY == 1)
        // use sqlite3 c++ api to get the first row, 
        // but get the values many times
        assert (sqlite3_step(stmt) == SQLITE_ROW); 
        
        for (int j = 0; j < recd_num; ++j) {
            // get values for each column
            int id = sqlite3_column_int(stmt, 0);  
            int int1 = sqlite3_column_int(stmt, 1);
            int int2 = sqlite3_column_int(stmt, 2);
            int int3 = sqlite3_column_int(stmt, 3);

            id_sum += id;
            int1_sum += int1, int2_sum += int2, int3_sum += int3;
        }
#elif (PERF_SQLITE_STEP_ONLY == 1)
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // get values from column id 
            int id = sqlite3_column_int(stmt, 0);  
            id_sum += id;
        }
#else
        // iterate over the result set
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // get values for each column
            int id   = sqlite3_column_int(stmt, 0);  
            int int1 = sqlite3_column_int(stmt, 1);
            int int2 = sqlite3_column_int(stmt, 2);
            int int3 = sqlite3_column_int(stmt, 3);

            id_sum += id;
            int1_sum += int1, int2_sum += int2, int3_sum += int3;

            // number of rows modified by the last statement
            int inst_num = sqlite3_changes(db);  
            assert(inst_num == 0);
        }
#endif

        sqlite3_finalize(stmt);  // finalize the prepared statement
    }
    if (PERF_OUTPUT_SQL_RESULT) {
        std::cout << ">> Scan all records result:" ;
        std::cout << "ID Sum: " << id_sum << ", int1 Sum: " << int1_sum;
        std::cout << ", int2 Sum: " << int2_sum << ", int3 Sum: " << int3_sum << std::endl;
    }


    auto start_lookup = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < query_num; ++i) {
        const char* select_sql = 
            "SELECT id, int1, int2, int3 FROM person WHERE id = 500;";
        if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return 1;
        }

        // get column number and type
        int col_num = sqlite3_column_count(stmt);  
        assert(col_num == 4);
        assert(sqlite3_column_type(stmt, 0) == SQLITE_INTEGER);
        assert(sqlite3_column_type(stmt, 1) == SQLITE_INTEGER);
        assert(sqlite3_column_type(stmt, 2) == SQLITE_INTEGER);
        assert(sqlite3_column_type(stmt, 3) == SQLITE_INTEGER);

        std::string id_str  (sqlite3_column_name(stmt, 0));
        std::string int1_str(sqlite3_column_name(stmt, 1));
        std::string int2_str(sqlite3_column_name(stmt, 2));
        std::string int3_str(sqlite3_column_name(stmt, 3));
        assert(id_str == "id" && int1_str == "int1" && int2_str == "int2" && int3_str == "int3");

        // iterate over the result set
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // get values for each column
            int id = sqlite3_column_int(stmt, 0);  
            int int1 = sqlite3_column_int(stmt, 1);
            int int2 = sqlite3_column_int(stmt, 2);
            int int3 = sqlite3_column_int(stmt, 3);

            if (PERF_OUTPUT_SQL_RESULT && (i == 0)) {
                std::cout << ">> Lookup result: ";
                std::cout << id << ", " << int1 << ", " << int2 << ", " << int3 << std::endl;
            }

            // number of rows modified by the last statement
            int inst_num = sqlite3_changes(db);  
            assert(inst_num == 0);
        }

        sqlite3_finalize(stmt);  // finalize the prepared statement
    }


    auto end = std::chrono::high_resolution_clock::now();
    sqlite3_close(db);  // close the database

    auto insert_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(start_scan - start_insert).count();
    auto scan_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(start_lookup - start_scan).count();
    auto lookup_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_lookup).count();
    if (PERF_OUTPUT_SQL_RESULT) {
        std::cout << "Insert Time: " << insert_eslapse << " ms" << std::endl;
        std::cout << "Scan Time: " << scan_eslapse << " ms" << std::endl;
        std::cout << "Lookup Time: " << lookup_eslapse << " ms" << std::endl;
    }
    else {
        std::cout << insert_eslapse << ",";
        std::cout << scan_eslapse   << ",";
        std::cout << lookup_eslapse << std::endl;
    }
    std::cout << std::endl;

    return 0;
} // test_sqlite_performance