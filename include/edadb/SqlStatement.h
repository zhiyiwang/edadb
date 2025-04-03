/***
 * @file SqlStatement.hpp
 * @brief SqlStatement.hpp generates the SQL statement for the given type.
 */

#pragma once

#include <vector>   
#include <iostream>
#include <sstream>

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/vector.hpp>

#include "Cpp2SqlType.h"
#include "TypeMetaData.h"

namespace edadb {

/**
 * @struct SqlStatementBase
 * @brief This struct is the base class for generating SQL statements.
 * @details This class provides the basic functionality for generating SQL statements.
 */
struct SqlStatementBase {

protected:  // some utility functions

//////// Trans Binary Value to String ////////////////////////////////////
    /**
     * @brief Convert binary value to string template.
     * @param val The binary value.
     * @return The string value.
     */
    template <typename ValueType>
    static std::string binary2String(ValueType &val) {
        return std::to_string(val);
    }
    
    /**
     * @brief Convert string value to string.
     * @param val The binary value.
     * @return The string value.
     */
    static std::string binary2String(std::string &val) {
        return std::string("\'") + val + "\'";
    }


//////// Appenders //////////////////////////////////////////////////////

    struct Appender4Name {
    private:
        std::stringstream& ss;  
        int idx;

    public:
        Appender4Name(std::stringstream& s) : ss(s), idx(0) {}  

        template <typename ValueType>
        void operator()(ValueType const& x) {
            ss << (idx++ > 0 ? ", " : "") << x.second;
        }
    };


    struct Appender4Value {
    private:
        std::stringstream& ss;  
        int idx;

    public:
        Appender4Value(std::stringstream& s) : ss(s), idx(0) {}  

        template<typename ValueType>
        void operator()(ValueType val) {
            ss << (idx++ > 0 ? ", " : "") << *val;
        }

        void operator()(std::string* val) {
            ss << (idx++ > 0 ? ", " : "") << "\'" << *val << "\'";
        }
    };


    struct Appender4PlaceHolder {
    private:
        std::stringstream& ss;  
        int idx;

    public:
        Appender4PlaceHolder(std::stringstream& s) : ss(s), idx(0) {}  

        template<typename ValueType>
        void operator()(ValueType val) {
            ss << (idx++ > 0 ? ", " : "") << "?";
        }
    };
};


/**
 * @class SqlStatement
 * @brief This class generates the SQL statement for the given type.
 * @tparam T The class type.
 */
template<class T>
struct SqlStatement : public SqlStatementBase {

//////// Appenders //////////////////////////////////////////////////////
    /**
     * @brief Appender for name and type.
     * @param sql The SQL statement to append.
     * @param idx The index of the element.
     */
    struct Appender4NameType {
    private:
        std::string& sql;
        int idx;

    public:
        Appender4NameType(std::string& sql) : sql(sql), idx(0) {}

        template <typename ValueType>
        void operator()(ValueType const& x) 
        {
            // Column name:
            // use defined column name in TypeMetaData<T>::column_names()
            std::string name = TypeMetaData<T>::column_names()[idx];

            // ValueType:
            //   defined using boost::fusion::make_pair<int*>(std::string("name"))
            //   the type is boost::fusion::pair<int*, std::string> 
            //  the first_type is int*; the second_type is std::string, which is the member name
            using ElemType = typename ValueType::first_type;
            using CppType = typename std::remove_const<typename std::remove_pointer<ElemType>::type>::type;
            std::string sqlTypeString = edadb::getSqlTypeString<CppType> ();

            if(idx++ == 0) {
                sql += name + " " + sqlTypeString + " PRIMARY KEY";  
            } else {
                sql += ", " + name + " " + sqlTypeString;
            }
        }
    };


//////// Sql Statement /////////////////////////////////////////////////////////
public: // create table
    static std::string const& createTableStatement() {
        static std::string sql;
        if (sql.empty()) {
            sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, Appender4NameType(sql));
            sql += ")";
        }
        return sql;
    }


public: // insert
    /**
     * @brief Generate the insert statement with place holders.
     * @return The insert statement.
     */
    static std::string insertPlaceHolderStatement() {
        static std::string sql;
        if (sql.empty()) {
            std::stringstream ss;
            ss << "INSERT INTO \"{}\" (";
            auto names = TypeMetaData<T>::column_names();
            for (int i = 0; i < names.size(); ++i) {
                ss << (i > 0 ? ", " : "") << names[i];
            }
            ss << ") VALUES (";
            for (int i = 0; i < names.size(); ++i) {
                ss << (i > 0 ? ", " : "") << "?";
            }
            ss << ");";
            sql.assign(ss.str());
        }
        return sql;
    }

