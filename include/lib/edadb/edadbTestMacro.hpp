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
* macro T2TALL DbMapAll全部写在宏中的版本
*/

/// @class IdbSite  // example 
class IdbSite {
    public:
      std::string name;
      int width;
      int height;
      int not_saved;
      
    
      IdbSite() : name("Default"), width(11), height(22), not_saved(33) {}
    
      IdbSite(std::string n, int w = 11, int h = 22, int t = 33)
          : name(n), width(w), height(h), not_saved(t) {}
    
      void print() {
        std::cout << "IdbSite(name=" << name
                  << ", width=" << width
                  << ", height=" << height
                  << ", not_saved=" << not_saved << ")\n";
      }
      
    };
    
    /// @class Array2  // example 
    class Array2 {
      public:
        std::string name;
        int width;
        int height;
        int not_saved;
        
      
        Array2() : name("Default"), width(11), height(22), not_saved(33) {}
      
        Array2(std::string n, int w = 11, int h = 22, int t = 33)
            : name(n), width(w), height(h), not_saved(t) {}
      
        void print() {
          std::cout << "Array(name=" << name
                    << ", width=" << width
                    << ", height=" << height
                    << ", not_saved=" << not_saved << ")\n";
        }
        
      };
    
    /// @class IdbSites  // example 
    class IdbSites {
      public:
        std::string name;
        // int width;
        // int height;
        // int not_saved;
        // SubClass subobj;
        std::vector<IdbSite> idbsite_array;
        std::vector<Array2> array2_array;
        
      
        IdbSites() : name("Default") {}
      
        IdbSites(std::string n)
            : name(n) {}
      
        void print() {
          std::cout << "IdbSite(name=" << name << ")\n";
        }
        
    };
    

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

