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
* ss 和 string 的速度
*/

namespace edadb{



template<class T>
    class Singleton : public boost::noncopyable {
    public:
        static T &i() {
            static T _me;
            return _me;
        }
    };

    template<typename T>
        struct ConvertCPPTypeToSOCISupportType {
            using type = T;
        };

    template<typename T>
        struct ConvertCPPTypeToSOCISupportType<T*> {
            using type = T;
        };

    template<>
        struct ConvertCPPTypeToSOCISupportType<float> {
            using type = double;
        };

    template<>
        struct ConvertCPPTypeToSOCISupportType<std::uint64_t> {
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


template <typename T>
    std::string createTableStrSubObj(const std::string& prefix);

template <typename T>
    std::string InsertStrSubObj(const std::string& prefix);

template <typename T>
    std::string SubObjVal(T* obj);

template <typename T>
void updateSubObjVal(T* obj, std::vector<std::string>& vec);

    

template<class T>
    struct SqlString {
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
                    else
                        str += ", \'" + /* *val */std::to_string(*val) + "\'"; 
                }
                else{
                    // if(type == "__COMPOSITE__"){
                    if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                        // ss << ", \'" << *val << "\'";
                        str += edadb::SubObjVal<ObjType>(val);
                    }
                    else
                        str += ", \'" + /* *val */std::to_string(*val) + "\'"; 
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
            // int cnt;
            fillSO(std::string& str_i) : str(str_i)/*, cnt(0)*/ {}  

            template<typename O>
            void operator()(O val) {
                // cnt++;
                using ObjType = std::decay_t<decltype(*val)>; 
                
                    if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                        // ss << ", \'" << *val << "\'";
                        str += edadb::SubObjVal<ObjType>(val);
                    }
                    else
                        str += ", \'" + /* *val */std::to_string(*val) + "\'"; 
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
            CTForeachHelper(std::string& sql) : sql(sql), cnt(0) {}

            template <typename O>
            void operator()(O const& x) 
            {
                cnt++;
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();

                if(cnt == 1) 
                    sql += x.second + " " + type + " PRIMARY KEY";  // DbMap<subclass>
                else {
                    edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                    #if 0 // EDADB_DEBUG
                    std::cout << "ret ?= kUnknown " << (ret == DbTypes::kUnknown) << "\n";
                    std::cout << "ret ?= kInteger " << (ret == DbTypes::kInteger) << "\n";
                    std::cout << "ret ?= kReal " << (ret == DbTypes::kReal) << "\n";
                    std::cout << "ret ?= kNumeric " << (ret == DbTypes::kNumeric) << "\n";
                    std::cout << "ret ?= kText " << (ret == DbTypes::kText) << "\n";
                    std::cout << "ret ?= kComposite " << (ret == DbTypes::kComposite) << "\n";
                    std::cout<<"DbTypes::kComposite = "<<typeid(DbTypes::kComposite).name()<<"\n";
                    #endif
                    if (ret == DbTypes::kComposite) // 尽量减少字符串比较
                    // if (type == "__COMPOSITE__") // CppTypeToDbType<O>::ret 不能直接判等
                    {
                        sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_");
                    }
                    else 
                        sql += ", " + x.second + " " + type;
                }
                
            }
        };

        struct CTSOForeachHelper { // create table sub obj
        private:
            std::string& sql;
            const std::string& prefix;
            int cnt;
        public:
            CTSOForeachHelper(std::string& sql,const std::string& prefix) : sql(sql), prefix(prefix), cnt(0) {}

            template <typename O>
            void operator()(O const& x) 
            {
                cnt++;
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();
                if(cnt == 1) 
                    sql += prefix + x.second + " " + type;  // DbMap<subclass>
                else {
                    
                    edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                    if (ret == DbTypes::kComposite)
                        sql += ", " + edadb::createTableStrSubObj<ObjType>(prefix + x.second + "_");
                    else 
                        sql += ", " + prefix + x.second + " " + type;
                }
                
            }
        };

