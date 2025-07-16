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
 * @brief TypeMetaTag is used to specify the type of meta data for the functions
 */
enum struct TypeMetaTag : uint8_t {
    INVD = 0, // invalid, default value
    ALL = 1, // all user defined columns
    PKONLY = 2, // primary key ONLY, type meta data contains primary key columns only
    Max  = 3  // Not available, used fok'k enum size
};


/**
 * @brief type meta data info: class name and table name
 * 1. Name: function name
 * 2. Ret:  return data type
 * 3. Def:  default value to return
 */
#define EDADB_TYPE_META_INFO \
X(class_name, std::string const&, std::string{}) \
X(table_name, std::string const&, std::string{})

/**
 * @brief type meta data columns: use TypeMetaTag to specify the columns
 * 1. Name:    function name
 * 2. Ret: return data type
 * 3. Def: default value to return
 * 4. FuncParam: param for calling func using TypeMetaTag param
 * 5. TempParam: param for template function defined by TypeMetaTag
 */
#define EDADB_TYPE_META_COLS \
X(member_names,   const std::vector<std::string>&,std::vector<std::string>{},(TypeMetaTag),()) \
X(column_names,   const std::vector<std::string>&,std::vector<std::string>{},(TypeMetaTag),()) \
X(tuple_type_pair,TupTypePairType const&,         TupTypePairType{},         (TypeMetaTag),()) \
X(getVal,         TupType,                        TupType{},             (T*, TypeMetaTag),(T*))

/**
 * @brief TypeMetaData provides a way to get the meta data of a class.
 *   Here we only declare the template class, and the specialization will be defined in Table4Class.h.
 * @tparam T The class type.
 */
template<typename T>
struct TypeMetaData {
    using TupType          = boost::fusion::vector<>;
    using TupTypePairType  = boost::fusion::vector<>;

    // use EDADB_TYPE_META_INFO to define meta info functions
    #define X(Name, Ret, Def) \
    inline static Ret Name() {                                      \
        static_assert(always_false<T>::value,                         \
            "TypeMetaData<T> must be specialized by TABLE4CLASS_...");\
        static Ret v = Def;                                     \
        return v;                                                     \
    }
    EDADB_TYPE_META_INFO
    #undef X

    // use EDADB_TYPE_META_COLS to define MetaTypeTag param functions
    // no need to switch between TypeMetaTag, since static_assert will fail
    #define X(Name, Ret, Def, FuncParam, TempParam) \
    inline static Ret Name FuncParam {                                \
        static_assert(always_false<T>::value,                         \
            "TypeMetaData<T> must be specialized by TABLE4CLASS_...");\
        static Ret v = Def;                                           \
        return v;                                                     \
    }
    EDADB_TYPE_META_COLS
    #undef X

    // use TypeMetaTag as function template parameter
    // this is used to provide a way to get the meta data of a class using a tag
    #define X(Name, Ret, Def, FuncParam, TempParam) \
    template<TypeMetaTag Tag> \
    inline static Ret Name TempParam {                                \
        static_assert(always_false<T>::value,                         \
            "TypeMetaData<T> must be specialized by TABLE4CLASS_...");\
        static Ret v = Def;                                           \
        return v;                                                     \
    }
    EDADB_TYPE_META_COLS
    #undef X
};

#undef EDADB_TYPE_META_DATA_FUNCS





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
        std::string templ_name = "TypeMetaData<" + class_name + ">";

        std::cout << templ_name << " Static Members: " << std::endl;
        std::cout << templ_name << "::class_name() = " 
            << edadb::TypeMetaData<T>::class_name() << std::endl;
        std::cout << templ_name << "::table_name() = "
            << edadb::TypeMetaData<T>::table_name() << std::endl;

        std::cout << templ_name << "::member_names() = ";
        auto names =
            edadb::TypeMetaData<T>::template member_names<TypeMetaTag::UDEF>();
        for(auto n : names)
            std::cout << n << " ";
        std::cout << std::endl;

        std::cout << templ_name << "::column_names() = ";
        auto cols =
            edadb::TypeMetaData<T>::template column_names<TypeMetaTag::UDEF>();
        for(auto c : cols) 
            std::cout << c << " ";
        std::cout << std::endl;

        std::cout << templ_name << "::tuple_type_pair() = ";
        auto ttp =
            edadb::TypeMetaData<T>::template tuple_type_pair<TypeMetaTag::UDEF>();
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




#if 0
template<typename T>
struct TypeMetaData {
    using TupType          = boost::fusion::vector<>;
    using TupTypePairType  = boost::fusion::vector<>;

public: // class and table name
    inline static std::string const& class_name() {
        /* Dierectly use this class will cause error during linking */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_ or _EDADB_DEFINE_TABLE_BY_COLUMN_ "
        );

        static const std::string n;
        return n;
    }

    inline static std::string const& table_name() {
        /* Dierectly use this class will cause error during linking */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_ or _EDADB_DEFINE_TABLE_BY_COLUMN_ "
        );

        static const std::string t;
        return t;
    }

    inline static const std::vector<std::string>& member_names() {
        /* Dierectly use this class will cause error during linking */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_ or _EDADB_DEFINE_TABLE_BY_COLUMN_ "
        );

        static const std::vector<std::string> v;
        return v;
    }

    inline static const std::vector<std::string>& column_names() {
        /* Dierectly use this class will cause error during linking */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_ or _EDADB_DEFINE_TABLE_BY_COLUMN_ "
        );

        static const std::vector<std::string> v;
        return v;
    }

    inline static TupType getVal(T* /*obj*/) {
        /* Dierectly use this class will cause error during linking */
        static_assert(always_false<T>::value,
            "TypeMetaData<T> must be specialized by macro _EDADB_DEFINE_TABLE_BY_CLASS_ or _EDADB_DEFINE_TABLE_BY_COLUMN_ "
        );

        return TupType{};
    }
}; // TypeMetaData
#endif 
