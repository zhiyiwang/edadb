/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include "test_funcs.h"

int main(void) {

    //// soci interface func test ////
    if (0) {
        // test the soci interface
        test_sugar_interface();

        // test the core interface
        test_core_interface();

        // test the soci orm interface
        test_soci_orm();
    }

    //// soci performance test ////
    if (1) {
        // record number to insert into database
        uint64_t recd_num = 1000;
        // query number to run "select * from table", since the single query is too fast
        uint64_t query_num = 10000;

        // sugar interface performance
        test_sugar_performance (recd_num, query_num);

        // core interface performance
        test_core_performance  (recd_num, query_num);

        // sqlite3 c++ interface performance
        test_sqlite_performance(recd_num, query_num);
    }

    return 0;
}

