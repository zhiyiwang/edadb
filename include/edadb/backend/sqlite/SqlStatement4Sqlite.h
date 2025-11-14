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

#include "edadb/Config.h"
#include "edadb/Cpp2SqlTypeTrait.h"
#include "edadb/TypeMetaData.h"
#include "edadb/VecMetaData.h"    
#include "edadb/SqlStatement.h"

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
     * @param this_fkc  this foreign key constraint.
     * @param work_fkc  work foreign key constraint.
     * @return The create table statement.
     */
    static std::string createTableStatement(
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc) {
        assert(this_fkc.fore_tab_name == work_fkc.prim_tab_name);

        std::string sql;
        sql = "CREATE TABLE IF NOT EXISTS \""+ this_fkc.fore_tab_name + "\" (";

        /*
         * get column name and type from TypeMetaData<T>::tuple_type_pair()
         * NOTE:
         * 1. column name: use user defined name
         * 2. type: is the SQL type name string, such as "INTEGER", "TEXT", etc.
         */
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type, work_fkc));
        assert(name.size() == type.size());
        assert(name.size() > 0);

        // Cpp2SqlTypeTrait<T> of class T decide whether define primary key
        sql += name[0] + " " + type[0];
        sql += Cpp2SqlTypeTrait<T>::hasPrimKey ? " PRIMARY KEY" : "";
        for (std::size_t i = 1; i < name.size(); ++i) {
            sql += ", " + name[i] + " " + type[i];
        }

        // add primary key columns for member object variables
        std::vector<std::string> pk_cols;
        std::vector<std::string> pk_types;
        collectNestedPkColumns(pk_cols, pk_types, work_fkc);
        assert(pk_cols.size() == pk_types.size());
        for (std::size_t i = 0; i < pk_cols.size(); ++i) {
            sql += ", " + pk_cols[i] + " " + pk_types[i];
        }


        // foreign key constraint
        if (this_fkc.valid()) {
            // add foreign key columns
            // must be only one foreign unique column
            sql += ", " + this_fkc.fore_col_name + " " + this_fkc.key_type;

            // add foreign key constraint
            sql += ", FOREIGN KEY (" + this_fkc.fore_col_name;
            sql += ") REFERENCES " + this_fkc.prim_tab_name;
            sql += " (" + this_fkc.prim_col_name + ") ON DELETE CASCADE ON UPDATE CASCADE";
        } // if 

        sql += ");";
        return sql;
    } // createTableStatement


    /**
     * @brief Generate the insert statement with place holders.
     * @return The insert statement.
     */
    static std::string insertPlaceHolderStatement(
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc) {
        assert(this_fkc.fore_tab_name == work_fkc.prim_tab_name);

        std::string sql;
        sql = "INSERT INTO \"" + this_fkc.fore_tab_name + "\" (";

        // get column and nested name
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type, work_fkc));
        assert (name.size() == type.size());
        assert (name.size() > 0);

        for (size_t i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", " : "") + name[i];
        }


        // add primary key columns for member object variables
        std::vector<std::string> pk_cols;
        std::vector<std::string> pk_types;
        collectNestedPkColumns(pk_cols, pk_types, work_fkc);
        assert(pk_cols.size() == pk_types.size());
        for (std::size_t i = 0; i < pk_cols.size(); ++i) {
            sql += ", " + pk_cols[i];
        }


        // foreign key constraint
        if (this_fkc.valid()) {
            // only one foreign unique column
            sql += ", " + this_fkc.fore_col_name;
        } 

        sql += ") VALUES (";

        // foreach name, append place holder for each column
        for (size_t i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", ?" : "?");
        }

        // foreach primary key member variable, append one place holder 
        for (size_t i = 0; i < pk_cols.size(); ++i) {
            sql += ", ?"; 
        } // for

        // has foreign key
        if (this_fkc.valid()) {
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
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc) {
        assert(this_fkc.fore_tab_name == work_fkc.prim_tab_name);

        std::string sql;
        sql = "UPDATE \"" + this_fkc.fore_tab_name + "\" SET ";

        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type, work_fkc));
        assert(name.size() == type.size());
        assert(name.size() > 0);

        for (int i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", " : "") + name[i] + " = ?";
        }


        // add primary key columns for member object variables
        std::vector<std::string> pk_cols;
        std::vector<std::string> pk_types;
        collectNestedPkColumns(pk_cols, pk_types, work_fkc);
        assert(pk_cols.size() == pk_types.size());
        for (std::size_t i = 0; i < pk_cols.size(); ++i) {
            // append place holder for each primary key
            sql += ", " + pk_cols[i] + " = ?"; 
        }


        if (this_fkc.valid()) {
            sql += ", " + this_fkc.fore_col_name + " = ?";
//                sql += ", " + (fk.table + "_" + fk.column[i]);
        }
        sql += " WHERE " + name[0] + " = ?;";
        return sql;
    } // updatePlaceHolderStatement


    /**
     * @brief Generate the delete statement with place holders
     *          Only need to delete the record by primary key, which is the first column
     * @return The delete statement.
     */
    static std::string deletePlaceHolderStatement(const ForeignKeyConstraint& this_fkc) {
        std::string sql;
        sql = "DELETE FROM \"" + this_fkc.fore_tab_name + "\" WHERE ";
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
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc) {
        assert(this_fkc.fore_tab_name == work_fkc.prim_tab_name);

        // select col1, col2, ... from table_name;
        // colx is defined in TypeMetaData<T>::column_names() by TABLE4CLASS
        std::string sql;
        sql = "SELECT ";

        // query TypeMetaData<T> defined tuples
        std::vector<std::string> name, type;
        const auto vecs = TypeMetaData<T>::tuple_type_pair();
        boost::fusion::for_each(vecs, ColumnNameType<T>(name, type, work_fkc));
        assert (name.size() == type.size());
        assert (name.size() > 0);

        for (size_t i = 0; i < name.size(); ++i) {
            sql += (i > 0 ? ", " : "") + name[i];
        }


        // add primary key columns for member object variables
        std::vector<std::string> pk_cols;
        std::vector<std::string> pk_types;
        collectNestedPkColumns(pk_cols, pk_types, work_fkc);
        assert(pk_cols.size() == pk_types.size());
        for (std::size_t i = 0; i < pk_cols.size(); ++i) {
            // append place holder for each primary key
            sql += ", " + pk_cols[i];
        }


        // foreign key columns
        if (this_fkc.valid()) {
            sql += ", " + this_fkc.fore_col_name;
//            // add foreign key columns to the child table
//            for (size_t i = 0; i < fk.column.size(); ++i) {
//                sql += ", " + (fk.table + "_" + fk.column[i]);
//            } // for
        } 

        sql += " FROM \"" + this_fkc.fore_tab_name + "\""; // NO ";" at the end
        return sql;
    } // projectAllStatement


    /**
     * @brief Generate the scan statement with all column names
     * @param fk The foreign key columns
     * @return The scan statement with all column names
     */
    static std::string scanStatement(
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc) {
        std::string sql = projectAllStatement(this_fkc, work_fkc);
        return sql += ";"; 
    } // scanStatement


    /**
     * @brief Generate the query statement with place holders using predicate text 
     * @param fk The foreign key columns
     * @param pred The predicate text
     * @return The query statement
     */
    static std::string queryPredicateStatement(
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc, const std::string& pred) {
        std::string sql = projectAllStatement(this_fkc, work_fkc);
        sql += (pred.empty() ? "" : (" WHERE " + pred));
        return sql += ";";
    } // queryPredicateStatement 


    /**
     * @brief Generate the query statement with place holders using primary key
     * @param fk The foreign key columns
     * @return The query statement using primary key
    */
    static std::string queryPrimaryKeyStatement(
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc) {
        std::string sql = projectAllStatement(this_fkc, work_fkc);

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
    static std::string queryForeignKeyStatement(
            const ForeignKeyConstraint& this_fkc, ForeignKeyConstraint& work_fkc) {
        std::string sql = projectAllStatement(this_fkc, work_fkc);

        // get foreign key column name
        sql += " WHERE " + this_fkc.fore_col_name + " = ?";
//        std::string fk_name = fkc.prim_tab_name + "_" + fkc.prim_col_pref;
//        sql += " WHERE " + fk_name + " = ?";
        
        return sql += ";";
    } // queryForeignKeyStatement

private:
    /**
     * @brief Collect the nested primary key columns for member object variables.
     * @param pk_names The primary key column names.
     * @param pk_types The primary key column SQL types.
     * @param work_fkc The working foreign key constraint.
     */
    static void collectNestedPkColumns(std::vector<std::string>& pk_names,
                std::vector<std::string>& pk_types, ForeignKeyConstraint& work_fkc)
    {
        std::size_t idx = 0;
        const auto& pk_col_name = TypeMetaData<T>::pk_column_names();
        const auto& pk_mem_vec  = TypeMetaData<T>::pk_tuple_type_pair();
        const std::size_t N     = pk_col_name.size();
        boost::fusion::for_each(pk_mem_vec, [&](auto const& member_pair) {
            const std::string& col_name = pk_col_name[idx];

            using Pair = std::decay_t<decltype(member_pair)>;
//            using Pair = typename std::remove_reference<decltype(member_pair)>::type;
            using PkMemElemType = typename Pair::first_type;
            using PkMemDefType  = typename remove_const_and_pointer<PkMemElemType>::type;
            using PkMemCppType  = typename TypeInfoTrait<PkMemDefType>::CppType;
            static_assert(!TypeInfoTrait<PkMemDefType>::is_vector,
                "SqlStatementImpl::createTableStatement: PkMemElemType cannot be a vector type");

            const auto& nested_col_names = TypeMetaData<PkMemCppType>::column_names();
            assert(!nested_col_names.empty());
            const std::string& nested_pk_col = nested_col_names[0];

            std::string prefix   = work_fkc.getPrimaryColumnFullName(col_name);
            std::string full_col = prefix + "_" +
                    TypeMetaData<PkMemCppType>::table_name() + "_" + nested_pk_col;

            using NestedPair = std::decay_t<
                decltype(boost::fusion::at_c<0>(TypeMetaData<PkMemCppType>::tuple_type_pair()))
            >;
//            using NestedPair = std::remove_reference_t<
//                decltype(boost::fusion::at_c<0>(TypeMetaData<PkMemCppType>::tuple_type_pair()))
            using PkMemPkElemType = typename NestedPair::first_type;
            using PkMemPkDefType  = typename remove_const_and_pointer<PkMemPkElemType>::type;
            using PkMemPkCppType  = typename TypeInfoTrait<PkMemPkDefType>::CppType;

            std::string sql_type = getSqlTypeString<PkMemPkCppType>();

            pk_names.push_back(std::move(full_col));
            pk_types.push_back(std::move(sql_type));

            ++idx;
        });

        assert(idx == N);
    } // collectNestedPkColumns


public: // debug
    void print(T* obj1, T* obj2, const std::string &tn = "SqlStatementTestTable",
            ForeignKeyConstraint &tfk = ForeignKeyConstraint(),
            ForeignKeyConstraint &wfk = ForeignKeyConstraint()) {

        std::cout << "======== " << "SqlStatement <" << typeid(T).name() << "> ========" << std::endl;
        std::cout << "-------- Standard SQL statements --------" << std::endl;
        std::cout << "Create Table SQL: " << std::endl << "\t" 
            << createTableStatement(tfk, wfk) << std::endl;
        std::cout << "-------- SQL statements with Place Holder --------" << std::endl;
        std::cout << "Insert Place Holder SQL: " << std::endl << "\t" 
            << insertPlaceHolderStatement(tfk, wfk) << std::endl;
        std::cout << "ScanStatement SQL: " << std::endl << "\t"
            << scanStatement(tfk, wfk) << std::endl;
        std::cout << "Query Primary Key SQL: " << std::endl << "\t"
            << queryPrimaryKeyStatement(tfk, wfk) << std::endl;
        std::cout << "Query Predicate SQL: " << std::endl << "\t"
            << queryPredicateStatement(tfk, wfk, "col1 = ?") << std::endl;
        std::cout << "Update Place Holder SQL: " << std::endl << "\t"
            << updatePlaceHolderStatement(tfk, wfk) << std::endl;
        std::cout << "Delete Place Holder SQL: " << std::endl << "\t"
            << deletePlaceHolderStatement(tfk) << std::endl;
    } // print
}; // SqlStatementImpl


// if Config::backend_type == SQLITE, use SqlStatement4Sqlite as SqlStatement
template<typename T>
using SqlStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
        SqlStatementImpl<DbBackendType::SQLITE, T> >;

} // namespace edadb