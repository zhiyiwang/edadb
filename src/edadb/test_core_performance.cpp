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


int test_core_value_performance(uint64_t recd_num, uint64_t query_num)
{
    std::cout << "test_core_value_performance:" << std::endl;

    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "core.val.perf.db");

        uint64_t id = 0, age = 1;
        std::string name = "Alice";

        auto start = std::chrono::high_resolution_clock::now();

        // create table
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";

        // insert records
        {
            soci::statement st(sql);
            st.exchange(soci::use(id));
            st.exchange(soci::use(name));
            st.exchange(soci::use(age));
            st.alloc();
            st.prepare("INSERT INTO person (id, name, age) VALUES (:id, :name, :age)");
            st.define_and_bind();
            // insert by updating the value of id, name, and age
            for (uint64_t i = 0; i < recd_num; ++i) {
                st.execute(true);
                id += 1, age += 1;
            }
        }

        auto mid = std::chrono::high_resolution_clock::now();

        // query records
        uint64_t id_sum = 0, age_sum = 0, name_len = 0; 
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(id));
            st.exchange(soci::into(name));
            st.exchange(soci::into(age));
            st.alloc();
            st.prepare("SELECT id, name, age FROM person");
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
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
} // test_core_value_performance



int test_core_vector_performance(uint64_t recd_num, uint64_t query_num)
{
    std::cout << "test_core_vector_performance:" << std::endl;

    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "core.vec.perf.db");

        uint64_t bulk_size = 20; 
        uint64_t id = 0, age = 30;
        std::string name = "Alice";

        auto start = std::chrono::high_resolution_clock::now();

        // create table
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";

        // insert records
        {
            std::vector<int> ids, ages; 
            std::vector<std::string> names;
            for (uint64_t i = 0; i < bulk_size; ++i) {
                ids.push_back(id + i);
                names.push_back(name);
                ages.push_back(age + i);
            }

            soci::statement st(sql);
            st.exchange(soci::use(ids));
            st.exchange(soci::use(names));
            st.exchange(soci::use(ages));
            st.alloc();
            st.prepare("INSERT INTO person (id, name, age) VALUES (:id, :name, :age)");
            st.define_and_bind();
            // insert by updating the value of id, name, and age
            uint64_t run = recd_num / bulk_size;
            std::cout << "Run: " << run << std::endl;
            for (uint64_t i = 0; i < run; ++i)
            {
                std::cout << "Run Index: " << i << std::endl;
                std::cout << ids.size() << std::endl;
                std::cout << names.size() << std::endl;
                std::cout << ages.size() << std::endl;
                st.execute(true);

                for (uint64_t j = 0; j < bulk_size; ++j) {
                    ids[j] += bulk_size, ages[j] += bulk_size;
                    std::cout << "Index: " << i << ", ID: " << ids[j] << ", Name: " << names[j] << ", Age: " << ages[j] << std::endl;
                }
            }
        }
        return 0;

        auto mid = std::chrono::high_resolution_clock::now();

        // query records
        std::vector<int> ids, ages; 
        std::vector<std::string> names;
        ids.resize  (bulk_size);
        names.resize(bulk_size);
        ages.resize (bulk_size);

        uint64_t id_sum = 0, age_sum = 0, name_len = 0; 
        for (uint64_t i = 0; i < query_num; ++i)
        {
            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(names));
            st.exchange(soci::into(ages));
            st.alloc();
            st.prepare("SELECT id, name, age FROM person");
            st.define_and_bind();
            st.execute(false);
            while (st.fetch()) {
                for (uint64_t j = 0; j < bulk_size; ++j) {
                    id_sum += ids[j], age_sum += ages[j], name_len += names[j].length();
                }
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
} // test_core_value_performance



/**
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_core_performance(uint64_t recd_num, uint64_t query_num) {

    test_core_value_performance(recd_num, query_num);

//    test_core_vector_performance(recd_num, query_num);

    return 0;
} // test_core_performance
