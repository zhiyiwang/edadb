/**
 * @file MacroHelper.h
 * @brief Some helper macros for the project.
 */

#pragma once

#include <iostream> 
#include <boost/type_index.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/pair.hpp>

#include "Macro4Sqlite.h"


namespace edadb {

/**
 * @brief Always false type trait
 * @tparam T The type to check.
 */
template<typename>
inline constexpr bool always_false_v = false;

/**
 * @brief remove const and reference from a type
 */
template<class T>
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

/**
 * @brief remove const and reference from a type and make it a pointer
 */
template<typename T>
struct StripQualifiersAndMakePointer {
    using type = typename std::add_pointer<typename remove_cvref<T>::type>::type;
};

/**
 * @brief Print the type name of a given type
 */
template<typename T>
void print_type() {
    // Print the type name using boost::typeindex
    // boost::typeindex::type_id_with_cvr<T>()
    //    boost template function to get the type name with Const, Volatile, and Reference
    // .pretty_name()
    //    to get the human-readable name
    std::cout << boost::typeindex::type_id_with_cvr<T>().pretty_name();
}

} // namespace edadb