        struct InsertRowNames {
        private:
            // std::stringstream* ss;  
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
                    if(tmp == DbTypes::kComposite){
                    // if(type == "__COMPOSITE__"){
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
                    if(tmp == DbTypes::kComposite){
                    // if(type == "__COMPOSITE__"){
                        *str += " , " + edadb::InsertStrSubObj<ObjType>(prefix + x.second + "_");
                    }
                    else{
                        *str += " , " + prefix + x.second;
                    }
                }
            }
        };

        // struct InsertRowVal {
        // private:
        //     std::string& sql;
        //     int cnt;

        // public:
        //     InsertRowVal(std::string& sql) : sql(sql),cnt(0) {}

        //     template <typename O>
        //     void operator()(O const& x) 
        //     {
        //         cnt++;
        //         if(cnt == 1) 
        //             sql += ":" + x.second; // primary key 不能是subclass
        //         else{
        //             if(CppTypeToDbType<O>::ret == DbTypes::kComposite)
        //                 sql += " , :" + createTableStrSubObj(x.second + "_");
        //             else
        //                 sql += " , :" + x.second;
        //         }
        //     }
        // };

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
                    
                edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                if(ret == DbTypes::kComposite){
                    // /*part += */edadb::UpdateValStrSubObj<ObjType>(x,vec,x.second + "_")/*x.second + "_")*//* + " = "*/;
                    if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                        auto vecs = TypeMetaData<ObjType>::tuple_type_pair();
                        std::string prefix = x.second + "_";
                        auto p = new typename SqlString<T>::UpdateRowSO(vec,prefix);
                        boost::fusion::for_each(vecs, *p);
                    }
                    
                }
                else{
                    part = "";
                    if (!vec.empty()) {
                        part = ",";
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

                part = ",";
                // if (!vec.empty()) {
                //     part = ",";
                // }
                edadb::DbTypes ret = CppTypeToDbType<ObjType>::ret;
                if(ret == DbTypes::kComposite){
                    // /*part += */edadb::UpdateValStrSubObj<ObjType>(x,vec,x.second + "_")/*x.second + "_")*//* + " = "*/;
                    if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                        auto vecs = TypeMetaData<ObjType>::tuple_type_pair();
                        prefix += x.second + "_";
                        auto p = new typename SqlString<T>::UpdateRowSO(vec,prefix);
                        boost::fusion::for_each(vecs, *p);
                        delete p;
                    }
                }
                else{
                    part += prefix + x.second + " = ";
                    vec.push_back(part);
                }
                    
            }
        };
    
        // struct UpdateRowVal {
        // private:
        //     std::vector<std::string>& vec;
        //     std::string part;
        // public:
        //     UpdateRowVal(std::vector<std::string>& vec) : vec(vec) {}
        
        //     template <typename S>
        //     std::string add_quotaion(S const& x)
        //     {
        //         return std::to_string(x);
        //         else
        //             part += x.second + " = ";
        //         vec.push_back(part);
        //     }
        // };

    
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
                // else{
                //     part += std::to_string(*x);
                //     vec.push_back(part);
                // }
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
                    edadb::updateSubObjVal<ObjType>(x,vec);
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

        static std::string const& createTableStr() { // create_table_str
            static const auto vecs = TypeMetaData<T>::tuple_type_pair();
            static std::string sql;
            if (sql.empty()) {
                // sql = "CREATE TABLE IF NOT EXISTS \"{}\" (oid VARCHAR PRIMARY KEY, oid_ref VARCHAR, parent_table_reference VARCHAR, parent_column_name VARCHAR";
                sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
                auto p = new SqlString<T>::CTForeachHelper(sql); //CTForeachHelper
                boost::fusion::for_each(vecs, *p);
                sql += ")";
            }
            return sql;
        }

        std::string insertRowStr(T *obj) {
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            std::string str = "INSERT INTO \"{}\" (";
            /*std::stringstream ss;
            ss << "INSERT INTO \"{}\" (";*/
            boost::fusion::for_each(vecs, SqlString<T>::InsertRowNames(&str));
            str += ") VALUES (";
            boost::fusion::for_each(vals, fill(str));
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
        
        std::string deleteRowStr(T *obj){
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            auto &first_pair = boost::fusion::at_c<0>(vecs);
            // std::stringstream ss;
            std::string str;
            str += "DELETE FROM \"{}\" WHERE " +first_pair.second + 
            " = " + firstColumnVal(*(boost::fusion::at_c<0>(vals))) + ";";
            return str;
        }


        std::string updateRowStr(T *obj) {
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            // std::stringstream ss;
            std::string str = "UPDATE \"{}\" SET ";
            std::vector<std::string>v1,v2;
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



template <typename T>
    std::string createTableStrSubObj(const std::string& prefix) {
        
        std::string sql;
        if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){

            /*const */auto vecs = TypeMetaData<T>::tuple_type_pair();

                auto p = new typename SqlString<T>::CTSOForeachHelper(sql, prefix);// CTSOForeachHelper
                boost::fusion::for_each(vecs, *p);

        }
        
        return sql;
    }

template <typename T>
    std::string InsertStrSubObj(const std::string& prefix) {

        // std::stringstream ss;
        std::string str;
        if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){
            /*const */auto vecs = TypeMetaData<T>::tuple_type_pair();
                auto p = new typename SqlString<T>::InsertRowNamesSO(&str, prefix);// CTSOForeachHelper
                boost::fusion::for_each(vecs, *p);
        }
        
        return str;
    }

