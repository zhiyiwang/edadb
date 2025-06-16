/**
 * @file VecMetaData.h
 * @brief VecMetaData.h provides a way to get the meta data of a class with vector.
 */

#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "TraitUtils.h"

namespace edadb {

/**
 * @brief VecMetaData provides a way to get the meta data of a class with vector
 * @tparam T The class type.
 */
template<typename T>
struct VecMetaData {
    using VecElem = boost::fusion::vector<>;
    using TupTypePairType = boost::fusion::vector<>;
  
    inline static auto tuple_type_pair() -> TupTypePairType const& {
        /* Dierectly use this class will cause error during link */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_WITH_VECTOR_ "
        );
    
        static const TupTypePairType t{};
        return t;
    }
    inline static VecElem getVecElem(T*) {
        /* Dierectly use this class will cause error during link. */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_WITH_VECTOR_ "
        );
        return VecElem{};
    }
    inline static const std::vector<std::string>& vec_field_names() {
        /* Dierectly use this class will cause error during link. */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_WITH_VECTOR_ "
        );
        static const std::vector<std::string> v{};
        return v;
    }
  
    enum VecId { MAX = 0 };
};

/**
 * @brief VecMetaDataPrinter provides a way to print the values of a class with vector
 * @tparam T The class type.
 */
template<typename T>
struct VecMetaDataPrinter {
private:
    uint32_t index = 0; // print index

public:
    VecMetaDataPrinter () = default;
    ~VecMetaDataPrinter() = default;

public:
    void printStatic() {
        std::cout << "VecMetaDataPrinter::printStatic():" << std::endl;

        // print each vector elem (vector<T> *)
        std::cout << "VecMetaData<T>::VecElem" << std::endl;
        print_type<typename VecMetaData<T>::VecElem>();
        std::cout << std::endl << std::endl;

        // print vector elem type
        std::cout << "VecMetaData<T>::TupTypePairType" << std::endl;
        print_type<typename VecMetaData<T>::TupTypePairType>();
        std::cout << std::endl << std::endl;

        // print each vector elem (boost::fusion::pair<std::vector<IdbRect>*, string) 
        std::cout << "VecMetaData<T>::tuple_type_pair() return type : ";
        std::cout << "boost::fusion::pair<std::vector<T>*, string>" << std::endl;
        auto ttp = VecMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(ttp,
            [](auto v){
                std::cout << "    < " ;
                print_type<typename decltype(v)::first_type>();
                std::cout << " , " << v.second << " >" << std::endl;
            }
        );
        std::cout << std::endl;


        // print each vector elem pointer (std::vector<IdbRect> *)
        std::cout << "VecMetaData<T>::getVecElem()" << std::endl;
        T obj;
        auto vec_elem = VecMetaData<T>::getVecElem(&obj);
        boost::fusion::for_each(vec_elem,
            [](auto v){
                std::cout << "  " << v << std::endl;
            }
        );
        std::cout << std::endl;

        // print field names
        std::cout << "VecMetaData<T>::vec_field_names()" << std::endl;
        auto vec_field_names = VecMetaData<T>::vec_field_names();
        for(auto n : vec_field_names)
            std::cout << "  " << n << std::endl;
        std::cout << std::endl;


        // get VecMetaData<T>::VecId MAX Value
        std::cout << "VecMetaData<T>::VecId MAX Value" << std::endl;
        constexpr auto enum_num = static_cast< std::underlying_type_t<typename VecMetaData<T>::VecId> >
            ( VecMetaData<T>::MAX );
        const auto &names = VecMetaData<T>::vec_field_names();
        for(std::underlying_type_t<typename VecMetaData<T>::VecId> i = 0; i < enum_num; ++i) {
            auto id = static_cast<typename VecMetaData<T>::VecId>(i);
            std::cout << "id=" << i << "   name=" << names[i] << std::endl;
        }
    } // printStatic
}; // VecMetaDataPrinter

} // namespace edadb 