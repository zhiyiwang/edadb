/**
 * @file DbMap.h
 * @brief DbMap.h provides a way to map objects to relations in the database.0
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <typeindex>

#include <boost/fusion/include/pair.hpp> 
#include <boost/fusion/include/at_c.hpp> 
#include <boost/fusion/include/for_each.hpp>

#include "TypeStack.h"
#include "TraitUtils.h"
#include "DbMapBase.h"
#include "SqlStatement.h"
#include "backend/sqlite/SqlStatement4Sqlite.h"
#include "TypeMetaData.h"
#include "VecMetaData.h"
#include "Table4Class.h"


namespace edadb {

/**
 * @brief DbMap class maps objects to relations.
 * @tparam T The class type.
 */
template <typename T>
class DbMap : public DbMapBase {
public:
    class DbStmtOp; // DB statement operation 
    class Writer; // write object to database
    class Reader; // read object from database

protected:
    const std::string table_name; // by TypeMetaData<T> using TABLE4CLASS
    ForeignKey       foreign_key; // foreign key constraint
    std::vector<DbMapBase *> child_dbmap_vec; // vector of child DbMap

public:
    DbMap(const std::string &n = TypeMetaData<T>::table_name(),
        const ForeignKey &fk = ForeignKey()) : table_name(n), foreign_key(fk) 
    {}

    ~DbMap() {
        for (auto &dbmap : child_dbmap_vec) {
            delete dbmap; dbmap = nullptr;
        } // for 
    }

public:
    const std::string       &getTableName () { return table_name;  }
    ForeignKey              &getForeignKey() { return foreign_key; }
    std::vector<DbMapBase*> &getChildDbMap() { return child_dbmap_vec; }
    DbMapBase*getChild(size_t i) { return child_dbmap_vec.at(i); }

public:
    /**
     * @brief Create the table for the class.
     * @param run_sql If true, the function will begin a transaction and commit it after the creation.
     * @return true if success; otherwise, false.
     */
    bool createTable(bool run_sql = true) {
        if (!manager.isConnected()) {
            std::cerr << "DbMap::createTable: not inited" << std::endl;
            return false;
        }

        // create this table by SqlStatement<T> using this table_name
        if (run_sql) {
            const std::string sql = 
                SqlStatement<T>::createTableStatement(this->table_name, this->foreign_key);
            if (!manager.exec(sql)) {
                std::cerr << "DbMap::createTable: create table failed" << std::endl;
                return false;
            } // if
        } 
        else if (!child_dbmap_vec.empty()) {
            // child dbmap already created,
            // no need to create child table again
            return true; 
        }

        // table and column name sequence to build foreign key constraint
        std::vector<std::string> tab_names;
        std::vector<std::string> col_names;

        bool status = true;
        if constexpr (TypeInfoTrait<T>::sqlType == SqlType::CompositeVector) {
            /**
             * T is a composite vector type,
             * create child table for each element type in the vector
             */
            status = createCompositeVectorTable<TypeStack<T>>(this, tab_names, col_names, run_sql);
            if (!status) {
                std::cerr << "DbMap::createTable: createCompositeVectorTable failed" << std::endl;
                return status;
            }
        }
        else if constexpr (TypeInfoTrait<T>::sqlType == SqlType::Composite) {
            /**
             * T is a composite type,
             * recursively create child table for each nested composite vector/member type
             */
            status = createCompositeTable<TypeStack<T>>(this, tab_names, col_names, run_sql);
            if (!status) {
                std::cerr << "DbMap::createTable: createCompositeTable failed" << std::endl;
                return status;
            }
        }

        return status;
    } // createTable

    /**
     * @brief Initialize the DbMap, create child DbMap if necessary.
     * @return true if success; otherwise, false.
     */
    bool init(void) {
        return createTable(false);
    }
        

    /**
     * @brief Drop the table for the class.
     * @return true if success; otherwise, false.
     */
    bool dropTable() {
        if (!manager.isConnected()) {
            std::cerr << "DbMap::dropTable: not inited" << std::endl;
            return false;
        }

        const std::string sql = "DROP TABLE IF EXISTS \"" + this->table_name + "\";";
        return manager.exec(sql);
    }


private:
    /**
     * @brief Create the child table for the vector member variable.
     * @tparam TypeStack The type stack for tracking the parent-child relationship.
     * @param dbmap The root DbMap pointer, which is the primary-key-referenced table.
     * @param tab_names The table name vector for building foreign key constraint.
     * @param col_names The column name vector for building foreign key constraint.
     * @param run_sql If true, execute the create table SQL statement.
     * @return true if success; otherwise, false.
     */
    template <typename TypeStack>
    bool createCompositeVectorTable(DbMap<T> *dbmap, std::vector<std::string> &tab_names, std::vector<std::string> &col_names, bool run_sql) {
        bool status = true;

        using CompVectorType = typename Last<TypeStack>::type;
        tab_names.push_back(TypeMetaData<CompVectorType>::table_name());  

        auto seq = VecMetaData<CompVectorType>::tuple_type_pair();
        boost::fusion::for_each(seq, [&](auto elem){ 
            // early return if failed
            if (!status) return;

            using ChildVecPtrType = typename decltype(elem)::first_type;
            using ChildVecType = typename remove_const_and_pointer<ChildVecPtrType>::type;
            static_assert(TypeInfoTrait<ChildVecType>::is_vector,
                "DbMap::createTable4CompositeVectorMember: ChildVecPtr must be a vector type"
            );
            using ChildVecElemCppType = typename TypeInfoTrait<ChildVecType>::VecCppType;
            using TS = PushBack_t<TypeStack, ChildVecElemCppType>;
            col_names.push_back(elem.second); // member name 
            status = createChildTable<TS>(dbmap, tab_names, col_names, run_sql);
            col_names.pop_back();
        }); // for_each
        tab_names.pop_back();

        return status;
    } // createCompositeVectorTable