template<typename T> // for TABLE2TABLE_1_N_VEC
struct RelationMap{

};


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
            // std::vector<std::string>& temp_member_names;
            int cnt;
        public:
        CTForeachHelper(std::string& sql)
        : sql(sql)/*, temp_member_names(member_names)*/, cnt(0) {}

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
                        // sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_");
                        std::vector<std::string> sub_member_names;
                        sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_", sub_member_names);
                        member_names.insert(member_names.end(), sub_member_names.begin(), sub_member_names.end());
                    }
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
                    if (ret == DbTypes::kComposite)
                        sql += ", " + edadb::createTableStrSubObj<ObjType>(prefix + x.second + "_",temp_member_names);
                    else{
                        // std::cout<<"member_names sub now = "<<prefix + x.second<<"\n";
                        // std::cout << "Before CTSO push_back: member_names size = " << 
                        // "typeid(T).name() = " << typeid(T).name()<<temp_member_names.size() << std::endl;
                        temp_member_names.push_back(prefix + x.second);
                        // std::cout << "After push_back: _member_names size = " << SqlString<T>::member_names.size() << std::endl;
                        // SqlString<T>::member_names.push_back(prefix + x.second);
                        for(auto i: temp_member_names){
                            std::cout<<"member_names sub now = "<<i<<"\n";
                        }
                        sql += ", " + prefix + x.second + " " + type;
                    } 
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

                part = "";
                if (!vec.empty()) {
                    part = ",";
                }
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
            std::string str = "INSERT OR IGNORE INTO \"{}\" ("; // INSERT INTO
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

                    // if(cnt == 1) 
                        // sql += x.second + " " + type + " PRIMARY KEY";  // DbMap<subclass>
                    // else {
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
                    // }
                }
            };
      public:
        static std::string const& createTableStr() { // create_table_str
            static const auto vecs_T = TypeMetaData<T>::tuple_type_pair();
            static const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
            auto &first_pair_T = boost::fusion::at_c<0>(vecs_T);
            auto &first_pair_Q = boost::fusion::at_c<0>(vecs_Q);

            // using decay_t = typename decay<T>::type; decay_t移除const,volatile,引用 移除指针在后面ConvertCPPTypeToSOCISupportType()
            using FirstType_T = std::decay_t<decltype(boost::fusion::at_c<0>(typename TypeMetaData<T>::TupType()))>;
            using FirstType_Q = std::decay_t<decltype(boost::fusion::at_c<0>(typename TypeMetaData<Q>::TupType()))>;

            std::string type_T = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<FirstType_T>::type>();
            // std::string type_Q = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<FirstType_Q>::type>();
            static std::string sql;
            if (sql.empty()) {

                sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
                sql += TypeMetaData<T>::class_name() + "_" + first_pair_T.second + " " + type_T;
                
                auto p = new typename SqlStringT2T<T,Q>::CTForeachHelperT2T(sql); //CTForeachHelperT2T
                boost::fusion::for_each(vecs_Q, *p);
                sql += ", PRIMARY KEY (" + TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ", " + first_pair_Q.second + ")"; 
                sql += ", FOREIGN KEY (" + TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ") REFERENCES {}" + "(" + first_pair_T.second + ") ON DELETE CASCADE ON UPDATE CASCADE";
                sql += ")";
            }
            return sql;
        }

        std::string insertRowStr(T *obj1, Q *obj2) {
            // 获取 obj1 的主键值
            const auto vecs_T = TypeMetaData<T>::tuple_type_pair();
            const auto vals_T = TypeMetaData<T>::getVal(obj1);
            auto& first_pair_T = boost::fusion::at_c<0>(vecs_T);
            auto& first_val_T = boost::fusion::at_c<0>(vals_T);

            // 获取 obj2 的列和值
            const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
            const auto vals_Q = TypeMetaData<Q>::getVal(obj2);
            std::string str = "INSERT OR IGNORE INTO \"{}\" ("; // INSERT INTO
            /*std::stringstream ss;
            ss << "INSERT INTO \"{}\" (";*/
            str += TypeMetaData<T>::class_name() + "_" + first_pair_T.second + ", ";
            boost::fusion::for_each(vecs_Q, typename SqlString<Q>::InsertRowNames(&str));
            str += ") VALUES (";
            str += "\'" + *first_val_T + "\', ";
            boost::fusion::for_each(vals_Q, typename SqlString<Q>::fill(str));
            str += ");";
            return str;
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
            const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
            const auto vals_T = TypeMetaData<T>::getVal(obj1);
            const auto vals_Q = TypeMetaData<Q>::getVal(obj2);
            auto& first_pair_T = boost::fusion::at_c<0>(vecs_T); 
            auto& first_pair_Q = boost::fusion::at_c<0>(vecs_Q); 
            
            std::string where_condition = fmt::format(
                "{} = '{}' AND {} = '{}'",
                first_pair_T.second, 
                firstColumnVal(*(boost::fusion::at_c<0>(vals_T))), 
                first_pair_Q.second, 
                firstColumnVal(*(boost::fusion::at_c<0>(vals_Q))) 
            );
            std::string str;
            str += "DELETE FROM \"{}\" WHERE " + where_condition + ";";
            return str;
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

            /*const */auto vecs = TypeMetaData<T>::tuple_type_pair();

                auto p = new typename SqlString<T>::CTSOForeachHelper(sql, prefix,temp_member_names);// CTSOForeachHelper
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
        GetAllObjects(const soci::row& row, const std::vector<std::string>& member_names, int count = 0) :_row(row),_member_names(member_names), _count(count) {
            // _member_names = SqlString<T>::member_names;
            // for(auto i : _member_names){
            //     std::cout<<"member_names = "<< i<<"\n";
            // }
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
            #ifdef EDADB_DEBUG
                //std::cout<<"_member_names.size() = "<<_member_names.size()<<"\n";
            #endif
            const std::string& member_name = _member_names.at(_count);
            using ObjType = std::remove_pointer_t<std::decay_t<decltype(x)>>;
            if constexpr (std::is_class<ObjType>::value && (!std::is_same<ObjType, std::string>::value)){
                std::cout<<"if constexpr\n";
                auto vals = TypeMetaData<ObjType>::getVal(x);
                boost::fusion::for_each(vals,GetAllObjects<ObjType>(_row, _member_names, _count));
            }
            else{
                // std::cout<<"else\n";
                using SOCISupportType = typename ConvertCPPTypeToSOCISupportType<typename std::remove_pointer<O>::type>::type;
                *x = _row.get<SOCISupportType>(member_name);
            }
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
        static bool connectToDb(const std::string& db_connect_str); // to be deleted 禁用
        void setTableName(std::string tab_name){table_name = tab_name;}
        bool createTable(std::string tab_name);
        bool insertToDb(T *obj);
        bool deleteFromDb(T *obj);
        bool updateDb(T *obj);
        bool selectFromDb(std::vector<T> *vec, std::string where_str = "");
        template<typename Q>
        bool selectWithPK(Q PK_val, T *obj);

        bool insertToDbAll(T *obj1);

        template<typename array_class>
        bool insertArrayEleToDb(T* obj, array_class* array_obj);
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
        // for(auto i : SqlString<T>::member_names){
        //     std::cout<<"member_names before selectFrom Db = "<< i<<"\n";
        // }
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
                boost::fusion::for_each(vals,GetAllObjects<T>(row, SqlString<T>::member_names));
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

    class DbMapT2T_Base {
        public:
            // virtual bool createTable(const std::string tableName) = 0;
            virtual bool insertToDb(void* obj, void* arr_obj) = 0;
            virtual bool updateDb(void *obj1, void *obj2) = 0;
            virtual bool selectFromDb(std::vector<void*>* vec, std::string where_str) = 0;
    };

    template<typename T, typename Q>
    class DbMapT2T : public DbMapT2T_Base {
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
            std::cout<<TypeMetaData<T>::class_name()<<" "<<TypeMetaData<T>::class_name()<<"\n";
            std::string tab_name = fmt::format("{}.{}",TypeMetaData<T>::class_name(),TypeMetaData<Q>::class_name());  
        }
        table_name = tab_name;
        const static std::string sql = fmt::format(SqlStringT2T<T,Q>::createTableStr(), table_name, father_tab_name);
        #ifdef EDADB_DEBUG
            std::cout<<"createTable sql: "<<sql<<"\n";
        #endif
        try {
            DbBackend::i().session() << sql;
            // select_header = fmt::format(SqlString<T>::selectRowStr(),table_name);
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
            DbBackend::i().session()<<sql; //boost 
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
                Q* b = new Q;
                auto member_names_Q = SqlString<Q>::member_names;
                auto vals_Q = TypeMetaData<Q>::getVal(b);
                boost::fusion::for_each(vals_Q, GetAllObjects<Q>(row, member_names_Q));
                vec->emplace_back(std::move(*b));
                delete b;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "selectFromDb: " << e.what() << "\n";
            return false;
        }
    }
    
    template<typename T>
    struct InsertAllHelper {
        T* obj; // 指向 myclass1 对象的指针
        DbMapT2T_Base* dbMapT2T;
        InsertAllHelper(T* obj, DbMapT2T_Base* dbMapT2T) : obj(obj), dbMapT2T(dbMapT2T) {}
        template<typename F>
        void operator()(F& field) const {
            using FieldType = std::decay_t<decltype(field)>;
    
            using ValueType = typename std::decay_t<decltype(*field)>::value_type; // 获取 std::vector 的元素类型
            for (auto& item : *field) {
                dbMapT2T->insertToDb(obj, &item);
            }
        }

    };

    template<typename T>
    struct UpdateAllHelper {
        T* obj; // 指向 myclass1 对象的指针
        DbMapT2T_Base* dbMapT2T;
        UpdateAllHelper(T* obj, DbMapT2T_Base* dbMapT2T) : obj(obj), dbMapT2T(dbMapT2T) {}
        template<typename F>
        void operator()(F& field) const {
            using FieldType = std::decay_t<decltype(field)>;
    
            using ValueType = typename std::decay_t<decltype(*field)>::value_type; // 获取 std::vector 的元素类型
            for (auto& item : *field) {
                dbMapT2T->updateDb(obj, &item);
            }
        }
    };

    template<typename T>
    struct SelectAllHelper {
        T* obj; 
        DbMapT2T_Base* dbMapT2T; 
        std::string where_str; 

        SelectAllHelper(T* obj, DbMapT2T_Base* dbMapT2T, const std::string& where_str)
            : obj(obj), dbMapT2T(dbMapT2T), where_str(where_str) {}

        template<typename F>
        void operator()(F& field) const {
            using FieldType = std::decay_t<decltype(field)>;
            using ValueType = typename std::decay_t<decltype(*field)>::value_type; 

            // std::vector<ValueType> arr_results;
            std::vector<void*> void_results;
            // if (dbMapT2T->selectFromDb(&arr_results, where_str)) {
            //     *field = arr_results;
            // }
            if (dbMapT2T->selectFromDb(&void_results, where_str)) {
                std::vector<ValueType> arr_results;
                for (auto& void_ptr : void_results) {
                    ValueType* value_ptr = static_cast<ValueType*>(void_ptr);
                    arr_results.emplace_back(*value_ptr);
                    delete value_ptr;
                }
                *field = arr_results;
            }
            else {
                std::cerr << "Failed to select from array table.\n";
            }
        }
    };

    // template<typename T>
    // bool DbMap<T>::insertToDbAll(T *obj){
    //     try{
    //         auto arrays = RelationMap<T>::getVal(obj);
    //         auto p = new InsertAllHelper<T>(obj);
    //         boost::fusion::for_each(arrays,*p);
    //         return true;
    //     }
    //     catch (std::exception const& e) {
    //         std::cerr << "insertToDb: " << e.what() << "\n";
    //         return false;
    //     }
    // }


    //TABLE4CLASSWVEC( IdbSites, "", (name), (idbsite_array, array2_array) );
    template<typename T>
    class DbMapAll{ // class DbMapAll 出现在宏定义里面 更新gitlab后写readme
        private:
        //宏 
            std::string table_name;
            DbMap<T> db_map;
            DbMapT2T<T, IdbSite> dbmap_idbsite_array;
            DbMapT2T<T, Array2> dbmap_array2_array;
            // BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_ArrayFields_I, BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))\
        public:
            bool connectToDb(const std::string& db_connect_str){
                return DbBackend::i().connect(db_connect_str);
            }

            bool createTable(std::string tab_name){
                table_name = tab_name;
                return db_map.createTable(table_name);
            }

            bool createTable_idbsite_array(){
                std::string arr_tab_name = "IdbSites.idbsite_array";
                return dbmap_idbsite_array.createTable(arr_tab_name, table_name);
            }

            bool createTable_array2_array(){
                std::string arr_tab_name = "IdbSites.array2_array";
                return dbmap_array2_array.createTable(arr_tab_name, table_name);
            }

            bool createTableAll(std::string tab_name){
                return createTable(tab_name) && createTable_idbsite_array() && createTable_array2_array();
            }

            bool insertToDb(T *obj){return db_map.insertToDb(obj);}
            
            bool insertToDb_idbsite_array(T *obj1, IdbSite *obj2){
                return dbmap_idbsite_array.insertToDb(obj1, obj2);
            }

            bool insertToDb_array2_array(T *obj1, Array2 *obj2){
                return dbmap_array2_array.insertToDb(obj1, obj2);
            }

            bool insertToDbAll(T *obj){
                try{
                    insertToDb(obj); // 改一下实现变成 insert or ignore into ...
                    for(auto i : obj->idbsite_array){
                        insertToDb_idbsite_array(obj, &i);
                    }
                    for(auto i : obj->array2_array){
                        insertToDb_array2_array(obj, &i);
                    }
                    return true;
                }
                catch (std::exception const& e) {
                    std::cerr << "insertToDbAll: " << e.what() << "\n";
                    return false;
                }
            }

            bool deleteFromDb(T *obj){return db_map.deleteFromDb(obj);}

            bool deleteFromDb_idbsite_array(T *obj1, IdbSite *obj2){
                return dbmap_idbsite_array.deleteFromDb(obj1, obj2);
            }

            bool deleteFromDb_array2_array(T *obj1, Array2 *obj2){
                return dbmap_array2_array.deleteFromDb(obj1, obj2);
            }

            bool deleteFromDbAll(T *obj){
                try{
                    deleteFromDb(obj);
                    for(auto i : obj->idbsite_array){
                        deleteFromDb_idbsite_array(obj, &i);
                    }
                    for(auto i : obj->array2_array){
                        deleteFromDb_array2_array(obj, &i);
                    }
                    return true;
                }
                catch (std::exception const& e) {
                    std::cerr << "deleteFromDbAll: " << e.what() << "\n";
                    return false;
                }
            }

            bool updateDb(T *obj){return db_map.updateDb(obj);}

            bool updateDb_idbsite_array(T *obj1, IdbSite *obj2){
                return dbmap_idbsite_array.updateDb(obj1, obj2);
            }

            bool updateDb_array2_array(T *obj1, Array2 *obj2){
                return dbmap_array2_array.updateDb(obj1, obj2);
            }

            bool updateDbAll(T *obj){
                try{
                    updateDb(obj);
                    for(auto i : obj->idbsite_array){
                        updateDb_idbsite_array(obj, &i);
                    }
                    for(auto i : obj->array2_array){
                        updateDb_array2_array(obj, &i);
                    }
                    return true;
                }
                catch (std::exception const& e) {
                    std::cerr << "updateDbAll: " << e.what() << "\n";
                    return false;
                }
            }

            bool selectFromDb(std::vector<T> *vec, std::string where_str = ""){
                return db_map.selectFromDb(vec, where_str);
            }

            bool selectFromDb_idbsite_array(std::vector<IdbSite>* vec, std::string where_str = ""){
                return dbmap_idbsite_array.selectFromDb(vec, where_str);
            }

            bool selectFromDb_array2_array(std::vector<Array2>* vec, std::string where_str = ""){
                return dbmap_array2_array.selectFromDb(vec, where_str);
            }

            template<typename O>
            std::string stringVal(O val){
                return std::to_string(val);
            }
            
            std::string stringVal(std::string val){
                std::string str = "\'" + val + "\'";
                return str;
            }

            bool selectFromDbAll(std::vector<T> *vec, std::string where_str = ""){
                try{
                    selectFromDb(vec, where_str);
                    for(auto &i : *vec){
                        const auto vecs = TypeMetaData<T>::tuple_type_pair();
                        const auto vals = TypeMetaData<T>::getVal(&i);
                        auto &first_pair = boost::fusion::at_c<0>(vecs);
                        std::string arr_where_str = TypeMetaData<T>::class_name() + "_" + first_pair.second + " = " + stringVal(*(boost::fusion::at_c<0>(vals)));
                        selectFromDb_idbsite_array(&i.idbsite_array,arr_where_str);
                        selectFromDb_array2_array(&i.array2_array,arr_where_str);
                    }
                    return true;
                }
                catch (std::exception const& e) {
                    std::cerr << "selectFromDbAll: " << e.what() << "\n";
                    return false;
                }
            }
    };

//先写一个完整的实际的例子程序（保留，反复使用，宏太长不好看懂），然后再试着用宏替换
#define GENERATE_ArrayFields_I(z, n, ARRAY_FIELD_TUP) \
DbMapT2T<BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP), std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))>::type>::type> BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP)_dbmap;\

