/**
 * @file DbType.hpp
 * @brief Sqlite3 database type.
 */

#pragma once

#include "DbBackendType.hpp"
#include "SqlType.hpp"
#include "DbType.hpp"

namespace edadb {

/**
 * @enum DbType
 * @brief sqlite3 database types.
 *   https://www.sqlite.org/datatype3.html
 *   https://www.sqlite.org/c3ref/funclist.html
 */
enum class DbTypeSqlite : std::uint32_t {
    Unknown = 0,

    // sqlite3 storage classes
    Null,   // Null Value
    Integer,// signed integer: 0, 1, 2, 3, 4, 6, or 8 bytes depending on the magnitude of the value
    Real,   // 8-byte IEEE floating point number
    Text,   // text string using the database encoding
    Blob,   // blob of data

    // sqlite3 type affinity
    // may contain values using all five storage classes
    Numeric,  

    // user defined type
    Composite 
};


////////////////////////////////////////////////////////////////////////////////


/**
 * @struct DbTypeTraits specializations the DbType as DbTypeSqlite.
 * @brief Enumerates the database types: sqlite3 storage types.
 */
template<>
struct DbTypeTraits<DbBackendType::Sqlite> {
    using type = DbTypeSqlite;
};

/**
 * define DataType as DbTypeTraits<DbBackendType::Sqlite>::type.
 */
using DbType = DbTypeTraits<DbBackendType::Sqlite>::type;


////////////////////////////////////////////////////////////////////////////////


/**
 * Define template to specialize the DbType for sqlite3.
 */
#define SQL_TO_SQLITE_TYPE(SQL_TYPE, DB_TYPE) \
    template<> \
    struct Sql2DbType<DbBackendType::Sqlite, SQL_TYPE> { \
        static const DbTypeSqlite dbType = DB_TYPE; \
    };


SQL_TO_SQLITE_TYPE(SqlType::Char   , DbType::Integer)
SQL_TO_SQLITE_TYPE(SqlType::Varchar, DbType::Text)
SQL_TO_SQLITE_TYPE(SqlType::Text   , DbType::Text)

SQL_TO_SQLITE_TYPE(SqlType::Binary   , DbType::Blob)
SQL_TO_SQLITE_TYPE(SqlType::Varbinary, DbType::Blob)
SQL_TO_SQLITE_TYPE(SqlType::Blob     , DbType::Blob)

SQL_TO_SQLITE_TYPE(SqlType::TinyInt , DbType::Integer)
SQL_TO_SQLITE_TYPE(SqlType::SmallInt, DbType::Integer)
SQL_TO_SQLITE_TYPE(SqlType::Integer , DbType::Integer)
SQL_TO_SQLITE_TYPE(SqlType::BigInt  , DbType::Integer)

SQL_TO_SQLITE_TYPE(SqlType::Float   , DbType::Real)
SQL_TO_SQLITE_TYPE(SqlType::Real    , DbType::Real)
SQL_TO_SQLITE_TYPE(SqlType::Double  , DbType::Real)
SQL_TO_SQLITE_TYPE(SqlType::Decimal , DbType::Real)

SQL_TO_SQLITE_TYPE(SqlType::Boolean, DbType::Integer)

SQL_TO_SQLITE_TYPE(SqlType::Composite, DbType::Composite)



////////////////////////////////////////////////////////////////////////////////

/**
 * @fn dbTypeString
 * @brief use the datetype enum to get the string.
*/
template< DbType DT = DbType::Unknown >
inline std::string const& dbTypeString() {
    const static std::string ret = "UNKNOWN";
    return ret;
}

#define DB_TYPE_STRING(DB_TYPE, TYPE_STRING) \
    template<> \
    inline std::string const &dbTypeString<DB_TYPE>() { \
        const static std::string ret = TYPE_STRING; \
        return ret; \
    };

DB_TYPE_STRING(DbType::Null   , "NULL")
DB_TYPE_STRING(DbType::Integer, "INTEGER")
DB_TYPE_STRING(DbType::Real   , "REAL")
DB_TYPE_STRING(DbType::Text   , "VARCHAR")
DB_TYPE_STRING(DbType::Blob   , "BLOB")
DB_TYPE_STRING(DbType::Numeric, "NUMERIC")
DB_TYPE_STRING(DbType::Composite, "__COMPOSITE__")


////////////////////////////////////////////////////////////////////////////////


/**
 * get the string for the given DbType.
 * @return std::string Returns the DbType string.
 */
template<DbBackendType BT, typename T>
inline std::string const &getDbTypeString() {
    const auto db_type = Sql2DbType<BT, Cpp2SqlType<T>::sqlType>::dbType;
    return dbTypeString<db_type>();  
}


////////////////////////////////////////////////////////////////////////////////


} // namespace edadb



#ifdef _EDADB_UNIT_TEST_DBTYPE_HPP_

// compile the test.cpp
// g++ -std=c++17 -D_EDADB_UNIT_TEST_DBTYPE_HPP_ -I. -I./backend/sqlite test.cpp -o test

#define COMMA ,
#define _EDADB_DBTYPE_HPP_TEST_ASSERT_(SQL_TYPE, DB_TYPE, DB_TYPE_STRING) \
    assert(edadb::Sql2DbType< edadb::DbBackendType::Sqlite COMMA SQL_TYPE>::dbType == DB_TYPE); \
    assert(edadb::dbTypeString<DB_TYPE>() == DB_TYPE_STRING); 


int testDbType() {
    std::cout << "DbType Test" << std::endl;

    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Char   , edadb::DbType::Integer, "INTEGER")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Varchar, edadb::DbType::Text   , "VARCHAR")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Text   , edadb::DbType::Text   , "VARCHAR")

    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Binary   , edadb::DbType::Blob   , "BLOB")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Varbinary, edadb::DbType::Blob   , "BLOB")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Blob     , edadb::DbType::Blob   , "BLOB")

    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::TinyInt , edadb::DbType::Integer, "INTEGER")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::SmallInt, edadb::DbType::Integer, "INTEGER")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Integer , edadb::DbType::Integer, "INTEGER")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::BigInt  , edadb::DbType::Integer, "INTEGER")

    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Float   , edadb::DbType::Real   , "REAL")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Real    , edadb::DbType::Real   , "REAL")
    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Double  , edadb::DbType::Real   , "REAL")

    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Boolean, edadb::DbType::Integer, "INTEGER")

    _EDADB_DBTYPE_HPP_TEST_ASSERT_(edadb::SqlType::Composite, edadb::DbType::Composite, "__COMPOSITE__")


    std::cout << "DbType Test Passed" << std::endl;

    return 0;
}


#endif // _EDADB_UNIT_TEST_DBTYPE_HPP_