/**
 * @file CppTypeToDbType.hpp
 * @brief CppTypeToDbType.hpp provides a way to convert C++ types to
 *        database types.
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>


namespace edadb { 

/**
 * @enum DbTypes
 * @brief Enumerates the database types: sqlite3 storage types.
 *   https://www.sqlite.org/datatype3.html
 *   https://www.sqlite.org/c3ref/funclist.html
 */
enum class DbTypes : std::uint32_t {
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
 * @struct CppTypeToDbType
 * @brief This is a meta function that converts C++ types to database types.
 */
template<typename T>
struct CppTypeToDbType {
    // @brief The C++ type that is being converted.
    using cppType = T;

    /// @var static const DbTypes
    /// @brief Holds one of the values of the enum DbTypes
    static const DbTypes dbType = DbTypes::Unknown; // rename from ret
};

template<typename T>
struct CppTypeToDbType<T*> {
    using cppType = T;
    static const DbTypes dbType = CppTypeToDbType<T>::dbType;
};

template<typename T>
struct CppTypeToDbType<std::vector<T>>{
    // @brief The C++ type that is being converted.
    using cppType = std::vector<T>;

    /// @brief All the vectors are converted to strings
    static const DbTypes dbType = DbTypes::Integer;
};


// Define the mappings from 
#define DEFINE_TYPE_MAPPING(CppType, DbType) \
template<> \
struct CppTypeToDbType<CppType> { \
    using cppType = CppType; \
    static constexpr DbTypes dbType = DbType; \
};

// cpp types
DEFINE_TYPE_MAPPING(bool         , DbTypes::Integer)
DEFINE_TYPE_MAPPING(char         , DbTypes::Integer)
DEFINE_TYPE_MAPPING(unsigned char, DbTypes::Integer)
DEFINE_TYPE_MAPPING(short        , DbTypes::Integer)
DEFINE_TYPE_MAPPING(unsigned short, DbTypes::Integer)
DEFINE_TYPE_MAPPING(int          , DbTypes::Integer)
DEFINE_TYPE_MAPPING(unsigned int , DbTypes::Integer)
DEFINE_TYPE_MAPPING(long         , DbTypes::Integer)
DEFINE_TYPE_MAPPING(unsigned long, DbTypes::Integer)
DEFINE_TYPE_MAPPING(long long    , DbTypes::Integer)    
DEFINE_TYPE_MAPPING(unsigned long long, DbTypes::Integer)

DEFINE_TYPE_MAPPING(float        , DbTypes::Real)
DEFINE_TYPE_MAPPING(double       , DbTypes::Real)
DEFINE_TYPE_MAPPING(long double  , DbTypes::Real)

DEFINE_TYPE_MAPPING(std::string  , DbTypes::Text)
DEFINE_TYPE_MAPPING(std::wstring , DbTypes::Text)
DEFINE_TYPE_MAPPING(const char*  , DbTypes::Text)
DEFINE_TYPE_MAPPING(const wchar_t*, DbTypes::Text)


////////////////////////////////////////////////////////////////////////////////


/**
 * @fn cppTypeEnumToDbTypeString
 * @brief Convert cpp type to sqlite type, and return the string.
 * @return std::string Returns the SQL Type equivalent.
 */
template<DbTypes N = DbTypes::Unknown>
inline std::string const& cppTypeEnumToDbTypeString() {
    const static std::string ret = "UNKNOWN";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbTypes::Integer>() {
    const static std::string ret = "INTEGER";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbTypes::Real>() {
    const static std::string ret = "REAL";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbTypes::Text>() {
    const static std::string ret = "VARCHAR";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbTypes::Numeric>() {
    const static std::string ret = "NUMERIC";
    return ret;
}

template<>
inline std::string const &cppTypeEnumToDbTypeString<DbTypes::Composite>() {
    const static std::string ret = "__COMPOSITE__";
    return ret;
}

/// @fn cppTypeToDbTypeString
/// @brief This is the final piece of puzzle in the type conversion.
/// @details This function call the previous function specialization
///          which in turn calls the meta function to get the enum.
/**
 * @fn cppTypeToDbTypeString
 * @brief Convert cpp type to sqlite type, and return the string.
 * @return std::string Returns the SQL Type equivalent.
 */
template<typename T>
inline std::string const &cppTypeToDbTypeString() {
    return cppTypeEnumToDbTypeString< CppTypeToDbType<T>::dbType >();
}

////////////////////////////////////////////////////////////////////////////////


} // edadb