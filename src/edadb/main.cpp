/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include "test_funcs.h"

int main(void) {
//    // test the soci interface
//    test_sugar_interface();
//
//    // test the core interface
//    test_core_interface();
//
//    // test the soci orm interface
//    test_soci_orm();


    // test the soci interface performance
    uint64_t recd_num = 1000;
    uint64_t query_num = 10000;
    test_sugar_performance(recd_num, query_num);

    // test the core interface performance
    test_core_performance(recd_num, query_num);

    // test sqlite3 performance
    test_sqlite_performance(recd_num, query_num);

    return 0;
}

