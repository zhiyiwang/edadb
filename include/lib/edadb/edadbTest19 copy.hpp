#pragma once

#include <string>
#include <memory>
#include <set>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <bitset>
#include <third_party/fmt/format.hpp>
#include <third_party/rapidjson/rapidjson.h>
#include <third_party/rapidjson/document.h>
#include <third_party/json/json.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/preprocessor.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/include/copy.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/proto/proto.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <soci/error.h>
#include "lib/utils/MD5.hpp"
#include "lib/edadb/impl/EdadbCppTypeToSQLString.hpp"
// #include <sqlite3.h>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#define EDADB_DEBUG 1 

/*
* 支持vector的嵌套
*/



///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////
#define GENERATE_TupType_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupType_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypeNoPtr_I(z, n, CLASS_ELEMS_TUP) \
BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() \
std::remove_pointer< \
    std::remove_reference<decltype( \
        BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP):: \
        BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP) \
    )>::type \
>::type
#define GENERATE_TupTypeNoPtr(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypeNoPtr_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<edadb::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePair_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairNoPtr_I(z, n, CLASS_ELEMS_TUP) \
BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() \
boost::fusion::pair< \
    std::remove_cv_t<std::remove_reference_t< \
        decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)) \
    >>, \
    std::string \
>
#define GENERATE_TupTypePairNoPtr(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairNoPtr_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<edadb::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairObjNoPtr_I(z, n, CLASS_ELEMS_TUP) \
BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() \
boost::fusion::make_pair< \
    std::remove_cv_t<std::remove_reference_t< \
        decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)) \
    >> \
>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObjNoPtr(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObjNoPtr_I, CLASS_ELEMS_TUP)

#define EXPAND_member_names_I(z, n, ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)

#define GET_first_member(ELEMS_TUP) BOOST_PP_TUPLE_ELEM(0,ELEMS_TUP )

#define GENERATE_ObjVal_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)
#define GENERATE_ObjVal(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_ObjVal_I, CLASS_ELEMS_TUP)

#define GENERATE_ObjValPtr_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() &obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)
#define GENERATE_ObjValPtr(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_ObjValPtr_I, CLASS_ELEMS_TUP)


///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

/// TABLE4CLASS Start
// TABLE4CLASS(IdbSite, “maintable”, (id, subobj));
// TABLE4CLASS(IdbSite, “”, (sid, data));

#define TABLE4CLASS_COLNAME_INNER(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP, dbtype) \
BOOST_FUSION_ADAPT_STRUCT( myclass, BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP) ) \
namespace edadb{\
template<>\
struct CppTypeToDbType<myclass>{\
    static const DbTypes ret = dbtype;\
};\
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
    inline static const std::vector<std::string>& external_member_names(){\
        static const std::vector<std::string> names = {};\
        return names;\
    }\
    inline static TupType getVal(myclass * obj){\
        return TupType(GENERATE_ObjValPtr(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, myclass)));\
    }\
};\
}

#define TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP) \
TABLE4CLASS_COLNAME_INNER(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP, DbTypes::kComposite)

#define TABLE4CLASS(myclass, tablename, CLASS_ELEMS_TUP) \
TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)))

# define Table4ExternalClass(myclass, CLASS_ELEMS_TUP) \
namespace edadb{\
template<> struct TypeMetaData<myclass>{\
    inline static std::string const& class_name(){\
        static std::string const class_name = BOOST_STRINGIZE(myclass);\
        return class_name;\
    }\
};\
template<>\
struct CppTypeToDbType<myclass>{\
    static const DbTypes ret = DbTypes::kExternal;\
};\
}\
TABLE4CLASS(edadb::Shadow<myclass>, "" , CLASS_ELEMS_TUP);



#define TABLE4CLASSWVEC_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP, ARRAY_FIELD_TUP) \
TABLE4CLASS_COLNAME_INNER(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP, DbTypes::kCompositeVector); \
namespace edadb{\
    /*template<> struct IsWithVec<myclass> : boost::mpl::bool_<true> {};*/\
    template<> struct VecMetaData<myclass>{\
        using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))>;\
        inline static auto tuple_type_pair()->TupTypePairType const&{\
            static const TupTypePairType t{GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))};\
            return t;\
        }\
        using TupTypePairTypeNoPtr = boost::fusion::vector<GENERATE_TupTypePairNoPtr(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))>;\
        inline static auto tuple_type_pair_no_ptr()->TupTypePairTypeNoPtr const&{\
            static const TupTypePairTypeNoPtr t{GENERATE_TupTypePairObjNoPtr(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))};\
            return t;\
        }\
        using VecTupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))>;\
        using VecTupTypeNoPtr = boost::fusion::vector<GENERATE_TupTypeNoPtr(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))>;\
        inline static VecTupType getVecVal(myclass * obj){\
            return VecTupType(GENERATE_ObjValPtr(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass)));\
        }\
        inline static VecTupType getVecValNoPtr(myclass * obj){\
            return VecTupTypeNoPtr(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass)));\
        }\
        using VecAndValType = decltype(boost::fusion::zip(std::declval<TupTypePairTypeNoPtr>(),std::declval<VecTupTypeNoPtr>()));\
        inline static VecAndValType getVecAndVal(myclass *obj){\
            return boost::fusion::zip(tuple_type_pair_no_ptr(),getVecValNoPtr(obj));\
        }\
        inline static const std::vector<std::string>& vec_field_names(){\
            static const std::vector<std::string> names = {EXPAND_member_names(ARRAY_FIELD_TUP)};\
            return names;\
        }\
        static const int num = BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP);\
        enum VecId{BOOST_PP_TUPLE_REM_CTOR(ARRAY_FIELD_TUP)};\
    };\
}


#define TABLE4CLASSWVEC(myclass, tablename, CLASS_ELEMS_TUP,  ARRAY_FIELD_TUP) \
TABLE4CLASSWVEC_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)), ARRAY_FIELD_TUP) 

namespace edadb{


template<class T>
    class Singleton : public boost::noncopyable {
    public:
        static T &i() {
            static T _me;
            return _me;
        }
    };

// template <typename T>
// struct is_vector : std::false_type {};

// template <typename T, typename Alloc>
// struct is_vector<std::vector<T,Alloc>> : std::true_type {};

// template <typename T>
// inline constexpr bool is_vector_v = is_vector<T>::value;

// template<typename T>
// struct IsWithVec : boost::mpl::bool_<false> {
// };

template <typename T>
    class Shadow{
      public:
        virtual void fromShadow(T* obj);
        virtual void toShadow(T* obj);
    };

    template<typename T, typename = void>
        struct ConvertCPPTypeToSOCISupportType {
            using type = T;
        };

    template<typename T>
        struct ConvertCPPTypeToSOCISupportType<T*, void> {
            using type = T;
        };

    template<typename T>
        struct ConvertCPPTypeToSOCISupportType<T, std::enable_if_t<std::is_enum<T>::value>> {
            using type = int;
        };

    template<>
        struct ConvertCPPTypeToSOCISupportType<float, void> {
            using type = double;
        };

    template<>
        struct ConvertCPPTypeToSOCISupportType<std::uint64_t, void> {
            using type = unsigned long long;
        };

    /// @brief Works for all stuff where the default type conversion operator is overloaded.
    template<typename T>
    typename ConvertCPPTypeToSOCISupportType<T>::type convertToSOCISupportedType(T const& val) {
        const auto ret = static_cast<typename ConvertCPPTypeToSOCISupportType<T>::type>(val);
        return ret;
    }

    std::string convertToSOCISupportedType( char const* val) {
        const std::string ret = val;
        return ret;
    }

class DbBackend : public Singleton<DbBackend> {
    private:
        soci::session _sql_session;
    public:
        bool connect(std::string const &connect_str){
            const auto backend_factory = soci::sqlite3;
            _sql_session.open(backend_factory, connect_str);
            return true;
        }

