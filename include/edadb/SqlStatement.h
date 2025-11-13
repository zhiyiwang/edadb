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

#include "Shadow.h"
#include "DbBackendType.h"
#include "TypeInfoTrait.h"
#include "TypeMetaData.h"

namespace edadb {


//////// Table Relations //////////////////////////////////////////////////////
/**
 * @brief Foreign keys constraint.
 */
class ForeignKeyConstraint {
public: // constraint info
    std::string prim_tab_name = ""; // primary (parent) key table
    std::string prim_col_name = ""; // primary key column name, using prim_col_pref 

    std::string fore_tab_name = ""; // foreign (child) key table name, using fore_tab_pref
    std::string fore_col_name = ""; // foreign key column name, single column name only

    std::string key_type = ""; // foreign key type string

public: // prefix for column/table names during recursion
    std::string prim_col_pref = ""; // primary key column prefix
    std::string fore_tab_pref = ""; // foreign (child) key table prefix

public:
    ~ForeignKeyConstraint() = default;
    ForeignKeyConstraint() = default;
    ForeignKeyConstraint(const ForeignKeyConstraint& fkc) = default;

public:
    bool valid() const { return key_type.empty() == false; }

public:
    /**
     * @brief Get the full primary key column name with prefix.
     * @param last_name The last part of the column name.
     * @return The full primary key column name.
    */
    std::string getPrimaryColumnFullName(const std::string& last_name) const {
        return prim_col_pref.empty() ? last_name : prim_col_pref + "_" + last_name;
    } // getPrimaryColumnFullName

    /**
     * @brief Seal the foreign table full name with prefix.
     * @param last_name The last part of the column name.
     */
    void sealForeignColumnFullName(const std::string& last_name) {
        fore_col_name =
            prim_col_pref.empty() ? last_name : prim_col_pref + "_" + last_name;
    }

    void sealForeignTableFullName(void) {
        fore_tab_name = fore_tab_pref;
    } // getForeignTableFullName

    /**
     * @brief update the prefix for primary key column name and foreign table name.
     * @param pmem The member variable name in the parent class.
     * @param fore_tab The foreign (child) table name.
     */
    void updatePrefix(const std::string& pmem, const std::string& fore_tab) {
        // "[member names]" splited by "_"
        std::string pref = prim_col_pref.empty() ? "" : "_";
        prim_col_pref.append(pref + pmem);

        // "[table name]_[member name]" and splited by "_"
        assert(!fore_tab.empty());
        fore_tab_pref += "_" + pmem + "_" + fore_tab;
    } // updatePrefix
    
public:
    void print(const std::string& pref = "") const {
        std::cout << pref << "ForeignKeyConstraint:" << std::endl;
        std::cout << pref << "  Primary Table: " << prim_tab_name << std::endl;
        std::cout << pref << "    prim_col_name=" << prim_col_name << std::endl;
        std::cout << pref << "    prim_col_pref=" << prim_col_pref << std::endl;
        std::cout << pref << "  Foreign Table: " << fore_tab_pref << std::endl;
        std::cout << pref << "    fore_tab_name=" << fore_tab_name << std::endl;
        std::cout << pref << "    fore_col_name=" << fore_col_name << std::endl;
        std::cout << pref << "    fore_tab_pref=" << fore_tab_pref << std::endl;
        std::cout << pref << "    key_type=" << key_type << std::endl;
        std::cout << std::endl;
    } // print
}; // ForeignKeyConstraint

using FKC = ForeignKeyConstraint;



/**
 * @struct SqlStatementBase
 * @brief This struct is the base class for generating SQL statements.
 * @details This class provides the basic functionality for generating SQL statements.
 */
struct SqlStatementBase {
protected:  // some utility functions

//////// Column Infos //////////////////////////////////////////////////////
    /**
     * @brief Appender for column names to the vector.
     */
    template <typename T>
    struct ColumnNameType {
    private:
        std::vector<std::string>& name;
        std::vector<std::string>& type;
        FKC& fkc;
        uint32_t index = 0;

