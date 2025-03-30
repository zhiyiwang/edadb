/**
 * @file test.cpp
 * @brief unit test for the edadb library.
 */

#include <iostream>
#include <cassert>
#include <string>
#include <sstream>

#include "SqlType.hpp"
#include "DbType.hpp"
#include "Cpp2DbType.hpp"
#include "./backend/sqlite/DbTypeSqlite.hpp"


int main () {

#ifdef _EDADB_UNIT_TEST_SQLTYPE_HPP_
    // g++ -std=c++17 -D_EDADB_UNIT_TEST_SQLTYPE_HPP_ test.cpp -I. -I./backend/sqlite -o test
    testSqlType();
#endif

#ifdef _EDADB_UNIT_TEST_DBTYPE_HPP_
    // g++ -std=c++17 -D_EDADB_UNIT_TEST_DBTYPE_HPP_ -I. -I./backend/sqlite test.cpp -o test
    testDbType();
#endif

#ifdef _EDADB_UNIT_TEST_CPP2DBTYPE_HPP_
    // g++ -std=c++17 -D_EDADB_UNIT_TEST_CPP2DBTYPE_HPP_ -I. -I./backend/sqlite test.cpp -o test
    testCpp2DbType();
#endif

    return 0;
}