    template <typename TypeStack>
    bool createChildTable(DbMap<T> *dbmap, std::vector<std::string> &tab_names, std::vector<std::string> &col_names, bool run_sql) {
        using ParentType = typename LastTwo<TypeStack>::second_last;
        using ChildType  = typename LastTwo<TypeStack>::last;
        static_assert(!std::is_pointer_v<ChildType>,
            "DbMap::createChildTable: ChildType cannot be a pointer type");

        //////// create foreign key constraint ////////////////
        ForeignKey fk;

        /**
         * foreign key table (parent table) is dbmap's table name:
         * all composite/composite vector childrens are flatten to point to the root table
         */
        fk.table = dbmap->getTableName(); 


        // foreign key column name prefix
        std::string fk_name;
        for (auto &cn : col_names) {
            fk_name += "_" + cn + "_";
        }
        // append primary key column name to foreign key column name 
        const uint32_t primary_key_index = Config::fk_ref_pk_col_index;
        const auto &cols = TypeMetaData<ParentType>::column_names();
        fk_name += cols[primary_key_index];
        fk.column.push_back(fk_name);

        // foreign key types
        using PrimKeyType = typename boost::fusion::result_of::value_at_c<
                typename TypeMetaData<ParentType>::TupType, primary_key_index >::type;
        fk.type.push_back( getSqlTypeString<PrimKeyType>() );


        // primary column name is in DbMap<T> table, get the column name
        assert(TypeMetaData<T>::table_name() == tab_names[0]);
        assert(tab_names.size() == col_names.size());

        std::string child_table_name;
        for (uint32_t i = 0; i < col_names.size(); ++i) {
            std::string postfix = (i == 0) ? "" : "_";
            postfix += tab_names[i] + "_" + col_names[i];
            child_table_name.append(postfix);
        }

        // create child dbmap and table
        // NOTE: child dbmap for vector element points to root of Composite/CompositeVector type
        DbMap<ChildType> *child_dbmap = new DbMap<ChildType>(child_table_name, fk);
        child_dbmap_vec.push_back(child_dbmap);
        return child_dbmap->createTable(run_sql);
    } // createChildTable


private:
    /**
     * @brief Create the child table for the member variable.
     * @tparam TypeStack The type stack for tracking the parent-child relationship.
     * @param dbmap The root DbMap pointer, which is the primary-key-referenced table.
     * @param tab_names The table name vector for building foreign key constraint.
     * @param col_names The column name vector for building foreign key constraint.
     * @param run_sql If true, execute the create table SQL statement.
     * @return true if success; otherwise, false.
    */
    template <typename TypeStack>
    bool createCompositeTable(DbMap<T> *dbmap, std::vector<std::string> &tab_names, std::vector<std::string> &col_names, bool run_sql) {

        bool status = true;

        using CParentType = typename Last<TypeStack>::type;
        tab_names.push_back(TypeMetaData<CParentType>::table_name());

        /**
         * seq: TupTypePairType=boost::fusion::vector<BoostFusionPair1, BoostFusionPair2,...>
         * BoostFusionPairX = boost::fusion::pair<ElemType, ElemName>
         *   ElemType = T** or T* (defined T* or T in class)
         *   ElemName = string, the name of the element
         */
        auto seq = TypeMetaData<CParentType>::tuple_type_pair();
        boost::fusion::for_each(seq, [&](auto elem) { 
            using DefPtrType = typename decltype(elem)::first_type;
            using DefType = typename remove_const_and_pointer<DefPtrType>::type; 
            using DefCppType = typename TypeInfoTrait<DefType>::CppType;
            using TS = PushBack_t<TypeStack, DefCppType>;

            /**
             * For each composite/composite vector member variable,
             * recursively create child table
             */
            static constexpr SqlType sqlType = TypeInfoTrait<DefCppType>::sqlType;
            if constexpr (sqlType == SqlType::Composite) {
                col_names.push_back(elem.second); // member name
                status = createCompositeTable<TS>(dbmap, tab_names, col_names, run_sql);
                col_names.pop_back();
            }
            else if constexpr (sqlType == SqlType::CompositeVector) {
                col_names.push_back(elem.second); // member name
                status = createCompositeTable<TS>(dbmap, tab_names, col_names, run_sql);
                if (!status) return;

                status = createCompositeVectorTable<TS>(dbmap, tab_names, col_names, run_sql);
                col_names.pop_back();
            } // if-else
        }); // for_each

        tab_names.pop_back();

        return status;
    } // createCompositeTable
}; // class DbMap


} // namespace edadb