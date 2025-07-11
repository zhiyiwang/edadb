/**
 * @file Table4Class.hpp
 * @brief Define some macros to generate the table class.
 */

#pragma once

#include <boost/preprocessor.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/variant.hpp>

#include "TraitUtils.h"
#include "TypeMetaData.h"
#include "VecMetaData.h"


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
#define GENERATE_TupType_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_TupType_I, CLASS_ELEMS)


// macro expansion result:
//   boost::fusion::pair<std::string*, std::string>,
//   boost::fusion::pair<int*, std::string>,
//   boost::fusion::pair<int*, std::string>
#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<edadb::remove_cvref_and_make_pointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_TupTypePair_I, CLASS_ELEMS)

// macro expansion result:
//   boost::fusion::make_pair<std::string*>("name"),
//   boost::fusion::make_pair<int*>("width"),
//   boost::fusion::make_pair<int*>("height")
#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<edadb::remove_cvref_and_make_pointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS)


// macro expansion result:
//   "name", "width", "height"
#define EXPAND_member_names_I(z, n, ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)


// macro expansion result:
//   &(obj->name), &(obj->width), &(obj->height)
#define GENERATE_ObjVal_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() &obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS)
#define GENERATE_ObjVal(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_ObjVal_I, CLASS_ELEMS)

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////