        soci::session &session(){ // & return the original object
            return _sql_session;
        }
};

template< class T >
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template<typename T>
struct StripQualifiersAndMakePointer {
    using type = typename std::add_pointer<typename remove_cvref<T>::type>::type;
};

template<typename T>
struct TypeMetaData {
    using MT = boost::fusion::vector<void>;
    static std::string const& class_name();
    static boost::fusion::vector<boost::fusion::pair<void, std::string>> const& tuple_type_pair();
};

template<typename T>
struct VecMetaData;

template<typename T> 
struct ExternalReadWrite {
};

template<typename T, typename = void>
struct HasVecId : std::false_type {};

template<typename T>
struct HasVecId<T, std::void_t<typename VecMetaData<T>::VecId>> : std::true_type {};

template <typename T>
    std::string createTableStrSubObj(const std::string& prefix, std::vector<std::string>& temp_member_names);

template <typename T>
    std::string InsertStrSubObj(const std::string& prefix);

template <typename T>
    std::string SubObjVal(T* obj);

template <typename T>
void updateSubObjVal(T* obj, std::vector<std::string>& vec);

    

template<class T>
    struct SqlString {
    public:
        static std::vector<std::string> member_names;
    public:
        struct fill{
            std::string& str;  
            int cnt;
            fill(std::string& str_i) : str(str_i), cnt(0) {}  

            template<typename O>
            void operator()(O val) {
                cnt++;
                using ObjType = std::decay_t<decltype(*val)>; 
                if (cnt == 1){
                    if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                        std::cout<<"illegal\n";
                    }
                    else if constexpr(std::is_enum<ObjType>::value){
                        //枚举类型不能作为主键，不过也不会执行到这里来
                        // str += "\'" + std::to_string(static_cast<std::underlying_type_t<ObjType>>(*val)) + "\'";
                    }
                    else
                        str += /* *val */std::to_string(*val) ; 
                }
                else{
                    if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                        const edadb::DbTypes db_type = CppTypeToDbType<ObjType>::ret;
                        if constexpr (db_type == DbTypes::kComposite)
                            str += edadb::SubObjVal<ObjType>(val);
                        else if constexpr (db_type == DbTypes::kExternal){
                            // str += edadb::SubObjVal<Shadow<ObjType>>(val);
                            str += edadb::SubObjVal<ObjType>(val);
                            // edadb::DbOstream dos;
                            // if(edadb::ExternalReadWrite<ObjType>::write(val, &dos)){
                            //     const std::string& bin_data = dos.str();
                            //     std::string hex;
                            //     hex.reserve(bin_data.size() * 2 + 3); // x'...'
                            
                            //     hex += "x'";
                            //     for (uint8_t c : bin_data) {
                            //         hex += "0123456789ABCDEF"[c >> 4];
                            //         hex += "0123456789ABCDEF"[c & 0x0F];
                            //     }
                            //     hex += "'";
                            
                            //     str += ", " + hex;
                            // }
                            // else{
                            //     std::cout<<"ExternalReadWrite write failed\n";
                            // }
                        }
                    }
                    else if constexpr(std::is_enum<ObjType>::value){
                        str += ", " + std::to_string(static_cast<std::underlying_type_t<ObjType>>(*val)) ;
                    }
                    else
                        str += ", " + std::to_string(*val) ;
                }
            }

