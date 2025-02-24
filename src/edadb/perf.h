/**
 * @file perf.h
 * @brief test functions for the soci interface performance
 */

#ifndef PERF_H
#define PERF_H

// time measurement
#include <chrono> 
#include <string>
#include <string.h>

#define PERF_OUTPUT_SQL_RESULT 0


/**
 * @brief test the soci interface performance
 * @return 0 if successful, 1 otherwise
 */
int test_soci_performance(uint64_t recd_num, uint64_t query_num);


/**
 * @brief test the soci sugar interface performance for string
 * @return 0 if successful, 1 otherwise
 */
int test_sugar_performance_str(uint64_t recd_num, uint64_t query_num);

/**
 * @brief test the soci sugar interface performance for int
 * @return 0 if successful, 1 otherwise
 */
int test_sugar_performance_int(uint64_t recd_num, uint64_t query_num);


/**
 * @brief test the soci core interface performance using single value binding
 * @return 0 if successful, 1 otherwise
 */
int test_core_value_performance_str(uint64_t recd_num, uint64_t query_num);

/**
 * @brief test the soci core interface performance using single value binding
 * @return 0 if successful, 1 otherwise
 */
int test_core_value_performance_int(uint64_t recd_num, uint64_t query_num);


/**
 * @brief test the soci core interface performance using vector binding
 * @return 0 if successful, 1 otherwise
 */
int test_core_vector_performance_str(uint64_t recd_num, uint64_t query_num);

/**
 * @brief test the soci core interface performance using vector binding
 * @return 0 if successful, 1 otherwise
 */
int test_core_vector_performance_int(uint64_t recd_num, uint64_t query_num);


/**
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sqlite_performance_str(uint64_t recd_num, uint64_t query_num);

/**
 * @brief test the soci core interface performance
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sqlite_performance_int(uint64_t recd_num, uint64_t query_num);


///**
// * @brief test the soci core interface performance
// * https://soci.sourceforge.net/doc/master/interfaces/
// */
//int test_core_performance(uint64_t recd_num, uint64_t query_num);




#endif // PERF_H