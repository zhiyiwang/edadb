/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include "intfc.h"
#include "perf.h"


int main(void) {

//    //// soci interface func test ////
//    test_soci_interface();

    //// soci performance test ////
    uint64_t recd_num  = 10000;
    uint64_t query_num = 10000;
    for (int i = 0; i < 5; i++) {
        test_soci_performance(recd_num, query_num);
    }

    return 0;
}

