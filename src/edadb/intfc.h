/**
 * @file test_funcs.h
 * @brief test functions for the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#ifndef INTFC_H
#define INTFC_H

#include <stdint.h>
#include <string>

//////// test soci interface ////////

/**
 * @brief test sugar interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sugar_interface(void);

/**
 * @brief test core interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_core_interface(void);

/**
 * @brief test orm interface
 * https://soci.sourceforge.net/doc/master/types/#object-relational-mapping
 */
int test_soci_orm(void);

/**
 * @brief test the soci interface 
 */
int test_soci_interface(void);


//////// test duckdb interface ////////
/**
 * @brief test the duckdb interface
 */
int test_duckdb_interface(void);


#endif // INTFC_H