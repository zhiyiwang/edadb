/**
 * @file TypeMetaData.h
 * @brief TypeMetaData.h provides a way to get the meta data of a class.
 */

#pragma once

#include <string>
#include <iostream>

#include "TraitUtils.h"

namespace edadb {

/**
 * @brief TypeMetaData provides a way to get the meta data of a class.
 *   Here we only declare the template class, and the specialization will be defined in Table4Class.h.
 * @tparam T The class type.
 */
template<typename T>
struct TypeMetaData {
    using TupType          = boost::fusion::vector<>;
    using TupTypePairType  = boost::fusion::vector<>;
    using PkTupType        = boost::fusion::vector<>;
    using PkTupTypePairType= boost::fusion::vector<>;

    inline static std::string const& class_name() {
        /* Dierectly use this class will cause error during linking */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro");

        static const std::string n;
        return n;
    }

    inline static std::string const& table_name() {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro");

        static const std::string t;
        return t;
    }

    inline static const std::vector<std::string>& member_names() {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro");

        static const std::vector<std::string> v;
        return v;
    }

    inline static const std::vector<std::string>& pk_member_names() {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro"
        );

        static const std::vector<std::string> v;
        return v;
    } 

    inline static const std::vector<std::string>& column_names() {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro");

        static const std::vector<std::string> v;
        return v;
    }

    inline static const std::vector<std::string>& pk_column_names() {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro");

        static const std::vector<std::string> v;
        return v;
    }

    inline static auto tuple_type_pair() -> TupTypePairType const& {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro");

        static const TupTypePairType t{};
        return t;
    }

    inline static auto pk_tuple_type_pair() -> PkTupTypePairType const& {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro");

        static const PkTupTypePairType t{};
        return t;
    }

    inline static TupType getVal(T* /*obj*/) {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro"
        );

        return TupType{};
    }

    inline static PkTupType getPkVal(T* /*obj*/) {
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro TABLE4CLASS macro"
        );

        return PkTupType{};
    }
}; // TypeMetaData



/**
 * @brief TypeMetaDataPrinter provides a way to print the values of a class.
 * @tparam T The class type.
 */
template<typename T>
struct TypeMetaDataPrinter {
private:
    uint32_t index = 0; // print index

public:
    TypeMetaDataPrinter () = default;
    ~TypeMetaDataPrinter() = default;

public:
    void printStatic() {
        std::string class_name = TypeMetaData<T>::class_name();
        std::string temp_name = "TypeMetaData<" + class_name + ">";
        std::cout << temp_name << " Static Members: " << std::endl;
        std::cout << temp_name << "::table_name() = "
            << edadb::TypeMetaData<T>::table_name() << std::endl;
        std::cout << temp_name << "::class_name() = " 
            << edadb::TypeMetaData<T>::class_name() << std::endl;

        std::cout << temp_name << "::member_names() = ";
        auto names = edadb::TypeMetaData<T>::member_names();
        for(auto n : names)
            std::cout << n << " ";
        std::cout << std::endl;
        auto pk_names = edadb::TypeMetaData<T>::pk_member_names();
        std::cout << temp_name << "::pk_member_names() = ";
        for(auto pk : pk_names)
            std::cout << pk << " ";
        std::cout << std::endl;

        std::cout << temp_name << "::column_names() = ";
        auto cols = edadb::TypeMetaData<T>::column_names();
        for(auto c : cols) 
            std::cout << c << " ";
        std::cout << std::endl;
        std::cout << temp_name << "::pk_column_names() = ";
        auto pk_cols = edadb::TypeMetaData<T>::pk_column_names();
        for(auto pk : pk_cols)
            std::cout << pk << " ";
        std::cout << std::endl;

        std::cout << temp_name << "::tuple_type_pair() = ";
        auto ttp = edadb::TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(ttp, [](auto p){ std::cout << p.second << " "; });
        std::cout << temp_name << "::pk_tuple_type_pair() = ";
        auto pk_ttp = edadb::TypeMetaData<T>::pk_tuple_type_pair();
        boost::fusion::for_each(pk_ttp, [](auto p){ std::cout << p.second << " "; });
        std::cout << std::endl;

        std::cout<<std::endl<<std::endl;
    }


public:
    using TupType   = typename TypeMetaData<T>::TupType;
    using PkTupType = typename TypeMetaData<T>::PkTupType;
    void print(T* obj) {
        std::string class_name = TypeMetaData<T>::class_name();
        std::string temp_name = "TypeMetaData<" + class_name + ">";
        std::cout << temp_name << " Object Values: " << obj << std::endl;

        index = 0;
        std::cout << "All Values: " << std::endl;
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);

        // keep index: all primary key values following all values
        std::cout << "Primary Key Values: " << std::endl;
        auto pk_values = TypeMetaData<T>::getPkVal(obj);
        boost::fusion::for_each(pk_values, *this);
    }

    // iterate through the values and print the index and value
    template <typename ElemType>
    void operator()(const ElemType& elem) {
        std::cout << "Index [" << index++ << "]: ";
        if constexpr (std::is_same_v<std::decay_t<decltype(elem)>, std::string*>) {
            std::cout << "String Value: " << *elem << std::endl;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(elem)>, int*>) {
            std::cout << "Integer Value: " << *elem << std::endl;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(elem)>, double*>) {
            std::cout << "Double Value: " << *elem << std::endl;
        } else {
            std::cout << "Unknown Type" << std::endl;
        }
    }
}; // class TypeMetaDataPrinter


}  // namespace edadb