template <typename T>
    std::string SubObjVal(T* obj){
        // std::stringstream ss;
        std::string str;
        if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){

            const auto vals = TypeMetaData<T>::getVal(obj);
            // const auto vecs = TypeMetaData<T>::tuple_type_pair();
            auto p = new typename SqlString<T>::fillSO(str);// CTSOForeachHelper
            boost::fusion::for_each(vals, *p);
            delete p;
        }
        
        return str;
    }

// template <typename T>
//     void UpdateValStrSubObj(T* obj, std::vector<std::string>& vec, std::string prefix) {
//         const auto vecs = TypeMetaData<T>::tuple_type_pair();
//         auto p = new typename SqlString<T>::UpdateRowSO(vec,prefix);
//         boost::fusion::for_each(vecs, *p);
//         delete p;
//     }

template <typename T>
    void/*std::string*/ updateSubObjVal(T* obj, std::vector<std::string>& vec){

        // if constexpr (std::is_class<T>::value && (!std::is_same<T, std::string>::value)){

            const auto vals = TypeMetaData<T>::getVal(obj);

            auto p = new typename SqlString<T>::UpdateRowVal(vec);
            boost::fusion::for_each(vals, *p);
            delete p;
        // }
        
    }

struct SimpleOID {
    /// @typedef SelfType=SimpleOID
    using SelfType = SimpleOID;
    /// @typedef OidByteArrayType=std::array<std::uint8_t, 16>
    /// @brief To hold the 16 byte array representation of the OID.
    using OidByteArrayType = std::array<std::uint8_t, 16>;
    /// @typedef TagType = boost::uuids::uuid
    /// @brief Type to hold oid value
    using TagType = boost::uuids::uuid;
    /// @var TagType tag
    /// @brief Holds the UUID generated.
    TagType tag;

     SimpleOID() noexcept : tag() {}

    ~SimpleOID() = default;

    /// @fn SimpleOID
    /// @brief Pass other oid to populate this.
    /// @param other Passed in oid
    SimpleOID(const SimpleOID& other) :
            tag(other.tag) {
    }

    SimpleOID(const std::string& other) {
        boost::uuids::string_generator gen;
        tag = gen(other);
    }

    /// @fn populateAll
    /// @brief Gets a new UUID and populates that.
    ///        Uses boost::uuid library for the task
    void populate() {
        tag = boost::uuids::random_generator()();
    }

    /// @fn clear
    /// @brief clears the high and low values.
    void clear() {
        for (boost::uuids::uuid::iterator it = tag.begin(); it != tag.end(); ++it) {
            *it = 0;
        }
    }

    /// @fn operator=
    /// @brief Copies the value of high and low from the RHS. Deep copy
    SelfType& operator=(SelfType const &in_other) {
        tag = in_other.tag;
        return *this;
    }

    /// @fn operator=
    /// @brief Copies the value of high and low from the RHS. Deep copy
    SelfType& operator=(std::string const &in_other) {
        boost::uuids::string_generator gen;
        tag = gen(in_other);
        return *this;
    }

    /// @fn operator==
    /// @brief Compares high and low of the LHS and RHS.
    ///        Returns true when both low and high are same.
    /// @return true when both low and high are same else false
    bool operator==(SelfType const &in_other) const {
        return tag == in_other.tag;
    }