// Example:
//   GENERATE_CLASS_TYPEMETADATA(IdbSite, "table_name",
//      (name, width, height), ("iname","iwidth","iheight"))
// Explain:
//   Create TypeMetaData class for IdbSite for the member varibles (name, width, height),
//   use ("iname","iwidth","iheight")) as name instead
#define GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, COLNAMES, SQLTYPE) \
BOOST_FUSION_ADAPT_STRUCT(CLASSNAME, BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS)) ; \
namespace edadb { \
template<> \
struct Cpp2SqlTypeTrait<CLASSNAME>{\
    static constexpr SqlType sqlType = SQLTYPE; \
};\
template<> struct TypeMetaData<CLASSNAME>{ \
    using T = CLASSNAME; \
    \
    /* class and table name */ \
    inline static std::string const& class_name(){\
        static std::string const class_name = BOOST_STRINGIZE(CLASSNAME);\
        return class_name;\
    }\
    inline static std::string const& table_name(){\
        static std::string const table_name = TABLENAME;\
        return table_name;\
    }\
    \
    /* member names */ \
    inline static const std::vector<std::string>& member_names(TypeMetaTag tag) { \
        switch (tag) { \
            case TypeMetaTag::UDEF: \
                return member_names_impl<TypeMetaTag::UDEF>(); \
            case TypeMetaTag::PKEY: \
                return member_names_impl<TypeMetaTag::PKEY>(); \
            default: \
                static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
                return std::vector<std::string>{}; \
    } \
    template<TypeMetaTag Tag> \
    inline static const std::vector<std::string>& member_names_impl()  { \
        if constexpr (Tag == TypeMetaTag::UDEF) { \
            static const std::vector<std::string> names = {EXPAND_member_names(CLASS_ELEMS)}; \
            return names; \
        } else if constexpr (Tag == TypeMetaTag::PKEY) { \
            static const std::vector<std::string> names = { \
                BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS)) \
            }; \
            return names; \
        } else { \
            static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
            return std::vector<std::string>{}; \
        } \
    } \
    \
    /* column names */ \
    inline static const std::vector<std::string>& column_names(TypeMetaTag tag) { \
        switch (tag) { \
            case TypeMetaTag::UDEF: \
                return column_names_impl<TypeMetaTag::UDEF>(); \
            case TypeMetaTag::PKEY: \
                return column_names_impl<TypeMetaTag::PKEY>(); \
            default: \
                static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
                return std::vector<std::string>{}; \
        } \
    } \
    template<TypeMetaTag Tag> \
    inline static const std::vector<std::string>& column_names_impl() { \
        if constexpr (Tag == TypeMetaTag::UDEF) { \
            static const std::vector<std::string> names = {BOOST_PP_TUPLE_REM_CTOR(COLNAMES)}; \
            return names; \
        } else if constexpr (Tag == TypeMetaTag::PKEY) { \
            static const std::vector<std::string> names = { \
                BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, COLNAMES)) \
            }; \
            return names; \
        } else { \
            static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
            return std::vector<std::string>{}; \
        } \
    } \
    \
    /* TupType for getVal */ \
    template<TypeMetaTag Tag> struct TupTypeStruct; \
    template<TypeMetaTag Tag> \
    using TupType = typename TupTypeStruct<Tag>::type; \
    using TupTypeVariant = boost::variant<TupType<TypeMetaTag::UDEF>, TupType<TypeMetaTag::PKEY>>;\
    \
    inline static TupTypeVariant getVal(CLASSNAME* obj, TypeMetaTag tag) { \
        switch (tag) { \
            case TypeMetaTag::UDEF: \
                return getVal_impl<TypeMetaTag::UDEF>(obj); \
            case TypeMetaTag::PKEY: \
                return getVal_impl<TypeMetaTag::PKEY>(obj); \
            default: \
                static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
                return TupTypeVariant{}; \
        } \
    } \
    template<TypeMetaTag Tag> \
    inline static TupTypeVariant getVal_impl(CLASSNAME* obj) { \
        if constexpr (Tag == TypeMetaTag::UDEF) { \
            return TupType<TypeMetaTag::UDEF>{ \
                GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME)) \
            }; \
        } else if constexpr (Tag == TypeMetaTag::PKEY) { \
            return TupType<TypeMetaTag::PKEY>{ \
                GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT( \
                    (BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS)), CLASSNAME)) \
            }; \
        } else { \
            static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
            return TupTypeVariant{}; \
        } \
    } \
    \
    /* TupTypePair for */ \
    template<TypeMetaTag Tag> struct TupTypePairStruct; \
    template<TypeMetaTag Tag> \
    using TupTypePairType = typename TupTypePairStruct<Tag>::type; \
    using TupTypePairVariant = boost::variant<TupTypePairType<TypeMetaTag::UDEF>, TupTypePairType<TypeMetaTag::PKEY>>;\
    \
    inline static TupTypePairVariant tuple_type_pair(CLASSNAME* obj, TypeMetaTag tag) { \
        switch (tag) { \
            case TypeMetaTag::UDEF: \
                return tuple_type_pair_impl<TypeMetaTag::UDEF>(obj); \
            case TypeMetaTag::PKEY: \
                return tuple_type_pair_impl<TypeMetaTag::PKEY>(obj); \
            default: \
                static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
                return TupTypePairVariant{}; \
        } \
    } \
    template<TypeMetaTag Tag> \
    inline static TupTypePairVariant tuple_type_pair_impl(CLASSNAME* obj) { \
        if constexpr (Tag == TypeMetaTag::UDEF) { \
            return TupTypePair<TypeMetaTag::UDEF>{ \
                GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME)) \
            }; \
        } else if constexpr (Tag == TypeMetaTag::PKEY) { \
            return TupTypePair<TypeMetaTag::PKEY>{ \
                GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT( \
              u     (BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS)), CLASSNAME)) \
            }; \
        } else { \
            static_assert(always_false<T>::value, "TypeMetaTag must be either UDEF or PKEY"); \
            return TupTypePairVariant{}; \
        } \
    } \
}; /* TypeMetaData<CLASSNAME> */ \
\
template<> struct TypeMetaData<CLASSNAME>::TupTypeStruct<TypeMetaTag::UDEF> { \
    using type = boost::fusion::vector< /* all columns in CLASS_ELEMS */ \
        GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME)) \
    >; \
}; \
template<> struct TypeMetaData<CLASSNAME>::TupTypeStruct<TypeMetaTag::PKEY> { \
    using type = boost::fusion::vector< /* only first column in CLASS_ELEMS */ \
        GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT( \
            (BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS)), CLASSNAME)) \
    >; \
}; \
\
template<> struct TypeMetaData<CLASSNAME>::TupTypePairStruct<TypeMetaTag::UDEF> { \
    using type = boost::fusion::vector< /* all columns in CLASS_ELEMS */ \
        GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME)) \
    >; \
}; \
template<> struct TypeMetaData<CLASSNAME>::TupTypePairStruct<TypeMetaTag::PKEY> { \
    using type = boost::fusion::vector< /* only first column in CLASS_ELEMS */ \
        GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT( \
            (BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS)), CLASSNAME)) \
    >; \
}; \
} /* namespace edadb */
// namespace edadb for Macro GENERATE_CLASS_TYPEMETADATA




/**
 * @fn TABLE4CLASS_COLNAME
 * @brief TABLE4CLASS_COLNAME is a macro to define a table for a class with column names.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS The tuple of class elements.
 * @param COLNAMES The tuple of column names.
 */
