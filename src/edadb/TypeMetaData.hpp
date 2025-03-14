/**
 * @file TypeMetaData.hpp
 * @brief TypeMetaData.hpp provides a way to get the meta data of a class.
 */


#pragma once

namespace edadb {

template<class T>
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template<typename T>
struct StripQualifiersAndMakePointer {
    using type = typename std::add_pointer<typename remove_cvref<T>::type>::type;
};


template<typename T>
struct IsComposite : boost::mpl::bool_<false> {
};


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



///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////

// macro example: (IdbSite, name, width, height)

// macro expansion result:
//   boost::fusion::vector<std::string*, int*, int*>
#define GENERATE_TupType_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupType_I, CLASS_ELEMS_TUP)

// macro expansion result:
//   boost::fusion::pair<std::string*, std::string>,
//   boost::fusion::pair<int*, std::string>,
//   boost::fusion::pair<int*, std::string>
#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<edadb::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePair_I, CLASS_ELEMS_TUP)

// macro expansion result:
//   boost::fusion::make_pair<std::string*>("name"),
//   boost::fusion::make_pair<int*>("width"),
//   boost::fusion::make_pair<int*>("height")
#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<edadb::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS_TUP)

// macro expansion result:
//   "name", "width", "height"
#define EXPAND_member_names_I(z, n, ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)


#define GET_first_member(ELEMS_TUP) BOOST_PP_TUPLE_ELEM(0,ELEMS_TUP)

// macro expansion result:
//   obj->name, obj->width, obj->height
#define GENERATE_ObjVal_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() &obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)
#define GENERATE_ObjVal(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_ObjVal_I, CLASS_ELEMS_TUP)



///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

// macro example:
//   TABLE4CLASS_COLNAME(IdbSite, "table_name", (name, width, height), ("iname","iwidth","iheight"))
#define TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP) \
BOOST_FUSION_ADAPT_STRUCT( myclass, BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP) ) \
namespace edadb{\
template<>\
struct CppTypeToDbType<myclass>{\
    static const DbTypes ret = DbTypes::kComposite;\
};\
template<> struct IsComposite<myclass> : boost::mpl::bool_<true> {};\
template<> struct TypeMetaData<myclass>{\
    using TupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, myclass))>;\
    using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, myclass))>;\
    using T = myclass;\
    \
    inline static auto tuple_type_pair()->TupTypePairType const&{\
        static const TupTypePairType t{GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, myclass))};\
        return t;\
    }\
    inline static std::string const& class_name(){\
        static std::string const class_name = BOOST_STRINGIZE(myclass);\
        return class_name;\
    }\
    inline static std::string const& table_name(){\
        static std::string const table_name = tablename;\
        return table_name;\
    }\
    inline static const std::vector<std::string>& member_names(){\
        static const std::vector<std::string> names = {EXPAND_member_names(CLASS_ELEMS_TUP)};\
        return names;\
    }\
    inline static const std::vector<std::string>& column_names(){\
        static const std::vector<std::string> names = {BOOST_PP_TUPLE_REM_CTOR(COLNAME_TUP)};\
        return names;\
    }\
    inline static TupType getVal(myclass * obj){\
        return TupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, myclass)));\
    }\
};\
}

// macro example:
//   TABLE4CLASS(IdbSite, "table_name", (name, width, height))
#define TABLE4CLASS(myclass, tablename, CLASS_ELEMS_TUP) \
TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)))

