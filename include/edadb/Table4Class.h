/**
 * @file Table4Class.hpp
 * @brief Define some macros to generate the table class.
 */

#pragma once

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
//       IdbCoordinate coord;
//       IdbPin pin;
//   };

// macro expansion result:
//   boost::fusion::vector<std::string*, int*, int*, IdbCoordinate*, IdbPin*>
#define GENERATE_TupType_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_TupType_I, CLASS_ELEMS)


// macro expansion result:
//   boost::fusion::pair<std::string*, std::string /* = "name" */>,
//   boost::fusion::pair<int*, std::string /* = "width" */>,
//   boost::fusion::pair<int*, std::string /* = "height" */>,
//   boost::fusion::pair<IdbCoordinate*, std::string /* = "coord" */>,
//   boost::fusion::pair<IdbPin*, std::string /* = "pin" */>
#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<edadb::remove_cvref_and_make_pointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_TupTypePair_I, CLASS_ELEMS)

// macro expansion result:
//   boost::fusion::make_pair<std::string*>("name"),
//   boost::fusion::make_pair<int*>("width"),
//   boost::fusion::make_pair<int*>("height")
//   boost::fusion::make_pair<IdbCoordinate*>("coord"),
//   boost::fusion::make_pair<IdbPin*>("pin")
#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<edadb::remove_cvref_and_make_pointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS)


// macro expansion result:
//   "name", "width", "height", "coord", "pin"
#define EXPAND_member_names_I(z, n, ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)


// macro expansion result:
//   &(obj->name), &(obj->width), &(obj->height), &(obj->coord), &(obj->pin)
#define GENERATE_ObjVal_I(z, n, CLASS_ELEMS) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() &obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS)
#define GENERATE_ObjVal(CLASS_ELEMS) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS), 1), GENERATE_ObjVal_I, CLASS_ELEMS)


#define PP_HAS_ELEMS(TUPLE) \
    BOOST_PP_GREATER( BOOST_PP_TUPLE_SIZE(TUPLE), 1 )

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

// Example:
//   GENERATE_CLASS_TYPEMETADATA_WITH_PKEY(IdbSite, "edadb_idb_site",
//      (name, width, height, coord), ("iname","iwidth","iheight", "icoord"), (pin), ("ipin"), SqlType::Composite)
// Explain:
//   Create TypeMetaData class for IdbSite:
//      the member varibles (name, width, height, coord) will use all columns in the table,
//      the primary key columns are (pin), only the column "ipin" will be used as primary key.
#define GENERATE_CLASS_TYPEMETADATA_WITH_PKEY(CLASSNAME, TABLENAME, \
    CLASS_ELEMS, COL_NAMES, PK_ELEMS, PK_NAMES, SQLTYPE) \
BOOST_FUSION_ADAPT_STRUCT(CLASSNAME, BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS) ) \
namespace edadb{\
template<> \
struct Cpp2SqlTypeTrait<CLASSNAME>{\
    static constexpr SqlType sqlType = SQLTYPE; \
};\
template<> struct TypeMetaData<CLASSNAME>{\
    using TupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME))>;\
    using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME))>;\
    using PkTupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(PK_ELEMS, CLASSNAME))>;\
    using PkTupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(PK_ELEMS, CLASSNAME))>;\
    \
    using T = CLASSNAME;\
    \
    inline static std::string const& class_name(){\
        static std::string const class_name = BOOST_STRINGIZE(CLASSNAME);\
        return class_name;\
    }\
    inline static std::string const& table_name(){\
        static std::string const table_name = TABLENAME;\
        return table_name;\
    }\
    inline static const std::vector<std::string>& member_names(){\
        static const std::vector<std::string> names = \
            {EXPAND_member_names(CLASS_ELEMS)};\
        return names;\
    }\
    inline static const std::vector<std::string>& pk_member_names(){\
        static const std::vector<std::string> names = \
            {EXPAND_member_names(PK_ELEMS)};\
        return names;\
    }\
    inline static const std::vector<std::string>& column_names(){\
        static const std::vector<std::string> names = \
            {BOOST_PP_TUPLE_REM_CTOR(COL_NAMES)};\
        return names;\
    }\
    inline static const std::vector<std::string>& pk_column_names(){\
        static const std::vector<std::string> names = \
            {BOOST_PP_TUPLE_REM_CTOR(PK_NAMES)};\
        return names;\
    }\
    inline static auto tuple_type_pair()->TupTypePairType const&{\
        static const TupTypePairType t\
            {GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME))};\
        return t;\
    }\
    inline static auto pk_tuple_type_pair()->PkTupTypePairType const&{\
        static const PkTupTypePairType t\
            {GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(PK_ELEMS, CLASSNAME))};\
        return t;\
    }\
    inline static TupType getVal(CLASSNAME * obj){\
        return TupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME)));\
    }\
    inline static PkTupType getPkVal(CLASSNAME * obj){\
        return PkTupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(PK_ELEMS, CLASSNAME)));\
    }\
};\
}
// namespace edadb for Macro GENERATE_CLASS_TYPEMETADATA_WITH_PKEY


