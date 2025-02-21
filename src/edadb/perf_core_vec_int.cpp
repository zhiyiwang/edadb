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


int test_core_vector_performance_int(uint64_t recd_num, uint64_t query_num)
{
    uint64_t bulk_size = 20; 
    uint64_t run_num = recd_num / bulk_size;
    std::cout << "\033[1;31mPerformance: soci core vector interface for int\033[0m"
        << ", run_num: " << run_num << std::endl;

    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "core.vector.perf.int.db");

        // create table
        const char* create_table_sql =
            "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, int1 INTEGER, int2 INTEGER, int3 INTEGER);";
        sql << create_table_sql;

        uint64_t id = 0, int1 = 0, int2 = 1, int3 = 2;
        std::vector<int> ids(bulk_size, id);
        std::vector<int> int1s(bulk_size, int1), int2s(bulk_size, int2), int3s(bulk_size, int3);
        for (uint64_t i = 0; i < bulk_size; ++i) {
            ids[i] = id + i; int1s[i] = int1 + i; int2s[i] = int2 + i; int3s[i] = int3 + i;
        }


        // insert records
        auto start_insert = std::chrono::high_resolution_clock::now();
        sql << "BEGIN;";

        const char *insert_sql = 
            "INSERT INTO person (id, int1, int2, int3) VALUES (:id, :int1, :int2, :int3)";
        {

            soci::statement st(sql);
            st.exchange(soci::use(ids));
            st.exchange(soci::use(int1s));
            st.exchange(soci::use(int2s));
            st.exchange(soci::use(int3s));
            st.alloc();
            st.prepare(insert_sql);
            st.define_and_bind();
            for (uint64_t i = 0; i < run_num; ++i)
            {
                st.execute(true);
                for (uint64_t j = 0; j < bulk_size; ++j) {
                    ids[j] += bulk_size;
                    int1s[j] += bulk_size, int2s[j] += bulk_size, int3s[j] += bulk_size;
                }

                // manually reset the statement
                soci::details::statement_backend *be = st.get_backend();
                dynamic_cast<soci::sqlite3_statement_backend *>(be)->reset();
            }
        }
        sql << "COMMIT;";


        // scan all records
        auto start_scan = std::chrono::high_resolution_clock::now();
        ids.clear(), int1s.clear(), int2s.clear(), int3s.clear();

        uint64_t id_sum = 0, int1_sum = 0, int2_sum = 0, int3_sum = 0;
        const char *scan_sql = "SELECT id, int1, int2, int3 FROM person;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            ids.resize(bulk_size);
            int1s.resize(bulk_size), int2s.resize(bulk_size), int3s.resize(bulk_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(int1s));
            st.exchange(soci::into(int2s));
            st.exchange(soci::into(int3s));
            st.alloc();
            st.prepare(scan_sql);
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
                for (uint64_t j = 0; j < ids.size(); ++j) {
                    id_sum += ids[j];
                    int1_sum += int1s[j], int2_sum += int2s[j], int3_sum += int3s[j];
                }

                ids.resize(bulk_size);
                int1s.resize(bulk_size), int2s.resize(bulk_size), int3s.resize(bulk_size);
            }
        }
        std::cout << ">> Scan all records result:" ;
        std::cout << "ID Sum: " << id_sum << ", int1 Sum: " << int1_sum
            << ", int2 Sum: " << int2_sum << ", int3 Sum: " << int3_sum << std::endl;


        // lookup records
        auto start_lookup = std::chrono::high_resolution_clock::now();
        const char *lookup_sql = "SELECT id, int1, int2, int3 FROM person WHERE id = 500;";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            ids.resize(bulk_size);
            int1s.resize(bulk_size), int2s.resize(bulk_size), int3s.resize(bulk_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(int1s));
            st.exchange(soci::into(int2s));
            st.exchange(soci::into(int3s));
            st.alloc();
            st.prepare(lookup_sql);
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
                for (uint64_t j = 0; j < ids.size(); ++j) {
                    if (i == 0) {
                        std::cout << ">> Lookup result: ";
                        std::cout << ids[j] << ", " << int1s[j] << ", " << int2s[j] << ", " << int3s[j] << std::endl;
                    }
                }

                ids.resize(bulk_size);
                int1s.resize(bulk_size), int2s.resize(bulk_size), int3s.resize(bulk_size);
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
} // test_core_vector_performance_str