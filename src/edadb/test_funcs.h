/**
 * @file test_funcs.h
 * @brief test functions for the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#ifndef TEST_FUNCS_H
#define TEST_FUNCS_H

#include <stdint.h>
#include <chrono> // time measurement
#include <string>

//// test the soci interface //// 

/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sugar_interface(void);

/**
 * @brief test the soci core interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_core_interface(void);

/**
 * @brief test the soci orm interface
 * https://soci.sourceforge.net/doc/master/types/#object-relational-mapping
 */
int test_soci_orm(void);


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


#endif // TEST_FUNCS_H