#define GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, COL_NAMES, SQLTYPE) \
BOOST_FUSION_ADAPT_STRUCT(CLASSNAME, BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS)) \
namespace edadb{\
template<> \
struct Cpp2SqlTypeTrait<CLASSNAME>{\
    static constexpr SqlType sqlType = SQLTYPE; \
};\
template<> struct TypeMetaData<CLASSNAME>{\
    using TupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME))>;\
    using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME))>;\
    using PkTupType = boost::fusion::vector<>;\
    using PkTupTypePairType = boost::fusion::vector<>;\
    \
    using T = CLASSNAME;\
    \
    inline static std::string const& class_name(){\
        static std::string const class_name = BOOST_STRINGIZE(CLASSNAME);\
        return class_name;\
    }\
    inline static std::string const& table_name(){\
        static std::string const table_name = TABLENAME;\
        return table_name;\
    }\
    inline static const std::vector<std::string>& member_names(){\
        static const std::vector<std::string> names = \
            {EXPAND_member_names(CLASS_ELEMS)};\
        return names;\
    }\
    inline static const std::vector<std::string>& pk_member_names(){\
        static const std::vector<std::string> names = {};\
        return names;\
    }\
    inline static const std::vector<std::string>& column_names(){\
        static const std::vector<std::string> names = \
            {BOOST_PP_TUPLE_REM_CTOR(COL_NAMES)};\
        return names;\
    }\
    inline static const std::vector<std::string>& pk_column_names(){\
        static const std::vector<std::string> names = {};\
        return names;\
    }\
    inline static auto tuple_type_pair()->TupTypePairType const&{\
        static const TupTypePairType t\
            {GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME))};\
        return t;\
    }\
    inline static auto pk_tuple_type_pair()->PkTupTypePairType const&{\
        static const PkTupTypePairType t{};\
        return t;\
    }\
    inline static TupType getVal(CLASSNAME * obj){\
        return TupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS, CLASSNAME)));\
    }\
    inline static PkTupType getPkVal(CLASSNAME * obj){\
        return PkTupType();\
    }\
};\
}
// namespace edadb for Macro GENERATE_CLASS_TYPEMETADATA



/**
 * @fn TABLE4CLASS_COLNAME
 * @brief TABLE4CLASS_COLNAME is a macro to define a table for a class with column names.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS The tuple of class elements.
 * @param COL_NAMES The tuple of column names.
 */
#define TABLE4CLASS_COLNAME(CLASSNAME, TABLENAME, CLASS_ELEMS, COL_NAMES) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, COL_NAMES, SqlType::Composite)

#define TABLE4CLASS_COLNAME_WITH_PKEY(CLASSNAME, TABLENAME, CLASS_ELEMS, COL_NAMES, PK_ELEMS, PK_NAMES) \
GENERATE_CLASS_TYPEMETADATA_WITH_PKEY(CLASSNAME, TABLENAME, CLASS_ELEMS, COL_NAMES, PK_ELEMS, PK_NAMES, SqlType::Composite)

/**
 * @fn TABLE4CLASS
 * @brief TABLE4CLASS is a macro to define a table for a class.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS The tuple of class elements.
 */
#define TABLE4CLASS(CLASSNAME, TABLENAME, CLASS_ELEMS) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, (EXPAND_member_names(CLASS_ELEMS)), SqlType::Composite)

