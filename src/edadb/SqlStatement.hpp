/***
 * @file SqlStatement.hpp
 * @brief SqlStatement.hpp generates the SQL statement for the given type.
 */

#pragma once

#include <sstream>

namespace edadb {

template<class T>
struct SqlStatement {

// create table 
private: 
    struct CreateTableHelper {
    private:
        std::string& sql;
        int cnt;

    public:
        CreateTableHelper(std::string& sql) : sql(sql), cnt(0) {}

        template <typename O>
        void operator()(O const& x) 
        {
            // get O object type
            using ObjType = typename std::remove_const<typename std::remove_pointer
                    <typename O::first_type>::type >::type;
            
            // use ObjType to get cpptype using CppTypeToDbType
            // then use the cppType as template parameter to get the dbType
            std::string dbType = edadb::cppTypeToDbTypeString<
                    typename edadb::CppTypeToDbType<ObjType>::cppType >();

            if(cnt++ == 0) {
//                sql += x.second + " " + dbType + " PRIMARY KEY";  
                sql += x.second + " " + dbType;  
            } else {
                sql += ", " + x.second + " " + dbType;
            }
        }
    };

public:
    static std::string const& createTableStatement() {
        static const auto vecs = TypeMetaData<T>::tuple_type_pair();
        static std::string sql;
        if (sql.empty()) {
            sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
            auto p = new SqlStatement<T>::CreateTableHelper(sql); 
            boost::fusion::for_each(vecs, *p);
            sql += ")";
        }
        return sql;
    }


// insert 
private:
    struct InsertName {
    private:
        std::stringstream* ss;  
        int cnt;

    public:
        InsertName(std::stringstream* s) : ss(s), cnt(0) {}  

        template <typename O>
        void operator()(O const& x) 
        {
            if (cnt++ > 0) {
                *ss << ", ";
            }
            *ss << x.second;
        }
    };

    struct InsertValue {
        std::stringstream& ss;  
        int cnt;
        InsertValue(std::stringstream& s) : ss(s), cnt(0) {}  

        template<typename O>
        void operator()(O val) {
            if (cnt++ > 0) {
                ss << ", ";
            }
            ss << *val;
        }

        void operator()(std::string* val) {
            if (cnt++ > 0) {
                ss << ", ";

            }
            ss << "\'" << *val << "\'";
        }
    };

    struct InsertPlaceHolder {
        std::stringstream& ss;  
        int cnt;
        InsertPlaceHolder(std::stringstream& s) : ss(s), cnt(0) {}  

        template<typename O>
        void operator()(O val) {
            if (cnt++ > 0) {
                ss << ", ";
            }
            ss << "?";
        }
    };

public:
    /**
     * @brief Generate the insert statement with place holders.
     * @return The insert statement.
     */
    static std::string insertPlaceHolderStatement() {
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        std::stringstream ss;
        ss << "INSERT INTO \"{}\" (";
        boost::fusion::for_each(vecs, SqlStatement<T>::InsertName(&ss));
        ss << ") VALUES (";
        boost::fusion::for_each(vecs, InsertPlaceHolder(ss));
        ss << ");";
        return ss.str();
    }

    std::string insertStatement(T* obj) {
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        const auto vals = TypeMetaData<T>::getVal(obj);
        std::stringstream ss;
        ss << "INSERT INTO \"{}\" (";
        boost::fusion::for_each(vecs, SqlStatement<T>::InsertName(&ss));
        ss << ") VALUES (";
        boost::fusion::for_each(vals, InsertValue(ss));
        ss << ");";
        return ss.str();
    }

public:
    static std::string const &scanStatement() {
        static const auto vecs = TypeMetaData<T>::tuple_type_pair();
        static std::string sql;
        std::stringstream ss;
        if (sql.empty()) {
            // select col1, col2, ... from table_name;
            boost::fusion::for_each(vecs, SqlStatement<T>::InsertName(&ss));
            sql = "SELECT " + ss.str() + " FROM \"{}\";";
        }
        return sql;
    }
};




} // namespace edadb