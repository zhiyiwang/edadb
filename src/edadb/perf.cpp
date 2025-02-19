/**
 * @file perf.cpp
 * @brief test functions for the soci interface performance
 */

 #include "perf.h"

 /**
  * @brief test the soci interface performance
  * @return 0 if successful, 1 otherwise
  */
int test_soci_performance(uint64_t recd_num, uint64_t query_num)
{
    test_sugar_performance(recd_num, query_num);

    test_core_performance(recd_num, query_num);

    test_sqlite_performance(recd_num, query_num);

    return 0;
} // test_soci_performance