    public:
        ColumnNameType(std::vector<std::string>& n, std::vector<std::string>& t,
            ForeignKeyConstraint& f) : name(n), type(t), fkc(f), index(0) {}

        /**
         * @brief Appender for column names to the vector.
         * @param x is TypeMetaData::tuple_type_pair() element:
         *     x.first  - pointer to the member variable
         *     x.second - the member name
         */
        template <typename TuplePair>
        void operator()(TuplePair const& x) {
            // ElemType is the pointer type pointing to DefType defined in class T
            using ElemType= typename TuplePair::first_type;
            using DefType = typename remove_const_and_pointer<ElemType>::type;
            using CppType = typename TypeInfoTrait<DefType>::CppType;
            static_assert(TypeInfoTrait<DefType>::is_vector == false,
                "SqlStatementBase::ColumnNameType: ElemType cannot be a vector type");

            // use variable's sql type decides the column type
            const std::vector<std::string>& cnames = TypeMetaData<T>::column_names();
            std::string column_name = cnames[index++];

            static constexpr SqlType sqlType = TypeInfoTrait<DefType>::sqlType;

            /**
             * Check if the SQL type is composite or composite vector.
             * Both types need to be expanded to get the nested column names.
             * For SqlType::CompositeVector type,
             * edadb::DbMap<T> needs to create its child table as primary-key-referenced table.
             */
            if constexpr ((sqlType == SqlType::Composite) || (sqlType == SqlType::CompositeVector)) {
                assert((index > 0) &&
                    "SqlStatementBase::ColumnNameType::operator(): composite type should not be the first element");

                const auto vecs = TypeMetaData<CppType>::tuple_type_pair();
                FKC next_fkc(fkc);
                next_fkc.updatePrefix(column_name, TypeMetaData<CppType>::table_name());
                boost::fusion::for_each(
                    vecs, ColumnNameType<CppType>(name, type, next_fkc));
            }
            else if constexpr (sqlType == SqlType::External) {
                assert ((index > 0) &&
                    "SqlStatementBase::ColumnNameType::operator(): external type should not be the first element");

                const auto vecs = TypeMetaData<Shadow<CppType>>::tuple_type_pair();
                FKC next_fkc(fkc);
                next_fkc.updatePrefix(column_name, TypeMetaData<Shadow<CppType>>::table_name());
                boost::fusion::for_each(
                    vecs, ColumnNameType<Shadow<CppType>>(name, type, next_fkc));
            }
            else if constexpr ((!std::is_enum<CppType>::value /* SqlType::Unknown */) &&
                ((sqlType == SqlType::CompositeVector) || (sqlType == SqlType::Unknown))) {
                assert(false &&
                    "SqlStatementBase::ColumnNameType::operator(): SqlType::CompositeVector or SqlType::Unknown type should not be expanded");
            }  
            else {
                // use the user defined column name
                name.push_back(fkc.getPrimaryColumnFullName(column_name));
                type.push_back(getSqlTypeString<CppType>());
            } // if constexpr sqlType
        } // operator()
    }; // ColumnNameType
}; // SqlStatementBase



/**
 * @struct SqlStatementImpl
 * @brief This struct is the implementation of SqlStatementBase.
 * @details This struct is the implementation of SqlStatementBase for different database backends, which is assigned by DbBackendType.
*/
template <DbBackendType DBType, typename T>
struct SqlStatementImpl : public SqlStatementBase {
    static_assert(always_false<T>::value, "DbBackendType is not supported");
};


/**
 * SqlStatement is defined in backend, such as backend/sqlite/SqlStatement4Sqlite.h
 *  using SqlStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
 *      SqlStatementImpl<DbBackendType::SQLITE>>;
 */


} // namespace edadb