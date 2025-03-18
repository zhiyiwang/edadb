/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <iostream>

#include "func.h"
#include "intfc.h"
#include "perf.h"


int main(void) {
#if 0 
    //// soci interface func test ////
    test_soci_interface();

    //// soci performance test ////
    uint64_t run_num = 1;
    uint64_t recd_num  = 10000;
    uint64_t query_num = 10000;
    for (uint64_t i = 0; i < run_num; i++) {
        test_soci_performance(recd_num, query_num);

        // keep the last db file
        if ((i + 1 < run_num) && (system("rm -f *.db") != 0)) {
            std::cerr << "Failed to remove db files" << std::endl;
            return 1;
        }
    }
#endif

    test_sqlite_lock();
//    test_sqlite_stmt();

    return 0;
}