#define TABLE4CLASSWVEC(myclass, tablename, CLASS_ELEMS_TUP,  ARRAY_FIELD_TUP) \
TABLE4CLASSWVEC_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)), ARRAY_FIELD_TUP) \ 
template<typename myclass>\
    class DbMapAll{\ 
        private:\ 
            DbMap<myclass> db_map;\
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_ArrayFields_I, BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))\
        public:\
            bool insertToDb(myclass *obj){return db_map.insertToDb(obj);}\
            bool insertToDb_idbsite_array(myclass *obj1, IdbSite *obj2){\
                return dbmap_idbsite_array.insertToDb(obj1, obj2);\
            }\
            bool insertToDb_array2_array(myclass *obj1, Array2 *obj2){\
                return dbmap_array2_array.insertToDb(obj1, obj2);\
            }\
            bool insertToDbAll(myclass *obj){\
                insertToDb(obj);\
                for(auto i : obj->idbsite_array){\
                    insertToDb_idbsite_array(obj, &i);\
                }\
                for(auto i : obj->array2_array){\
                    insertToDb_array2_array(obj, &i);\
                }\
                return true;\
            }\
    };

    
    


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



#define TABLE2TABLE_1_N_VEC(myclass1, ARRAY_FIELD_TUP) \
namespace edadb{\
    template<>\
        struct RelationMap<myclass1> { \
        using TupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass1))>;\
        inline static TupType getVal(myclass1 * obj){\
            return TupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass1)));\
        }\
        inline static const std::vector<std::string>& field_names(){\
            static const std::vector<std::string> names = {EXPAND_member_names(ARRAY_FIELD_TUP)};\
            return names;\
        }\
    };\
}



