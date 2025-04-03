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
* 影子类支持外部类
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
struct ExternalReadWrite {
};

template<typename T> // for TABLE2TABLE_1_N_VEC
struct RelationMap{

};

// 输出流（内存缓冲区版）
class DbOstream : public std::ostream {
  public:
    DbOstream() : std::ostream(&buffer_) {}

    std::string str() const {
        return buffer_.str();
    }
    
    // 获取二进制数据指针
    const char* data() const { return buffer_.str().data(); }
    
    // 获取数据长度
    size_t size() const { return buffer_.str().size(); }

  private:
    std::stringbuf buffer_;
};

// 输入流（内存缓冲区版）
class DbIstream : public std::istream {
  public:
    DbIstream(const char* data, size_t len)
        : std::istream(&buffer_), buffer_(std::string(data, len), std::ios_base::in) {}
    
  private:
    std::stringbuf buffer_;
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
                    // CppTypeToDbType<O>::ret 不能直接判等
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

        static std::string const& createTableStr() { // create_table_str
            static const auto vecs = TypeMetaData<T>::tuple_type_pair();
            static std::string sql;
            if (sql.empty()) {
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
            std::string str = "INSERT INTO \"{}\" ("; // INSERT INTO
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
            std::string str;
            str += "DELETE FROM \"{}\" WHERE " +first_pair.second + 
            " = " + firstColumnVal(*(boost::fusion::at_c<0>(vals))) + ";";
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
            static const auto vecs_Q = TypeMetaData<Q>::tuple_type_pair();
            auto &first_pair_T = boost::fusion::at_c<0>(vecs_T);
            auto &first_pair_Q = boost::fusion::at_c<0>(vecs_Q);

            // using decay_t = typename decay<T>::type; decay_t移除const,volatile,引用 移除指针在后面ConvertCPPTypeToSOCISupportType()
            using FirstType_T = std::decay_t<decltype(boost::fusion::at_c<0>(typename TypeMetaData<T>::TupType()))>;
            using FirstType_Q = std::decay_t<decltype(boost::fusion::at_c<0>(typename TypeMetaData<Q>::TupType()))>;

            std::string type_T = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<FirstType_T>::type>();
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
            std::string str = "INSERT INTO \"{}\" ("; // INSERT INTO
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
                "{} = {} AND {} = {}",
                TypeMetaData<T>::class_name() + "_" + first_pair_T.second, 
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


void dbgPrint(){std::cout<<"Edadb Test 14 starting \n";}

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
                int count = 0;
                boost::fusion::for_each(vals,GetAllObjects<T>(row, SqlString<T>::member_names, count));
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
                int count = 0;
                boost::fusion::for_each(vals_Q, GetAllObjects<Q>(row, member_names_Q, count));
                vec->emplace_back(std::move(*b));
                delete b;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "selectFromDb: " << e.what() << "\n";
            return false;
        }
    }




} // end of edadb namespace




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
        return TupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(CLASS_ELEMS_TUP, myclass)));\
    }\
};\
}

#define TABLE4CLASS(myclass, tablename, CLASS_ELEMS_TUP) \
TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)))

# define Table4ExternalClass(myclass, CLASS_ELEMS_TUP) \
namespace edadb{\
template<>\
struct CppTypeToDbType<myclass>{\
    static const DbTypes ret = DbTypes::kExternal;\
};\
}\
TABLE4CLASS(edadb::Shadow<myclass>, "" , CLASS_ELEMS_TUP);

#if 0

# define Table4ExternalClass(myclass, writeFunc, readFunc) \
namespace edadb{\
template<>\
struct CppTypeToDbType<myclass>{\
    static const DbTypes ret = DbTypes::kExternal;\
};\
template<> struct ExternalReadWrite<myclass>{\
    static bool write(myclass* obj, DbOstream* so) {\
        return writeFunc(obj, so);\
    }\
    static bool read(myclass* obj, DbIstream* si) {\
        return readFunc(obj, si);\
    }\
};\
}

