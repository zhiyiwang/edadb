/**
 * @file Cpp2SqlTypeTrait.h
 * @brief Cpp2SqlTypeTrait.h provides a way to convert C++ types to SQL type.
 *      https://en.wikipedia.org/wiki/SQL#SQL_data_types
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "SqlType.h"
#include "TraitUtils.h"


namespace edadb {
 
/**
 * @struct Cpp2SqlTypeTrait
 * @brief This is a trait that converts C++ types to SQL types.
 */
template<typename T>
struct Cpp2SqlTypeTrait {
public: // static const vars
    static constexpr SqlType sqlType = SqlType::Unknown; 

public: // static variable vars
    // table for the cpp table has explicit primary key definition 
    static inline bool hasPrimKey = true;
//    static void setHasPrimKey(bool value) { hasPrimKey = value; }
};

// template specialization mapping from C++ types to SQL types
// Note:
// For the std c++ type pointer, we also map it to the same SQL type as the non-pointer type.
#define MAP_CPP_TO_SQL_TYPE(CPP_TYPE, SQL_TYPE) \
    template<> \
    struct Cpp2SqlTypeTrait<CPP_TYPE> { \
        static constexpr SqlType sqlType = SQL_TYPE; \
        static inline bool hasPrimKey = false;  \
    }; \
    template<> \
    struct Cpp2SqlTypeTrait<CPP_TYPE*> { \
        static constexpr SqlType sqlType = Cpp2SqlTypeTrait<CPP_TYPE>::sqlType; \
        static inline bool hasPrimKey = Cpp2SqlTypeTrait<CPP_TYPE>::hasPrimKey;  \
    }; 



//TODO:
//  do not support const char* currently: template specialization fails 
//  MAP_CPP_TO_SQL_TYPE(const char*   , SqlType::Text)

MAP_CPP_TO_SQL_TYPE(std::string   , SqlType::Text)

MAP_CPP_TO_SQL_TYPE(char          , SqlType::TinyInt)
MAP_CPP_TO_SQL_TYPE(signed char   , SqlType::TinyInt)
MAP_CPP_TO_SQL_TYPE(unsigned char , SqlType::TinyInt)
MAP_CPP_TO_SQL_TYPE(short         , SqlType::SmallInt)
MAP_CPP_TO_SQL_TYPE(unsigned short, SqlType::SmallInt)
MAP_CPP_TO_SQL_TYPE(int           , SqlType::Integer)
MAP_CPP_TO_SQL_TYPE(unsigned int  , SqlType::Integer)
MAP_CPP_TO_SQL_TYPE(long          , SqlType::BigInt)
MAP_CPP_TO_SQL_TYPE(unsigned long , SqlType::BigInt)
MAP_CPP_TO_SQL_TYPE(long long     , SqlType::BigInt)
MAP_CPP_TO_SQL_TYPE(unsigned long long, SqlType::BigInt)

MAP_CPP_TO_SQL_TYPE(float         , SqlType::Real)
MAP_CPP_TO_SQL_TYPE(double        , SqlType::Double)
MAP_CPP_TO_SQL_TYPE(long double   , SqlType::Double)

MAP_CPP_TO_SQL_TYPE(bool          , SqlType::Boolean)


////////////////////////////////////////////////////////////////////////////////
 

 /**
 * @fn getSqlTypeString
 * @brief get the SQL type string for the given C++ type.
 * @return std::string Returns the SQL Type equivalent.
 */
template<typename T>
inline std::string const &getSqlTypeString() {
    using U = canonical_t<T>;
    return sqlTypeString< Cpp2SqlTypeTrait<U>::sqlType >();
} // getSqlTypeString

 
} // namespace edadb