            void operator()(std::string* val) {
                cnt++;
                if (cnt == 1)
                    str += "\'" + *val + "\'";
                else
                    str += ", \'" + *val + "\'"; // string 不是kComposite 不能是subclass
            }
        };
        
        struct fillSO{
            std::string& str;  
            fillSO(std::string& str_i) : str(str_i)/*, cnt(0)*/ {}  

            template<typename O>
            void operator()(O val) {
                using ObjType = std::decay_t<decltype(*val)>; 
                
                    if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                        str += edadb::SubObjVal<ObjType>(val);
                    }
                    else
                        str += ", " + std::to_string(*val) ; 
            }

            void operator()(std::string* val) {
                    str += ", \'" + *val + "\'"; // string 不是kComposite 不能是subclass
            }
        };


        struct CTForeachHelper { //create table
        private:
            std::string& sql;
            int cnt;
        public:
        CTForeachHelper(std::string& sql)
        : sql(sql), cnt(0) {}

            template <typename O>
            void operator()(O const& x) 
            {
                cnt++;
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();

                if(cnt == 1) {
                    member_names.push_back(x.second);
                    sql += x.second + " " + type + " PRIMARY KEY";  // DbMap<subclass>
                }
                    
                else {
                    edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                    if (ret == DbTypes::kComposite) // 尽量减少字符串比较
                    {
                        std::vector<std::string> sub_member_names;
                        sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_", sub_member_names);
                        member_names.insert(member_names.end(), sub_member_names.begin(), sub_member_names.end());
                    }
                    /*---- for External Class Start---- */
                    else if(ret == DbTypes::kExternal){
                        // SqlString<T>::member_names.push_back(x.second);
                        // sql += ", " + x.second + " BLOB";
                        std::vector<std::string> sub_member_names;
                        sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_", sub_member_names);
                        member_names.insert(member_names.end(), sub_member_names.begin(), sub_member_names.end());
                    }
                    /*---- for External Class End---- */
                    else {
                        SqlString<T>::member_names.push_back(x.second);
                        sql += ", " + x.second + " " + type;
                    }
                }
                
            }
        };

        struct CTSOForeachHelper { // create table sub obj
        private:
            std::string& sql;
            const std::string& prefix;
            std::vector<std::string>& temp_member_names; // store the member names of the subclass
            int cnt;
        public:
            CTSOForeachHelper(std::string& sql,const std::string& prefix, std::vector<std::string>& temp_member_names) : sql(sql), prefix(prefix),temp_member_names(temp_member_names), cnt(0) {}

            template <typename O>
            void operator()(O const& x) 
            {
                cnt++;
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();
                if(cnt == 1) {
                    temp_member_names.push_back(prefix + x.second);
                    sql += prefix + x.second + " " + type;  // DbMap<subclass>
                }
                else {
                    
                    edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                    if (ret == DbTypes::kComposite || ret == DbTypes::kExternal)
                        sql += ", " + edadb::createTableStrSubObj<ObjType>(prefix + x.second + "_",temp_member_names);
                    /*---- for External Class Start---- */
                    // else if(ret == DbTypes::kExternal){
                    //     temp_member_names.push_back(prefix + x.second);
                    //     sql += ", " + prefix + x.second + " BLOB";
                    // }
                    /*---- for External Class End---- */
                    else{
                        temp_member_names.push_back(prefix + x.second);
                        sql += ", " + prefix + x.second + " " + type;
                    } 
                }
                
            }
        };

        struct InsertRowNames {
        private:
            std::string* str;
            int cnt;

        public:
            InsertRowNames(std::string* str_i) : str(str_i), cnt(0) {}  

            template <typename O>
            void operator()(O const& x) 
            {
                cnt++;
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();
                if(cnt == 1) 
                    *str += x.second; // primary key 不能是subclass
                else{
                    edadb::DbTypes tmp = CppTypeToDbType<ObjType>::ret;
                    if(tmp == DbTypes::kComposite || tmp == DbTypes::kExternal){
                        *str += " , " + edadb::InsertStrSubObj<ObjType>(x.second + "_");
                    }
                    else{
                        *str += " , " + x.second;
                    }
                }
            }
        };

        struct InsertRowNamesSO {
        private:
            std::string* str;  
            const std::string& prefix;
            int cnt;

        public:
            InsertRowNamesSO(std::string* str_i,const std::string& prefix) : str(str_i), prefix(prefix), cnt(0) {}  

            template <typename O>
            void operator()(O const& x) 
            {
                cnt++;
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();
                if(cnt == 1) 
                    *str += prefix + x.second; // primary key 不能是subclass
                else{
                    edadb::DbTypes tmp = CppTypeToDbType<ObjType>::ret;
                    if(tmp == DbTypes::kComposite || tmp == DbTypes::kExternal){
                        *str += " , " + edadb::InsertStrSubObj<ObjType>(prefix + x.second + "_");
                    }
                    else{
                        *str += " , " + prefix + x.second;
                    }
                }
            }
        };

        struct UpdateRow {
        private:
            std::vector<std::string>& vec;
            std::string part;

        public:
            UpdateRow(std::vector<std::string>& vec) : vec(vec) {}

            template <typename O>
            void operator()(O const& x)
            {
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                    
                const edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                if constexpr (ret == DbTypes::kComposite){
                    auto vecs = TypeMetaData<ObjType>::tuple_type_pair();
                    std::string prefix = x.second + "_";
                    auto p = new typename SqlString<T>::UpdateRowSO(vec,prefix);
                    boost::fusion::for_each(vecs, *p);
                }
                else if constexpr(ret == DbTypes::kExternal){
                    auto vecs = TypeMetaData<Shadow<ObjType>>::tuple_type_pair();
                    std::string prefix = x.second + "_";
                    auto p = new typename SqlString<T>::UpdateRowSO(vec,prefix);
                    boost::fusion::for_each(vecs, *p);
                }
                else{
                    part = "";
                    if (!vec.empty()) {
                        part = " ,";
                    }
                    part += x.second + " = ";
                    vec.push_back(part);
                }
                    
            }
        };

        struct UpdateRowSO {
        private:
            std::vector<std::string>& vec;
            std::string part;
            std::string prefix;

        public:
            UpdateRowSO(std::vector<std::string>& vec,std::string &prefix) : vec(vec),prefix(prefix) {}

            template <typename O>
            void operator()(O const& x)
            {
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;

                part = "";
                if (!vec.empty()) {
                    part = " ,";
                }
                const edadb::DbTypes dbType = CppTypeToDbType<ObjType>::ret;
                if constexpr (dbType == DbTypes::kComposite){
                    auto vecs = TypeMetaData<ObjType>::tuple_type_pair();
                    std::string tmp_prefix = prefix + x.second + "_";
                    auto p = new typename SqlString<T>::UpdateRowSO(vec,tmp_prefix);
                    boost::fusion::for_each(vecs, *p);
                }
                else if constexpr (dbType == DbTypes::kExternal){
                    auto vecs = TypeMetaData<Shadow<ObjType>>::tuple_type_pair();
                    std::string tmp_prefix = prefix + x.second + "_";
                    auto p = new typename SqlString<T>::UpdateRowSO(vec,tmp_prefix);
                    boost::fusion::for_each(vecs, *p);
                }
                else{
                    part += prefix + x.second + " = ";
                    vec.push_back(part);
                }
                    
            }
        };

    
        struct UpdateRowVal {
        private:
            std::vector<std::string>& vec;
            std::string part;
        public:
            UpdateRowVal(std::vector<std::string>& vec) : vec(vec) {}
        
            template <typename S>
            std::string add_quotation(S const& x)
            {
                return std::to_string(x);
            }

            std::string add_quotation(std::string const& x)
            {
                return "\'" + x + "\'";
            }

            template <typename O>
            void operator()(O x)
            {
                using ObjType = std::decay_t<decltype(*x)>;
                if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                    const edadb::DbTypes db_type = CppTypeToDbType<ObjType>::ret;
                    if constexpr (db_type == DbTypes::kComposite || db_type == DbTypes::kExternal)
                        edadb::updateSubObjVal<ObjType>(x,vec);
                    // else if constexpr (db_type == DbTypes::kExternal){
                        
                    // }
                }
                else if constexpr(std::is_enum<ObjType>::value){
                    vec.push_back(std::to_string(static_cast<std::underlying_type_t<ObjType>>(*x)));
                }
                else{
                    part = "";
                    part += std::to_string(*x);
                    vec.push_back(part);
                }
            }

            void operator()(std::string *x)
            {
                part = "";
                part += add_quotation(*x);
                vec.push_back(part);
            }
        };

        struct UpdateRowValSO {
        private:
            std::vector<std::string>& vec;
            std::string part;
        public:
            UpdateRowValSO(std::vector<std::string>& vec) : vec(vec) {}
        
            template <typename S>
            std::string add_quotaion(S const& x)
            {
                return std::to_string(x);
            }

            std::string add_quotaion(std::string const& x)
            {
                return "\'" + x + "\'";
            }

            template <typename O>
            void operator()(O* x)
            {
                using ObjType = std::decay_t<decltype(*x)>;
                part = "";
                if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                    edadb::updateSubObjVal<ObjType>(x,vec);
                }
                else{
                    part += std::to_string(*x);
                    vec.push_back(part);
                }
            }

            void operator()(std::string * x)
            {
                part = "";
                part += add_quotaion(*x);
                vec.push_back(part);
            }
        };

    public:

        static std::string createTableStr(std::string& fk_col, std::string& fk_type,std::string& table_name, std::string& parent_table_name) {
            static const auto vecs = TypeMetaData<T>::tuple_type_pair();
            std::string sql = "CREATE TABLE IF NOT EXISTS \"" + table_name + "\" (";
            SqlString<T>::CTForeachHelper helper(sql); //CTForeachHelper
            boost::fusion::for_each(vecs, helper);
            // std::cout<<"fk_col = "<<fk_col<<"fk_type = "<<fk_type<<"\n";
            if((fk_col != "") && (fk_type != "")){
                sql += "," + parent_table_name + "_" + fk_col + " " + fk_type;
                sql += ", FOREIGN KEY (" + parent_table_name + "_" + fk_col + ") REFERENCES " + parent_table_name + "(" + fk_col + ") ON DELETE CASCADE ON UPDATE CASCADE";
            }
            
            sql += ")";
            return sql;
        }

        std::string insertRowStr(T *obj,std::string fk_col,std::string comma_fk_col_val, std::string parent_table_name) {
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            std::string str = "INSERT INTO \"{}\" ("; // INSERT INTO
            boost::fusion::for_each(vecs, SqlString<T>::InsertRowNames(&str));
            if(fk_col != "")
                str += ", " + parent_table_name + "_" + fk_col;
            str += ") VALUES (";
            boost::fusion::for_each(vals, fill(str));
            str += comma_fk_col_val;
            str += ");";
            return str;
        }

        static std::string const& dropTableStr() {
            static const std::string sql = "DROP TABLE IF EXISTS \"{}\"";
            return sql;
        }

        template<typename O>
        std::string firstColumnVal(O val){
            return std::to_string(val);
        }
        
        std::string firstColumnVal(std::string val){
            std::string str = "\'" + val + "\'";
            return str;
        }
        
        std::string wherePKStr(T *obj){
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            auto &first_pair = boost::fusion::at_c<0>(vecs);
            std::string str = first_pair.second + 
            " = " + firstColumnVal(*(boost::fusion::at_c<0>(vals))) + ";";
            return str;
        }

        std::string deleteRowStr(T *obj){
            std::string str;
            str += "DELETE FROM \"{}\" WHERE " +wherePKStr(obj);
            return str;
        }

        


        std::string updateRowStr(T *obj) {
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            std::string str = "UPDATE \"{}\" SET ";
            std::vector<std::string>v1 = {},v2 = {};
            boost::fusion::for_each(vecs, SqlString<T>::UpdateRow(v1));
            boost::fusion::for_each(vals, SqlString<T>::UpdateRowVal(v2));
            for(unsigned long i = 0;i<v1.size();i++){
                str += v1[i] + v2[i];
            }
            auto &first_pair = boost::fusion::at_c<0>(vecs);
            str +=  " WHERE "+ first_pair.second + " = " + firstColumnVal(*(boost::fusion::at_c<0>(vals))) + ";"; //如果不是std::string则隐式转换
            return str;
        }

        static std::string const& selectRowStr() {
            static const std::string sql = "SELECT * FROM \"{}\" ";
            return sql;
        }

        static std::string const& selectRowStrPK(std::string pkVal) {
            static const auto vecs = TypeMetaData<T>::tuple_type_pair();
            auto &first_pair = boost::fusion::at_c<0>(vecs);
            static const std::string sql = "SELECT * FROM \"{}\" where "
            + first_pair.second + " = " + pkVal;
            return sql;
        }

    };


// 在类外定义静态成员
template<typename T>
    std::vector<std::string> SqlString<T>::member_names; 
    
