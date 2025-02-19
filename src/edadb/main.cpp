/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include "intfc.h"
#include "perf.h"


int main(void) {

    //// soci interface func test ////
    test_soci_interface();

    //// soci performance test ////
    uint64_t recd_num  = 10000;
    uint64_t query_num = 10000;
    test_soci_performance(10000, 10000);

    return 0;
}

