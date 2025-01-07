

// #include "lib/edadb/edadb.hpp"
#include "macro.hpp"
#include <iostream>
#include </usr/local/include/soci/error.h>
#include </usr/local/include/soci/soci.h>
#include </usr/local/include/soci/sqlite3/soci-sqlite3.h>
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
#include "cpptypetodbtype.hpp"
// #include <soci/use.h>
// #include <soci/values-exchange.h>
// #include <soci/statement.h>
// #include <soci/once-temp-type.h>


template<typename T>
struct SimpleObjHolder {
    T* obj_ptr;
    SimpleObjHolder(T* obj_ptr_in) :obj_ptr(obj_ptr_in) {}
    ~SimpleObjHolder() {
        obj_ptr = nullptr;
    }
};



namespace test{
    template<typename T>
    struct ConvertCPPTypeToSOCISupportType {
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

    template<typename T>
    struct IsComposite : boost::mpl::bool_<false> {
    };

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
                    // const std::string obj_name = edadb::TypeMetaData<ObjType>::member_names().at(const_cast<FromBase*>(this)->_count++);
                    // x = _val.get<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name);
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
                    // const std::string obj_name = edadb::TypeMetaData<ObjType>::member_names().at(const_cast<ToBase*>(this)->_count++);
                    
                    // const_cast<ToBase*>(this)->_val.set(obj_name, x);
                }
            };

        public:
            inline static void to_base(ObjType const& obj, soci::values& v, soci::indicator& ind) {
                // only init the obj v ind
            }
        };


    template<typename T>
    struct type_conversion<SimpleObjHolder<T>> {
        using ObjectHolderType = SimpleObjHolder<T>;
        using ObjType = T;

    private:
        template<typename O, bool IsComposite = false>
        struct FromBaseOperation {
            // inline static void execute(O& x, const std::string& obj_name, soci::values const& val, const edadb::SimpleOID& /*parent_oid*/) {
            //     x = val.get<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name);
            // }
        };

        template<typename O>
        struct FromBaseOperation<O, true> {
            // inline static void execute(O& x, const std::string& obj_name, soci::values const& val, const edadb::SimpleOID& parent_oid) {
                
            // }
        };

        struct FromBase {
        private:
            soci::values const& _val;
            // const edadb::SimpleOID& _oid;
            std::uint16_t _count;
            soci::indicator& _ind;

        public:
            // FromBase(soci::values const& val, edadb::SimpleOID const& oid, soci::indicator& ind) :_val(val), _oid(oid), _count(2), _ind(ind) {}

            template<typename O>
            void operator()(O& x) const {
                // const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<FromBase*>(this)->_count++);
                // FromBaseOperation<O, edadb::IsComposite<O>::value>::execute(x, obj_name, _val, _oid);
            }
        };

    public:
        inline static void from_base(soci::values const& v, soci::indicator ind, ObjectHolderType& obj_holder) {
            // ObjType& obj = *(obj_holder.obj_ptr);
            // boost::fusion::for_each(obj, FromBase(v,edadb::SimpleOID(), ind));
        }

    private:
        template<typename O, bool IsComposite = false>
        struct ToBaseOperation {
            inline static void execute(O& x, const std::string& obj_name, soci::values& val, /*const edadb::SimpleOID& parent_oid,*/ soci::indicator& ind) {
                std::cout<<"val.set\n";
                val.set<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name, x, ind);
            }
        };

        template<typename O>
        struct ToBaseOperation<O, true> {
            // inline static void execute(O& x, const std::string& obj_name, soci::values& val, const edadb::SimpleOID& parent_oid, soci::indicator& ind) {
            //     std::cout<<"What happened in execute ToBaseOperation\n";
                
            // }
        };

        struct ToBase {
        private:
            soci::values& _val;
            std::uint_fast32_t _count;
            soci::indicator& _ind;

        public:
            ToBase(soci::values& val, soci::indicator& ind) :_val(val), _count(2), _ind(ind) {}

            // std::vector<std::string> mem_name = {"", "", "tmp", "width", "height"};
            template<typename O>
            void operator()(O& x)  const{
                // const std::string obj_name = mem_name[_count++];
                const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<ToBase*>(this)->_count++);
                ToBaseOperation<O, IsComposite<O>::value>::execute(x, obj_name, _val, _ind);
            }
        };


    public:
        inline static void to_base(ObjectHolderType& obj_holder, soci::values& v, soci::indicator& ind) {
            ObjType& obj = *(obj_holder.obj_ptr);
            boost::fusion::for_each(obj, ToBase(v,  ind));
        }
    };
} // namespace test

namespace soci{
    template<typename T>
    struct type_conversion<SimpleObjHolder<T>> {
        using ObjectHolderType = SimpleObjHolder<T>;
        // using ObjType = T;
        // typedef values base_type;
        using base_type = values;
        // using base_type = T;

        // in indicator out indicator -> enum i_ok i_null i_truncated. out = in
        inline static void from_base(soci::values const& v, soci::indicator ind, ObjectHolderType& obj_holder) { 
            // edadb::type_conversion<ObjectHolderType>::from_base(v, ind, obj_holder);
            std::cout<<"from_base\n";
        }

        inline static void to_base(ObjectHolderType& obj_holder, soci::values& v, soci::indicator& ind) { //in out indicator out = in
            test::type_conversion<ObjectHolderType>::to_base(obj_holder, v, ind);
            std::cout<<"ind = "<<ind<<"\n";
            std::cout<<"to_base\n";
        }
    };
}

/// @class IdbSite  // Layout 
struct IdbSite {
    int tmp;
    int width;
    double height;
    IdbSite(int a1 = 123,double a2 = 2.34,int t = 555) : width(a1), height(a2), tmp(t){}
    IdbSite& operator=(const IdbSite& p) {
        width = p.width;
        height = p.height;
        return *this;
    }
    
    
};

TABLE4CLASS( (IdbSite, width, height) );

struct func{
    // func(int v):val(v){};
    // int val;
    void operator()(auto val){
        std::cout<<val.second<<" ";
    }
};

int main() {
    
// using TupType = boost::fusion::vector<GENERATE_TupType(CLASS_ELEMS_TUP)>;\
// using TupTypePairType
    boost::fusion::for_each(test::TypeMetaData<IdbSite>::TupType,func());

    return 0;

    soci::session session;
    std::string sql;
    //connect
    const auto backend_factory = soci::sqlite3;
    std::string connect_str = "demo.db";
    session.open(backend_factory, connect_str);
    //create table
    try {
        sql = "CREATE TABLE IF NOT EXISTS \"IdbSite_table\" (width INTEGER PRIMARY KEY, height REAL);";
        session<< sql;
    }
    catch (std::exception const & e) {
        std::cerr << "createTable: " << e.what() << std::endl;
    }
    //insert to db
    IdbSite * obj = new IdbSite(6,7.7,8);
    const SimpleObjHolder<IdbSite> obj_holder(obj); 
    // SimpleObjHolder<IdbSite> obj_holder(obj); //如果不是 const 则还会调用from_base
    try{
        sql = "INSERT INTO \"IdbSite_table\" (width , height) VALUES (:width , :height)";
        session<<sql, soci::use(obj_holder); 
        std::cout<<"After use calling\n";
        // int w = 11;
        // double h = 2.2;
        // sql = "INSERT INTO \"IdbSite_table\" (width , height) VALUES (:width , :height)";
        // session<<sql, soci::use(w),
        // soci::use(h); 
    }
    catch (std::exception const& e) {
        std::cerr << "insertToDb: " << e.what() << "\n";
    }

	    
    return 0;
}

