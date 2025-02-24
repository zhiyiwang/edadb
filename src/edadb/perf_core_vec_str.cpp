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


int test_core_vector_performance_str(uint64_t recd_num, uint64_t query_num)
{
    uint64_t bulk_size = 20; 
    uint64_t run_num = recd_num / bulk_size;
    std::cout << "\033[1;31mPerformance: soci core vector interface for string\033[0m"
        << ", run_num: " << run_num << std::endl;

    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "core.vector.perf.str.db");
        sql.set_query_context_logging_mode(soci::log_context::never);

        // create table
        const char* create_table_sql =
            "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, str1 TEXT, str2 TEXT, str3 TEXT);";
        sql << create_table_sql;

        uint64_t id = 0;
        std::string str1 = "Alice", str2 = "Bob", str3 = "Charlie";
        std::vector<int> ids(bulk_size, id);
        std::vector<std::string> str1s(bulk_size, str1), str2s(bulk_size, str2), str3s(bulk_size, str3);
        for (uint64_t i = 0; i < bulk_size; ++i) {
            ids[i] = id + i;
        }


        // insert records
        auto start_insert = std::chrono::high_resolution_clock::now();
        sql << "BEGIN;";

        const char *insert_sql = 
            "INSERT INTO person (id, str1, str2, str3) VALUES (:id, :str1, :str2, :str3)";
        {

            soci::statement st(sql);
            st.exchange(soci::use(ids));
            st.exchange(soci::use(str1s));
            st.exchange(soci::use(str2s));
            st.exchange(soci::use(str3s));
            st.alloc();
            st.prepare(insert_sql);
            st.define_and_bind();
            for (uint64_t i = 0; i < run_num; ++i)
            {
                st.execute(true);
                for (uint64_t j = 0; j < bulk_size; ++j) {
                    ids[j] += bulk_size;
                }

                // manually reset the statement
                soci::details::statement_backend *be = st.get_backend();
                dynamic_cast<soci::sqlite3_statement_backend *>(be)->reset();
            }
        }
        sql << "COMMIT;";


        // scan all records
        auto start_scan = std::chrono::high_resolution_clock::now();
        ids.clear(), str1s.clear(), str2s.clear(), str3s.clear();

        uint64_t id_sum = 0, str1_len = 0, str2_len = 0, str3_len = 0;
        const char *scan_sql = "SELECT id, str1, str2, str3 FROM person;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            ids.resize(bulk_size);
            str1s.resize(bulk_size), str2s.resize(bulk_size), str3s.resize(bulk_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(str1s));
            st.exchange(soci::into(str2s));
            st.exchange(soci::into(str3s));
            st.alloc();
            st.prepare(scan_sql);
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
                for (uint64_t j = 0; j < ids.size(); ++j) {
                    id_sum += ids[j];
                    str1_len += str1s[j].length(), str2_len += str2s[j].length(),
                    str3_len += str3s[j].length();
                }

                ids.resize(bulk_size);
                str1s.resize(bulk_size), str2s.resize(bulk_size), str3s.resize(bulk_size);
            }
        }
        if (PERF_OUTPUT_SQL_RESULT) {
            std::cout << ">> Scan all records result:" ;
            std::cout << "ID Sum: " << id_sum << ", str1 Length: " << str1_len
                << ", str2 Length: " << str2_len << ", str3 Length: " << str3_len << std::endl;
        }


        // lookup records
        auto start_lookup = std::chrono::high_resolution_clock::now();
        const char *lookup_sql = "SELECT id, str1, str2, str3 FROM person WHERE id = 500;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            ids.resize(bulk_size);
            str1s.resize(bulk_size), str2s.resize(bulk_size), str3s.resize(bulk_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(str1s));
            st.exchange(soci::into(str2s));
            st.exchange(soci::into(str3s));
            st.alloc();
            st.prepare(lookup_sql);
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
                for (uint64_t j = 0; j < ids.size(); ++j) {
                    if (PERF_OUTPUT_SQL_RESULT && (i == 0)) {
                        std::cout << ">> Lookup result: ";
                        std::cout << ids[j] << ", " << str1s[j] << ", " << str2s[j] << ", " << str3s[j];
                        std::cout << std::endl;
                    }
                }

                ids.resize(bulk_size);
                str1s.resize(bulk_size), str2s.resize(bulk_size), str3s.resize(bulk_size);
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
} // test_core_vector_performance_str