#endif

#if 0

#define TABLE4CLASSWEXTERNAL_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, EXTERNAL_TUP, COLNAME_TUP) \
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
    inline static const std::vector<std::string>& external_member_names(){\
        static const std::vector<std::string> names = { \
            EXPAND_member_names(EXTERNAL_TUP) \
        };\
        return names;\
    }\
    inline static const std::vector<std::string>& member_names(){\
        static const std::vector<std::string> names = { \
            EXPAND_member_names(CLASS_ELEMS_TUP), \
            EXPAND_member_names(EXTERNAL_TUP) \
        };\
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

#endif

#if 0
#define TABLE4CLASSWEXTERNAL_COLNAME(myclass, tablename, CLASS_ELEMS_TUP,  EXTERNAL_TUP, COLNAME_TUP) \
BOOST_FUSION_ADAPT_STRUCT( myclass, BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP) ) \
namespace edadb{\
template<>\
struct CppTypeToDbType<myclass>{\
    static const DbTypes ret = DbTypes::kComposite;\
};\
template<> struct TypeMetaData<myclass>{\
    using TupType = boost::fusion::vector<GENERATE_TupType(BOOST_PP_TUPLE_PUSH_FRONT(BOOST_PP_VARIADIC_TO_TUPLE( \
        BOOST_PP_TUPLE_ENUM(CLASS_ELEMS_TUP), \
        BOOST_PP_TUPLE_ENUM(EXTERNAL_TUP)), myclass))>;\
    using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(BOOST_PP_TUPLE_PUSH_FRONT(BOOST_PP_VARIADIC_TO_TUPLE( \
        BOOST_PP_TUPLE_ENUM(CLASS_ELEMS_TUP), \
        BOOST_PP_TUPLE_ENUM(EXTERNAL_TUP)), myclass))>;\
    using T = myclass;\
    \
    inline static auto tuple_type_pair()->TupTypePairType const&{\
        static const TupTypePairType t{GENERATE_TupTypePairObj(BOOST_PP_TUPLE_PUSH_FRONT(BOOST_PP_VARIADIC_TO_TUPLE( \
            BOOST_PP_TUPLE_ENUM(CLASS_ELEMS_TUP), \
            BOOST_PP_TUPLE_ENUM(EXTERNAL_TUP)), myclass))};\
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
        static const std::vector<std::string> names = {EXPAND_member_names(BOOST_PP_VARIADIC_TO_TUPLE( \
            BOOST_PP_TUPLE_ENUM(CLASS_ELEMS_TUP), \
            BOOST_PP_TUPLE_ENUM(EXTERNAL_TUP)))};\
        return names;\
    }\
    inline static const std::vector<std::string>& column_names(){\
        static const std::vector<std::string> names = {BOOST_PP_TUPLE_REM_CTOR(COLNAME_TUP)};\
        return names;\
    }\
    inline static TupType getVal(myclass * obj){\
        return TupType(GENERATE_ObjVal(BOOST_PP_TUPLE_PUSH_FRONT(\
            BOOST_PP_VARIADIC_TO_TUPLE( \
            BOOST_PP_TUPLE_ENUM(CLASS_ELEMS_TUP), \
            BOOST_PP_TUPLE_ENUM(EXTERNAL_TUP)), myclass)));\
    }\
};\
}


#define TABLE4CLASSWEXTERNAL(myclass, tablename, CLASS_ELEMS_TUP, EXTERNAL_TUP) \
TABLE4CLASSWEXTERNAL_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, EXTERNAL_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP), EXPAND_member_names(EXTERNAL_TUP)))

#endif


//先写一个完整的实际的例子程序（保留，反复使用，宏太长不好看懂），然后再试着用宏替换
#define GENERATE_ArrayFields_I(z, n, ARRAY_FIELD_TUP) \
DbMapT2T< \
    BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP), \
    typename std::remove_reference<\
        decltype(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP)) \
    >::type::value_type \
> BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP), _dbmap);


#define GENERATE_CreateTable_I(z, n, ARRAY_FIELD_TUP) \
bool BOOST_PP_CAT(createTable_,BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))() { \
    std::string arr_tab_name = BOOST_PP_STRINGIZE( \
        BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP).BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP) \
    ); \
    return BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP), _dbmap).createTable(arr_tab_name, table_name); \
}

#define GENERATE_CreateTableAll_I(z, n, ARRAY_FIELD_TUP) \
&& BOOST_PP_CAT(createTable_ , BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(0, n), ARRAY_FIELD_TUP)()) 

#define GENERATE_InsertToTable_I(z, n, ARRAY_FIELD_TUP) \
bool BOOST_PP_CAT(insertToDb_,BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP) *obj1, typename std::remove_reference<\
    decltype(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))>::type::value_type *obj2) { \
    return BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP), _dbmap).insertToDb(obj1, obj2); \
}

#define GENERATE_InsertToTableAll_I(z, n, ARRAY_FIELD_TUP) \
for(auto i : obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP)){\
    BOOST_PP_CAT(insertToDb_, BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(obj, &i);\
}

#define GENERATE_DeleteFromDb_I(z, n, ARRAY_FIELD_TUP) \
bool BOOST_PP_CAT(deleteFromDb_,BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP) *obj1, typename std::remove_reference<\
    decltype(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))>::type::value_type *obj2) { \
    return BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP), _dbmap).deleteFromDb(obj1, obj2); \
}

#define GENERATE_DeleteFromDbAll_I(z, n, ARRAY_FIELD_TUP) \
for(auto i : obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP)){\
    BOOST_PP_CAT(deleteFromDb_, BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(obj, &i);\
}

#define GENERATE_UpdateDb_I(z, n, ARRAY_FIELD_TUP) \
bool BOOST_PP_CAT(updateDb_,BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP) *obj1, typename std::remove_reference<\
    decltype(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))>::type::value_type *obj2) { \
    return BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP), _dbmap).updateDb(obj1, obj2); \
}

#define GENERATE_UpdateDbAll_I(z, n, ARRAY_FIELD_TUP) \
for(auto i : obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP)){\
    BOOST_PP_CAT(updateDb_, BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(obj, &i);\
}

#define GENERATE_SelectFromDb_I(z, n, ARRAY_FIELD_TUP) \
bool BOOST_PP_CAT(selectFromDb_,BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(std::vector<typename std::remove_reference<\
    decltype(BOOST_PP_TUPLE_ELEM(0, ARRAY_FIELD_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))>::type::value_type>* vec, std::string where_str = "") { \
    return BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP), _dbmap).selectFromDb(vec, where_str); \
}

#define GENERATE_SelectFromDbAll_I(z, n, ARRAY_FIELD_TUP) \
    BOOST_PP_CAT(selectFromDb_,BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP))(&i.BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), ARRAY_FIELD_TUP),arr_where_str);


