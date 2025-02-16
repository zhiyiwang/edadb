/**
 * @file test_sugar_performance.cpp
 * @brief test the soci sugar interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <vector>
#include <string>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "test_funcs.h"

/**
 * @brief test the soci sugar interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sugar_performance(uint64_t recd_num, uint64_t query_num) {

    std::cout << "test_sugar_performance:" << std::endl;

    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "sugar.perf.db");

        // create table
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";

        auto start = std::chrono::high_resolution_clock::now();

        // insert records
        std::string name = "Alice";
        for (uint64_t i = 0; i < recd_num; ++i) {
            // insert records
            // each statement will be prepared and executed separately
            std::string sql_str =
                "INSERT INTO person (id, name, age) VALUES ("
                    + std::to_string(i) + ", '" + name + "', " + std::to_string(1 + i) + ")";
            sql << sql_str;
        } 

        auto mid = std::chrono::high_resolution_clock::now();


        uint64_t id_sum = 0, age_sum = 0, name_len = 0; 
        for (uint64_t i = 0; i < query_num; ++i) {
            // query records
            soci::rowset<soci::row> rs = sql.prepare << "SELECT id, name, age FROM person";
            for (const auto& row : rs) {
                int id = row.get<int>("id");
                std::string name = row.get<std::string>("name");
                int age = row.get<int>("age");

                id_sum += id, age_sum += age, name_len += name.length();
            }
        } 
        std::cout << "ID Sum: " << id_sum << ", Age Sum: " << age_sum << ", Name Length: " << name_len << std::endl;

        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "Insert Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count() << " ms" << std::endl;
        std::cout << "Query Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count() << " ms" << std::endl;
        std::cout << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } 

    return 0;
} // test_sugar_performance