/**
 * @file SqlType.hpp
 * @brief SqlType.hpp defines the standard SQL types for database.
 *      https://en.wikipedia.org/wiki/SQL#SQL_data_types
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace edadb {

/**
 * @enum SqlType
 * @brief standard SQL types for database.
 *      https://en.wikipedia.org/wiki/SQL#SQL_data_types
 */
enum class SqlType : std::uint32_t {
    Unknown = 0,

    // SQL char types
    Char,       // fixed-length character string, c-string
    Varchar,    // variable-length character string, with a maximum length
    Text,       // variable-length character string, with no maximum length

    // SQL binary types
    Binary,     // fixed-length binary string
    Varbinary,  // variable-length binary string
    Blob,       // variable-length binary string

    // SQL numeric types
    TinyInt,    // 1-byte integer
    SmallInt,   // 2-byte integer
    Integer,    // 4-byte integer
    BigInt,     // 8-byte integer

    Float,      // user specified precision, exact value
    Real,       // 4-byte floating point number
    Double,     // 8-byte floating point number
    Decimal,    // fixed-point number

    // SQL boolean types
    Boolean,    // true or false


    // user defined type
    Composite,


//////// SQL types are not supported currently ////////////////////
//
//    // SQL national char types
//    NChar,      // fixed-length national character string, unicode
//    NVarchar,   // variable-length national character string
//    NText,      // variable-length national character string
//
//    // SQL date and time types
//    Date,       // date
//    Time,       // time
//    Timestamp,  // date and time
//
//    // SQL Interval types
//    Interval,   // time interval
//
//////// SQL types are not supported currently ////////////////////

    Max
};


////////////////////////////////////////////////////////////////////////////////


/**
 * @struct Cpp2SqlType
 * @brief This is a meta function that converts C++ types to SQL types.
 */
template<typename T>
struct Cpp2SqlType {
    // @brief The C++ type that is being converted.
    using cppType = T;

    /// @var static const SqlType
    /// @brief Holds one of the values of the enum SqlType
    static const SqlType sqlType = SqlType::Unknown; 
};

template<typename T>
struct Cpp2SqlType<T*> {
    using cppType = T;
    static const SqlType sqlType = Cpp2SqlType<T>::sqlType;
};



// Define the cpp type to sql type conversion
#define CPP_TO_SQL_TYPE(CPP_TYPE, SQL_TYPE) \
    template<> \
    struct Cpp2SqlType<CPP_TYPE> { \
        using cppType = CPP_TYPE; \
        static const SqlType sqlType = SQL_TYPE; \
    };

CPP_TO_SQL_TYPE(bool         , SqlType::Boolean)
CPP_TO_SQL_TYPE(char         , SqlType::TinyInt)
CPP_TO_SQL_TYPE(unsigned char, SqlType::TinyInt)
CPP_TO_SQL_TYPE(short        , SqlType::SmallInt)
CPP_TO_SQL_TYPE(unsigned short, SqlType::SmallInt)
CPP_TO_SQL_TYPE(int          , SqlType::Integer)
CPP_TO_SQL_TYPE(unsigned int , SqlType::Integer)
CPP_TO_SQL_TYPE(long         , SqlType::BigInt)
CPP_TO_SQL_TYPE(unsigned long, SqlType::BigInt)
CPP_TO_SQL_TYPE(long long    , SqlType::BigInt)
CPP_TO_SQL_TYPE(unsigned long long, SqlType::BigInt)

CPP_TO_SQL_TYPE(float        , SqlType::Real)
CPP_TO_SQL_TYPE(double       , SqlType::Double)
CPP_TO_SQL_TYPE(long double  , SqlType::Double)

CPP_TO_SQL_TYPE(std::string  , SqlType::Text)
CPP_TO_SQL_TYPE(const char*  , SqlType::Text)

//CPP_TO_SQL_TYPE(std::wstring , SqlType::NText)
//CPP_TO_SQL_TYPE(const wchar_t*, SqlType::NText)


////////////////////////////////////////////////////////////////////////////////


/**
 * @fn sqlTypeString: template specialization for each SQL type
 * @brief get SQL type string for the given SQL type.
 * @return std::string Returns the SQL Type equivalent.
 */
template<SqlType N = SqlType::Unknown>
inline std::string const& sqlTypeString() {
    const static std::string ret = "UNKNOWN";
    return ret;
}


