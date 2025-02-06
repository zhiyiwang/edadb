/**
 * @file test_funcs.h
 * @brief test functions for the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#ifndef TEST_FUNCS_H
#define TEST_FUNCS_H

/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sugar_interface(void);

/**
 * @brief test the soci core interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_core_interface(void);

/**
 * @brief test the soci orm interface
 * https://soci.sourceforge.net/doc/master/types/#object-relational-mapping
 */
int test_soci_orm(void);

#endif // TEST_FUNCS_H