#define TABLE4CLASSWVEC_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP, ARRAY_FIELD_TUP) \
TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP); \
TABLE2TABLE_1_N_VEC(myclass, ARRAY_FIELD_TUP)

#define TABLE4CLASSWVEC(myclass, tablename, CLASS_ELEMS_TUP,  ARRAY_FIELD_TUP) \
TABLE4CLASSWVEC_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)), ARRAY_FIELD_TUP) 

#if 0

#define TABLE2TABLE_1_N_VEC(myclass1, CLASS_ELEMS_TUP, ARRAY_FIELD_TUP) 

#define TABLE2TABLE_1_N_VEC(myclass1,myclass2,myclass1_array_field) \
namespace edadb{\
    template<>\
    struct RelationMap<myclass1> { \
        static std::vector<std::pair<std::string, std::string>> relation; \
        inline static void add_relation() { \
            relation.push_back({#myclass1, #myclass2}); \
        } \
    }; \
    std::vector<std::pair<std::string, std::string>> RelationMap<myclass1>::relation; \
    struct RelationMapInitializer_##myclass1##_##myclass2 { \
        RelationMapInitializer_##myclass1##_##myclass2() { \
            RelationMap<myclass1>::add_relation(); \
        } \
    }; \
    static RelationMapInitializer_##myclass1##_##myclass2 relationMapInitializer_##myclass1##_##myclass2; \
}
#endif