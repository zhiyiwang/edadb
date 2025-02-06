/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include "test_funcs.h"

int main(void) {
    // test the soci interface
    test_sugar_interface();

    // test the core interface
    test_core_interface();

    // test the soci orm interface
    test_soci_orm();


    return 0;
}

