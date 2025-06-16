/**
 * @file SqlType.h
 * @brief SqlType.h provides a way to define SQL types.
 *      https://en.wikipedia.org/wiki/SQL#SQL_data_types
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "TraitUtils.h"


namespace edadb {

/**
 * @enum SqlType enum class
 * @brief Enumerates the SQL types:
 *      https://en.wikipedia.org/wiki/SQL#SQL_data_types
 */
enum class SqlType : std::uint32_t {
    Unknown = 0,

    // SQL char types
    Char,       // fixed-length character string, c-string
    Varchar,    // variable-length character string, with a maximum length
    Text,       // variable-length character string, with no maximum length

    // SQL numeric types
    TinyInt,    // 1-byte integer
    SmallInt,   // 2-byte integer
    Integer,    // 4-byte integer
    BigInt,     // 8-byte integer

    Float,      // user specified precision, exact value
    Real,       // 4-byte floating point number
    Double,     // 8-byte floating point number
    Decimal,    // fixed-point number

    // SQL binary types
    Binary,     // fixed-length binary string
    VarBinary,  // variable-length binary string
    Blob,       // variable-length binary string

    // SQL boolean types
    Boolean,    // true or false

    // user defined type
    Composite,  // inner class object
    External,   // external class object
    CompositeVector, // vector of inner class object

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
 * @fn sqlTypeString: template specialization for each SQL type
 * @brief get SQL type string for the given SQL type.
 * @return std::string Returns the SQL Type equivalent.
 */
template<SqlType N = SqlType::Unknown>
inline std::string const& sqlTypeString() {
    static_assert(N == SqlType::Unknown,
        "sqlTypeString: SqlType must be specialized for each SQL type");

    const static std::string ret = "UNKNOWN";
    return ret;
} // sqlTypeString


/**
 * @macro SQL_TYPE_STRING
 * @brief Macro to define the SQL type string for each SqlType.
 * @param SQL_TYPE The SqlType enum value.
 * @param TYPE_STRING The corresponding SQL type string.
 */
#define SQL_TYPE_STRING(SQL_TYPE, TYPE_STRING) \
    template<> \
    inline std::string const &sqlTypeString<SQL_TYPE>() { \
        const static std::string ret = TYPE_STRING; \
        return ret; \
    };

SQL_TYPE_STRING(SqlType::Char     , "TINYINT") // use TINYINT for char
SQL_TYPE_STRING(SqlType::Varchar  , "VARCHAR")
SQL_TYPE_STRING(SqlType::Text     , "TEXT")

SQL_TYPE_STRING(SqlType::TinyInt  , "TINYINT")
SQL_TYPE_STRING(SqlType::SmallInt , "SMALLINT")
SQL_TYPE_STRING(SqlType::Integer  , "INTEGER")
SQL_TYPE_STRING(SqlType::BigInt   , "BIGINT")

SQL_TYPE_STRING(SqlType::Binary   , "BINARY")
SQL_TYPE_STRING(SqlType::VarBinary, "VARBINARY")
SQL_TYPE_STRING(SqlType::Blob     , "BLOB")

SQL_TYPE_STRING(SqlType::Float    , "FLOAT")
SQL_TYPE_STRING(SqlType::Real     , "REAL")
SQL_TYPE_STRING(SqlType::Double   , "DOUBLE")
SQL_TYPE_STRING(SqlType::Decimal  , "DECIMAL")

SQL_TYPE_STRING(SqlType::Boolean  , "BOOLEAN")

SQL_TYPE_STRING(SqlType::Composite      , "__COMPOSITE__")
SQL_TYPE_STRING(SqlType::External       , "__EXTERNAL__" )
SQL_TYPE_STRING(SqlType::CompositeVector, "__COMPOSITE_VECTOR__")

} // namespace edadb