template<typename T, typename Q>
    struct SqlStringT2T{
      public:
        static std::vector<std::string> member_names;
      public:
        struct CTForeachHelperT2T { //create table
            private:
                std::string& sql;
                int cnt;
            public:
                CTForeachHelperT2T(std::string& sql) : sql(sql), cnt(0) {}

                template <typename O>
                void operator()(O const& x) 
                {
                    cnt++;
                    using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                    std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();

                        edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                        if (ret == DbTypes::kComposite) // 尽量减少字符串比较
                        {
                            std::vector<std::string> sub_member_names;
                            sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_", sub_member_names);
                            member_names.insert(member_names.end(), sub_member_names.begin(), sub_member_names.end());
                        }
                        else {
                            SqlStringT2T<T,Q>::member_names.push_back(x.second);
                            sql += ", " + x.second + " " + type;
                        }
                }
            };
      public:
        static std::string const& createTableStr() { // create_table_str
            static const auto vecs_T = TypeMetaData<T>::tuple_type_pair();
            auto &first_pair_T = boost::fusion::at_c<0>(vecs_T);
            using FirstType_T = std::decay_t<decltype(boost::fusion::at_c<0>(typename TypeMetaData<T>::TupType()))>;
            std::string type_T = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<FirstType_T>::type>();
            static std::string sql;


            constexpr edadb::DbTypes db_type = CppTypeToDbType<Q>::ret;
            if constexpr (db_type == DbTypes::kComposite){
                static const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
                auto &first_pair_Q = boost::fusion::at_c<0>(vecs_Q);
                using FirstType_Q = std::decay_t<decltype(boost::fusion::at_c<0>(typename TypeMetaData<Q>::TupType()))>;
                if (sql.empty()) {
                    sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
                    sql += TypeMetaData<T>::class_name() + "_" + first_pair_T.second + " " + type_T;
                    
                    auto p = new typename SqlStringT2T<T,Q>::CTForeachHelperT2T(sql); //CTForeachHelperT2T
                    boost::fusion::for_each(vecs_Q, *p);
                    sql += ", PRIMARY KEY (" + TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ", " + first_pair_Q.second + ")"; 
                    sql += ", FOREIGN KEY (" + TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ") REFERENCES {}" + "(" + first_pair_T.second + ") ON DELETE CASCADE ON UPDATE CASCADE";
                    sql += ")";
                }
            }
            else if constexpr (db_type == DbTypes::kExternal) {
                static const auto vecs_Q = TypeMetaData<Shadow<Q>>::tuple_type_pair();
                auto &first_pair_Q = boost::fusion::at_c<0>(vecs_Q);
                using FirstType_Q = std::decay_t<decltype(boost::fusion::at_c<0>(typename TypeMetaData<Shadow<Q>>::TupType()))>;
            // }
                if (sql.empty()) {
                    sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
                    sql += TypeMetaData<T>::class_name() + "_" + first_pair_T.second + " " + type_T;
                    
                    auto p = new typename SqlStringT2T<T,Q>::CTForeachHelperT2T(sql); //CTForeachHelperT2T
                    boost::fusion::for_each(vecs_Q, *p);
                    sql += ", PRIMARY KEY (" + TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ", " + first_pair_Q.second + ")"; 
                    sql += ", FOREIGN KEY (" + TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ") REFERENCES {}" + "(" + first_pair_T.second + ") ON DELETE CASCADE ON UPDATE CASCADE";
                    sql += ")";
                }
            }
            return sql;
        }

        std::string insertRowStr(T *obj1, Q *obj2) {
            const auto vecs_T = TypeMetaData<T>::tuple_type_pair();
            const auto vals_T = TypeMetaData<T>::getVal(obj1);
            auto& first_pair_T = boost::fusion::at_c<0>(vecs_T);
            auto& first_val_T = boost::fusion::at_c<0>(vals_T);
            const edadb::DbTypes db_type = CppTypeToDbType<Q>::ret;

            // 分情况处理 Q 的逻辑
            if constexpr (db_type == DbTypes::kComposite){
                const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
                const auto vals_Q = TypeMetaData<Q>::getVal(obj2);
                std::string str = "INSERT INTO \"{}\" ("; // INSERT INTO
                str += TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ", ";
                boost::fusion::for_each(vecs_Q, typename SqlString<Q>::InsertRowNames(&str));
                str += ") VALUES (";
                str += "\'" + *first_val_T + "\', ";
                boost::fusion::for_each(vals_Q, typename SqlString<Q>::fill(str));
                str += ");";
                return str;
            }
            else if constexpr (db_type == DbTypes::kExternal){
                const auto vecs_Q = TypeMetaData<Shadow<Q>>::tuple_type_pair();
                Shadow<Q> *shadowObj = new Shadow<Q>;
                shadowObj->toShadow(obj2); 
                const auto vals_Q = TypeMetaData<Shadow<Q>>::getVal(shadowObj);
                std::string str = "INSERT INTO \"{}\" ("; // INSERT INTO
                str += TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ", ";
                boost::fusion::for_each(vecs_Q, typename SqlString<Shadow<Q>>::InsertRowNames(&str));
                str += ") VALUES (";
                str += "\'" + *first_val_T + "\', ";
                boost::fusion::for_each(vals_Q, typename SqlString<Shadow<Q>>::fill(str));
                str += ");";
                return str;
            }
        }

        template<typename O>
        std::string firstColumnVal(O val){
            return std::to_string(val);
        }
        
        std::string firstColumnVal(std::string val){
            std::string str = "\'" + val + "\'";
            return str;
        }

        std::string deleteRowStr(T *obj1, Q *obj2){
            const auto vecs_T = TypeMetaData<T>::tuple_type_pair();
            const auto vals_T = TypeMetaData<T>::getVal(obj1);
            auto& first_pair_T = boost::fusion::at_c<0>(vecs_T);

            const edadb::DbTypes db_type = CppTypeToDbType<Q>::ret;

            if constexpr (db_type == DbTypes::kComposite){
                const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
                const auto vals_Q = TypeMetaData<Q>::getVal(obj2);
                auto& first_pair_Q = boost::fusion::at_c<0>(vecs_Q);

                std::string where_condition = fmt::format(
                    "{} = {} AND {} = {}",
                    TypeMetaData<T>::class_name() + "_" + first_pair_T.second, 
                    firstColumnVal(*(boost::fusion::at_c<0>(vals_T))), 
                    first_pair_Q.second, 
                    firstColumnVal(*(boost::fusion::at_c<0>(vals_Q))) 
                );
                return "DELETE FROM \"{}\" WHERE " + where_condition + ";";
            }
            else if constexpr (db_type == DbTypes::kExternal){
                const auto vecs_Q = TypeMetaData<Shadow<Q>>::tuple_type_pair();  
                auto shadowObj = new Shadow<Q>;
                shadowObj->toShadow(obj2);
                const auto vals_Q = TypeMetaData<Shadow<Q>>::getVal(shadowObj);
                auto& first_pair_Q = boost::fusion::at_c<0>(vecs_Q);

                std::string where_condition = fmt::format(
                    "{} = {} AND {} = {}",
                    TypeMetaData<T>::class_name() + "_" + first_pair_T.second,
                    firstColumnVal(*boost::fusion::at_c<0>(vals_T)),
                    first_pair_Q.second,
                    firstColumnVal(*boost::fusion::at_c<0>(vals_Q))
                );
                return "DELETE FROM \"{}\" WHERE " + where_condition + ";";
            }
        }
        
        std::string updateRowStr(T *obj1, Q *obj2) {
            const auto vecs_T = TypeMetaData<T>::tuple_type_pair();
            const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
            const auto vals_T = TypeMetaData<T>::getVal(obj1);
            const auto vals_Q = TypeMetaData<Q>::getVal(obj2);
            auto& first_pair_T = boost::fusion::at_c<0>(vecs_T);
            auto& first_pair_Q = boost::fusion::at_c<0>(vecs_Q);  
            std::vector<std::string> v1 = {}, v2 = {};
            boost::fusion::for_each(vecs_Q, typename SqlString<Q>::UpdateRow(v1)); 
            boost::fusion::for_each(vals_Q, typename SqlString<Q>::UpdateRowVal(v2)); 
            std::string set_clause = "";
            std::string foreign_key_column = TypeMetaData<T>::class_name() + "_" + first_pair_T.second;

            v1[1] = v1[1].substr(1); // 生硬地去掉','
            for (auto i = 1; i < v1.size(); ++i) {
                set_clause += v1[i] + v2[i];
            }

            std::string where_condition = fmt::format(
                "{} = {} AND {} = {}",
                foreign_key_column,
                firstColumnVal(*(boost::fusion::at_c<0>(vals_T))), 
                first_pair_Q.second,
                firstColumnVal(*(boost::fusion::at_c<0>(vals_Q))) 
            );

            std::string str = "UPDATE \"{}\" SET ";
            str += set_clause + " WHERE " + where_condition + ";";
            return str;
        }

    };