#define TABLE2TABLE_1_N_VEC(myclass, ARRAY_FIELD_TUP) \
namespace edadb{\
template<typename myclass>\
    class DbMapAll{\ 
        private:\ 
            std::string table_name;\
            DbMap<myclass> db_map;\
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_ArrayFields_I, BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass))\
        public:\
            bool connectToDb(const std::string& db_connect_str){\
                return DbBackend::i().connect(db_connect_str);\
            }\
            bool createTable(std::string tab_name){\
                table_name = tab_name;\
                return db_map.createTable(table_name);\
            }\
            BOOST_PP_REPEAT( \
                BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), \
                GENERATE_CreateTable_I, \
                BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass) \
            ) \
            bool createTableAll(std::string tab_name){\
                return createTable(tab_name) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_CreateTableAll_I, ARRAY_FIELD_TUP);\
            }\
            bool insertToDb(myclass *obj){return db_map.insertToDb(obj);}\
            BOOST_PP_REPEAT( \
                BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), \
                GENERATE_InsertToTable_I, \
                BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass) \
            ) \
            bool insertToDbAll(myclass *obj){\
                try{\
                    insertToDb(obj);\
                    BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_InsertToTableAll_I, BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass));\
                    return true;\
                }\
                catch (std::exception const& e) {\
                    std::cerr << "insertToDbAll: " << e.what() << "\n";\
                    return false;\
                }\
            }\
            bool deleteFromDb(myclass *obj){return db_map.deleteFromDb(obj);}\
            BOOST_PP_REPEAT( \
                BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), \
                GENERATE_DeleteFromDb_I, \
                BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass) \
            ) \
            bool deleteFromDbAll(myclass *obj){\
                try{\
                    deleteFromDb(obj);\
                    BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_DeleteFromDbAll_I, BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass));\
                    return true;\
                }\
                catch (std::exception const& e) {\
                    std::cerr << "deleteFromDbAll: " << e.what() << "\n";\
                    return false;\
                }\
            }\
            bool updateDb(myclass *obj){return db_map.updateDb(obj);}\
            BOOST_PP_REPEAT( \
                BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), \
                GENERATE_UpdateDb_I, \
                BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass) \
            ) \
            bool updateDbAll(myclass *obj){\
                try{\
                    updateDb(obj);\
                    BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_UpdateDbAll_I, BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass));\
                    return true;\
                }\
                catch (std::exception const& e) {\
                    std::cerr << "updateDbAll: " << e.what() << "\n";\
                    return false;\
                }\
            }\
            bool selectFromDb(std::vector<myclass> *vec, std::string where_str = ""){\
                return db_map.selectFromDb(vec, where_str);\
            }\
            BOOST_PP_REPEAT( \
                BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), \
                GENERATE_SelectFromDb_I, \
                BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass) \
            ) \
            template<typename O>\
            std::string stringVal(O val){\
                return std::to_string(val);\
            }\
            std::string stringVal(std::string val){\
                std::string str = "\'" + val + "\'";\
                return str;\
            }\
            bool selectFromDbAll(std::vector<myclass> *vec, std::string where_str = ""){\
                try{\
                    selectFromDb(vec, where_str);\
                    for(auto &i : *vec){\
                        const auto vecs = TypeMetaData<myclass>::tuple_type_pair();\
                        const auto vals = TypeMetaData<myclass>::getVal(&i);\
                        auto &first_pair = boost::fusion::at_c<0>(vecs);\
                        std::string arr_where_str = TypeMetaData<myclass>::class_name() + "_" + first_pair.second + " = " + stringVal(*(boost::fusion::at_c<0>(vals)));\
                        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARRAY_FIELD_TUP), GENERATE_SelectFromDbAll_I, BOOST_PP_TUPLE_PUSH_FRONT(ARRAY_FIELD_TUP, myclass));\
                    }\
                    return true;\
                }\
                catch (std::exception const& e) {\
                    std::cerr << "selectFromDbAll: " << e.what() << "\n";\
                    return false;\
                }\
            }\
    };\
}


#define TABLE4CLASSWVEC_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP, ARRAY_FIELD_TUP) \
TABLE4CLASS_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, COLNAME_TUP); \
TABLE2TABLE_1_N_VEC(myclass, ARRAY_FIELD_TUP)


#define TABLE4CLASSWVEC(myclass, tablename, CLASS_ELEMS_TUP,  ARRAY_FIELD_TUP) \
TABLE4CLASSWVEC_COLNAME(myclass, tablename, CLASS_ELEMS_TUP, (EXPAND_member_names(CLASS_ELEMS_TUP)), ARRAY_FIELD_TUP) 


