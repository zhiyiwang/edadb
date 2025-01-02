///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////
#define GENERATE_TupType_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupType_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<test::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePair_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<test::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS_TUP)

#define EXPAND_member_names_I(z, n, ELEMS_TUP) ,BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)

#define GET_first_member(ELEMS_TUP) BOOST_PP_TUPLE_ELEM(0,ELEMS_TUP )

/// SPECIALIZE_BUN_HELPER Start
#define TABLE4CLASS(CLASS_ELEMS_TUP) BOOST_FUSION_ADAPT_STRUCT( BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP) ) \
namespace test{\
template<>\
struct CppTypeToDbType<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
static const DbTypes ret = DbTypes::kComposite;\
};\
}\
namespace test{\
template<> struct IsComposite<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> : boost::mpl::bool_<true> {};\
template<>\
struct TypeMetaData<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using TupType = boost::fusion::vector<GENERATE_TupType(CLASS_ELEMS_TUP)>;\
using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(CLASS_ELEMS_TUP)>;\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static auto tuple_type_pair()->TupTypePairType const&{\
static const TupTypePairType t{GENERATE_TupTypePairObj(CLASS_ELEMS_TUP)};\
return t;\
}\
inline static std::string const& class_name(){\  
static std::string const class_name = BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
return class_name;\
}\
inline static const std::vector<std::string>& member_names(){\
static const std::vector<std::string> names = {"oid_high", "oid_low" EXPAND_member_names(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))};\
return names;\
}\
};\
}