template<typename T, typename Q>
    std::vector<std::string> SqlStringT2T<T,Q>::member_names; 

template <typename T>
    std::string createTableStrSubObj(const std::string& prefix, std::vector<std::string>& temp_member_names) {
        
        std::string sql;
        if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){
            const edadb::DbTypes dbType = CppTypeToDbType<T>::ret;
            if constexpr (dbType == DbTypes::kComposite){
                /*const */auto vecs = TypeMetaData<T>::tuple_type_pair();
                auto p = new typename SqlString<T>::CTSOForeachHelper(sql, prefix,temp_member_names);// CTSOForeachHelper
                boost::fusion::for_each(vecs, *p);
            }
            else if constexpr (dbType == DbTypes::kExternal){
                /*const */auto vecs = TypeMetaData<edadb::Shadow<T>>::tuple_type_pair();
                auto p = new typename SqlString<edadb::Shadow<T>>::CTSOForeachHelper(sql, prefix,temp_member_names);// CTSOForeachHelper
                boost::fusion::for_each(vecs, *p);
            }
        }
        
        return sql;
    }

template <typename T>
    std::string InsertStrSubObj(const std::string& prefix) {

        std::string str;
        if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){
            const edadb::DbTypes dbType = CppTypeToDbType<T>::ret;
            if constexpr (dbType == DbTypes::kComposite){
            /*const */auto vecs = TypeMetaData<T>::tuple_type_pair();
                auto p = new typename SqlString<T>::InsertRowNamesSO(&str, prefix);// CTSOForeachHelper
                boost::fusion::for_each(vecs, *p);
            }
            else if constexpr (dbType == DbTypes::kExternal){
                auto vecs = TypeMetaData<Shadow<T>>::tuple_type_pair();
                auto p = new typename SqlString<Shadow<T>>::InsertRowNamesSO(&str, prefix);
                boost::fusion::for_each(vecs, *p);
            }
        }
        
        return str;
    }

template <typename T>
    std::string SubObjVal(T* obj){
        std::string str;
        if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){
            const edadb::DbTypes dbType = CppTypeToDbType<T>::ret;
            if constexpr (dbType == DbTypes::kComposite){
                const auto vals = TypeMetaData<T>::getVal(obj);
                // const auto vecs = TypeMetaData<T>::tuple_type_pair();
                auto p = new typename SqlString<T>::fillSO(str);// CTSOForeachHelper
                boost::fusion::for_each(vals, *p);
                delete p;
            }
            else if constexpr (dbType == DbTypes::kExternal){
                Shadow<T> *shadowObj = new Shadow<T>;
                shadowObj->toShadow(obj); // 临时变量
                const auto vals = TypeMetaData<Shadow<T>>::getVal(shadowObj);
                auto p = new typename SqlString<Shadow<T>>::fillSO(str);// CTSOForeachHelper
                boost::fusion::for_each(vals, *p);
                delete p;
                delete shadowObj;
                // edadb::DbOstream dos;
                // if(edadb::ExternalReadWrite<T>::write(obj, &dos)){
                //     const std::string& bin_data = dos.str();
                //     std::string hex;
                //     hex.reserve(bin_data.size() * 2 + 3); // x'...'
                
                //     hex += "x'";
                //     for (uint8_t c : bin_data) {
                //         hex += "0123456789ABCDEF"[c >> 4];
                //         hex += "0123456789ABCDEF"[c & 0x0F];
                //     }
                //     hex += "'";
                
                //     str += ", " + hex;
                // }
            }
        }
        
        return str;
    }


template <typename T>
    void/*std::string*/ updateSubObjVal(T* obj, std::vector<std::string>& vec){

        if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){
            const edadb::DbTypes dbType = CppTypeToDbType<T>::ret;
            if constexpr (dbType == DbTypes::kComposite){
                const auto vals = TypeMetaData<T>::getVal(obj);
                auto p = new typename SqlString<T>::UpdateRowVal(vec);
                boost::fusion::for_each(vals, *p);
                delete p;
            }
            else if constexpr (dbType == DbTypes::kExternal){
                Shadow<T> *shadowObj = new Shadow<T>;
                shadowObj->toShadow(obj); 
                const auto vals = TypeMetaData<Shadow<T>>::getVal(shadowObj);
                auto p = new typename SqlString<Shadow<T>>::UpdateRowVal(vec);
                boost::fusion::for_each(vals, *p);
                delete p;
                delete shadowObj;
            }
        }
        
    }


template<typename T>
    struct IsComposite : boost::mpl::bool_<false> {
    };



template<typename T>
struct GetAllObjects {
    private:
        const soci::row& _row;
        const std::vector<std::string>& _member_names;
        int& _count;

    public:
        GetAllObjects(const soci::row& row, const std::vector<std::string>& member_names, int& count) :_row(row),_member_names(member_names), _count(count) {
        }
        template <typename O> 
        void operator()(O& x){
            #ifdef EDADB_DEBUG
                //std::cout<<"_member_names.size() = "<<_member_names.size()<<"\n";
            #endif
            const std::string& member_name = _member_names.at(_count);
            using ObjType = std::remove_pointer_t<std::decay_t<decltype(x)>>;
            if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                const edadb::DbTypes db_type = CppTypeToDbType<ObjType>::ret;
                if constexpr(db_type == DbTypes::kComposite){
                    auto vals = TypeMetaData<ObjType>::getVal(x);
                    GetAllObjects<ObjType> sub_loader(_row, _member_names, _count);
                    boost::fusion::for_each(vals,sub_loader);
                }
                else if constexpr(db_type == DbTypes::kExternal){
                    Shadow<ObjType> *shadowObj = new Shadow<ObjType>;
                    auto vals = TypeMetaData<Shadow<ObjType>>::getVal(shadowObj);
                    GetAllObjects<Shadow<ObjType>> sub_loader(_row, _member_names, _count);
                    boost::fusion::for_each(vals,sub_loader);
                    shadowObj->fromShadow(x);
                    delete shadowObj;


                //     soci::blob sql_blob = _row.move_as<soci::blob>(member_name); // get 不行 用move_as方法转移所有权 https://soci.sourceforge.net/doc/master/types/
                //     std::size_t blob_size = sql_blob.get_len();

                //     std::vector<char> buffer(blob_size);
                //     sql_blob.read(0, buffer.data(), blob_size); //std::size_t offset, void *buf, std::size_t toRead
                    
                //     edadb::DbIstream dis(buffer.data(), blob_size);
                //     ExternalReadWrite<ObjType>::read(x, &dis);
                //     _count++;
                }
            }
            else{
                if constexpr (std::is_enum<ObjType>::value) {
                    // 显式转换 int → 枚举
                    int raw_value = _row.get<int>(member_name);
                    *x = static_cast<ObjType>(raw_value);
                } 
                else {
                    // 基础类型直接赋值
                    using SOCISupportType = typename ConvertCPPTypeToSOCISupportType<ObjType>::type;
                    *x = _row.get<SOCISupportType>(member_name);
                }
                _count++;
            }
        }
    };

namespace VecTabHelper{
    template<typename T>
        struct createVecTable;
    template<typename T>
        struct insertVecTable;
    template<typename T>
        struct deleteVecTable;
    template<typename T>
        struct updateVecTable;
    template<typename T>
        struct selectVecTable;
}

