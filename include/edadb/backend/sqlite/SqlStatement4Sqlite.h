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
#include "Cpp2SqlTypeTrait.h"
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
public: 
    /**
     * @brief create the table statement.
     * @param fk The foreign key constraint.
     * @return The create table statement.
     */
    static std::string createTableStatement(
                const std::string& table_name, const ForeignKey& fk) {

        std::string sql;
        sql = "CREATE TABLE IF NOT EXISTS \""+ table_name +"\" (";

        /*
         * get column name and type from TypeMetaData<T>::tuk'kle_type_pair()
         * NOTE:
         * 1. column name: use user defined name
         * 2. type: is the SQL type name string, such as "INTEGER", "TEXT", etc.
         */
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
        assert (name.size() == type.size());
        assert (name.size() > 0);

        // table columns by object member variables
        for (int i = 0; i < name.size(); ++i) {
            sql += (i == 0) ? (name[i] + " " + type[i] + " PRIMARY KEY") :
                (", " + name[i] + " " + type[i]);
        }

        // foreign key constraints
        if (fk.valid()) {
            assert (fk.column.size() == fk.type.size());

            // add foreign key columns to the child table
            std::vector<std::string> child_column; // referencing column 
            const std::string fk_pref = fk.table + "_";
            size_t fk_num = fk.column.size();
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
        return sql;
    } // createTableStatement


    /**
     * @brief Generate the insert statement with place holders.
     * @return The insert statement.
     */
    static std::string insertPlaceHolderStatement(
                const std::string& table_name, const ForeignKey& fk) {

        std::string sql;
        sql = "INSERT INTO \"" + table_name + "\" (";

        // get column and nested name
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
        assert (name.size() == type.size());
        assert (name.size() > 0);

        for (size_t i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", " : "") + name[i];
        }

        // foreign key columns
        if (fk.valid()) {
            assert(fk.column.size() == fk.type.size());

            // add foreign key columns to the child table
            for (size_t i = 0; i < fk.column.size(); ++i) {
                sql += ", " + (fk.table + "_" + fk.column[i]);
            } // for
        } 

        sql += ") VALUES (";

        // foreach name, append place holder for each column
        for (size_t i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", ?" : "?");
        }

        // has foreign key
        if (fk.valid()) {
            for (size_t i = 0; i < fk.column.size(); ++i) 
                sql += ", ?"; 
        }

        sql += ");";
        return sql;
    } // insertPlaceHolderStatement


    /**
     * @brief Generate the update statement with place holders.
     * @return The update statement.
     */
    static std::string updatePlaceHolderStatement(
                const std::string& table_name, const ForeignKey& fk) {

        std::string sql;
        sql = "UPDATE \"" + table_name + "\" SET ";

        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
        assert(name.size() == type.size());
        assert(name.size() > 0);

        for (int i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", " : "") + name[i] + " = ?";
        }
        if (fk.valid()) {
            assert(fk.column.size() == fk.type.size());

            // add foreign key columns to the child table
            for (size_t i = 0; i < fk.column.size(); ++i) {
                sql += ", " + (fk.table + "_" + fk.column[i]) + " = ?";
            } // for
        }
        sql += " WHERE " + name[0] + " = ?;";
        return sql;
    } // updatePlaceHolderStatement


    /**
     * @brief Generate the delete statement with place holders
     *          Only need to delete the record by primary key, which is the first column
     * @return The delete statement.
     */
    static std::string deletePlaceHolderStatement(const std::string& table_name) {
        std::string sql;
        sql = "DELETE FROM \"" + table_name + "\" WHERE ";
        auto name = TypeMetaData<T>::column_names();
        sql += name[0] + " = ?;";
        return sql;
    }


    /**
     * @brief Generate the project statement with all column names without tail ";"
     * @param fk The foreign key columns
     * @return The project statement with all column names, also include the foreign key columns
     */ 
    static std::string projectAllStatement(
                const std::string& table_name, const ForeignKey& fk) {

        // select col1, col2, ... from table_name;
        // colx is defined in TypeMetaData<T>::column_names() by TABLE4CLASS
        std::string sql;
        sql = "SELECT ";

        // query TypeMetaData<T> defined tuples
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type));
        assert (name.size() == type.size());
        assert (name.size() > 0);

        for (int i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", " : "") + name[i];
        }

        // foreign key columns
        if (fk.valid()) {
            assert (fk.column.size() == fk.type.size());

            // add foreign key columns to the child table
            for (size_t i = 0; i < fk.column.size(); ++i) {
                sql += ", " + (fk.table + "_" + fk.column[i]);
            } // for
        } 

        sql += " FROM \"" + table_name + "\""; // NO ";" at the end
        return sql;
    } // projectAllStatement


    /**
     * @brief Generate the scan statement with all column names
     * @param fk The foreign key columns
     * @return The scan statement with all column names
     */
    static std::string scanStatement(const std::string& table_name, const ForeignKey& fk) {
        std::string sql = projectAllStatement(table_name, fk);
        return sql += ";"; 
    } // scanStatement


    /**
     * @brief Generate the query statement with place holders using predicate text 
     * @param fk The foreign key columns
     * @param pred The predicate text
     * @return The query statement
     */
    static std::string queryPredicateStatement(const std::string& table_name,
                const ForeignKey& fk, const std::string& pred) {
        std::string sql = projectAllStatement(table_name, fk);
        sql += (pred.empty() ? "" : (" WHERE " + pred));
        return sql += ";";
    } // queryPredicateStatement 


    /**
     * @brief Generate the query statement with place holders using primary key
     * @param fk The foreign key columns
     * @return The query statement using primary key
    */
    static std::string queryPrimaryKeyStatement(const std::string& table_name,
                const ForeignKey& fk) {
        std::string sql = projectAllStatement(table_name, fk);

        // get the first as the primary key to query
        auto pk_name =
            TypeMetaData<T>::column_names()[Config::fk_ref_pk_col_index];
        sql += " WHERE " + pk_name + " = ?";

        return sql += ";";
    } // queryPrimaryKeyStatement


    /**
     * @brief Generate the query statement with place holders using foreign key
     * @param fk The foreign key columns
     * @return The query statement using foreign key
     */
    static std::string queryForeignKeyStatement(const std::string& table_name,
                const ForeignKey& fk) {
        std::string sql = projectAllStatement(table_name, fk);

        // get foreign key column name
        std::string fk_name = fk.table + "_" + fk.column[0];
        sql += " WHERE " + fk_name + " = ?";
        
        return sql += ";";
    } // queryForeignKeyStatement


