/**
 * @file TypeInfoTrait.h
 * @brief TypeInfoTrait.h provides a way to extract type information from a given type.
 */

#pragma once

#include <vector>

#include "TraitUtils.h"
#include "SqlType.h"
#include "Cpp2SqlTypeTrait.h"


namespace edadb {

/**
 * @struct ScalarTypeInfo
 * @brief This is a trait that extracts type information from a scalar type,
 *     such as int, float*, double, std::string, etc.
 */
template<typename T>
struct ScalarTypeInfo {
    static constexpr bool is_pointer = std::is_pointer<T>::value;

    // Raw is the type without pointer:
    //   If T is not a pointer, Raw is T.
    using Raw = std::remove_const_t<std::remove_pointer_t<T>>;
    static_assert(!std::is_pointer<Raw>::value,
        "ScalarTypeInfo do not support multilevel pointer type pointing to scalar type");

    using CppType = Raw;
    static constexpr SqlType sqlType = Cpp2SqlTypeTrait<CppType>::sqlType;

    // Only available for VectorTypeInfo, 
    //   but defined here to avoid compilation error
    using VecElemType = void; // no vector element type
    using VecElemInfo = void; // no vector element type information
    using VecCppType  = void; // no vector element type information

    static constexpr bool elemIsPointer = false;
    static constexpr SqlType elemSqlType = SqlType::Unknown;

public:
    /**
     * @brief Get the CppType value pointer from the given object to bind to the database.
     * @param obj The object of type T.
     * @return CppType* Returns a pointer to the CppType Value binding.
     */
    static CppType* getCppPtr2Bind(T* obj) {
        if constexpr (is_pointer) {
            return *obj; // T* is CppType**
        } else {
            return  obj; // T* is CppType*
        }
    } // getCppPtr2Bind 

    /**
     * @brief Get the CppType value pointer from the given object to fetch from the database.
     * @param obj The object of type T.
     * @return CppType* Returns a pointer to the CppType Value fetching.
     */
    static CppType* getCppPtr2Fetch(T* obj) {
        if constexpr (is_pointer) {
            // T* is CppType**:
            //   we need to allocate a new CppType object to fetch the value
            return (*obj = new CppType()); 
        } else {
            return obj; // T* is CppType*
        }
    } // getCppPtr2Fetch
}; // ScalarTypeInfo



/**
 * @struct VectorTypeInfo
 * @brief This is a trait that extracts type information from a vector type,
 *     such as std::vector<int>, std::vector<float>*, etc.
 */
template<typename T>
struct VectorTypeInfo {
    static constexpr bool is_pointer = std::is_pointer<T>::value;

    // Row is vector<VecElemType>:
    //   If T is vector<VecElemType>, Raw is also vector<VecElemType>.
    using Raw = std::remove_const_t<std::remove_pointer_t<T>>;
    static_assert(!std::is_pointer<Raw>::value,
        "VectorTypeInfo do not support multilevel pointer type pointing to vector type");
    static_assert(is_vector<Raw>::value,
        "VectorTypeInfo only supports std::vector<T> or std::vector<T>*");

    using CppType = Raw; // always vector<VecElemType> 
    static constexpr SqlType sqlType = SqlType::Unknown; // vector type is not a scalar type
//    static constexpr SqlType sqlType = VecElemInfo::sqlType;

    using VecElemType = typename Raw::value_type;
    using VecElemInfo = ScalarTypeInfo<VecElemType>;
    using VecCppType  = typename VecElemInfo::CppType;

    // vector element type information
    static constexpr bool elemIsPointer = VecElemInfo::is_pointer;
    static constexpr SqlType elemSqlType = VecElemInfo::sqlType;

public:
    /**
     * @brief Get the CppType value pointer from the given object.
     * @param obj The object of type T.
     * @return CppType* Returns a pointer to the CppType Value.
     */
    static CppType* getCppPtr2Bind(T* obj) {
        if constexpr (is_pointer) {
            return *obj; // T* is vector<Elem>**
        } else {
            return  obj; // T* is vector<Elem>*
        }
    } // getCppPtr2Bind

    /**
     * @brief Get the CppType value pointer from the given object to fetch from the database.
     * @param obj The object of type T.
     * @return CppType* Returns a pointer to the CppType Value fetching.
     */
    static CppType* getCppPtr2Fetch(T* obj) {
        if constexpr (is_pointer) {
            // T* is vector<Elem>**
            //   we need to allocate a new CppType object to fetch the value
            return *obj = new std::vector<VecElemType>();
        } else {
            return obj; // T* is vector<Elem>*
        }
    } // getCppPtr2Fetch
}; // VectorTypeInfo



/**
 * @struct TypeInfoTrait
 * @brief This is a wrapper trait that extracts type information from a given type T.
 *     such as int, float, double*, std::vector<int>, std::vector<float>*, etc.
 */
template<typename T>
struct TypeInfoTrait {
    using NoPtrT = std::remove_const_t<std::remove_pointer_t<T>>; // remove const and pointer
    static constexpr bool is_vector = is_vector<NoPtrT>::value;
    using Info = std::conditional_t<
        is_vector, VectorTypeInfo<T>, ScalarTypeInfo<T>
    >;

    static constexpr bool is_pointer = Info::is_pointer;

    using            CppType =typename Info::CppType;
    static constexpr SqlType sqlType = Info::sqlType;

    // vector element type information
    using VecElemType = typename Info::VecElemType;
    using VecElemInfo = typename Info::VecElemInfo;
    using VecCppType  = typename Info::VecCppType;

    static constexpr bool elemIsPointer = Info::elemIsPointer;
    static constexpr SqlType elemSqlType = Info::elemSqlType;

public:
    /**
     * @brief Get the CppType value pointer from the given object.
     * @param obj The object of type T.
     * @return CppType* Returns a pointer to the CppType Value.
     */
    static CppType* getCppPtr2Bind(T* obj) {
        return Info::getCppPtr2Bind(obj);
    } // getCppPtr2Bind

    static CppType* getCppPtr2Fetch(T* obj) {
        return Info::getCppPtr2Fetch(obj);
    } // getCppPtr2Fetch
}; // TypeInfoTrait


} // namespace edadb