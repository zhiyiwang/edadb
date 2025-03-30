/**
 * @file Cpp2DbType.hpp
 * @brief Cpp2DbType.hpp provides a way to convert C++ types to
 *        database types.
 */ 

#pragma once

#include "DbBackendType.hpp"
#include "SqlType.hpp"
#include "DbType.hpp"
#include "DbTypeSqlite.hpp"



namespace edadb {   

/**
 * @struct 
 * @brief This is a meta function that converts C++ types to database types.
 */
template<DbBackendType BT, typename T>
struct Cpp2DbType {
public:
    static const Cpp2SqlType<T> cpp_2_sql_type;
    static const Sql2DbType<BT, cpp_2_sql_type.sqlType> sql_2_db_type;

public:
    using cppType = typename Cpp2SqlType<T>::cppType;
    static constexpr SqlType sqlType = Cpp2SqlType<T>::sqlType;
    static constexpr DbType dbType = Sql2DbType<BT, sqlType>::dbType;
    

public:
    static std::string getSqlTypeString() {
        // use namespace to avoid name collision: declare the function in the namespace
        return edadb::getSqlTypeString<T>();
    }

    static std::string getDbTypeString() {
        // use namespace to avoid name collision: declare the function in the namespace
        return edadb::getDbTypeString<BT, T>();
    }
};

} // namespace edadb



#ifdef _EDADB_UNIT_TEST_CPP2DBTYPE_HPP_

// compile the test.cpp

#include <iostream>
#include <cassert>
#include <string>

// Test macro
#define COMMA ,
#define _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(BCK_TYPE, CPP_TYPE, SQL_TYPE, DB_TYPE, STR) \
    static_assert(edadb::Cpp2DbType<BCK_TYPE COMMA CPP_TYPE>::sqlType == SQL_TYPE); \
    static_assert(edadb::Cpp2DbType<BCK_TYPE COMMA CPP_TYPE>::dbType == DB_TYPE); \
    assert(typeid(typename edadb::Cpp2DbType<BCK_TYPE COMMA CPP_TYPE>::cppType) == typeid(CPP_TYPE)); \
    assert(edadb::getDbTypeString<BCK_TYPE COMMA CPP_TYPE>() == STR);

// Usage example
void runTests() {
    // Add more tests here
}



int testCpp2DbType() {
    std::cout << "Cpp2DbType Test" << std::endl;

    // test the Cpp2DbType
    constexpr edadb::DbBackendType bck = edadb::DbBackendType::Sqlite;

    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, bool, edadb::SqlType::Boolean, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, char, edadb::SqlType::TinyInt, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, unsigned char, edadb::SqlType::TinyInt, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, short, edadb::SqlType::SmallInt, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, unsigned short, edadb::SqlType::SmallInt, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, int, edadb::SqlType::Integer, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, unsigned int, edadb::SqlType::Integer, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, long, edadb::SqlType::BigInt, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, unsigned long, edadb::SqlType::BigInt, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, long long, edadb::SqlType::BigInt, edadb::DbType::Integer, "INTEGER");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, unsigned long long, edadb::SqlType::BigInt, edadb::DbType::Integer, "INTEGER");

    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, float, edadb::SqlType::Real, edadb::DbType::Real, "REAL");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, double, edadb::SqlType::Double, edadb::DbType::Real, "REAL");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, long double, edadb::SqlType::Double, edadb::DbType::Real, "REAL"); 

    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, std::string, edadb::SqlType::Text, edadb::DbType::Text, "VARCHAR");
    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, const char*, edadb::SqlType::Text, edadb::DbType::Text, "VARCHAR");

//    _EDADB_CPP2DBTYPE_TEST_SQLITE_ASSERT_(bck, edadb::DbType::Composite, edadb::SqlType::Composite, edadb::DbType::Composite, "__COMPOSITE__");

    std::cout << "Cpp2DbType Done" << std::endl;

    return 0;
}

#endif // _EDADB_UNIT_TEST_CPP2DBTYPE_HPP_