#define SQL_TYPE_STRING(SQL_TYPE, TYPE_STRING) \
    template<> \
    inline std::string const &sqlTypeString<SQL_TYPE>() { \
        const static std::string ret = TYPE_STRING; \
        return ret; \
    };

SQL_TYPE_STRING(SqlType::Char   , "TINYINT")
SQL_TYPE_STRING(SqlType::Varchar, "VARCHAR")
SQL_TYPE_STRING(SqlType::Text   , "TEXT")

//SQL_TYPE_STRING(SqlType::NChar   , "NCHAR")
//SQL_TYPE_STRING(SqlType::NVarchar, "NVARCHAR")
//SQL_TYPE_STRING(SqlType::NText   , "NTEXT")

SQL_TYPE_STRING(SqlType::Binary   , "BINARY")
SQL_TYPE_STRING(SqlType::Varbinary, "VARBINARY")
SQL_TYPE_STRING(SqlType::Blob     , "BLOB")

SQL_TYPE_STRING(SqlType::TinyInt , "TINYINT")
SQL_TYPE_STRING(SqlType::SmallInt, "SMALLINT")
SQL_TYPE_STRING(SqlType::Integer , "INTEGER")
SQL_TYPE_STRING(SqlType::BigInt  , "BIGINT")

SQL_TYPE_STRING(SqlType::Float   , "FLOAT")
SQL_TYPE_STRING(SqlType::Real    , "REAL")
SQL_TYPE_STRING(SqlType::Double  , "DOUBLE")
SQL_TYPE_STRING(SqlType::Decimal , "DECIMAL")

SQL_TYPE_STRING(SqlType::Boolean, "BOOLEAN")

SQL_TYPE_STRING(SqlType::Composite, "__COMPOSITE__")


////////////////////////////////////////////////////////////////////////////////


/**
 * @fn getSqlTypeString
 * @brief get the SQL type string for the given C++ type.
 * @return std::string Returns the SQL Type equivalent.
 */
template<typename T>
inline std::string const &getSqlTypeString() {
    const SqlType sql_type = Cpp2SqlType<T>::sqlType;
    return sqlTypeString<sql_type>();
}


////////////////////////////////////////////////////////////////////////////////


} // namespace edadb


#ifdef _EDADB_UNIT_TEST_SQLTYPE_HPP_

// compile the test.cpp 

#include <iostream>
#include <typeinfo>
#include <cassert>

#define _EDADB_SQLTYPE_HPP_TEST_ASSERT_(CPP_TYPE, SQL_TYPE, SQL_TYPE_STRING) \
    assert(typeid(edadb::Cpp2SqlType<CPP_TYPE>::cppType) == typeid(CPP_TYPE)); \
    assert(edadb::Cpp2SqlType<CPP_TYPE>::sqlType == SQL_TYPE); \
    assert(edadb::sqlTypeString<SQL_TYPE>() == SQL_TYPE_STRING);


int testSqlType() {
    std::cout << "SqlType Test" << std::endl;

    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(bool         , edadb::SqlType::Boolean, "BOOLEAN")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(char         , edadb::SqlType::TinyInt, "TINYINT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(unsigned char, edadb::SqlType::TinyInt, "TINYINT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(short        , edadb::SqlType::SmallInt, "SMALLINT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(unsigned short, edadb::SqlType::SmallInt, "SMALLINT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(int          , edadb::SqlType::Integer , "INTEGER")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(unsigned int , edadb::SqlType::Integer , "INTEGER")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(long         , edadb::SqlType::BigInt  , "BIGINT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(unsigned long, edadb::SqlType::BigInt  , "BIGINT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(long long    , edadb::SqlType::BigInt  , "BIGINT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(unsigned long long, edadb::SqlType::BigInt, "BIGINT")

    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(float        , edadb::SqlType::Real   , "REAL")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(double       , edadb::SqlType::Double , "DOUBLE")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(long double  , edadb::SqlType::Double , "DOUBLE")

    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(std::string  , edadb::SqlType::Text   , "TEXT")
    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(const char*  , edadb::SqlType::Text   , "TEXT")

//    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(std::wstring , edadb::SqlType::NText  , "NTEXT")
//    _EDADB_SQLTYPE_HPP_TEST_ASSERT_(const wchar_t*, edadb::SqlType::NText  , "NTEXT")

    std::cout << "SqlType Test Passed" << std::endl;

    return 0;
}
#endif  // _EDADB_UNIT_TEST_SQLTYPE_HPP_