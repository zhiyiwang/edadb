/**
 * @file test_core_performance.cpp
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <vector>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "test_funcs.h"

/**
 * @brief test the soci core interface performance using single value binding
 */
int test_core_value_performance(uint64_t recd_num, uint64_t query_num)
{
    std::cout << "test_core_value_performance:" << std::endl;

    try {
        // create session and table
        soci::session sql(soci::sqlite3, "core.value.perf.db");
        sql << perf_create_table_sql;


        // insert records via single value binding
        auto start_insert = std::chrono::high_resolution_clock::now();
        uint64_t id = 0, age = 1;
        std::string name = "Alice";

        {
            soci::statement st(sql);
            st.exchange(soci::use(id));
            st.exchange(soci::use(name));
            st.exchange(soci::use(age));
            st.alloc();
            st.prepare(perf_insert_table_sql);
            st.define_and_bind();
            // insert and updating the value of id, name, and age
            for (uint64_t i = 0; i < recd_num; ++i) {
                st.execute(true);
                id += 1, age += 1; // ignore the name, which is std::string
            }
        }


        auto start_scan = std::chrono::high_resolution_clock::now();

        // query records via single value binding
        uint64_t id_sum = 0, age_sum = 0, name_len = 0; 
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(id));
            st.exchange(soci::into(name));
            st.exchange(soci::into(age));
            st.alloc();
            st.prepare(perf_scan_table_sql);
            st.define_and_bind();
            st.execute(false); // read all records
            while (st.fetch()) {
                id_sum += id, age_sum += age, name_len += name.length();
            }
        }
        std::cout << ">> Scan all records result:";
        std::cout << "ID Sum: " << id_sum << ", Age Sum: " << age_sum
            << ", Name Length: " << name_len << std::endl;


        std::cout << ">> Lookup result: ";
        auto start_lookup = std::chrono::high_resolution_clock::now();
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(id));
            st.exchange(soci::into(name));
            st.exchange(soci::into(age));
            st.alloc();
            st.prepare(perf_lookup_table_sql);
            st.define_and_bind();
            st.execute(false); // read all records

            while (st.fetch()) {
                if (i == 0)
                    std::cout << id << ", " << name << ", " << age << std::endl;
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
} // test_core_value_performance



int test_core_bulk_performance(uint64_t recd_num, uint64_t query_num)
{
    uint64_t bulk_size = 20; 
    uint64_t run_num = recd_num / bulk_size;
    std::cout << "test_core_bulk_performance: use bulk_size: " << bulk_size
        << ", run_num: " << run_num << std::endl;

    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "core.bulk.perf.db");

        // create table
        sql << perf_create_table_sql;

        auto start_insert = std::chrono::high_resolution_clock::now();

        // insert records
        {
            std::string name = "Alice";
            std::vector<std::string> names(bulk_size, name);
            uint64_t id = 0, age = 1;
            std::vector<int> ids(bulk_size, id), ages(bulk_size, age);
            for (uint64_t i = 0; i < bulk_size; ++i) {
                ids[i] = id + i, ages[i] = age + i;
            }

            soci::statement st(sql);
            st.exchange(soci::use(ids));
            st.exchange(soci::use(names));
            st.exchange(soci::use(ages));
            st.alloc();
            st.prepare(perf_insert_table_sql);
            st.define_and_bind();
            // insert by updating the value of id, name, and age
            for (uint64_t i = 0; i < run_num; ++i)
            {
//                std::cout << "Run Index: " << i 
//                    << "  ids size " << ids.size() 
//                    << ", names size " << names.size()
//                    << ", ages size " << ages.size() << std::endl;
//                std::cout << "Index: " << i << ", ID: " << ids[j] << ", Name: " << names[j]
//                    << ", Age: " << ages[j] << std::endl;
//                std::cout << std::flush;
                st.execute(true);

                for (uint64_t j = 0; j < bulk_size; ++j) {
                    ids[j] += bulk_size, ages[j] += bulk_size;
                }
            }
        }

        auto start_scan = std::chrono::high_resolution_clock::now();

        // query records
        std::vector<int> ids, ages; 
        std::vector<std::string> names;

        uint64_t id_sum = 0, age_sum = 0, name_len = 0; 
        for (uint64_t i = 0; i < query_num; ++i)
        {
            ids.resize(bulk_size), names.resize(bulk_size), ages.resize(bulk_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(names));
            st.exchange(soci::into(ages));
            st.alloc();
            st.prepare(perf_scan_table_sql);
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
                for (uint64_t j = 0; j < ids.size(); ++j) {
                    id_sum += ids[j], age_sum += ages[j], name_len += names[j].length();
                }

                ids.resize(bulk_size), names.resize(bulk_size), ages.resize(bulk_size);
            }
        }
        std::cout << ">> Scan all records result:" ;
        std::cout << "ID Sum: " << id_sum << ", Age Sum: " << age_sum
            << ", Name Length: " << name_len << std::endl;


        auto start_lookup = std::chrono::high_resolution_clock::now();
        std::cout << ">> Lookup result: ";
        for (uint64_t i = 0; i < query_num; ++i)
        {
            ids.resize(bulk_size), names.resize(bulk_size), ages.resize(bulk_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(names));
            st.exchange(soci::into(ages));
            st.alloc();
            st.prepare(perf_lookup_table_sql);
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
                for (uint64_t j = 0; j < ids.size(); ++j) {
                    if (i == 0)
                        std::cout << ids[j] << ", " << names[j] << ", " << ages[j] << std::endl;
                }

                ids.resize(bulk_size), names.resize(bulk_size), ages.resize(bulk_size);
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
} // test_core_bulk_performance



/**
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_core_performance(uint64_t recd_num, uint64_t query_num) {

    test_core_value_performance(recd_num, query_num);

    test_core_bulk_performance (recd_num, query_num);

    return 0;
} // test_core_performance
