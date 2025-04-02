/**
 * @file Cpp2SqlType.h
 * @brief Cpp2SqlType.h provides a way to convert C++ types to SQL type.
 *      https://en.wikipedia.org/wiki/SQL#SQL_data_types
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

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
    Varbinary,  // variable-length binary string
    Blob,       // variable-length binary string

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
    using cppType = T;
    static constexpr SqlType sqlType = SqlType::Unknown; // constexpr: no need to define
};

template<typename T>
struct Cpp2SqlType<T*> {
    using cppType = T;
    static constexpr SqlType sqlType = Cpp2SqlType<T>::sqlType;
};


// template specialization mapping
#define CPP_TO_SQL_TYPE(CPP_TYPE, SQL_TYPE) \
    template<> \
    struct Cpp2SqlType<CPP_TYPE> { \
        using cppType = CPP_TYPE; \
        static constexpr SqlType sqlType = SQL_TYPE; \
    };

CPP_TO_SQL_TYPE(bool          , SqlType::Boolean)
CPP_TO_SQL_TYPE(char          , SqlType::TinyInt)
CPP_TO_SQL_TYPE(unsigned char , SqlType::TinyInt)
CPP_TO_SQL_TYPE(short         , SqlType::SmallInt)
CPP_TO_SQL_TYPE(unsigned short, SqlType::SmallInt)
CPP_TO_SQL_TYPE(int           , SqlType::Integer)
CPP_TO_SQL_TYPE(unsigned int  , SqlType::Integer)
CPP_TO_SQL_TYPE(long          , SqlType::BigInt)
CPP_TO_SQL_TYPE(unsigned long , SqlType::BigInt)
CPP_TO_SQL_TYPE(long long     , SqlType::BigInt)
CPP_TO_SQL_TYPE(unsigned long long, SqlType::BigInt)

CPP_TO_SQL_TYPE(float         , SqlType::Real)
CPP_TO_SQL_TYPE(double        , SqlType::Double)
CPP_TO_SQL_TYPE(long double   , SqlType::Double)

CPP_TO_SQL_TYPE(std::string   , SqlType::Text)
CPP_TO_SQL_TYPE(const char*   , SqlType::Text)


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

SQL_TYPE_STRING(SqlType::Char     , "TINYINT") // use TINYINT for char
SQL_TYPE_STRING(SqlType::Varchar  , "VARCHAR")
SQL_TYPE_STRING(SqlType::Text     , "TEXT")

SQL_TYPE_STRING(SqlType::TinyInt  , "TINYINT")
SQL_TYPE_STRING(SqlType::SmallInt , "SMALLINT")
SQL_TYPE_STRING(SqlType::Integer  , "INTEGER")
SQL_TYPE_STRING(SqlType::BigInt   , "BIGINT")

SQL_TYPE_STRING(SqlType::Binary   , "BINARY")
SQL_TYPE_STRING(SqlType::Varbinary, "VARBINARY")
SQL_TYPE_STRING(SqlType::Blob     , "BLOB")

SQL_TYPE_STRING(SqlType::Float    , "FLOAT")
SQL_TYPE_STRING(SqlType::Real     , "REAL")
SQL_TYPE_STRING(SqlType::Double   , "DOUBLE")
SQL_TYPE_STRING(SqlType::Decimal  , "DECIMAL")

SQL_TYPE_STRING(SqlType::Boolean  , "BOOLEAN")

SQL_TYPE_STRING(SqlType::Composite, "__COMPOSITE__")


////////////////////////////////////////////////////////////////////////////////
 
 
/**
 * @fn getSqlTypeString
 * @brief get the SQL type string for the given C++ type.
 * @return std::string Returns the SQL Type equivalent.
 */
template<typename T>
inline std::string const &getSqlTypeString() {
    // use SqlType to get the SQL type string
    return sqlTypeString< Cpp2SqlType<T>::sqlType >();
}
 
 
 ////////////////////////////////////////////////////////////////////////////////
 
 
 } // namespace edadb