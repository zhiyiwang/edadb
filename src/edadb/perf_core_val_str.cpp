/**
 * @file test_core_value_performance_str.cpp
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <vector>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "perf.h"

int test_core_value_performance_str(uint64_t recd_num, uint64_t query_num)
{
    std::cout << "\033[1;31mPerformance: soci core value interface for string\033[0m" << std::endl;

    try {
        // create session and table
        soci::session sql(soci::sqlite3, "core.value.perf.str.db");
        sql.set_query_context_logging_mode(soci::log_context::never);
        const char* create_table_sql = 
            "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, str1 TEXT, str2 TEXT, str3 TEXT);";
        sql << create_table_sql;


        // insert records via single value binding
        auto start_insert = std::chrono::high_resolution_clock::now();
        sql << "BEGIN;";

        uint64_t id = 0;
        std::string str1 = "Alice", str2 = "Bob", str3 = "Charlie";
        const char* insert_sql = 
            "INSERT INTO person (id, str1, str2, str3) VALUES (:id, :str1, :str2, :str3)";
        {
            soci::statement st(sql);
            st.exchange(soci::use(id));
            st.exchange(soci::use(str1));
            st.exchange(soci::use(str2));
            st.exchange(soci::use(str3));
            st.alloc();
            st.prepare(insert_sql);
            st.define_and_bind();
            // insert and updating the value of id, name, and age
            for (uint64_t i = 0; i < recd_num; ++i) {
                st.execute(true);
                id += 1; // ignore the str1, str2, and str3
            }
        }
        sql << "COMMIT;";


        // scan all records
        auto start_scan = std::chrono::high_resolution_clock::now();

        // query records via single value binding
        id = 0, str1.clear(), str2.clear(), str3.clear();
        uint64_t id_sum = 0, str1_len = 0, str2_len = 0, str3_len = 0;
        const char *scan_sql = "SELECT id, str1, str2, str3 FROM person;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(id));
            st.exchange(soci::into(str1));
            st.exchange(soci::into(str2));
            st.exchange(soci::into(str3));
            st.alloc();
            st.prepare(scan_sql);
            st.define_and_bind();
            st.execute(false); // read all records
            while (st.fetch()) {
                id_sum += id;
                str1_len += str1.length(), str2_len += str2.length(), str3_len += str3.length();
            }
        }
        if (PERF_OUTPUT_SQL_RESULT) {
            std::cout << ">> Scan all records result:";
            std::cout << "ID Sum: " << id_sum << ", str1 Length: " << str1_len
                << ", str2 Length: " << str2_len << ", str3 Length: " << str3_len << std::endl;
        }


        // lookup records via single value binding
        auto start_lookup = std::chrono::high_resolution_clock::now();

        id = 0, str1.clear(), str2.clear(), str3.clear();
        const char *lookup_sql = "SELECT id, str1, str2, str3 FROM person WHERE id = 500;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(id));
            st.exchange(soci::into(str1));
            st.exchange(soci::into(str2));
            st.exchange(soci::into(str3));
            st.alloc();
            st.prepare(lookup_sql);
            st.define_and_bind();
            st.execute(false); // read all records

            while (st.fetch()) {
                if (PERF_OUTPUT_SQL_RESULT && (i == 0)) {
                    std::cout << ">> Lookup result: ";
                    std::cout << id << ", " << str1 << ", " << str2 << ", " << str3 << std::endl;
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto insert_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(start_scan - start_insert).count();
        auto scan_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(start_lookup - start_scan).count();
        auto lookup_eslapse = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_lookup).count();
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
