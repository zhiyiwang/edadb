/**
 * @file intfc.cpp
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

 #include "intfc.h"

int test_soci_interface(void) {

    if (test_sugar_interface() != 0) {
        printf("test_sugar_interface failed\n");
        return -1;
    }

    if (test_core_interface() != 0) {
        printf("test_core_interface failed\n");
        return -1;
    }

    if (test_soci_orm() != 0) {
        printf("test_soci_orm failed\n");
        return -1;
    }

    return 0;
} // test_soci_interface