/**
 * @file perf_sugar_str.cpp
 * @brief test the soci sugar interface performance for int
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <vector>
#include <string>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "perf.h"

int test_sugar_performance_int(uint64_t recd_num, uint64_t query_num)
{
    std::cout << "\033[1;31mPerformance: sugar interface for int\033[0m" << std::endl;
    try {
        // create session and table
        soci::session sql(soci::sqlite3, "sugar.perf.int.db");
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, int1 INTEGER, int2 INTEGER, int3 INTEGER);";


        // insert records
        auto start_insert = std::chrono::high_resolution_clock::now();
        sql << "BEGIN;";
        for (uint64_t i = 0; i < recd_num; ++i) {
            // each statement will be prepared and executed separately
            std::string sql_str =
                "INSERT INTO person (id, int1, int2, int3) VALUES ("
                    + std::to_string(i) + ", "
                    + std::to_string(i) + ", " + std::to_string(i+1) + ", " + std::to_string(i+2) + ")";
            sql << sql_str;
        }
        sql << "COMMIT;";


        // scan all records
        auto start_scan = std::chrono::high_resolution_clock::now();
        uint64_t id_sum = 0, int1_sum = 0, int2_sum = 0, int3_sum = 0;
        for (uint64_t i = 0; i < query_num; ++i) {
            // query records
            soci::rowset<soci::row> rs =
                sql.prepare << "SELECT id, int1, int2, int3 FROM person;";
            for (const auto& row : rs) {
                // get the column values via names
                int id = row.get<int>("id");
                int int1 = row.get<int>("int1");
                int int2 = row.get<int>("int2");
                int int3 = row.get<int>("int3");

                // // get the column values via positions
                // int id = row.get<int>(0);
                // int int1 = row.get<int>(1);
                // int int2 = row.get<int>(2);
                // int int3 = row.get<int>(3);

                id_sum += id;
                int1_sum += int1, int2_sum += int2, int3_sum += int3; 
            }
        } 
        std::cout << ">> Scan all records result:";
        std::cout << "ID Sum: " << id_sum << ", Int1 Sum: " << int1_sum
            << ", Int2 Sum: " << int2_sum << ", Int3 Sum: " << int3_sum << std::endl;


        auto start_lookup = std::chrono::high_resolution_clock::now();
        for (uint64_t i = 0; i < query_num; ++i) {
            // query records
            soci::rowset<soci::row> rs =
                sql.prepare << "SELECT id, int1, int2, int3 FROM person WHERE id = 500;";
            for (const auto& row : rs) {
                // get the column values via names
                int id = row.get<int>("id");
                int int1 = row.get<int>("int1");
                int int2 = row.get<int>("int2");
                int int3 = row.get<int>("int3");

                // // get the column values via positions
                // int id = row.get<int>(0);
                // int int1 = row.get<int>(1);
                // int int2 = row.get<int>(2);
                // int int3 = row.get<int>(3);

                if (i == 0)  {
                    std::cout << ">> Lookup result: ";
                    std::cout << id << ", " << int1 << ", " << int2 << ", " << int3 << std::endl;
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Insert Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(start_scan - start_insert).count() << " ms" << std::endl;
        std::cout << "Scan Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(start_lookup - start_scan).count() << " ms" << std::endl;
        std::cout << "Lookup Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_lookup).count() << " ms" << std::endl;
        std::cout << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } 

    return 0;
} 