#define TABLE4CLASS_WITH_PKEY(CLASSNAME, TABLENAME, CLASS_ELEMS, PK_ELEMS) \
GENERATE_CLASS_TYPEMETADATA_WITH_PKEY(CLASSNAME, TABLENAME, CLASS_ELEMS, (EXPAND_member_names(CLASS_ELEMS)), PK_ELEMS, (EXPAND_member_names(PK_ELEMS)), SqlType::Composite)

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
TABLE4CLASS(edadb::Shadow<CLASSNAME>, "#edadb_shadow_table", CLASS_ELEMS)

#define TABLE4EXTERNALCLASS_WITH_PKEY(CLASSNAME, CLASS_ELEMS, PK_ELEMS) \
namespace edadb{\
template<>\
struct Cpp2SqlTypeTrait<CLASSNAME>{\
    static constexpr SqlType sqlType = SqlType::External; \
};\
}\
TABLE4CLASS_WITH_PKEY(edadb::Shadow<CLASSNAME>, "#edadb_shadow_table", CLASS_ELEMS, PK_ELEMS) 



/**
 * @fn TABLE4CLASS_WVEC_COLNAME
 * @brief TABLE4CLASS_WVEC_COLNAME is a macro to define a table for a class with vector and column names.
 * @param CLASSNAME The name of the class.
 * @param TABLENAME The name of the table.
 * @param CLASS_ELEMS The tuple of class elements.
 * @param COL_NAMES The tuple of column names.
 * @param VEC_ELEMS The tuple of vector elements.
 */
#define TABLE4CLASS_WVEC_COLNAME(CLASSNAME, TABLENAME, CLASS_ELEMS, COL_NAMES, VEC_ELEMS) \
GENERATE_CLASS_TYPEMETADATA(CLASSNAME, TABLENAME, CLASS_ELEMS, COL_NAMES, SqlType::Composite) \
namespace edadb { \
  template<> struct VecMetaData<CLASSNAME> { \
    using VecElem = boost::fusion::vector< \
      GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(VEC_ELEMS, CLASSNAME)) \
    >; \
    using TupTypePairType = boost::fusion::vector< \
      GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(VEC_ELEMS, CLASSNAME)) \
    >; \
    \
    inline static auto tuple_type_pair() -> TupTypePairType const& { \
      static const TupTypePairType t{ \
        GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(VEC_ELEMS, CLASSNAME)) \
      }; \
      return t; \
    } \
    inline static VecElem getVecElem(CLASSNAME* obj) { \
      return VecElem( \
        GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(VEC_ELEMS, CLASSNAME)) \
      ); \
    } \
    inline static const std::vector<std::string>& vec_field_names() { \
      static const std::vector<std::string> names = { \
        EXPAND_member_names(VEC_ELEMS) \
      }; \
      return names; \
    } \
    enum VecId { \
      /* if VEC_ELEMS not empty, enum the VEC_ELEMS tuples */ \
      BOOST_PP_IF( \
        BOOST_PP_TUPLE_SIZE(VEC_ELEMS), \
        BOOST_PP_TUPLE_ENUM, \
        BOOST_PP_EMPTY \
      )(VEC_ELEMS) \
      /* if VEC_ELEMS not empty, append an extra comma */ \
      BOOST_PP_IF( \
        BOOST_PP_TUPLE_SIZE(VEC_ELEMS), \
        BOOST_PP_COMMA, \
        BOOST_PP_EMPTY \
      )() \
      /* append an extra enum value MAX */ \
      MAX = BOOST_PP_IF( \
        BOOST_PP_TUPLE_SIZE(VEC_ELEMS), \
        BOOST_PP_TUPLE_SIZE(VEC_ELEMS), \
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
 * @param VEC_ELEMS vector class elements, which is a tuple(class object)
 */
#define TABLE4CLASS_WVEC(CLASSNAME, TABLENAME, CLASS_ELEMS, VEC_ELEMS) \
TABLE4CLASS_WVEC_COLNAME(CLASSNAME, TABLENAME, CLASS_ELEMS, (EXPAND_member_names(CLASS_ELEMS)), VEC_ELEMS)

