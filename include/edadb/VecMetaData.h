/**
 * @file VecMetaData.h
 * @brief VecMetaData.h provides a way to get the meta data of a class with vector.
 */

#pragma once

#include <string>
#include <iostream>
#include <vector>

namespace edadb {

/**
 * @brief VecMetaData provides a way to get the meta data of a class with vector
 * @tparam T The class type.
 */
template<typename T>
struct VecMetaData;

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
        std::cout << "VecMetaData<T>::VecElem" << std::endl;
        print_type<typename VecMetaData<T>::VecElem>();
        std::cout << std::endl;

        std::cout << "VecMetaData<T>::TupTypePairType" << std::endl;
        print_type<typename VecMetaData<T>::TupTypePairType>();
        std::cout << std::endl;

        std::cout << "VecMetaData<T>::tuple_type_pair()" << std::endl;
        auto ttp = VecMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(ttp,
            [](auto p){
                std::cout << "  ";
                print_type<typename decltype(p)::first_type>();
                std::cout << "  " << p.second;
                std::cout << std::endl;
            }
        );
        std::cout << std::endl;

        std::cout << "VecMetaData<T>::getVecElem()" << std::endl;
        T obj;
        auto vec_elem = VecMetaData<T>::getVecElem(&obj);
        boost::fusion::for_each(vec_elem, [](auto p){ std::cout << "  " << p << std::endl; });
        std::cout << std::endl;

        std::cout << "VecMetaData<T>::vec_field_names()" << std::endl;
        auto vec_field_names = VecMetaData<T>::vec_field_names();
        for(auto n : vec_field_names)
            std::cout << "  " << n << std::endl;
        std::cout << std::endl;


        // get VecMetaData<T>::VecId MAX Value
        constexpr auto enum_num = static_cast< std::underlying_type_t<typename VecMetaData<T>::VecId> >
            ( VecMetaData<T>::MAX );
        const auto &names = VecMetaData<T>::vec_field_names();

        // iterate through the VecId enum values
        for(std::underlying_type_t<typename VecMetaData<T>::VecId> i = 0; i < enum_num; ++i) {
            auto id = static_cast<typename VecMetaData<T>::VecId>(i);
            std::cout 
              << "id=" << i 
              << "   name=" << names[i] 
              << std::endl;
        }
    } // printStatic

}; // VecMetaDataPrinter

} // namespace edadb 