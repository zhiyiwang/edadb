/**
 * @file Cpp2DbType.h
 * @brief Cpp2DbType.h provides a way to convert C++ types to database types.
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace edadb { 

/**
 * @enum DbType
 * @brief Enumerates the database types: sqlite3 storage types.
 *   https://www.sqlite.org/datatype3.html
 *   https://www.sqlite.org/c3ref/funclist.html
 */
enum class DbType : std::uint32_t {
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
 * @struct Cpp2DbType dec
 * @brief This is a meta function that converts C++ types to database types.
 */
template<typename T>
struct Cpp2DbType {
    using cppType = T;
    static constexpr DbType dbType = DbType::Unknown; // constexpr: no need to define
};

template<typename T>
struct Cpp2DbType<T*> {
    using cppType = T;
    static constexpr DbType dbType = Cpp2DbType<T>::dbType;
};


// template specialization mapping
#define DEFINE_TYPE_MAPPING(CPP_TYPE, DB_TYPE) \
template<> \
struct Cpp2DbType<CPP_TYPE> { \
    using cppType = CPP_TYPE; \
    static constexpr DbType dbType = DB_TYPE; \
};

DEFINE_TYPE_MAPPING(bool          , DbType::Integer)
DEFINE_TYPE_MAPPING(char          , DbType::Integer)
DEFINE_TYPE_MAPPING(unsigned char , DbType::Integer)
DEFINE_TYPE_MAPPING(short         , DbType::Integer)
DEFINE_TYPE_MAPPING(unsigned short, DbType::Integer)
DEFINE_TYPE_MAPPING(int           , DbType::Integer)
DEFINE_TYPE_MAPPING(unsigned int  , DbType::Integer)
DEFINE_TYPE_MAPPING(long          , DbType::Integer)
DEFINE_TYPE_MAPPING(unsigned long , DbType::Integer)
DEFINE_TYPE_MAPPING(long long     , DbType::Integer)    
DEFINE_TYPE_MAPPING(unsigned long long, DbType::Integer)

DEFINE_TYPE_MAPPING(float         , DbType::Real)
DEFINE_TYPE_MAPPING(double        , DbType::Real)
DEFINE_TYPE_MAPPING(long double   , DbType::Real)

DEFINE_TYPE_MAPPING(std::string   , DbType::Text)
DEFINE_TYPE_MAPPING(std::wstring  , DbType::Text)
DEFINE_TYPE_MAPPING(const char*   , DbType::Text)
DEFINE_TYPE_MAPPING(const wchar_t*, DbType::Text)


////////////////////////////////////////////////////////////////////////////////


template<DbType N = DbType::Unknown>
inline std::string const& cppTypeEnumToDbTypeString() {
    const static std::string ret = "UNKNOWN";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbType::Integer>() {
    const static std::string ret = "INTEGER";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbType::Real>() {
    const static std::string ret = "REAL";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbType::Text>() {
    const static std::string ret = "VARCHAR";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbType::Numeric>() {
    const static std::string ret = "NUMERIC";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbType::Composite>() {
    const static std::string ret = "__COMPOSITE__";
    return ret;
}

/**
 * @fn cppTypeToDbTypeString
 * @brief Convert cpp type to sqlite type, and return the string.
 * @return std::string Returns the SQL Type equivalent.
 */
template<typename T>
inline std::string const &cppTypeToDbTypeString() {
    return cppTypeEnumToDbTypeString< Cpp2DbType<T>::dbType >();
}


////////////////////////////////////////////////////////////////////////////////


} // edadb