#define TABLE4CLASS_COLNAME(CLASSNAME, TABLENAME, CLASS_ELEMS, COLNAMES) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, COLNAMES, SqlType::Composite) 

/**
 * @fn TABLE4CLASS
 * @brief TABLE4CLASS is a macro to define a table for a class.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS The tuple of class elements.
 */
#define TABLE4CLASS(CLASSNAME, TABLENAME, CLASS_ELEMS) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, (EXPAND_member_names(CLASS_ELEMS)), SqlType::Composite)

/**
 * @fn Table4ExternalClass
 * @brief Table4ExternalClass is a macro to define a private table for a class.
 *      the private table is a expanded table of the class, which is similar to the composite table.
 *      Hence, the private table is not a real table in the database.
 * @param CLASSNAME The name of the class. 
 * @param CLASS_ELEMS The tuple of class elements.
 */
#define TABLE4EXTERNALCLASS(CLASSNAME, CLASS_ELEMS) \
namespace edadb{\
template<>\
struct Cpp2SqlTypeTrait<CLASSNAME>{\
    static constexpr SqlType sqlType = SqlType::External; \
};\
}\
TABLE4CLASS(edadb::Shadow<CLASSNAME>, "#shadow_table_for_external_class", CLASS_ELEMS) 



/**
 * @fn TABLE4CLASS_WVEC_COLNAME
 * @brief TABLE4CLASS_WVEC_COLNAME is a macro to define a table for a class with vector and column names.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS The tuple of class elements.
 * @param COLNAMES The tuple of column names.
 * @param VECTOR_ELEMS The tuple of vector elements.
 */
#define TABLE4CLASS_WVEC_COLNAME(CLASSNAME, TABLENAME, CLASS_ELEMS, COLNAMES, VECTOR_ELEMS) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, COLNAMES, SqlType::CompositeVector); \
namespace edadb { \
template<> struct VecMetaData<CLASSNAME> { \
    using VecElem = boost::fusion::vector< \
        GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS, CLASSNAME)) \
    >; \
    using TupTypePairType = boost::fusion::vector< \
        GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS, CLASSNAME)) \
    >; \
    \
    inline static auto tuple_type_pair()->TupTypePairType const& { \
        static const TupTypePairType t{ \
            GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS, CLASSNAME)) \
        }; \
        return t; \
    } \
    inline static VecElem getVecElem(CLASSNAME* obj) { \
      return VecElem( \
        GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(VECTOR_ELEMS, CLASSNAME)) \
      ); \
    } \
    inline static const std::vector<std::string>& vec_field_names() { \
      static const std::vector<std::string> names = { \
        EXPAND_member_names(VECTOR_ELEMS) \
      }; \
      return names; \
    } \
    enum VecId { \
      /* if VECTOR_ELEMS not empty, enum the VECTOR_ELEMS tuples */ \
      BOOST_PP_IF( \
        BOOST_PP_TUPLE_SIZE(VECTOR_ELEMS), \
        BOOST_PP_TUPLE_ENUM, \
        BOOST_PP_EMPTY \
      )(VECTOR_ELEMS) \
      /* if VECTOR_ELEMS not empty, append an extra comma */ \
      BOOST_PP_IF( \
        BOOST_PP_TUPLE_SIZE(VECTOR_ELEMS), \
        BOOST_PP_COMMA, \
        BOOST_PP_EMPTY \
      )() \
      /* append an extra enum value MAX */ \
      MAX = BOOST_PP_IF( \
        BOOST_PP_TUPLE_SIZE(VECTOR_ELEMS), \
        BOOST_PP_TUPLE_SIZE(VECTOR_ELEMS), \
        0 \
      ) \
    }; \
  }; \
}
// namespace edadb for Macro TABLE4CLASS_WVEC_COLNAME



/**
 * @fn TABLE4CLASS_WVEC
 * @brief TABLE4CLASS_WVEC is a macro to define a table for a class with vector members.
 * @param CLASSNAME class name defined by cpp
 * @param TABLENAME table name in the database
 * @param CLASS_ELEMS non-vector class elements, which is a tuple(cpp type or class object)
 * @param VECTOR_ELEMS vector class elements, which is a tuple(class object)
 */
#define TABLE4CLASS_WVEC(CLASS_NAME, TABLE_NAME, CLASS_ELEMS, VECTOR_ELEMS) \
TABLE4CLASS_WVEC_COLNAME(CLASS_NAME, TABLE_NAME, CLASS_ELEMS, (EXPAND_member_names(CLASS_ELEMS)), VECTOR_ELEMS)
