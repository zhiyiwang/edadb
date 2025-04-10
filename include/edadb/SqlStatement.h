/***
 * @file SqlStatement.hpp
 * @brief SqlStatement.hpp generates the SQL statement for the given type.
 */

#pragma once

#include <assert.h>
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


//////// Column Infos //////////////////////////////////////////////////////
    /**
     * @brief Appender for column names to the vector.
     */
    struct ColumnNameType {
    private:
        std::vector<std::string>& name;
        std::vector<std::string>& type;

    public:
        ColumnNameType(std::vector<std::string>& n, std::vector<std::string>& t) : name(n), type(t) {}

        /**
         * @brief Appender for column names to the vector.
         * @param x is TypeMetaData::tuple_type_pair() element:
         *     x.first  - pointer to the member variable
         *     x.second - the member name
         */
        template <typename TuplePair>
        void operator()(TuplePair const& x) {
            // use the type of the first element to get the SQL type string
            using ElemType = typename TuplePair::first_type;
            using CppType  = typename std::remove_const<typename std::remove_pointer<ElemType>::type>::type;
            std::string sqlTypeString = edadb::getSqlTypeString<CppType> ();
            type.push_back(sqlTypeString);

            // use the variable name as the column name
            name.push_back(x.second);
        }
    };

    /**
     * @brief Appender for column names to the vector.
     */
    struct ColumnValues {
    private:
        std::vector<std::string>& values;
    
    public:
        ColumnValues(std::vector<std::string>& v) : values(v) {}

        /**
         * @brief Appender for column values to the vector.
         * @param v is TypeMetaData::TupType element:
         *     v  - pointer to the member variable, such as obj->name
         */
        template <typename ValueType>
        void operator()(ValueType& v) {
            values.push_back(binary2String(*v));
        }
    };
}; // SqlStatementBase



/**
 * @class SqlStatement
 * @brief This class generates the SQL statement for the given type.
 * @tparam T The class type.
 */
template<class T>
struct SqlStatement : public SqlStatementBase {

//////// Standard SQL Statements //////////////////////////////////////
public: 
    static std::string const& createTableStatement() {
        static std::string sql;
        if (sql.empty()) {
            /*
             * get column names and types from TypeMetaData<T>::tuple_type_pair()
             * NOTE:
             * 1. name is the name of the member variable, we use user defined column name instead
             * 2. type is the SQL type name string, such as "INTEGER", "TEXT", etc.
             */
            std::vector<std::string> name, type;
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, ColumnNameType(name, type));

            // get and check column names
            const std::vector<std::string>& col_name = TypeMetaData<T>::column_names();
            assert(name.size() == col_name.size());

            sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";
            for (int i = 0; i < col_name.size(); ++i) {
                sql += (i == 0) ? (col_name[i] + " " + type[i] + " PRIMARY KEY") :
                    (", " + col_name[i] + " " + type[i]);
            }
            sql += ")";
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
        std::vector<std::string> values;
        boost::fusion::for_each(TypeMetaData<T>::getVal(obj), ColumnValues(values));
        for (int i = 0; i < values.size(); ++i) {
            ss << (i > 0 ? ", " : "") << values[i];
        }
        ss << ");";
        return ss.str();
    }


    static std::string const &scanStatement() {
        // select col1, col2, ... from table_name;
        // colx is defined in TypeMetaData<T>::column_names() by TABLE4CLASS
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


    static std::string const updateStatement(T* org_obj, T* new_obj) {
        std::string sql = "UPDATE \"{}\" SET ";

        // get names and values
        std::vector<std::string> names;
        names = TypeMetaData<T>::column_names();

        std::vector<std::string> values;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(TypeMetaData<T>::getVal(new_obj), ColumnValues(values));

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


//////// SQL Statements using Place Holder ////////////////////////
public:
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

    /**
     * @brief Generate the update statement with place holders.
     * @return The update statement.
     */
    static std::string updatePlaceHolderStatement() {
        static std::string sql;
        if (sql.empty()) {
            sql = "UPDATE \"{}\" SET ";
            auto names = TypeMetaData<T>::column_names();
            for (int i = 0; i < names.size(); ++i) {
                sql += (i > 0 ? ", " : "") + names[i] + " = ?";
            }
            sql += " WHERE " + names[0] + " = ?;";
        }
        return sql;
    }

    /**
     * @brief Generate the delete statement with place holders.
     * @return The delete statement.
     */
    static std::string deletePlaceHolderStatement() {
        static std::string sql;
        if (sql.empty()) {
            sql = "DELETE FROM \"{}\" WHERE ";
            auto names = TypeMetaData<T>::column_names();
            sql += names[0] + " = ?;";
        }
        return sql;
    }



public: // debug
    void print(T* obj1, T* obj2) {
        std::cout << "======== " << "SqlStatement <" << typeid(T).name() << "> ========" << std::endl;
        std::cout << "-------- Standard SQL statements --------" << std::endl;
        std::cout << "Create Table SQL: " << std::endl << "\t" 
            << createTableStatement() << std::endl;
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
        std::cout << std::endl;

        std::cout << "-------- SQL statements with Place Holder --------" << std::endl;
        std::cout << "Insert Place Holder SQL: " << std::endl << "\t" 
            << insertPlaceHolderStatement() << std::endl;
        std::cout << "Update Place Holder SQL: " << std::endl << "\t"
            << updatePlaceHolderStatement() << std::endl;
        std::cout << "Delete Place Holder SQL: " << std::endl << "\t"
            << deletePlaceHolderStatement() << std::endl;
    }
};


} // namespace edadb