public: // debug
    void print(T* obj1, T* obj2, const std::string &tn = "SqlStatementTestTable",
            const ForeignKey &fk = ForeignKey()) const {

        std::cout << "======== " << "SqlStatement <" << typeid(T).name() << "> ========" << std::endl;
        std::cout << "-------- Standard SQL statements --------" << std::endl;
        std::cout << "Create Table SQL: " << std::endl << "\t" 
            << createTableStatement(tn, fk) << std::endl;
        std::cout << "-------- SQL statements with Place Holder --------" << std::endl;
        std::cout << "Insert Place Holder SQL: " << std::endl << "\t" 
            << insertPlaceHolderStatement(tn, fk) << std::endl;
        std::cout << "ScanStatement SQL: " << std::endl << "\t"
            << scanStatement(tn, fk) << std::endl;
        std::cout << "Query Primary Key SQL: " << std::endl << "\t"
            << queryPrimaryKeyStatement(tn, fk) << std::endl;
        std::cout << "Query Predicate SQL: " << std::endl << "\t"
            << queryPredicateStatement(tn, fk, "col1 = ?") << std::endl;
        std::cout << "Update Place Holder SQL: " << std::endl << "\t"
            << updatePlaceHolderStatement(tn, fk) << std::endl;
        std::cout << "Delete Place Holder SQL: " << std::endl << "\t"
            << deletePlaceHolderStatement(tn) << std::endl;
    } // print
}; // SqlStatementImpl


// if Config::backend_type == SQLITE, use SqlStatement4Sqlite as SqlStatement
template<typename T>
using SqlStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
        SqlStatementImpl<DbBackendType::SQLITE, T> >;

} // namespace edadb