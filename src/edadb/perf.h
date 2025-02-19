/**
 * @file perf.h
 * @brief test functions for the soci interface performance
 */

#ifndef PERF_H
#define PERF_H

// time measurement
#include <chrono> 
#include <string>


//// test the soci interface performance ////

const std::string perf_create_table_sql = 
    "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);";
const std::string perf_insert_table_sql = 
    "INSERT INTO person (id, name, age) VALUES (:id, :name, :age)";
const std::string perf_scan_table_sql = 
    "SELECT id, name, age FROM person;";
const std::string perf_lookup_table_sql = 
    "SELECT id, name, age FROM person WHERE id = 500;";

/**
 * @brief test the soci sugar interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sugar_performance(uint64_t recd_num, uint64_t query_num);

/**
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_core_performance(uint64_t recd_num, uint64_t query_num);

/**
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sqlite_performance(uint64_t recd_num, uint64_t query_num);

/**
 * @brief test the soci interface performance
 * @return 0 if successful, 1 otherwise
 */
int test_soci_performance(uint64_t recd_num, uint64_t query_num);


#endif // PERF_H