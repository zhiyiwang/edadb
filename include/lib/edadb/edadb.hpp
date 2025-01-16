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
// #include "blib/bun/impl/DbBackend.hpp"
// #include "blib/bun/impl/DbLogger.hpp"
// #include "blib/bun/impl/SimpleOID.hpp"
#include "lib/edadb/impl/EdadbCppTypeToSQLString.hpp"
// #include <sqlite3.h>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#define EDADB_DEBUG 1 

namespace edadb{

//TABLE4CLASS((classname, primarykey, fields ...))
// #define TABLE4CLASS(CLASS_ELEMS_TUP) 



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
    boost::fusion::vector<boost::fusion::pair<void, std::string>> const& tuple_type_pair();
};

// template<typename T,typename Q>
// Q PK(T* obj){
//     static const auto vecs = TypeMetaData<T>::tuple_type_pair();
//     auto &first_pair = boost::fusion::at_c<0>(vecs);
//     return Q(first_pair.second);
// }


template<class T>
    struct SqlString {
    private:
        struct fill{
            std::stringstream& ss;  
            int cnt;
            fill(std::stringstream& ss_i) : ss(ss_i), cnt(0) {}  

            void operator()(auto val){
                if(cnt == 0)
                    ss <<*val;
                else
                    ss << ", " << *val;
                cnt++;
            }
        };

        struct CTForeachHelper {
        private:
            std::string& sql;
            int cnt;
        public:
            CTForeachHelper(std::string& sql) : sql(sql), cnt(0) {std::cout<<"Create table constructor\n";}

            template <typename O>
            void operator()(O const& x) 
            {
                cnt++;
                using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                static const std::string composite_type = edadb::cppTypeEnumToDbTypeString<DbTypes::kComposite>();
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();
                // If the type is of an object type then we will use VARCHAR to store the hash value
                if (composite_type == type) {
                    type = "VARCHAR";
                }
                std::cout<<"operator print "<<x.second<<"\n";
                if(cnt == 1) 
                    sql += x.second + " " + type + " PRIMARY KEY";  
                else 
                    sql += ", " + x.second + " " + type;
                
            }
        };

        struct InsertRowNames {
        private:
            // std::string& sql;
            std::stringstream* ss;  
            int cnt;

        public:
            InsertRowNames(std::stringstream* ss_i) : ss(ss_i), cnt(0) {}  
            // InsertRowNames(std::& sql) : cnt(0) {}

            template <typename O>
            void operator()(O const& x) 
            {
                if(cnt == 0) 
                    *ss << x.second;
                else
                    *ss << " , " << x.second;
                cnt++;
            }
        };

        struct InsertRowVal {
        private:
            std::string& sql;
            int cnt;

        public:
            InsertRowVal(std::string& sql) : sql(sql),cnt(0) {}

            template <typename O>
            void operator()(O const& x) 
            {
                 cnt++;
                if(cnt == 1) 
                    sql += ":" + x.second;
                else
                    sql += " , :" + x.second;
            }
        };

        // struct UpdateRow {
        // private:
        //     std::string& sql;

        // public:
        //     UpdateRow(std::string& sql) : sql(sql) {}

        //     template <typename O>
        //     void operator()(O const& x) const
        //     {
        //         if (!sql.empty()) {
        //             sql += ",";
        //         }
        //         sql += x.second + " = {}";
        //     }
        // };

        // struct UpdateRowVal {
        // private:
        //     std::string sql;

        // public:
        //     UpdateRowVal(std::string sql) : sql(sql) {}

        //     void operator()(auto x)
        //     {
        //         std::cout<<"sql = "<<sql<<"std::to_string = "<<std::to_string(*x);
        //         sql = fmt::format(sql,std::to_string(*x));
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
                part = "";
                if (!vec.empty()) {
                    part = ",";
                }
                part += x.second + " = ";
                vec.push_back(part);
            }
        };

    
        struct UpdateRowVal {
        private:
            std::vector<std::string>& vec;
            std::string part;
        public:
            UpdateRowVal(std::vector<std::string>& vec) : vec(vec) {}
        
            template <typename O>
            void operator()(O* x)
            {
                part = "";
                part += std::to_string(*x);
                vec.push_back(part);
            }

            // template<typename std::string>
            void operator()(std::string * x)
            {
                part = "";
                part += *x;
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
                // boost::fusion::for_each(vecs, SqlString<T>::CTForeachHelper(sql));
                sql += ")";
            }
            return sql;
        }

        std::string insertRowStr(T *obj) {
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            std::stringstream ss;
            ss << "INSERT INTO \"{}\" (";
            boost::fusion::for_each(vecs, SqlString<T>::InsertRowNames(&ss));
            ss << ") VALUES (";
            boost::fusion::for_each(vals, fill(ss));
            ss << ");";
            return ss.str();
        }

        static std::string const& dropTableStr() {
            static const std::string sql = "DROP TABLE IF EXISTS \"{}\"";
            return sql;
        }

        static std::string const& deleteRowStr(T *obj){
            static const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            auto &first_pair = boost::fusion::at_c<0>(vecs);
            static const std::string sql = "DELETE FROM \"{}\" WHERE " 
            + first_pair.second + " = " + std::to_string(*(boost::fusion::at_c<0>(vals)))+";";
            return sql;
        }


        std::string updateRowStr(T *obj) {
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            const auto vals = TypeMetaData<T>::getVal(obj);
            std::stringstream ss;
            std::string sql = "UPDATE \"{}\" SET ";
                std::string sql1;
                std::vector<std::string>v1,v2;
                // std::vector<std::string>sql_vec; // tobedone
                // for (auto& x : vecs) {
                //     if (!sql1.empty()) {
                //         sql1 += ",";
                //     }
                //     sql1 += x.second + " = {}";
                // }
                boost::fusion::for_each(vecs, SqlString<T>::UpdateRow(v1));
                boost::fusion::for_each(vals, SqlString<T>::UpdateRowVal(v2));
                for(unsigned long i = 0;i<v1.size();i++){
                    sql += v1[i] + v2[i];
                }
                auto &first_pair = boost::fusion::at_c<0>(vecs);
                sql += /*sql1 +*/ " WHERE "+ first_pair.second + " = " + std::to_string(*(boost::fusion::at_c<0>(vals)))+";";
                // std::cout<<"vecs[0].second == "<<first_pair.second<<"\n";
            return sql;
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

// template<typename T>
//     auto to_valid_query_string(T const& val) -> T const& {
//         T const& ret = val;
//         return ret;
//     }

//     auto to_valid_query_string(std::string const& val, std::string const sym = "\"") -> std::string {
//         const std::string ret_str = sym + val + sym;
//         return ret_str;
//     }

//     auto to_valid_query_string(const char* val, std::string const sym = "\"") -> std::string {
//         const std::string ret_str = sym + std::string(val) + sym;
//         return ret_str;
//     }

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
            FromBase(soci::values const& val) :_val(val), _count(2) {} //the num of obj to skip

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
            ToBase(soci::values& val) :_val(val), _count(2) {}

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
            // const edadb::SimpleOID& oid_ref = parent_oid;
            // const std::string& parent_table_reference = TypeMetaData<ObjType>::class_name();
            // TODO
            // using RetType = std::vector<std::pair<std::unique_ptr <O>, SimpleOID>>;
            // const RetType values = edadb::QueryHelper<O>::getAllNestedObjectssWithQuery("", oid_ref, parent_table_reference, obj_name);
            // if (!values.empty()) {
            //     x = *values.at(0).first;
            // }
            // else {
            //     l().error("FromBaseOperation::execute: Could not fetch object values");
            // }
        }
    };

    struct FromBase {
    private:
        soci::values const& _val;
        const edadb::SimpleOID& _oid;
        std::uint16_t _count;
        soci::indicator& _ind;

    public:
        FromBase(soci::values const& val, edadb::SimpleOID const& oid, soci::indicator& ind) :_val(val), _oid(oid), _count(2), _ind(ind) {}

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
            // const edadb::SimpleOID& oid_ref = parent_oid;
            // const std::string& parent_table_reference = TypeMetaData<ObjType>::class_name();
            // const std::string& parent_column_name = obj_name;
            // Do not delete if parent oid is nill. If parent
            // oid is nill then this object is not yet persisted.
            // if (oid_ref.empty() != 0) { 
                // Delete everything before inserting. There is no
                // update essentially. Just delete and insert of any nested objects.
                    // QueryHelper<O>::deleteObj(oid_ref, &x);
            // }
            // Get the oid of the nested object after persisting it
            // const edadb::SimpleOID oid = QueryHelper<O>::persistObj(&x, oid_ref, parent_table_reference, parent_column_name);
            // Get the json representation of the nested object and store it in the parent object
            // const std::string oids = oid.to_json();
            // TODO : Fix this. This has the context of current query execution. Needs to set the value in parent.
            // val.set<typename ConvertCPPTypeToSOCISupportType<std::string>::type>(obj_name, oids, ind);
        }
    };

    struct ToBase {
    private:
        soci::values& _val;
        // const edadb::SimpleOID& _oid;
        std::uint_fast32_t _count;
        soci::indicator& _ind;

    public:
        ToBase(soci::values& val, /*edadb::SimpleOID const& oid,*/ soci::indicator& ind) :_val(val), /*_oid(oid),*/ _count(2), _ind(ind) {}

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
        inline static void impl(TA& x, const soci::row& row, const std::string& member_name/*, const std::string& /*oid_ref*/) {
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
            _member_names(TypeMetaData<T>::member_names()), _count(2) {
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
            std::cout<<member_name<<"\n";
            
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
            std::cout<<sql<<"\n";
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
            std::cout<<"insertToDb: sql "<<sql<<"\n";
        #endif
        try{
            // DbBackend::i().session()<<sql, soci::use(obj_holder); //boost 
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
        const static std::string sql = fmt::format(SqlString<T>::deleteRowStr(obj), table_name);
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
        // SimpleObjHolder<T> obj_holder(obj);
        #ifdef EDADB_DEBUG
            std::cout<<sql<<"\n";
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
            std::cout<<sql<<std::endl;
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

    // template<typename T>
    // template<typename Q>
    // bool DbMap<T>::selectWithPK(Q PK_val, T *obj){
    //     static const std::string sql = fmt::format(SqlString<T>::selectRowStrPK(std::to_string(PK_val)),table_name);
    //     try {
    //         DbBackend::i().session() << sql,soci::into(*obj);
    //         return true;
    //     }
    //     catch (std::exception const& e) {
    //         std::cerr << "selectFromDbPK: "<< e.what() << "\n";
    //         return false;
    //     }
    // }

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

#define EXPAND_member_names_I(z, n, ELEMS_TUP) ,BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)

#define GET_first_member(ELEMS_TUP) BOOST_PP_TUPLE_ELEM(0,ELEMS_TUP )

#define GENERATE_ObjVal_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() &obj->BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)
#define GENERATE_ObjVal(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_ObjVal_I, CLASS_ELEMS_TUP)

// /// @brief generate the query and query fields
// #define DEFINE_CLASS_STATIC_VARS_QUERY_I(z, n, CLASS_ELEMS_TUP) boost::proto::terminal<edadb::query::QueryVariablePlaceholderIndex<n>>::type const F<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n, 1), CLASS_ELEMS_TUP);
// #define DEFINE_CLASS_STATIC_VARS_QUERY(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), DEFINE_CLASS_STATIC_VARS_QUERY_I, CLASS_ELEMS_TUP)

// #define GENERATE_CLASS_STATIC_VARS_QUERY_I(z, n, ELEMS_TUP) static boost::proto::terminal<edadb::query::QueryVariablePlaceholderIndex<n>>::type const BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP);
// #define GENERATE_CLASS_STATIC_VARS_QUERY(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), GENERATE_CLASS_STATIC_VARS_QUERY_I, ELEMS_TUP)

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

/// TABLE4CLASS Start
#define TABLE4CLASS(CLASS_ELEMS_TUP) BOOST_FUSION_ADAPT_STRUCT( BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP) ) \
namespace edadb{\
template<>\
struct CppTypeToDbType<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
    static const DbTypes ret = DbTypes::kComposite;\
};\
template<> struct IsComposite<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> : boost::mpl::bool_<true> {};\
template<> struct TypeMetaData<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
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
    inline static TupType getVal(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) * obj){\
        return TupType(GENERATE_ObjVal(CLASS_ELEMS_TUP));\
    }\
};\
}

#if 0
template<>\
inline auto to_json<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) const& value) -> std::string {\
return QueryHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>::objToJson(value);\
}\
}\
namespace edadb{ namespace query{\
namespace {\
template<>\
struct F<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> {\
GENERATE_CLASS_STATIC_VARS_QUERY(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
};\
DEFINE_CLASS_STATIC_VARS_QUERY(CLASS_ELEMS_TUP)\
}\
}
}
#endif
