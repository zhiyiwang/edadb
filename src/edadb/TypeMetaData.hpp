/**
 * @file TypeMetaData.hpp
 * @brief TypeMetaData.hpp provides a way to get the meta data of a class.
 */


#pragma once

namespace edadb {


/**
 * @brief TypeMetaData provides a way to get the meta data of a class.
 * @tparam T The class type.
 */
template<typename T>
struct TypeMetaData {
    using MT = boost::fusion::vector<void>;
    static std::string const& class_name();
    boost::fusion::vector<boost::fusion::pair<void, std::string>> const& tuple_type_pair();
};


/**
 * @brief TypeMetaDataPrinter provides a way to print the values of a class.
 * @tparam T The class type.
 */
template<typename T>
struct TypeMetaDataPrinter {
private:
    uint32_t index = 0;

public:
    TypeMetaDataPrinter () = default;
    ~TypeMetaDataPrinter() = default;

public:
    void printTypeMetaDataStaticMembers(void) {
        std::string class_name = TypeMetaData<T>::class_name();
        std::string templ_name = "TypeMetaData<" + class_name + ">";

        std::cout << templ_name << " Static Members: " << std::endl;
        std::cout << templ_name << "::class_name() = "<< edadb::TypeMetaData<T>::class_name()<< std::endl;
        std::cout << templ_name << "::table_name() = "<< edadb::TypeMetaData<T>::table_name()<< std::endl;
        std::cout << templ_name << "::member_names() = ";
        auto names = edadb::TypeMetaData<T>::member_names();
        for(auto n : names)
            std::cout << n << " ";
        std::cout << std::endl;

        std::cout << templ_name << "::column_names() = ";
        auto cols = edadb::TypeMetaData<T>::column_names();
        for(auto c : cols) 
            std::cout << c << " ";
        std::cout << std::endl;

        std::cout << templ_name << "::tuple_type_pair() = ";
        auto ttp = edadb::TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(ttp, [](auto p){ std::cout << p.second << " "; });
        std::cout<<std::endl<<std::endl;
    }


public:
    using TupType = typename TypeMetaData<T>::TupType;

    void print(T* obj) {
        std::string class_name = TypeMetaData<T>::class_name();
        std::string templ_name = "TypeMetaData<" + class_name + ">";
        std::cout << templ_name << " Object Values: " << obj << std::endl;

        index = 0;
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
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