    /// @fn operator!=
    /// @brief Returns true if either or both of low and high
    ///        does not match. Internally calls operator==
    /// @return false only when both high and low matches else true
    bool operator!=(SelfType const &in_other) const {
        return !operator==(in_other);
    }

    /// @fn toByteArray
    /// @brief Convert the OID to byte array and then return.
    ///        The byte representation is not stored, so every time
    ///        this function is called this will generate again.
    /// @return OidByteArrayType 16 Byte representation of the OID
    OidByteArrayType toByteArray() const {
        OidByteArrayType ret{0};
        std::copy(tag.begin(), tag.end(), ret.begin());
        return ret;
    }
    
    /// @fn to_json
    /// @brief Returns the json representation of the UUID
    std::string to_json() const {
        const std::string json = "{" + fmt::format("'oid': '{}'", to_string()) + "}";
        return json;
    }

    /// @fn to_string
    /// @brief Returns the string representation of the UUID
    std::string to_string() const {
        const std::string ret = boost::uuids::to_string(tag);
        return ret;
    }
    
    /// @fn empty
    /// @brief Returns true if the oid is nil else returns false
    bool empty() const {
        return tag.is_nil();
    }
};




template<typename T>
    struct SimpleObjHolder {
        T* obj_ptr;
        // SimpleOID const& oid;
        // SimpleObjHolder(T* obj_ptr_in, SimpleOID const& oid_in) :obj_ptr(obj_ptr_in), oid(oid_in) {}
        SimpleObjHolder(T* obj_ptr_in) :obj_ptr(obj_ptr_in) {}
        ~SimpleObjHolder() {
            obj_ptr = nullptr;
        }
    };

template<typename T>
    struct IsComposite : boost::mpl::bool_<false> {
    };

/// @brief SOCI conversion helpers
    /// @class type_conversion
    /// @brief partial specialization to support regular objects T
    /// @details
    template<typename T>
    struct type_conversion {
    public:
        using ObjType = T;
    private:
        struct FromBase {
        private:
            soci::values const& _val;
            std::uint_fast32_t _count;

        public:
            FromBase(soci::values const& val) :_val(val), _count(0) {} //_count is the num of obj to skip

            template<typename O>
            void operator()(O& x) const {
                const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<FromBase*>(this)->_count++);
                x = _val.get<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name);
            }
        };

    public:
        inline static void from_base(soci::values const& v, soci::indicator, ObjType& obj) {
            //boost::fusion::for_each(obj, FromBase(v));
        }

    private:
        struct ToBase {
        private:
            soci::values& _val;
            std::uint_fast32_t _count;

        public:
            ToBase(soci::values& val) :_val(val), _count(0) {}

            template<typename O>
            void operator()(O const& x) const {
                const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<ToBase*>(this)->_count++);
                const_cast<ToBase*>(this)->_val.set(obj_name, x);
            }
        };

    public:
        inline static void to_base(ObjType const& obj, soci::values& v, soci::indicator& ind) {
            //boost::fusion::for_each(obj, ToBase(v)); // only init the obj v ind
        }
    };


/// @class type_conversion
/// @brief partial specialization to support SimpleObjHolder<T>
/// @details
template<typename T>
struct type_conversion<SimpleObjHolder<T>> {
    using ObjectHolderType = SimpleObjHolder<T>;
    using ObjType = T;

private:
    template<typename O, bool IsComposite = false>
    struct FromBaseOperation {
        inline static void execute(O& x, const std::string& obj_name, soci::values const& val, const edadb::SimpleOID& /*parent_oid*/) {
            x = val.get<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name);
        }
    };

    template<typename O>
    struct FromBaseOperation<O, true> {
        inline static void execute(O& x, const std::string& obj_name, soci::values const& val, const edadb::SimpleOID& parent_oid) {
            //
        }
    };

    struct FromBase {
    private:
        soci::values const& _val;
        const edadb::SimpleOID& _oid;
        std::uint16_t _count;
        soci::indicator& _ind;

    public:
        FromBase(soci::values const& val, edadb::SimpleOID const& oid, soci::indicator& ind) :_val(val), _oid(oid), _count(0), _ind(ind) {}

        template<typename O>
        void operator()(O& x) const {
            // std::cout<<"What happened in FromBase\n";
            const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<FromBase*>(this)->_count++);
            FromBaseOperation<O, IsComposite<O>::value>::execute(x, obj_name, _val, _oid);
        }
    };

