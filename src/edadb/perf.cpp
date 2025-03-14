/**
 * @file perf.cpp
 * @brief test functions for the soci interface performance
 */

 #include "func.h"
 #include "perf.h"

 /**
  * @brief test the soci interface performance
  * @return 0 if successful, 1 otherwise
  */
int test_soci_performance(uint64_t recd_num, uint64_t query_num)
{
#if ((PERF_SQLITE_VALUE_ONLY == 0) && (PERF_SQLITE_STEP_ONLY == 0))
    test_sugar_performance_str(recd_num, query_num);
    test_sugar_performance_int(recd_num, query_num);

    test_core_value_performance_str(recd_num, query_num);
    test_core_value_performance_int(recd_num, query_num);

    test_core_vector_performance_str(recd_num, query_num);
    test_core_vector_performance_int(recd_num, query_num);

    test_sqlite_performance_str(recd_num, query_num);
    test_sqlite_performance_int(recd_num, query_num);
#else
    // only test the sqlite3 interface performance
    test_sqlite_performance_str(recd_num, query_num);
    test_sqlite_performance_int(recd_num, query_num);
#endif

    return 0;
} // test_soci_performance