void dbgPrint(){std::cout<<"Edadb Test 15 starting \n";}
// template<typename T>
    // class DbMapV; //声明一下 

template<typename T>
    class DbMap {
      public:
        //Members
        std::string table_name;
        std::string select_header;
        // DbMapV<T> dbmap_v;
        std::vector<void *> array_sub;
      public:
        static bool connectToDb(const std::string& db_connect_str); // to be deleted 禁用
        void setTableName(std::string tab_name){table_name = tab_name;}
        bool createTable(std::string parent_tab_name,std::string fk_col="", std::string fk_ref="");
        bool insertToDb(T *obj, bool self_only = true,std::string fk_col="", std::string comma_fk_col_val="",std::string parent_tab_name = "");
        bool deleteFromDb(T *obj);
        bool updateDb(T *obj);
        bool selectFromDb(std::vector<T> *vec,std::string fk_col = "",  std::string parent_tab_name = "", std::string where_str = "");//(std::vector<T> *vec, std::string where_str = "");
        template<typename Q>
        bool selectWithPK(Q PK_val, T *obj);

        // template<typename U = T> // 使用U=T,HasVecId<>检查会推迟到getSub函数被调用时
        // auto getSub(typename VecMetaData<U>::VecId id)
        //     -> std::enable_if_t<HasVecId<U>::value,void*>;

        void* getSub(int id);
            

        // bool createTable4Vectors();

    };

    template<typename T>
    bool DbMap<T>::connectToDb(const std::string& db_connect_str){
        auto ret = DbBackend::i().connect(db_connect_str);
        return ret;
    }
    // template<typename T>
    // template<typename U>
    // auto DbMap<T>::getSub(VecMetaData<U>::VecId id)
    //     -> std::enable_if_t<HasVecId<U>::value,void*>
    // {
    //     int i = (int)id;
    //     assert(0 <= i && i < VecMetaData<U>::num);
    //     return array_sub[i];
    // }
    template<typename T>
        void* DbMap<T>::getSub(int id)
    {
        if constexpr(CppTypeToDbType<T>::ret==DbTypes::kCompositeVector){
            int i = (int)id;
            assert(0 <= i && i < VecMetaData<T>::num); // array_sub.size()
            return array_sub[i];
        }
        else return NULL;
    }

    // template<std::enable_if<CppTypeToDbType<T>::ret==DbTypes::kCompositeVector, T>>
    // auto DbMap<T>::getSub(VecMetaData<T>::VecId id)
    //     -> std::enable_if_t<HasVecId<T>::value,void*>
    // {
    //     int i = (int)id;
    //     assert(0 <= i && i < VecMetaData<T>::num);
    //     return array_sub[i];
    // }