public:
    /// @fn from_base Setting the values in the object
    /// @brief This will take the database value and put it in the object. Database -> Object
    inline static void from_base(soci::values const& v, soci::indicator ind, ObjectHolderType& obj_holder) {
        ObjType& obj = *(obj_holder.obj_ptr);
        // const edadb::SimpleOID& oid = obj_holder.oid;
        boost::fusion::for_each(obj, FromBase(v,edadb::SimpleOID(), ind));
    }

private:
    template<typename O, bool IsComposite = false>
    struct ToBaseOperation {
        inline static void execute(O& x, const std::string& obj_name, soci::values& val, /*const edadb::SimpleOID& parent_oid,*/ soci::indicator& ind) {
            val.set<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name, x, ind);
        }
    };

    template<typename O>
    struct ToBaseOperation<O, true> {
        inline static void execute(O& x, const std::string& obj_name, soci::values& val, const edadb::SimpleOID& parent_oid, soci::indicator& ind) {
            std::cout<<"What happened in execute ToBaseOperation\n";
        }
    };

    struct ToBase {
    private:
        soci::values& _val;
        // const edadb::SimpleOID& _oid;
        std::uint_fast32_t _count;
        soci::indicator& _ind;

    public:
        ToBase(soci::values& val, /*edadb::SimpleOID const& oid,*/ soci::indicator& ind) :_val(val), /*_oid(oid),*/ _count(0), _ind(ind) {}

        template<typename O>
        void operator()(O& x) const {
                        // std::cout<<"What happened in ToBase\n";
            const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<ToBase*>(this)->_count++);
                // std::cout<< " obj_name = " << obj_name << "\n"; 
            ToBaseOperation<O, IsComposite<O>::value>::execute(x, obj_name, _val, /*_oid,*/ _ind);
        }
    };

public:
    /// @fn to_base Setting the values in the database
    /// @brief This will take the object value and persist it in the database. Object -> Database
    inline static void to_base(ObjectHolderType& obj_holder, soci::values& v, soci::indicator& ind) {
        ObjType& obj = *(obj_holder.obj_ptr);
        // const edadb::SimpleOID& oid = obj_holder.oid;
        // std::cout<<"Before for_each ToBase\n";
        boost::fusion::for_each(obj, ToBase(v, /*edadb::SimpleOID(),*/ ind));
    }
};

template<typename TA, bool IsComposite>
    struct GetAllObjectsImpl {
        inline static void impl(TA& x, const soci::row& row, const std::string& member_name/*, const std::string& oid_ref*/) {
            x = row.get<typename ConvertCPPTypeToSOCISupportType<TA>::type>(member_name);
        }
    };

template<typename T>
struct GetAllObjects {
    private:
        const soci::row& _row;
        const std::vector<std::string>& _member_names;
        int _count;

    public:
        GetAllObjects(const soci::row& row) :_row(row),
            _member_names(TypeMetaData<T>::member_names()), _count(0) {
        }
#if 0
        template <typename O>
        void operator()(O& x) const {
            const std::string& member_name = _member_names.at(const_cast<GetAllObjects*>(this)->_count++);
            GetAllObjectsImpl<O, IsComposite<O>::value>::impl(x, _row, member_name);
            x = _row.get<typename ConvertCPPTypeToSOCISupportType<O>::type>(_member_names.at(const_cast<GetAllObjects*>(this)->_count++));
        }
#endif
        template <typename O> 
        void operator()(O& x){

            const std::string& member_name = _member_names.at(_count);
            // std::cout<<member_name<<"\n";
            
            using SOCISupportType = typename ConvertCPPTypeToSOCISupportType<typename std::remove_pointer<O>::type>::type;
            // std::cout<<"name = "<<typeid(x).name()<<"\n";
            *x = _row.get<SOCISupportType>(member_name);
            _count++;
        }
    };


void dbgPrint(){std::cout<<"Edadb starting \n";}

