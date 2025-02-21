/**
 * @file test_core_value_performance_int.cpp
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <vector>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "perf.h"

int test_core_value_performance_int(uint64_t recd_num, uint64_t query_num)
{
    std::cout << "\033[1;31mPerformance: soci core value interface for int\033[0m" << std::endl;

    try {
        // create session and table
        soci::session sql(soci::sqlite3, "core.value.perf.int.db");
        const char* create_table_sql = 
            "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, int1 INTEGER, int2 INTEGER, int3 INTEGER);";
        sql << create_table_sql;


        // insert records via single value binding
        auto start_insert = std::chrono::high_resolution_clock::now();
        sql << "BEGIN;";

        uint64_t id = 0, int1 = 0, int2 = 1, int3 = 2;
        const char* insert_sql = 
            "INSERT INTO person (id, int1, int2, int3) VALUES (:id, :int1, :int2, :int3)";
        {
            soci::statement st(sql);
            st.exchange(soci::use(id));
            st.exchange(soci::use(int1));
            st.exchange(soci::use(int2));
            st.exchange(soci::use(int3));
            st.alloc();
            st.prepare(insert_sql);
            st.define_and_bind();
            // insert and updating the value of id, name, and age
            for (uint64_t i = 0; i < recd_num; ++i) {
                st.execute(true);
                id += 1, int1 += 1, int2 += 1, int3 += 1;
            }
        }
        sql << "COMMIT;";


        // scan all records
        auto start_scan = std::chrono::high_resolution_clock::now();

        // query records via single value binding
        id = 0, int1 = 0, int2 = 0, int3 = 0;
        uint64_t id_sum = 0, int1_sum = 0, int2_sum = 0, int3_sum = 0;
        const char *scan_sql = "SELECT id, int1, int2, int3 FROM person;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(id));
            st.exchange(soci::into(int1));
            st.exchange(soci::into(int2));
            st.exchange(soci::into(int3));
            st.alloc();
            st.prepare(scan_sql);
            st.define_and_bind();
            st.execute(false); // read all records
            while (st.fetch()) {
                id_sum += id;
                int1_sum += int1, int2_sum += int2, int3_sum += int3;
            }
        }
        std::cout << ">> Scan all records result:";
        std::cout << "ID Sum: " << id_sum << ", int1 Sum: " << int1_sum
            << ", int2 Sum: " << int2_sum << ", int3 Sum: " << int3_sum << std::endl;


        // lookup records via single value binding
        auto start_lookup = std::chrono::high_resolution_clock::now();

        id = 0, int1 = 0, int2 = 0, int3 = 0;
        const char *lookup_sql = "SELECT id, int1, int2, int3 FROM person WHERE id = 500;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(id));
            st.exchange(soci::into(int1));
            st.exchange(soci::into(int2));
            st.exchange(soci::into(int3));
            st.alloc();
            st.prepare(lookup_sql);
            st.define_and_bind();
            st.execute(false); // read all records
            while (st.fetch()) {
                if (i == 0) {
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