/*    template<typename Integer,
             std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
    T(Integer) : type(int_t) {}
    */
    template<typename T> 
    bool DbMap<T>::createTable(std::string parent_tab_name,std::string fk_col, std::string fk_type){
        // if constexpr(HasVecId<T>) array_sub.resize(VecMetaData<T>:: num);
        if(table_name == "")table_name = TypeMetaData<T>::table_name();//tab_name;
        std::string sql = SqlString<T>::createTableStr(fk_col,fk_type, table_name, parent_tab_name);// 加两个参数
        #ifdef EDADB_DEBUG
            std::cout<<"createTable sql: "<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            select_header = fmt::format(SqlString<T>::selectRowStr(),table_name);

            if constexpr (CppTypeToDbType<T>::ret == DbTypes::kCompositeVector){
                auto vecs = VecMetaData<T>::tuple_type_pair();
                auto &first_pair = boost::fusion::at_c<0>(TypeMetaData<T>::tuple_type_pair());
                using FirstType = typename std::decay_t<decltype(first_pair)>::first_type;
                std::string child_fk_col = first_pair.second;
                std::string child_fk_type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<FirstType>::type>();
                // std::cout<<"fk_col = "<<fk_col<<"fk_type = "<<fk_type<<"\n";

                for_each(vecs,VecTabHelper::createVecTable<T>(this,table_name, child_fk_col,child_fk_type));
                //应该有一个循环for_each(VecMetaData<T>::tuple_type_pair(),func{下面的事})
                //表名的前缀
                //pk foreign key 传入
                //递归调用DbMap<subT>::createTable,以上两行作为参数，或者名字拼出来
                //产生三个参数：1.前缀扩展的表名当前表的tablename+‘_’+vector成员变量名
                //2.当前table名字+'_'+主键名和类型作为子表的属性列
                //3.foreign key (2的名字) references 当前表名(当前表主键名)
            }
            return true;
        }
        catch (std::exception const & e) {
            std::cerr << "createTable: " << e.what() << std::endl;
            return false;
        }
    }

    template<typename T>
    bool DbMap<T>::insertToDb(T *obj, bool self_only,std::string fk_col, std::string comma_fk_col_val, std::string parent_tab_name){
        SqlString<T> sql_string;
        const std::string sql = fmt::format(sql_string.insertRowStr(obj,fk_col,comma_fk_col_val, parent_tab_name), table_name);
        #ifdef EDADB_DEBUG
            std::cout<<"insertToDb sql: "<<sql<<"\n";
        #endif
        try{
            DbBackend::i().session()<<sql; //boost 
            if(self_only)return true;
            //如果self_only不做下面的
            if constexpr (CppTypeToDbType<T>::ret == DbTypes::kCompositeVector){
                // std::cout<<"insertToDb if constexpr kCompositeVector\n";
                /*auto vecs = VecMetaData<T>::tuple_type_pair();
                auto vals = VecMetaData<T>::getVecVal(obj);
                auto zipped = boost::fusion::zip(vecs,vals);*/
                auto vecs_and_vals = VecMetaData<T>::getVecAndVal(obj);
                auto &first_pair = boost::fusion::at_c<0>(TypeMetaData<T>::tuple_type_pair());
                fk_col = first_pair.second;
                comma_fk_col_val = std::string(", ") + "'" + *boost::fusion::at_c<0>(TypeMetaData<T>::getVal(obj)) + "'";
                
                
                for_each(vecs_and_vals,VecTabHelper::insertVecTable<T>(this, fk_col, comma_fk_col_val, table_name));

                // for(int i = 0;i<VecMetaData<T>::num;i++){
                //     auto cur_vec = boost::fusion::at_c<i>(vecs);
                //     auto cur_val = boost::fusion::at_c<i>(vals);
                //     // using container_type =  typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;   
                //     // using elem_type = typename container_type::value_type;
                //     using elem_type = typename std::remove_const<typename std::remove_pointer<typename std::decay_t<decltype(cur_vec)>::first_type>::type>::type::value_type;
                //     DbMap<elem_type>* sub_dbmap = array_sub[i];
                //     for(auto obj : *cur_val){
                //         sub_dbmap->insertToDb(&obj,false, fk_col, comma_fk_col_val);
                //     }
                // }
                //应该有一个循环for_each(VecMetaData<T>::tuple_type_pair(),func{下面的事})
                //对于每一个sub,写一个循环依次处理vector中每一个元素 
                //当前表的pk传入，底层foreignkey列需要更新
                //可能外层的对象作为一个类型传进去
                //再调用sub的insertToDb
                //也可以实现一个insertVecToDb，insertToDb调用insertVecToDb，也可以不实现，自己决策
            }
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "insertToDb: " << e.what() << "\n";
            return false;
        }
    }

    // template<typename T,typename P>
    // bool DbMap<T>::insertVecToDb(T *obj, P * parent, bool self_only){
    //     //parent主键作为外键传进
    //     //也可能在createTable时候记下前缀，传

    // }

    
    template<typename T>
    bool DbMap<T>::deleteFromDb(T *obj){
        SqlString<T> sql_string;
        const std::string sql = fmt::format(sql_string.deleteRowStr(obj), table_name);
        #ifdef EDADB_DEBUG
            std::cout<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            std::cout<<"Delete it!\n";
            if constexpr (CppTypeToDbType<T>::ret == DbTypes::kCompositeVector){ // 
                auto vecs_and_vals = VecMetaData<T>::getVecAndVal(obj);
                auto &first_pair = boost::fusion::at_c<0>(TypeMetaData<T>::tuple_type_pair());
                
                
                for_each(vecs_and_vals,VecTabHelper::deleteVecTable<T>(this));
            }
            return true;
        }
        catch (std::exception const & e) {
            std::cerr << "deleteObj: " << e.what() << "\n";
            return false;
        }
    }

    template<typename T>
    bool DbMap<T>::updateDb(T *obj){
        SqlString<T> sql_string;
        const std::string sql = fmt::format(sql_string.updateRowStr(obj),table_name);
        #ifdef EDADB_DEBUG
            std::cout<<"updateDb sql = "<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            if constexpr (CppTypeToDbType<T>::ret == DbTypes::kCompositeVector){ // 
                auto vecs_and_vals = VecMetaData<T>::getVecAndVal(obj);
                auto &first_pair = boost::fusion::at_c<0>(TypeMetaData<T>::tuple_type_pair());
                for_each(vecs_and_vals,VecTabHelper::updateVecTable<T>(this));
            }
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "updateToDb: "<< e.what() << "\n";
            return false;
        }
    }

    template<typename T>
    bool DbMap<T>::selectFromDb(std::vector<T> *vec,std::string fk_col,  std::string parent_tab_name, std::string where_str){
        std::string sql;
        if(where_str.size() > 0) 
            sql = select_header + "where "+ where_str; 
        else 
            sql = select_header;
        try {
            #ifdef EDADB_DEBUG
            std::cout<<"selectFromDb sql = "<<sql<<std::endl;
            #endif
            soci::rowset<soci::row>rows = (DbBackend::i().session().prepare << sql);
            
            vec->clear();
            for(soci::rowset<soci::row>::const_iterator row_itr = rows.begin();row_itr != rows.end();++row_itr){
                auto const& row = *row_itr;
                T *a = new T; // 要求提供默认构造
                auto vals = TypeMetaData<T>::getVal(a);
                int count = 0;
                std::vector<std::string> member_names_with_vec = SqlString<T>::member_names;
                if constexpr (CppTypeToDbType<T>::ret == DbTypes::kCompositeVector){
                    if(fk_col != "")
                        member_names_with_vec.push_back(parent_tab_name + "_" +fk_col);
                }
                boost::fusion::for_each(vals,GetAllObjects<T>(row, member_names_with_vec, count));
                if constexpr (CppTypeToDbType<T>::ret == DbTypes::kCompositeVector){ // 
                    auto vecs_and_vals = VecMetaData<T>::getVecAndVal(a);
                    auto &first_pair = boost::fusion::at_c<0>(TypeMetaData<T>::tuple_type_pair());
                    std::string child_fk_col = first_pair.second;
                    std::string child_where_str = table_name + "_" + child_fk_col + " = " + "'" + *boost::fusion::at_c<0>(TypeMetaData<T>::getVal(a)) + "'" ;
                    for_each(vecs_and_vals,VecTabHelper::selectVecTable<T>(this, child_fk_col, table_name, child_where_str));
                }
                vec->emplace_back(std::move(*a));
            }
            
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "selectFromDb: "<< e.what() << "\n";
            return false;
        }
    }

    template<typename T>
    template<typename Q>
    bool DbMap<T>::selectWithPK(Q PK_val, T *obj){
        static const std::string sql = fmt::format(SqlString<T>::selectRowStrPK(std::to_string(PK_val)),table_name);
        try {
            std::cout<<sql<<std::endl;
            soci::rowset<soci::row>rows = (DbBackend::i().session().prepare << sql);
            auto const& row = *rows.begin();
            auto vals = TypeMetaData<T>::getVal(obj);
            boost::fusion::for_each(vals, GetAllObjects<T>(row, SqlString<T>::member_names));
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "selectFromDbPK: "<< e.what() << "\n";
            return false;
        }
    }


    template<typename T, typename Q>
    class DbMapT2T {
      public:
        //Members
        std::string table_name;
        std::string select_header;
        
      public:
        static bool connectToDb(const std::string& db_connect_str); // 禁用
        void setTableName(std::string tab_name){table_name = tab_name;}
        bool createTable(std::string tab_name, std::string father_tab_name);
        bool insertToDb(T *obj1, Q *obj2);
        bool insertToDb(void* obj1, void* obj2){
            return insertToDb(static_cast<T*>(obj1), static_cast<Q*>(obj2));
        }
        
        
        bool deleteFromDb(T *obj1, Q *obj2);
        bool updateDb(T *obj1, Q *obj2);
        bool updateDb(void* obj1, void* obj2){
            return updateDb(static_cast<T*>(obj1), static_cast<Q*>(obj2));
        }
        bool selectFromDb(std::vector<Q>* vec, std::string where_str = "");
        bool selectFromDb(std::vector<void*>* vec, std::string where_str = "") {
            // return selectFromDb(static_cast<std::vector<Q>*>(vec), where_str);
            std::vector<Q> q_vec;
            if (!selectFromDb(&q_vec, where_str)) {
                return false;
            }
            vec->clear();
            for (auto& item : q_vec) {
                vec->push_back(static_cast<void*>(new Q(item))); // 每个都转
            }
            return true;
        }
    };

    template<typename T, typename Q>
    bool DbMapT2T<T,Q>::connectToDb(const std::string& db_connect_str){
        auto ret = DbBackend::i().connect(db_connect_str);
        return ret;
    }

    template<typename T, typename Q>
    bool DbMapT2T<T,Q>::createTable(std::string tab_name, std::string father_tab_name){ // 这里tab_name不许空
        if(tab_name == ""){ // 这里名字不允许修改只能是这个，
            // std::cout<<TypeMetaData<T>::class_name()<<" "<<TypeMetaData<T>::class_name()<<"\n";
            tab_name = fmt::format("{}_{}",TypeMetaData<T>::class_name(),TypeMetaData<Q>::class_name());  
        }
        table_name = tab_name;
        // std::cout<<"tablename = "<<table_name<<"\n";
        const static std::string sql = fmt::format(SqlStringT2T<T,Q>::createTableStr(), table_name, father_tab_name);
        #ifdef EDADB_DEBUG
            std::cout<<"createTable sql: "<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            // select_header = fmt::format(SqlString<T>::selectRowStr(),table_name);
            /*if constexpr(IsWithVec<Q>::value){
                Q * q = new Q;
                auto vec_vals = VecMetaData<Q>::getVecVal(q);
                boost::fusion::for_each(vec_vals,VecTabHelper::createVecTable<Q>(q,table_name));
            }*/
            return true;
        }
        catch (std::exception const & e) {
            std::cerr << "createTable: " << e.what() << std::endl;
            return false;
        }
    }

    template<typename T, typename Q>
    bool DbMapT2T<T,Q>::insertToDb(T *obj1, Q *obj2){
        SqlStringT2T<T,Q> sql_string;
        const std::string sql = fmt::format(sql_string.insertRowStr(obj1,obj2), table_name);
        #ifdef EDADB_DEBUG
            std::cout<<"insertToDb sql: "<<sql<<"\n";
        #endif
        try{
            DbBackend::i().session()<<sql;
            /*if constexpr(IsWithVec<Q>::value){
                auto vec_vals = VecMetaData<Q>::getVecVal(obj2);
                boost::fusion::for_each(vec_vals,VecTabHelper::insertVecTable<Q>(obj2));
            }*/
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "insertToDb: " << e.what() << "\n";
            return false;
        }
    }

    template<typename T, typename Q>
    bool DbMapT2T<T,Q>::deleteFromDb(T *obj1, Q *obj2){
        SqlStringT2T<T,Q> sql_string;
        const std::string sql = fmt::format(sql_string.deleteRowStr(obj1,obj2), table_name);
        #ifdef EDADB_DEBUG
            std::cout<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            std::cout<<"Delete it!\n";
            return true;
        }
        catch (std::exception const & e) {
            std::cerr << "deleteObj: " << e.what() << "\n";
            return false;
        }
    }

    template<typename T, typename Q>
    bool DbMapT2T<T,Q>::updateDb(T *obj1, Q *obj2){
        SqlStringT2T<T,Q> sql_string;
        const std::string sql = fmt::format(sql_string.updateRowStr(obj1,obj2),table_name);
        #ifdef EDADB_DEBUG
            std::cout<<"updateDb sql = "<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "updateToDb: "<< e.what() << "\n";
            return false;
        }
    }

    template<typename T, typename Q>
    bool DbMapT2T<T, Q>::selectFromDb(std::vector<Q>* vec, std::string where_str) {
        std::string sql = fmt::format(
            "SELECT * FROM \"{}\"", table_name
        );
        if (!where_str.empty())
            sql += " WHERE " + where_str;
        try {
            #ifdef EDADB_DEBUG
                std::cout << "selectFromDb sql = " << sql << std::endl;
            #endif
            soci::rowset<soci::row> rows = (DbBackend::i().session().prepare << sql);

            vec->clear();
            for (soci::rowset<soci::row>::const_iterator row_itr = rows.begin(); row_itr != rows.end(); ++row_itr) {
                auto const& row = *row_itr;
                const edadb::DbTypes db_type = CppTypeToDbType<Q>::ret;
                if constexpr (db_type == DbTypes::kComposite){
                    Q* b = new Q;
                    auto member_names_Q = SqlString<Q>::member_names;
                    auto vals_Q = TypeMetaData<Q>::getVal(b);
                    int count = 0;
                    boost::fusion::for_each(vals_Q, GetAllObjects<Q>(row, member_names_Q, count));
                    vec->emplace_back(std::move(*b));
                    delete b;
                }
                else if constexpr (db_type == DbTypes::kExternal){
                    Shadow<Q> *shadowObj = new Shadow<Q>;
                    auto member_names_Q = SqlString<Shadow<Q>>::member_names;
                    auto vals_Q = TypeMetaData<Shadow<Q>>::getVal(shadowObj);
                    int count = 0;
                    boost::fusion::for_each(vals_Q, GetAllObjects<Q>(row, member_names_Q, count));
                    Q * b = new Q;
                    shadowObj->fromShadow(b);
                    vec->emplace_back(std::move(*b));
                    delete b;
                    delete shadowObj;
                }
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "selectFromDb: " << e.what() << "\n";
            return false;
        }
    }

namespace VecTabHelper{
    template<typename T>
    struct createVecTable{
      private:
        DbMap<T>* parent_map;
        std::string table_name;
        int cnt;
        std::string fk_col;
        std::string fk_type;
        std::vector<std::string> vec_field_names;
      public:
        createVecTable(DbMap<T>* parent,const std::string &tab_name, std::string fk_col, std::string fk_type): parent_map(parent), table_name(tab_name),cnt(0), fk_col(fk_col), fk_type(fk_type) {vec_field_names = VecMetaData<T>::vec_field_names();}
        template <typename O>
            void operator()(O const& x) 
            {
                using container_type =  typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                using elem_type = typename container_type::value_type;
                DbMap<elem_type>* sub_dbmap = new DbMap<elem_type>;
                sub_dbmap->setTableName(table_name+"_"+vec_field_names[cnt++]);
                std::cout<<"sub_dbmap->table_name = "<<sub_dbmap->table_name<<"\n";

                sub_dbmap->createTable(table_name, fk_col,fk_type);
                parent_map->array_sub.push_back(static_cast<void*>(sub_dbmap));
            }
    };

    template<typename T>
    struct insertVecTable{
      private:
        DbMap<T>* parent_map;
        std::string table_name;
        int cnt;
        std::string fk_col;
        std::string comma_fk_col_val;
      public:
        insertVecTable(DbMap<T>* parent,std::string fk_col,std::string comma_fk_col_val, std::string tab_name): parent_map(parent), table_name(tab_name), fk_col(fk_col), comma_fk_col_val(comma_fk_col_val), cnt(0) {}
        template <typename O>
            void operator()(O const& x) 
            {
                auto& pair_vec = boost::fusion::at_c<0>(x);
                auto& pair_val = boost::fusion::at_c<1>(x);

                using container_ptr_type = typename std::decay_t<decltype(pair_vec)>::first_type;

                using container_type = typename std::remove_pointer<container_ptr_type>::type;
                using elem_type = typename container_type::value_type;

                DbMap<elem_type>* sub_dbmap = static_cast<DbMap<elem_type>*>(parent_map->array_sub[cnt++]);
                for(auto& elem : *pair_val){
                    sub_dbmap->insertToDb(&elem, false, fk_col, comma_fk_col_val,table_name);
                }
                // sub_dbmap->insertToDb(elem_type*,false, fk_col, comma_fk_col_val);


                //T *obj, bool self_only,std::string fk_col, std::string comma_fk_col_val

                // using real_type = typename std::remove_pointer<O>::type;
                // using elem_type = typename real_type::value_type;
                // DbMapT2T<T,elem_type> temp_dbmap;
                // temp_dbmap.setTableName(TypeMetaData<T>::class_name()+"_"+TypeMetaData<elem_type>::class_name());
                // for(auto i : *x) // 解引用指针，遍历
                // temp_dbmap.insertToDb(obj,&i);
            }
    };

    template<typename T>
    struct deleteVecTable{
      private:
        DbMap<T>* parent_map;
        std::string table_name;
        int cnt;
      public:
        deleteVecTable(DbMap<T>* parent): parent_map(parent), cnt(0) {}
        template <typename O>
            void operator()(O const& x) 
            {
                auto& pair_vec = boost::fusion::at_c<0>(x);
                auto& pair_val = boost::fusion::at_c<1>(x);

                using container_ptr_type = typename std::decay_t<decltype(pair_vec)>::first_type;

                using container_type = typename std::remove_pointer<container_ptr_type>::type;
                using elem_type = typename container_type::value_type;

                DbMap<elem_type>* sub_dbmap = static_cast<DbMap<elem_type>*>(parent_map->array_sub[cnt++]);
                for(auto& elem : *pair_val){
                    sub_dbmap->deleteFromDb(&elem);
                }

            }
    };

    template<typename T>
    struct updateVecTable{
      private:
        DbMap<T>* parent_map;
        std::string table_name;
        int cnt;
      public:
        updateVecTable(DbMap<T>* parent): parent_map(parent), cnt(0) {}
        template <typename O>
            void operator()(O const& x) 
            {
                auto& pair_vec = boost::fusion::at_c<0>(x);
                auto& pair_val = boost::fusion::at_c<1>(x);

                using container_ptr_type = typename std::decay_t<decltype(pair_vec)>::first_type;

                using container_type = typename std::remove_pointer<container_ptr_type>::type;
                using elem_type = typename container_type::value_type;

                DbMap<elem_type>* sub_dbmap = static_cast<DbMap<elem_type>*>(parent_map->array_sub[cnt++]);
                for(auto& elem : *pair_val){
                    sub_dbmap->updateDb(&elem);
                }

            }
    };

    template<typename T>
    struct selectVecTable{
      private:
        DbMap<T>* parent_map;
        std::string table_name;
        std::string fk_col;
        std::string where_str;
        int cnt;
      public:
        selectVecTable(DbMap<T>* parent,std::string fk_col,std::string tab_name, std::string child_where_str): parent_map(parent), table_name(tab_name), fk_col(fk_col), where_str(child_where_str), cnt(0) {}
        
        template <typename O>
            void operator()(O const& x) 
            {
                auto& pair_vec = boost::fusion::at_c<0>(x);
                auto& pair_val = boost::fusion::at_c<1>(x);

                using container_ptr_type = typename std::decay_t<decltype(pair_vec)>::first_type;

                using container_type = typename std::remove_pointer<container_ptr_type>::type;
                using elem_type = typename container_type::value_type;

                DbMap<elem_type>* sub_dbmap = static_cast<DbMap<elem_type>*>(parent_map->array_sub[cnt++]);
                // std::string where_str = SqlString<elem_type>::wherePKStr(&elem);
                sub_dbmap->selectFromDb(pair_val, fk_col, table_name, where_str); // select * from where_str
                
            }
    };
}


} // end of edadb namespace