    std::string insertStatement(T* obj) {
        std::stringstream ss;
        ss << "INSERT INTO \"{}\" (";
        auto names = TypeMetaData<T>::column_names();
        for (int i = 0; i < names.size(); ++i) {
            ss << (i > 0 ? ", " : "") << names[i];
        }
        ss << ") VALUES (";
        const auto vals = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(vals, Appender4Value(ss));
        ss << ");";
        return ss.str();
    }


public: // scan
    static std::string const &scanStatement() {
        // select col1, col2, ... from table_name;
        static std::string sql;
        if (sql.empty()) {
            std::stringstream ss;
            auto names = TypeMetaData<T>::column_names();
            for (int i = 0; i < names.size(); ++i) {
                ss << (i > 0 ? ", " : "") << names[i];
            }
            sql = "SELECT " + ss.str() + " FROM \"{}\";";
        }
        return sql;
    }


public: // lookup by primary key
    static std::string const lookupStatement(T* obj) {
        static std::string sql;
        if (sql.empty()) {
            std::stringstream ss;
            auto names = TypeMetaData<T>::column_names();
            for (int i = 0; i < names.size(); ++i) {
                ss << (i > 0 ? ", " : "") << names[i];
            }
            sql = "SELECT " + ss.str() + " FROM \"{}\" WHERE ";

            auto pk_name = names[0]; // primary key is the first element
            sql += pk_name + " = ";
        }

        const auto vals = TypeMetaData<T>::getVal(obj);
        auto pk_val_ptr = boost::fusion::at_c<0>(vals);
        auto pk_val_str = binary2String(*pk_val_ptr);
        return sql + pk_val_str + ";";
    }


public: // delete by primary key
    static std::string const deleteStatement(T* obj) {
        static std::string sql_prefix;
        if (sql_prefix.empty()) {
            sql_prefix = "DELETE FROM \"{}\" WHERE ";

            // primary key is the first element in the tuple
            sql_prefix += TypeMetaData<T>::column_names()[0] + " = ";
        }

        const auto vals = TypeMetaData<T>::getVal(obj);
        auto pk_val_ptr = boost::fusion::at_c<0>(vals);
        auto pk_val_str = binary2String(*pk_val_ptr);
        return sql_prefix + pk_val_str + ";";
    }


private: // update by primary key
    struct UpdateNames {
    private:
        std::vector<std::string>& names;

    public:
        UpdateNames(std::vector<std::string>& n) : names(n) {}

        template <typename ValueType>
        void operator()(ValueType const& x) {
            names.push_back(x.second);
        }
    };

    struct UpdateValues {
    private:
        std::vector<std::string>& values;
    
    public:
        UpdateValues(std::vector<std::string>& v) : values(v) {}

        template <typename ValueType>
        void operator()(ValueType& x) {
            values.push_back(binary2String(*x));
        }
    };


public: 
    static std::string const updateStatement(T* org_obj, T* new_obj) {
        std::string sql = "UPDATE \"{}\" SET ";

        // get names and values
        std::vector<std::string> names;
        names = TypeMetaData<T>::column_names();

        std::vector<std::string> values;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(TypeMetaData<T>::getVal(new_obj), UpdateValues(values));

        // generate sql
        const uint32_t num = names.size();
        for (uint32_t i = 0; i < num; ++i) {
            sql += names[i] + " = " + values[i] + (i + 1 < num ? ", " : "");
        }

        // get primary key name and value
        auto pk_val_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(org_obj));
        auto pk_val_str = binary2String(*pk_val_ptr);
        sql += " WHERE " + names[0] + " = " + pk_val_str + ";";
        return sql;
    }


public: // debug
    void print(T* obj1, T* obj2) {
        std::cout << "SqlStatement<" << typeid(T).name() << ">" << " SQLs:" << std::endl;
        std::cout << "Create Table SQL: " << std::endl << "\t" 
            << createTableStatement() << std::endl;
        std::cout << "Insert Place Holder SQL: " << std::endl << "\t" 
            << insertPlaceHolderStatement() << std::endl;
        std::cout << "Insert SQL: " << std::endl << "\t" 
            << insertStatement(obj1) << std::endl;
        std::cout << "Insert SQL: " << std::endl << "\t" 
            << insertStatement(obj2) << std::endl;
        std::cout << "Scan SQL: " << std::endl << "\t" 
            << scanStatement() << std::endl;
        std::cout << "Lookup SQL: " << std::endl << "\t"
            << lookupStatement(obj1) << std::endl;
        std::cout << "Delete SQL: " << std::endl << "\t"
            << deleteStatement(obj1) << std::endl;
        std::cout << "Delete SQL: " << std::endl << "\t"
            << deleteStatement(obj2) << std::endl;
        std::cout << "Update SQL: " << std::endl << "\t"
            << updateStatement(obj1, obj2) << std::endl;
        std::cout << std::endl << std::endl;
    }
};


} // namespace edadb