template<typename T>
    class DbMap {
      public:
        //Members
        std::string table_name;
        std::string select_header;
        
      public:
        static bool connectToDb(const std::string& db_connect_str); 
        void setTableName(std::string tab_name){table_name = tab_name;}
        bool createTable(std::string tab_name);
        bool insertToDb(T *obj);
        bool deleteFromDb(T *obj);
        bool updateDb(T *obj);
        bool selectFromDb(std::vector<T> *vec, std::string where_str = "");
        template<typename Q>
        bool selectWithPK(Q PK_val, T *obj);
    };

    template<typename T>
    bool DbMap<T>::connectToDb(const std::string& db_connect_str){
        auto ret = DbBackend::i().connect(db_connect_str);
        return ret;
    }

    template<typename T> 
    bool DbMap<T>::createTable(std::string tab_name){
        table_name = tab_name;
        const static std::string sql = fmt::format(SqlString<T>::createTableStr(), table_name);
        #ifdef EDADB_DEBUG
            std::cout<<"createTable sql: "<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            select_header = fmt::format(SqlString<T>::selectRowStr(),table_name);
            return true;
        }
        catch (std::exception const & e) {
            std::cerr << "createTable: " << e.what() << std::endl;
            return false;
        }
    }

    template<typename T>
    bool DbMap<T>::insertToDb(T *obj){
        SqlString<T> sql_string;
        const std::string sql = fmt::format(sql_string.insertRowStr(obj), table_name);
        #ifdef EDADB_DEBUG
            std::cout<<"insertToDb sql: "<<sql<<"\n";
        #endif
        try{
            DbBackend::i().session()<<sql; //boost 
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "insertToDb: " << e.what() << "\n";
            return false;
        }
    }

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
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "updateToDb: "<< e.what() << "\n";
            return false;
        }
    }

    template<typename T>
    bool DbMap<T>::selectFromDb(std::vector<T> *vec, std::string where_str){
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
                T *a = new T;
                auto vals = TypeMetaData<T>::getVal(a);
                boost::fusion::for_each(vals,GetAllObjects<T>(row));
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
            boost::fusion::for_each(vals,GetAllObjects<T>(row));
            return true;
        }
        catch (std::exception const& e) {
            std::cerr << "selectFromDbPK: "<< e.what() << "\n";
            return false;
        }
    }


} // end of edadb namespace

namespace soci{
    /// @class type_conversion
    /// @brief SOCI class specialized to use SimpleObjHolder
    template<typename T>
    struct type_conversion<edadb::SimpleObjHolder<T>> {
        using ObjectHolderType = edadb::SimpleObjHolder<T>;
        using ObjType = T;
        typedef values base_type;

        /// @fn from_base
        /// @param soci::values const& v
        /// @param soci::indicator ind
        /// @param ObjectHolderType& obj_holder
        inline static void from_base(soci::values const& v, soci::indicator ind, ObjectHolderType& obj_holder) {
            edadb::type_conversion<ObjectHolderType>::from_base(v, ind, obj_holder);
        }

        /// @fn to_base
        /// @param ObjectHolderType const& obj_holder
        /// @param soci::values& v
        /// @param soci::indicator& ind
        inline static void to_base(ObjectHolderType& obj_holder, soci::values& v, soci::indicator& ind) {
            // std::cout<<"What happened in soci::to_base()\n";
            edadb::type_conversion<ObjectHolderType>::to_base(obj_holder, v, ind);
        }
    };
}


///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////
#define GENERATE_TupType_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupType_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<edadb::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePair_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<edadb::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS_TUP)

#define EXPAND_member_names_I(z, n, ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)

#define GET_first_member(ELEMS_TUP) BOOST_PP_TUPLE_ELEM(0,ELEMS_TUP )

#define GENERATE_ObjVal_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() &obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)
#define GENERATE_ObjVal(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_ObjVal_I, CLASS_ELEMS_TUP)


///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

/// TABLE4CLASS Start
// TABLE4CLASS(IdbSite, “maintable”, (id, subobj));
// TABLE4CLASS(IdbSite, “”, (sid, data));

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

#define TABLE4CLASS(myclass, tablename, CLASS_ELEMS_TUP) \
TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)))
