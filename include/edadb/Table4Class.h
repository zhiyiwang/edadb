/**
 * @file TypeMetaData.hpp
 * @brief TypeMetaData.hpp provides a way to get the meta data of a class.
 */

#pragma once

#include "TypeMetaData.h"


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


}  // namespace edadb



///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////

// macro example: 
//   class IdbSite {
//       std::string name;
//       int width;
//       int height;
//   };

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
//   &(obj->name), &(obj->width), &(obj->height)
#define GENERATE_ObjVal_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() &obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)
#define GENERATE_ObjVal(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_ObjVal_I, CLASS_ELEMS_TUP)



///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

// Example:
//   GENERATE_CLASS_TYPEMETADATA(IdbSite, "table_name",
//      (name, width, height), ("iname","iwidth","iheight"))
// Explain:
//   Create TypeMetaData class for IdbSite for the member varibles (name, width, height),
//   use ("iname","iwidth","iheight")) as name instead
#define GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS_TUP, COLNAME_TUP, SQLTYPE) \
BOOST_FUSION_ADAPT_STRUCT(CLASSNAME, BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP) ) \
namespace edadb{\
template<>\
struct Cpp2SqlType<CLASSNAME>{\
    static constexpr SqlType sqlType = SQLTYPE; \
};\
template<> struct IsComposite<CLASSNAME> : boost::mpl::bool_<true> {};\
template<> struct TypeMetaData<CLASSNAME>{\
    using TupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, CLASSNAME))>;\
    using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, CLASSNAME))>;\
    using T = CLASSNAME;\
    \
    inline static auto tuple_type_pair()->TupTypePairType const&{\
        static const TupTypePairType t{GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, CLASSNAME))};\
        return t;\
    }\
    inline static std::string const& class_name(){\
        static std::string const class_name = BOOST_STRINGIZE(CLASSNAME);\
        return class_name;\
    }\
    inline static std::string const& table_name(){\
        static std::string const table_name = TABLENAME;\
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
    inline static TupType getVal(CLASSNAME * obj){\
        return TupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, CLASSNAME)));\
    }\
};\
}



/**
 * @fn TABLE4CLASS_COLNAME
 * @brief TABLE4CLASS_COLNAME is a macro to define a table for a class with column names.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS_TUP The tuple of class elements.
 * @param COLNAME_TUP The tuple of column names.
 */
#define TABLE4CLASS_COLNAME(CLASSNAME, TABLENAME, CLASS_ELEMS_TUP, COLNAME_TUP) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS_TUP, COLNAME_TUP, SqlType::Composite) 

/**
 * @fn TABLE4CLASS
 * @brief TABLE4CLASS is a macro to define a table for a class.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS_TUP The tuple of class elements.
 */
#define TABLE4CLASS(CLASSNAME, TABLENAME, CLASS_ELEMS_TUP) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)), SqlType::Composite)

/**
 * @fn Table4ExternalClass
 * @brief Table4ExternalClass is a macro to define a private table for a class.
 *      the private table is a expanded table of the class, which is similar to the composite table.
 *      Hence, the private table is not a real table in the database.
 * @param CLASSNAME The name of the class. 
 * @param CLASS_ELEMS_TUP The tuple of class elements.
 */
#define Table4ExternalClass(CLASSNAME, CLASS_ELEMS_TUP) \
namespace edadb{\
template<>\
struct Cpp2SqlType<CLASSNAME>{\
    static constexpr SqlType sqlType = SqlType::External; \
};\
}\
TABLE4CLASS(edadb::Shadow<CLASSNAME>, "#shadow_table_for_external_class", CLASS_ELEMS_TUP) 



#if 0 

/**
 * @fn TABLE4CLASS_WVEC_COLNAME
 * @brief TABLE4CLASS_WVEC_COLNAME is a macro to define a table for a class with vector and column names.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS_TUP The tuple of class elements.
 * @param COLNAME_TUP The tuple of column names.
 * @param VECTOR_ELEMS_TUP The tuple of vector elements.
 */
#define TABLE4CLASS_WVEC_COLNAME(CLASSNAME, TABLENAME, CLASS_ELEMS_TUP, COLNAME_TUP, VECTOR_ELEMS_TUP) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS_TUP, COLNAME_TUP, SqlType::CompositeVector); \
namespace edadb{\
    template<> struct VecMetaData<CLASSNAME>{\
        using VecElem = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS_TUP, CLASSNAME))>;\
        using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS_TUP, CLASSNAME))>;\
        \
        inline static auto tuple_type_pair()->TupTypePairType const&{\
            static const TupTypePairType t{GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS_TUP, CLASSNAME))};\
            return t;\
        }\
       inline static VecElem getVecElem(CLASSNAME * obj){\
            return VecElem(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS_TUP, CLASSNAME)));\
        }\
        inline static const std::vector<std::string>& vec_field_names(){\
            static const std::vector<std::string> names = {EXPAND_member_names(VECTOR_ELEMS_TUP)};\
            return names;\
        }\
        enum VecId{ \
            BOOST_PP_IF(BOOST_PP_TUPLE_SIZE(VECTOR_ELEMS_TUP), BOOST_PP_TUPLE_REM_CTOR(VECTOR_ELEMS_TUP), ) \
            MAX = BOOST_PP_IF(BOOST_PP_TUPLE_SIZE(VECTOR_ELEMS_TUP), BOOST_PP_TUPLE_SIZE(VECTOR_ELEMS_TUP), 0)  \
        };\
    };\
}


/**
 * @fn TABLE4CLASS_WVEC
 * @brief TABLE4CLASS_WVEC is a macro to define a table for a class with vector.
 * @param CLASSNAME The name of the class.  
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS_TUP The tuple of class elements.
 * @param VECTOR_ELEMS_TUP The tuple of vector elements.
 */
#define TABLE4CLASS_WVEC(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP, VECTOR_ELEMS_TUP) \
TABLE4CLASS_WVEC_COLNAME(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)), VECTOR_ELEMS_TUP)

#endif


