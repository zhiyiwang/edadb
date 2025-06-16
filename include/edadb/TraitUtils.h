/**
 * @file TraitUtils.h
 * @brief Some trait utilities for the project.
 */

#pragma once

#include <boost/type_index.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/pair.hpp>

#include <iostream> 
#include <type_traits>


namespace edadb {

/**
 * @brief always_false: A type trait that always evaluatej to false.
 */
template<typename>
struct always_false : std::false_type {};

/**
 * @brief is_vector: A type trait to check if a type is a std::vector.
*/
template<typename T>
struct is_vector : std::false_type {};

/**
 * @brief is_vector specialization for std::vector.
 */
template<typename U, typename Alloc>
struct is_vector<std::vector<U, Alloc>> : std::true_type {};





/**
 * @brief remove const and reference from a type
 */
template<class T>
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
}; // remove_cvref


/**
 * @brief remove const and pointer from a type
 */
template<typename T>
struct remove_const_and_pointer {
//struct removeConstAndPointer {
    using type = std::remove_const_t<std::remove_pointer_t<T>>;
}; // remove_const_and_pointer


/**
 * @brief remove const and reference from a type and make g'gt a pointer
 */
template<typename T>
struct remove_cvref_and_make_pointer {
//struct StripQualifiersAndMakePointer {
    using type = typename std::add_pointer<typename remove_cvref<T>::type>::type;
}; // remove_cvref_and_make_pointer


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
} // print_type


} // namespace edadb