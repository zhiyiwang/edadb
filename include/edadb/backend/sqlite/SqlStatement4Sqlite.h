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

#include "Config.h"
#include "Cpp2SqlType.h"
#include "TypeMetaData.h"
#include "VecMetaData.h"    
#include "SqlStatement.h"

namespace edadb {

/**
 * @class SqlStatement
 * @brief This class generates the SQL statement for the given type.
 * @tparam T The class type.
 */
template<typename T>
struct SqlStatementImpl<DbBackendType::SQLITE, T> : public SqlStatementBase {
    
//////// Standard SQL Statements //////////////////////////////////////
public: 
    std::string createTableStatement(const ForeignKey& fk = ForeignKey()) {
        std::string sql;

        /*
         * get column name and type from TypeMetaData<T>::tuple_type_pair()
         * NOTE:
         * 1. column name: use user defined name
         * 2. type: is the SQL type name string, such as "INTEGER", "TEXT", etc.
         */
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));

        sql = "CREATE TABLE IF NOT EXISTS \"{}\" (";

        // table columns by object member variables
        for (int i = 0; i < name.size(); ++i) {
            sql += (i == 0) ? (name[i] + " " + type[i] + " PRIMARY KEY") :
                (", " + name[i] + " " + type[i]);
        }
        assert (name.size() == type.size());
        assert (name.size() > 0);

        // foreign key constraints
        if (fk.valid()) {
            assert (fk.column.size() == fk.type.size());

            const std::string fk_pref = fk.table + "_";
            size_t fk_num = fk.column.size();

            // add foreign key columns to the child table
            std::vector<std::string> child_column; // referencing column 
            for (size_t i = 0; i < fk_num; ++i) {
                child_column.push_back(fk_pref + fk.column[i]);
                sql += ", " + child_column.back() + " " + fk.type[i];
            } // for

            // add foreign key constraint
            sql += ", FOREIGN KEY (";
            for (size_t i = 0; i < fk_num; ++i) {
                sql += (i > 0 ? ", " : "") + child_column[i];
            } // for
            sql += ") REFERENCES " + fk.table + "(";
            for (size_t i = 0; i < fk_num; ++i) {
                sql += (i > 0 ? ", " : "") + fk.column[i];
            } // for
            sql += ") ON DELETE CASCADE ON UPDATE CASCADE";
        } // if 

        sql += ");";

//        std::cout << "[DEADB DEBUG]::create table sql statement: " << std::endl;
//        std::cout << "[DEADB DEBUG]::    " << sql << std::endl;
        return sql;
    }


    std::string insertStatement(T* obj) {
        std::stringstream ss;
        ss << "INSERT INTO \"{}\" (";

        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
        for (int i = 0; i < name.size(); ++i) {
            ss << (i > 0 ? ", " : "") << name[i];
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
            std::vector<std::string> name, type;
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
            for (int i = 0; i < name.size(); ++i) {
                ss << (i > 0 ? ", " : "") << name[i];
            }
            sql = "SELECT " + ss.str() + " FROM \"{}\";";
        }
        return sql;
    }


    static std::string const lookupStatement(T* obj) {
        static std::string sql;
        if (sql.empty()) {
            std::stringstream ss;
            std::vector<std::string> name, type;
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
            for (int i = 0; i < name.size(); ++i) {
                ss << (i > 0 ? ", " : "") << name[i];
            }
            sql = "SELECT " + ss.str() + " FROM \"{}\" WHERE ";

            auto pk_name = name[0]; // primary key is the first element
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

        // get name and values
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));

        std::vector<std::string> values;
        boost::fusion::for_each(TypeMetaData<T>::getVal(new_obj), ColumnValues(values));

        // generate sql
        const size_t num = name.size();
        for (size_t i = 0; i < num; ++i) {
            sql += name[i] + " = " + values[i] + (i + 1 < num ? ", " : "");
        }

        // get primary key name and value
        auto pk_val_ptr = boost::fusion::at_c<0>(TypeMetaData<T>::getVal(org_obj));
        auto pk_val_str = binary2String(*pk_val_ptr);
        sql += " WHERE " + name[0] + " = " + pk_val_str + ";";
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

            // get column and nested name
            std::vector<std::string> name, type;
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
            for (int i = 0; i < name.size(); ++i) {
                ss << (i > 0 ? ", " : "") << name[i];
            }
            ss << ") VALUES (";
            // foreach name, append place holder for each column
            for (int i = 0; i < name.size(); ++i) {
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

            std::vector<std::string> name, type;
            const auto vecs = TypeMetaData<T>::tuple_type_pair();
            boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
            for (int i = 0; i < name.size(); ++i) {
                sql += (i > 0 ? ", " : "") + name[i] + " = ?";
            }
            sql += " WHERE " + name[0] + " = ?;";
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
            auto name = TypeMetaData<T>::column_names();
            sql += name[0] + " = ?;";
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



// if Config::backend_type == SQLITE, use SqlStatement4Sqlite as SqlStatement
template<typename T>
using SqlStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
        SqlStatementImpl<DbBackendType::SQLITE, T> >;

} // namespace edadb