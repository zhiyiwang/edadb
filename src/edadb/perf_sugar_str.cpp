/**
 * @file perf_sugar_str.cpp
 * @brief test the soci sugar interface performance for string
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <vector>
#include <string>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "perf.h"

int test_sugar_performance_str(uint64_t recd_num, uint64_t query_num)
{
    std::cout << "\033[1;31mPerformance: sugar interface for string\033[0m" << std::endl;
    try {
        // create session and table
        soci::session sql(soci::sqlite3, "sugar.perf.str.db");
        sql.set_query_context_logging_mode(soci::log_context::never);
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, str1 TEXT, str2 TEXT, str3 TEXT);";


        // insert records
        auto start_insert = std::chrono::high_resolution_clock::now();
        std::string str1 = "Alice", str2 = "Bob", str3 = "Charlie";
        sql << "BEGIN;";
        for (uint64_t i = 0; i < recd_num; ++i) {
            // each statement will be prepared and executed separately
            std::string sql_str =
                "INSERT INTO person (id, str1, str2, str3) VALUES ("
                    + std::to_string(i) + ", '" + str1 + "', '" + str2 + "', '" + str3 + "')";
            sql << sql_str;
        } 
        sql << "COMMIT;";


        // scan all records
        auto start_scan = std::chrono::high_resolution_clock::now();
        uint64_t id_sum = 0, str1_len = 0, str2_len = 0, str3_len = 0;
        for (uint64_t i = 0; i < query_num; ++i) {
            // query records
            soci::rowset<soci::row> rs =
                sql.prepare << "SELECT id, str1, str2, str3 FROM person;";
            for (const auto& row : rs) {
                // get the column values via names
                int id = row.get<int>("id");
                std::string str1 = row.get<std::string>("str1");
                std::string str2 = row.get<std::string>("str2");
                std::string str3 = row.get<std::string>("str3");

                // // get the column values via positions
                // int id = row.get<int>(0);
                // std::string str1 = row.get<std::string>(1);
                // std::string str2 = row.get<std::string>(2);
                // std::string str3 = row.get<std::string>(3);

                id_sum += id;
                str1_len += str1.length(), str2_len += str2.length(), str3_len += str3.length();
            }
        } 
        if (PERF_OUTPUT_SQL_RESULT) {
            std::cout << ">> Scan all records result:";
            std::cout << "ID Sum: " << id_sum << ", Str1 Length: " << str1_len;
            std::cout << ", Str2 Length: " << str2_len << ", Str3 Length: " << str3_len << std::endl;
        }

        auto start_lookup = std::chrono::high_resolution_clock::now();
        for (uint64_t i = 0; i < query_num; ++i) {
            // query records
            soci::rowset<soci::row> rs =
                sql.prepare << "SELECT id, str1, str2, str3 FROM person WHERE id = 500;";
            for (const auto& row : rs) {
                // get the column values via names
                int id = row.get<int>("id");
                std::string str1 = row.get<std::string>("str1");
                std::string str2 = row.get<std::string>("str2");
                std::string str3 = row.get<std::string>("str3");

                // // get the column values via positions
                // int id = row.get<int>(0);
                // std::string str1 = row.get<std::string>(1);
                // std::string str2 = row.get<std::string>(2);
                // std::string str3 = row.get<std::string>(3);

                if ((PERF_OUTPUT_SQL_RESULT) && (i == 0))  {
                    std::cout << ">> Lookup result: ";
                    std::cout << id << ", " << str1 << ", " << str2 << ", " << str3 << std::endl;
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto insert_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(start_scan - start_insert).count();
        auto scan_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(start_lookup - start_scan).count() / query_num;
        auto lookup_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_lookup).count() / query_num;
        if (PERF_OUTPUT_SQL_RESULT) {
            std::cout << "Insert Time: " << insert_eslapse << " ms" << std::endl;
            std::cout << "Scan Time: " << scan_eslapse << " ms" << std::endl;
            std::cout << "Lookup Time: " << lookup_eslapse << " ms" << std::endl;
        }
        else {
            std::cout << insert_eslapse << std::endl;
            std::cout << scan_eslapse   << std::endl;
            std::cout << lookup_eslapse << std::endl;
        }
        std::cout << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } 

    return 0;
} 